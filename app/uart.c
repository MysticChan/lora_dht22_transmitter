#include "uart.h"
#include "string.h"

struct Serial_struct Serial_Data;

void uart_init(uint32_t baudrate)
{
  UART2_DeInit();
  UART2_Init (baudrate, UART2_WORDLENGTH_8D, UART2_STOPBITS_1, UART2_PARITY_NO, UART2_SYNCMODE_CLOCK_DISABLE, UART2_MODE_TXRX_ENABLE);
}


int putchar (int c)
{
    if(c == '\n')
    {
        UART2_SendData8('\r');
        while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
    }
    UART2_SendData8(c);
    while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
    return (c);
}

void SerialSendBuf( void )
{
    uint16_t i, lenth;
    lenth = strlen( Serial_Data.buffer );
    for( i=0; i<lenth; i++)
    {
        if( Serial_Data.buffer[i] == '\n' )
        {
            UART2_SendData8('\r');
            while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
        }
        UART2_SendData8( Serial_Data.buffer[i] );
        while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
    }
}

