#include "uart.h"
#include "led.h"
#include "dht22.h"
#include "timer.h"
#include "SX1278.h"
void PackMsg(dht22_data *ps);
void CLK_Init(void);
uint8_t buf[5];
void main(void)
{
  uint8_t len,res;
  uint16_t n1 = 0;
  uint16_t n2 = 0;
  CLK_Init();
  uart_init(115200);
  enableInterrupts();
  LED_Init();
  TimerDelay_Init();
  TimerBlink_Init();
  DHT22_Init();
  SX1278_Init();
  while(1)
  {
    if(DHT22_GetData())
    {
      PackMsg(&dht22_data_t);
      if( SX1278_LoRaTxPacket(buf, 5) )
      {
        printf("TX fail. \n");
        TimerDelay(1000);
      }
      else
      {
        n1++;
        res = SX1278_LoRaRxPacket(buf,&len,300);
        if(res == 0)
        {
          if(len == 2)
          {
            if((buf[0]&buf[1]) == RESET)
            {
              n2++;
              printf("Received respond. \n");
              TimerDelay(5000);
            }
          }
          else
          {
            printf("Got a package, but not respond. \n");
            TimerDelay(1000);
          }
          
        }
        else if(res == 1)
        {
            printf("Time out!\r\n");
        }
        else if(res == 2)
        {
            printf("CRC eeror!\r\n");
            TimerDelay(1000);
        }
      }
      printf("Transmited: %d\tReceived: %d .\n",n1,n2);  
    }
    
    
  }
}

void PackMsg(dht22_data* ps)
{
  buf[0] = ps->hMSB;
  buf[1] = ps->hLSB;
  buf[2] = ps->tMSB;
  buf[3] = ps->tLSB;
  buf[4] = buf[0]+buf[1]+buf[2]+buf[3];
}

void CLK_Init(void)
{
    CLK_DeInit();
    CLK_HSECmd(ENABLE);
    while(SET != CLK_GetFlagStatus(CLK_FLAG_HSERDY));
    CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
    CLK_ClockSwitchCmd(ENABLE); 
    CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO , CLK_SOURCE_HSE , DISABLE ,CLK_CURRENTCLOCKSTATE_DISABLE);
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
