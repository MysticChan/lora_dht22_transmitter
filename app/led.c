#include "led.h"
#include "uart.h"

void LED_Init(void)
{
    GPIO_DeInit(LED_PORT);
    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);
    sprintf( uartbuf, "LED Init\t\tOK\n");
    SerialSendBuf();
}
