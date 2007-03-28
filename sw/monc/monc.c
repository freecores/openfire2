/* simple boot monitor in c 
   for the openfire soc
   a.anton 27/02/2007 */
   
/* ----------------------------------------
   S{1,2,3}xxxxx --> Motorola S-record
   l		 --> load from PROM at SRAM start
   x <hex32>	 --> execute code at @
   d <hex32> <hex32> {1,2,4} --> dump starting at <hex32>, len=<hex32>, 1(bytes),2(halfw),4(words)
   w <hex32> <hex32> {1,2,4} --> write at <hex32> value=<hex32>  1(byte), 2(halfw), 4(word)
   f <hex32> <hex32> <hex32> --> fill starting at <hex32>, len=<hex32>, value=<hex32> (word)
   ---------------------------------------- */ 

#define	UARTS_STATUS_PORT	0x08000004
#define UART1_TX_FULL		0x10
#define UART1_DATA_AVAILABLE	0x01

#define UART1_TXRX		0x08000008

#define PROM_DATA		0x08000010
#define PROM_CONTROL		0x08000011
#define REQUEST_SYNC		0x01
#define REQUEST_BYTE		0x02
#define IS_PROM_SYNCED		0x04
#define IS_DATA_AVAILABLE	0x08

#define SRAM_START		0x04000000

#define MAX_LINE		128
#define BYTES_PER_LINE		16

// -------------------------------------

void uart1_printchar(unsigned char);
void uart1_printline(char *);
char uart1_readchar(void);
void uart1_readline(char *);

unsigned gethexchar(char);
unsigned ishexdigit(char);
char *gethex(char *, unsigned *, unsigned);

char puthexchar(unsigned);
void puthexstring(char *, unsigned, unsigned);

void process_Sline(void);
void dump(unsigned, int, unsigned);
void write(unsigned, unsigned, unsigned);
void fill(unsigned, int, unsigned);
void load_promfile(unsigned);
unsigned char prom_readbyte(void);

// -------------------------------------

char banner[]   = "MONC1\r\n";
char prompt[]   = "$ ";
char linefeed[] = "\r\n";
char error[]    = "ERROR\r\n";
char nofile[]   = "No file\r\n";
char loading[]  = "Loading...\r\n";

char input_buffer[MAX_LINE]; // = "d 0000 0010 1";	// input buffer (temporal)

// -------------------------------------

void main(void)
{
  unsigned p1, p2, p3;
  char *ptr;
	
  uart1_printline(banner);
  
main_loop:
  uart1_printline(prompt);
  uart1_readline(input_buffer);
  uart1_printline(linefeed);

  switch(input_buffer[0])
  {
    case 'd' :
    case 'w' : 
    case 'f' :    
    	       ptr = gethex(input_buffer + 2, &p1, 8);	// start address
    	       ptr = gethex(ptr + 1, &p2, 8);		// lenght or value
    	       ptr = gethex(ptr + 1, &p3, 8);		// width=1,2,4 or value
    	       if(input_buffer[0] == 'd') 	dump(p1, p2, p3);
    	       else if(input_buffer[0] == 'w') 	write(p1, p2, p3);
    	       else 				fill(p1, p2, p3);
    	       break;
    	       
    case 'S' : process_Sline();
    	       break;
    
    case 'l' : gethex(input_buffer + 2, &p1, 2);	// file-id
    	       load_promfile(p1);
    	       break;
    	       
    case 'x' : gethex(input_buffer + 2, &p1, 8);
      	       ((void (*)(void))p1)();
    	       break;
  }   	       
  goto main_loop;
}

// --------- uart #1 functions ----------
void uart1_printchar(unsigned char c)
{
  while( (*(unsigned char *) UARTS_STATUS_PORT) & UART1_TX_FULL );	// wait empty buffer
  *(char *) UART1_TXRX = c;
}

void uart1_printline(char *txt)
{
  while( *(unsigned char *)txt ) uart1_printchar( (unsigned char) *(txt++));
}

char uart1_readchar(void)
{
  while( ((*(unsigned char *) UARTS_STATUS_PORT) & UART1_DATA_AVAILABLE) == 0 );	// wait a received char
  return *(char *) UART1_TXRX;
}

void uart1_readline(char *buffer)
{
  char tmp;
  do
  {
    *(buffer++) = tmp = uart1_readchar();
    uart1_printchar(tmp);
  } while(tmp != 0x0 && tmp != '\n' && tmp != '\r');
}

// ------------ ascii 2 hex -------------
unsigned gethexchar(char c)
{
  if(c >= 'a') c = c - 'a' + '0' + 10;
  else if(c >= 'A') c = c - 'A' + '0' + 10;
  return c - '0';
}

