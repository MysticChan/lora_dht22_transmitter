#include "timer.h"
#include "led.h"
#include "uart.h"

static uint32_t system_ticks = 0;

void TimerDelay_Init(void)
{
    TIM1_DeInit();
    TIM1_TimeBaseInit((1000-1), TIM1_COUNTERMODE_UP, 7, 0);
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    TIM1_Cmd(ENABLE);
    sprintf( uartbuf, "TIM1 Init\t\tOK\n" );
    SerialSendBuf();
}

void TimerBlink_Init(void)  //default 1Hz
{
    TIM2_DeInit();
    TIM2_TimeBaseInit(TIM2_PRESCALER_256, 15624);
    TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
    TIM2_Cmd(ENABLE);
    sprintf( uartbuf, "TIM2 Init\t\tOK\n" );
    SerialSendBuf();
}

void BlinkFast(void)    //2Hz
{
    TIM2_Cmd(DISABLE);
    TIM2_DeInit();
    TIM2_TimeBaseInit(TIM2_PRESCALER_32, 31249);
    TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
    TIM2_Cmd(ENABLE);
    printf("Blink fast mode set.\n");
}

void BlinkNormal(void)    //1Hz
{
    TIM2_Cmd(DISABLE);
    TIM2_DeInit();
    TIM2_TimeBaseInit(TIM2_PRESCALER_256, 15624);
    TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
    TIM2_Cmd(ENABLE);
    printf("Blink normal mode set.\n");
}

static uint32_t TimerGetTicks(void)
{
    return system_ticks;
}

void TimerDelay(uint32_t delay_10ms)
{
    uint32_t now,target;
    now = TimerGetTicks();
    target = now + delay_10ms;
    while(1)
    {
        now = TimerGetTicks();
        if(now >= target)
        break;
        nop();
    }
}

void TIM1_SystemTickISR (void) __interrupt(11)
{
    system_ticks++;
    TIM1_ClearITPendingBit(TIM1_IT_UPDATE);
}

void TIM2_Update_ISR (void) __interrupt(13)
{
    LED_TOGGLE();
    TIM2_ClearITPendingBit(TIM2_IT_UPDATE);
}
