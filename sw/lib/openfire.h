/* peripherals address and configurations */
/* openfire soc - 20070327 - a.anton */

#define SP3SK_GPIO		0x08000000

#define SP3SK_GPIO_SEGMENTS_N	0x000000FF
#define SP3SK_GPIO_DRIVERS_N    0x00000F00
#define SP3SK_GPIO_PUSHBUTTONS	0x0000F000
#define SP3SK_GPIO_LEDS		0x00FF0000
#define SP3SK_GPIO_SWITCHES	0xFF000000

#define UARTS_STATUS_REGISTER   0x08000004

#define UART1_DATA_PRESENT      0x00000001
#define UART1_RX_HALF_FULL	0x00000002
#define UART1_RX_FULL		0x00000004
#define UART1_TX_HALF_FULL	0x00000008
#define UART1_TX_BUFFER_FULL    0x00000010

#define UART2_DATA_PRESENT	0x00010000
#define UART2_RX_HALF_FULL	0x00020000
#define UART2_RX_FULL		0x00040000
#define UART2_TX_HALF_FULL	0x00080000
#define UART2_TX_FULL		0x00100000

#define UART1_TXRX_DATA         0x08000008
#define UART2_TXRX_DATA		0x0800000C

#define PROM_READER		0x08000010
#define PROM_DATA		0x000000FF
#define PROM_REQUEST_SYNC	0x00000100
#define PROM_REQUEST_DATA	0x00000200
#define PROM_SYNCED		0x00000400
#define PROM_DATA_READY		0x00000800

#define TIMER1_PORT		0x08000014
#define TIMER1_VALUE		0x7FFFFFFF
#define TIMER1_CONTROL		0x80000000

#define INTERRUPT_ENABLE	0x08000018
#define INTERRUPT_TIMER1	0x00000001
#define INTERRUPT_UART1_RX	0x00000002
#define INTERRUPT_UART2_RX	0x00000004