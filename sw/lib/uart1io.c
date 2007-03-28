/* libio
	functions for low level use with libc
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#ifndef NULL
#define NULL 0
#endif

#define UARTS_STATUS_REGISTER	0x08000004
#define UART1_DATA_PRESENT	0x01
#define UART1_TX_BUFFER_FULL	0x10

#define UART1_TXRX_DATA		0x08000008

/* errno handling in a reentrant way *TODO?* */
int *__errno(void)
{
  return &errno;
}
/* outbyte -- shove a byte out the serial port. We wait till the byte  */
int outbyte( unsigned char c)
{
  while ( *(unsigned char *)UARTS_STATUS_REGISTER & UART1_TX_BUFFER_FULL );
  return (*(unsigned char*) UART1_TXRX_DATA = c);
}

/* inbyte -- get a byte from the serial port with eco and translates \r --> \n*/
unsigned char inbyte(void)
{
  unsigned char c;
  while ( (*(unsigned char *)UARTS_STATUS_REGISTER & UART1_DATA_PRESENT) == 0x0 );
  c = *(unsigned char*) UART1_TXRX_DATA;
  if(c == '\r') c = '\n';
  outbyte(c);
  return c;
}

/* havebyte() -- poll if a byte is available in the serial port */
int havebyte(void)
{
  return *(unsigned char *)UARTS_STATUS_REGISTER & UART1_DATA_PRESENT;
}

