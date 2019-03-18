#ifndef __DHT22_H__
#define __DHT22_H__

#include "stm8s.h"

#define SDA_PORT  GPIOC
#define SDA_PIN   GPIO_PIN_3
#define SDA_OUT   GPIO_Init( SDA_PORT, SDA_PIN, GPIO_MODE_OUT_PP_HIGH_FAST )
#define SDA_HIGH  GPIO_WriteHigh( SDA_PORT, SDA_PIN )
#define SDA_LOW   GPIO_WriteLow( SDA_PORT, SDA_PIN )
#define SDA_IN    GPIO_Init( SDA_PORT, SDA_PIN, GPIO_MODE_IN_PU_NO_IT )
#define SDA_READ  GPIO_ReadInputPin( SDA_PORT, SDA_PIN )
#define RST_CNT   TIM4_SetCounter(0)
#define GET_CNT   TIM4_GetCounter()

/* DHT22_GetReadings response codes */
#define DHT22_RCV_OK			      0 // Return with no error
#define DHT22_RCV_NO_RESPONSE		1 // No response from sensor
#define DHT22_RCV_BAD_ACK1	  	2 // Bad first half length of ACK impulse
#define DHT22_RCV_BAD_ACK2	  	3 // Bad second half length of ACK impulse
#define DHT22_RCV_TIMEOUT	    	4 // Timeout while receiving bits
#define DHT22_BAD_DATA			    5 // Bad data received

typedef struct _dht22_data
{
  volatile uint8_t rcv_response;
  volatile int16_t temperature;
  volatile int16_t humidity;
  uint8_t parity;
  uint8_t parity_rcv;
  uint8_t hMSB;
  uint8_t hLSB;
  uint8_t tMSB;
  uint8_t tLSB;
  uint8_t bits[40];
} dht22_data;

extern dht22_data dht22_data_t;
void DHT22_Init (void);
uint8_t DHT22_GetData(void);
#endif