unsigned ishexdigit(char c)
{
  return (c >= '0' && c <= '9') || 
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

char *gethex(char *string, unsigned *value, unsigned maxdigits)
{
  unsigned number = 0;
  
  while( ishexdigit( string[0] ) && maxdigits > 0)
  {
    number <<= 4;
    number |= gethexchar(string[0]);
    string++;
    maxdigits--;
  }
  
  *value = number;
  return string;
}
  
// ----------- hex 2 ascii ------------------
char puthexchar(unsigned n)
{
  n &= 0xF;
  return n + (n < 10 ? '0' : 'A' - 10);
}

void puthexstring(char *string, unsigned number, unsigned size)
{
  int n = size - 1;
  while(number && n >= 0)		// hex 2 ascii right to left
  {
    string[n] = puthexchar(number & 0xf);
    number >>= 4;
    n--;
  }
  while(n >= 0) string[n--] = '0';	// left padding with 0
}

// --------------------------------------------------------------
// process S1, S2 and S3 records
// http://www.amelek.gda.pl/avr/uisp/srecord.htm

void process_Sline(void)
{
  int tipo = input_buffer[1] - '0';	
  unsigned rec_len, address, checksum = 0, pos, byte, tmp;

  if(tipo < 1 || tipo > 3) return;		// process 1, 2 or 3 records only

  gethex(input_buffer + 2, &rec_len, 2);	// number of bytes in the record (address+data+checksum)  
  checksum += rec_len;
  gethex(input_buffer + 4, &address, tipo == 1 ? 4 : (tipo == 2 ? 6 : 8) );	// read start address
  pos = 4 + 2 + (tipo << 1);			// 1st byte of data is at...
  rec_len -= tipo == 1 ? 2 : (tipo == 2 ? 3 : 4);
  
  tmp = address;
  while(tipo >= 0)				// compute address checksum
  {
    checksum += tmp & 0xff;
    tmp >>= 8;
    tipo--;
  }

  while(rec_len-- > 1)				// read all data bytes and store in memory
  {
    gethex(input_buffer + pos, &byte, 2);	// read byte
    *(unsigned char *)address++ = (unsigned char) byte;
    checksum += byte;
    pos += 2;
  }

  gethex(input_buffer + pos, &byte, 2);		// read checksum
  checksum += byte;
  if( (checksum & 0xff) != 0xff) uart1_printline(error);	// verify checksum  
}

// ---------- dump memory region --------------------------------
void dump(unsigned start, int len, unsigned width)
{
  unsigned n = 0, pos, v;
  while(len > 0)
  {
    if(n == 0)					// init line: current address
    {
      puthexstring(input_buffer, start, 8);
      input_buffer[8] = ' ';
      pos = 9;
    }

    if(width == 1) v = *(unsigned char *)start;		// fetch data
    else if(width == 2) v = *(unsigned short *)start;
    else v = *(unsigned *)start;
    start += width;					// next address
    
    puthexstring(input_buffer + pos,  v, width << 1);  	// data 2 ascii
    pos += (width << 1);
    input_buffer[pos++] = ' ';
    n += width;
    len -= width;
    
    if(n >= BYTES_PER_LINE) 				// end of line
    {
showline:
      input_buffer[pos++] = '\r';
      input_buffer[pos++] = '\n';
      input_buffer[pos] = 0x0;
      uart1_printline(input_buffer);
      n = 0;
    }
  }
  if(n != 0) goto showline;		// hack to print incomplete lines
}

// ---------- write data to memory -------------
void write(unsigned address, unsigned value, unsigned width)
{
  if(width == 1)  *(unsigned char *)address = (unsigned char) value;
  else if(width == 2) *(unsigned short *)address = (unsigned short) value;
  else *(unsigned *)address = value;
}

// ----------- fill memory with dword ----------
void fill(unsigned start, int len, unsigned value)
{
  while(len-- > 0) *(unsigned *)(start++) = value;
}

// ----------- load file from prom ----------
void load_promfile(unsigned file_id)
{
  unsigned char *ptr = (unsigned char *)SRAM_START;	// start of SRAM
  unsigned char status, data;
  unsigned fileno, size;

  status = *(unsigned char *) PROM_CONTROL;
  if( !(status & IS_PROM_SYNCED) )		// not in sync .. exit
  {
    uart1_printline(nofile);
    return;
  }  
  uart1_printline(loading);
  
  fileno = prom_readbyte();			// ignore at the moment...
  size   = prom_readbyte();			// MSByte
  size <<= 8;
  size  |= prom_readbyte();
  size <<= 8;
  size  |= prom_readbyte();			// LSBbyte
  
  while(size-- > 0)				// read file
    *(unsigned char *) ptr++ = prom_readbyte();
}
 
unsigned char prom_readbyte(void)
{
  *(unsigned char *) PROM_CONTROL = REQUEST_BYTE;			// request byte
  while( !(*(unsigned char *) PROM_CONTROL & IS_DATA_AVAILABLE) );	// wait for data
  *(unsigned char *) PROM_CONTROL = 0;
  return *(unsigned char *) PROM_DATA;					// return byte 
}
