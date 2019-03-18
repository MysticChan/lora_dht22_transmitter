#ifndef __UART_H__
#define __UART_H__
#include "stm8s.h"
#include "stdio.h"
#define uartbuf Serial_Data.buffer
struct Serial_struct
{
    uint16_t lenth;
    uint8_t buffer[256];
};
extern struct Serial_struct Serial_Data;
/*
 * Perform UART startup initialization.
 */
void uart_init(uint32_t baudrate);
void SerialSendBuf( void );

#endif
