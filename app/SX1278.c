#include "SX1278.h"
#include "timer.h"
#include "uart.h"
// const uint8_t sx1276_7_8FreqTbl[][3] = 
// { 
//   {0x6B, 0x00, 0x00},  //428MHz  
//   {0x6B, 0x40, 0x00},  //429MHz
//   {0x6B, 0x80, 0x00},  //430MHz
//   {0x6B, 0xC0, 0x00},  //431MHz
//   {0x6C, 0x00, 0x00},  //432MHz
//   {0x6C, 0x40, 0x00},  //433MHz
//   {0x6C, 0x80, 0x00},  //434MHz
//   {0x6C, 0xC0, 0x00},  //435MHz
//   {0x6D, 0x00, 0x00},  //436MHz
//   {0x6D, 0x40, 0x00},  //437MHz
//   {0x6D, 0x80, 0x00},  //438MHz
//   {0x6D, 0xC0, 0x00}   //439MHz
// };
const uint8_t sx1278_Freq[3] = {0x6C, 0x40, 0x00};
//

#define SX1278_MAX_BUFSIZE  64 //received data max size

// uint8_t Lora_Freq          = LORAFREQ_430MHZ;
uint8_t Lora_Power         = LORAPOWER_20DBM;
uint8_t Lora_SpreadFactor  = 12;
uint8_t Lora_BandWide      = LORABW_125KHz;
uint8_t Lora_ErrorCoding   = ERROR_CODING_4_5;

#define SX1278_NSS_PORT         GPIOC     
#define SX1278_NSS              GPIO_PIN_4

#define SET_SX1278_NSS() 	   	GPIO_WriteHigh(GPIOC,GPIO_PIN_4)
#define CLR_SX1278_NSS() 		  GPIO_WriteLow(GPIOC,GPIO_PIN_4)

#define CLR_SX1278_RST()      GPIO_WriteLow(GPIOD,GPIO_PIN_1)
#define SET_SX1278_RST()      GPIO_WriteHigh(GPIOD,GPIO_PIN_1)
// #define SX1278_RST_OUT()      GPIO_Init(GPIOD, GPIO_PIN_1, GPIO_MODE_OUT_PP_HIGH_SLOW)
// #define SX1278_RST_IN()       GPIO_Init(GPIOD, GPIO_PIN_1, GPIO_MODE_IN_PU_NO_IT)

#define READ_SX1278_NIRQ()    GPIO_ReadInputPin(GPIOD,GPIO_PIN_0)



void SX1278_GPIO_Init(void) 
{
  GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_IN_PU_NO_IT);  //IRQ
  GPIO_Init(GPIOD, GPIO_PIN_1, GPIO_MODE_OUT_PP_HIGH_SLOW); //RST
}

void SX1278_SPI_Init(void) 
{
  GPIO_Init(GPIOC, GPIO_PIN_7, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(GPIOC, GPIO_PIN_4, GPIO_MODE_OUT_PP_HIGH_FAST);
  SET_SX1278_NSS();
  SPI_DeInit();
  SPI_Init(SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_2, SPI_MODE_MASTER, SPI_CLOCKPOLARITY_LOW, SPI_CLOCKPHASE_1EDGE, SPI_DATADIRECTION_2LINES_FULLDUPLEX, SPI_NSS_SOFT, 7);
  SPI_Cmd(ENABLE);
}

uint8_t SX1278_SPI_RW(uint8_t byte)
{
  while(SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET);
  SPI_SendData(byte);
  while(SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET);
  byte = SPI_ReceiveData();
  return byte;         
}

/**********************************************************
**Name:     SX1278_Read_Reg
**Function: SPI Read CMD
**Input:    adr -> address for read
**Output:   None
**********************************************************/
uint8_t SX1278_Read_Reg(uint8_t adr)
{
  uint8_t tmp;
  CLR_SX1278_NSS();
  SX1278_SPI_RW(adr); 
  tmp = SX1278_SPI_RW(adr);  
  SET_SX1278_NSS();
  return(tmp);
}

/**********************************************************
**Name:     SX1278_Write_Reg
**Function: SPI Write CMD
**Input:    uint8_t address & uint8_t data
**Output:   None
**********************************************************/
void SX1278_Write_Reg(uint8_t adr, uint8_t WrPara)  
{
	CLR_SX1278_NSS();
  SX1278_SPI_RW(adr|0x80);
	SX1278_SPI_RW(WrPara);//闂傚倷绀侀幉锟犲礉閺嶎厽鍋￠柍鍝勬噹閺嬩線鏌熼幑鎰靛殭婵☆偅锕㈤弻鏇㈠醇濠靛浂妫炲銈呯箰閻栫厧顫忛搹瑙勫磯闁靛ǹ鍎查悗楣冩⒑閸濆嫷鍎忔い顓犲厴閻涱喛绠涘☉娆忊偓濠氭煠閹帒鍔滄繛鍛灲濮婃椽宕崟顐熷亾閸洖纾归柡宥庡亐閸嬫挸顫濋悙顒€顏�
  SET_SX1278_NSS();
}
/**********************************************************
**Name:     SX1278_Burst_Read
**Function: SPI burst read mode
**Input:    adr-----address for read
**          ptr-----data buffer point for read
**          length--how many bytes for read
**Output:   None
**********************************************************/
void SX1278_Burst_Read(uint8_t adr, uint8_t *ptr, uint8_t length)
{
  uint8_t i;
  if(length<=1)                                            //length must more than one
    return;
  else
  {
    CLR_SX1278_NSS();
    SX1278_SPI_RW(adr); 
    for(i=0;i<length;i++)
    ptr[i] = SX1278_SPI_RW(0);
    SET_SX1278_NSS();  
  }
}

/**********************************************************
**Name:     SPISX1278_Burst_Write
**Function: SPI burst write mode
**Input:    adr-----address for write
**          ptr-----data buffer point for write
**          length--how many bytes for write
**Output:   none
**********************************************************/
void SX1278_Burst_Write(uint8_t adr, uint8_t *ptr, uint8_t length)
{ 
  uint8_t i;
  if(length > 0)                                            //length must more than one
  {   
    CLR_SX1278_NSS();        
    SX1278_SPI_RW(adr|0x80);
    for(i=0;i<length;i++)
        SX1278_SPI_RW(ptr[i]);
    SET_SX1278_NSS();  
  }
}

void SX1278_Standby(void)
{
  SX1278_Write_Reg(LR_RegOpMode,0x09);                              		//Standby//Low Frequency Mode
	//SX1278_Write_Reg(LR_RegOpMode,0x01);                              	 //Standby//High Frequency Mode
}

void SX1278_Sleep(void)
{
  SX1278_Write_Reg(LR_RegOpMode,0x08);                              		//Sleep//Low Frequency Mode
	//SX1278_Write_Reg(LR_RegOpMode,0x00);                            		 //Sleep//High Frequency Mode
}

/*********************************************************/
//LoRa mode
/*********************************************************/
void SX1278_EntryLoRa(void)
{
  SX1278_Write_Reg(LR_RegOpMode,0x88);//Low Frequency Mode
	//SX1278_Write_Reg(LR_RegOpMode,0x80);//High Frequency Mode
}

void SX1278_LoRaClearIrq(void)
{
  SX1278_Write_Reg(LR_RegIrqFlags,0xFF);
}

void SX1278_Config(void)
{
  uint8_t temp;
//闂傚倷鐒﹀鍧楀磻閹捐埖宕叉繝闈涱儏閺嬩線鏌熼幑鎰靛殭婵☆偅锕㈤弻鏇㈠醇濠靛浂妫炲銈呯箺閸╂牗绌辨繝鍥舵晬婵＄偠顕ф禍鍓р偓瑙勬礀濞层劑銆侀崨瀛樷拻濞达綀濮ょ涵鍫曟煕閻樿櫕宕岀€规洍鈧磭鏆﹂柛銉㈡櫊閳瑰繘鏌ｉ悢鍝ユ噧閻庢凹鍓熼、鏃傛崉鐞涒剝鏂€濡炪倖姊婚埛鍫ュ磿閺冨牊鐓涘ù锝堫潐瀹曞瞼鈧鍣崜鐔镐繆閸洖骞㈡俊銈咃梗缁憋箓姊婚崒娆愮グ婵炲娲熷畷浼村冀椤撶偟鐓戝┑鐐叉▕娴滄粌螞濮椻偓閺屾洟宕煎┑鍥舵濡炪倕绻愰悥鐓庮潖閾忚宕夐柕濞垮劜閻忎胶绱掑Δ浣哥伌闁哄本鐩獮瀣枎韫囧﹤浜鹃柡宥庡亐閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈嗗亗閹艰揪绱曢梼鈧梻鍌欒兌缁垶鎳濇ィ鍐╁仱闁靛ě鈧崑鎾愁潩椤掑倻楔闂佸搫琚崝鎴﹀箖閵忋倕宸濆┑鐘插缁辨洟姊绘担鍝ョШ妞ゃ儲鎹囧畷妤€顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堝醇閺囩啿鎷洪柣鐘充航閸斿苯鈻嶉幇鐗堢厵闁告垯鍊栫€氾拷
    SX1278_Sleep();                                      //Change modem mode Must in Sleep mode 
	TimerDelay(10);

	SX1278_EntryLoRa();  
	//SX1278_Write_Reg(0x5904);   //Change digital regulator form 1.6V to 1.47V: see errata note
    
	SX1278_Burst_Write(LR_RegFrMsb,(uint8_t *)sx1278_Freq,3);  //setting frequency parameter
 
	//setting base parameter 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠荤紓浣股戝▍銏ゆ⒑鐠恒劌娅愰柟鍑ゆ嫹
	SX1278_Write_Reg(LR_RegPaConfig,Lora_Power);             //Setting output power parameter  
	SX1278_Write_Reg(LR_RegOcp,0x0B);                              //RegOcp,Close Ocp 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堟晸閿燂拷 100mA
	SX1278_Write_Reg(LR_RegLna,0x23);                              //RegLNA,High & LNA Enable闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堝醇閺囩啿鎷洪柣鐘充航閸斿矂寮搁幋锔界厸閻庯綆浜堕悡鍏碱殽閻愯尙绠婚柡浣规崌閺佹捇鏁撻敓锟�
    
//	if(Lora_SpreadFactor==6)           //SFactor=6//闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻褍顫濋鈧埀顒佹倐閸┿垼绠涘☉娆屾嫼闁荤姵浜介崝宀勫几閹达附鐓涢悗锝庝憾閻撳吋顨ラ悙鑼闁诡喗绮撻幊鐐哄Ψ閿旂瓔浠ч梻鍌欑閹碱偊宕愰崼鏇炵９闁哄稁鍋€閸嬫挸顫濋幇浣圭秷闂佹寧绋掗崝娆撳极閹剧粯鏅搁柨鐕傛嫹6闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋悙顒€顏�
//	{
//		uint8_t tmp;
//		SX1278_Write_Reg(LR_RegModemConfig1,((Lora_BandWide<<4)+(Lora_ErrorCoding<<1)+0x01));//Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
//		SX1278_Write_Reg(LR_RegModemConfig2,((Lora_SpreadFactor<<4)+0x04+0x03));
//      
//		tmp = SX1278_Read_Reg(0x31);
//		tmp &= 0xF8;
//		tmp |= 0x05;
//		SX1278_Write_Reg(0x31,tmp);
//		SX1278_Write_Reg(0x37,0x0C);
//        Debug_Printf("SFactor=6\r\n");
//	} 
//	else
//	{                                                                                
		SX1278_Write_Reg(LR_RegModemConfig1,((Lora_BandWide<<4)+(Lora_ErrorCoding<<1)+0x00));//闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堟晸閿燂拷 & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋浣界缂備緡鍠栭悧鍡欏弲濡炪倕绻愰幊蹇撯枍閵忋倖鈷戦柛婵嗗閳ь剙鐖煎畷鎰板冀椤愮喎浜炬慨妯诲墯濞兼劗鈧灚婢樼€氼噣鍩€椤掍胶鈯曢柍绗哄劜閹峰懘宕滈幓鎺擃吙闂備浇娉曢崳锕傚箯閿燂拷
		SX1278_Write_Reg(LR_RegModemConfig2,((Lora_SpreadFactor<<4)+0x04+0x03));  //SFactor & CRC LNA gain set by the internal AGC loop 
//	}
//	SX1278_Write_Reg(LR_RegSymbTimeoutLsb,0xFF);                   //RegSymbTimeoutLsb Timeout = 0x3FF(Max) 
    
	SX1278_Write_Reg(LR_RegPreambleMsb,0x00);                       //RegPreambleMsb 
	SX1278_Write_Reg(LR_RegPreambleLsb,12);                      //RegPreambleLsb 8+4=12byte Preamble
    
	SX1278_Write_Reg(REG_LR_DIOMAPPING2,0x01);                     //RegDioMapping2 DIO5=00, DIO4=01
	
    SX1278_Standby();                                         //Entry standby mode

}

/*************************************************************************
 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡灞稿墲瀵板嫭绻濋崒婊呮晨缂傚倸鍊哥粔鏉懨洪銏㈠祦闊洦绋掗崐濠氭煠閹帒鍔滄繛鍛灲濮婃椽宕崟顐熷亾閸洖纾归柡宥庡亐閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堝醇閺囩啿鎷洪柣鐘充航閸斿矂寮稿澶嬬厾闁绘挸閰ｉ崣鍕煛娴ｅ摜效妞ゃ垺姊归幆鏃堝閻欌偓娴硷拷
 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堟晸閿燂拷
 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀€鍠栭獮鏍敇濠靛洤鈧絿绱撻崒姘毙㈡繛宸弮閻涱喛绠涘☉娆忊偓濠氭煠閹帒鍔滄繛鍛灲濮婃椽宕崟顐熷亾閸洖纾归柡宥庡亐閸嬫挸顫濋悙顒€顏�
***************************************************************************/
uint8_t SX1278_LoRaEntryRx(void)
{
  uint8_t addr = 0; 
  uint8_t t = 5;
  SX1278_Standby();             //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悗闈涙憸閹虫繈姊洪棃娑辨▓闁搞劋鍗抽獮鍐疀濞戞瑧鍘搁梺绋挎湰閿氶柍褜鍓氶悧婊堝焵椤掍浇澹樼紓宥咃工閻ｇ兘顢曢敂钘変簻闂佺ǹ绻愰崥瀣枍閵忋倖鈷戦柛婵嗗閳ь剙鐖煎畷鎰板冀椤愮喎浜炬慨姗嗗墰缁夋椽鏌＄仦璇插闁诡喓鍨介獮鍡涘级濞嗘儳娈濋梻鍌欑劍鐎笛勭瑹濡や胶顩叉繝闈涱儏閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堝醇閺囩啿鎷洪柣鐘充航閸斿矂寮稿澶嬬厾闁绘挸閰ｉ崣鍕煛娴ｅ摜效妞ゃ垺姊归幆鏃堝閻欌偓娴硷拷
  SX1278_Write_Reg(REG_LR_PADAC,0x84);                              //zhen and Rx
  SX1278_Write_Reg(LR_RegHopPeriod,0xFF);                          //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濇潏銊ユ畬闁句紮缍侀弻宥夊Ψ閵壯嶇礊缂佺虎鍘搁崑鎾绘⒒閸屾瑦绁版繛澶嬫礋瀹曚即骞囬弶鍨殤婵犵數濮甸懝鍓х不椤栫偞鐓熼柟瀵稿Х閹藉倿鏌￠崱鎰姦婵﹨娅ｉ幑鍕Ω閵夛妇鈧箖姊洪崫鍕靛剰妞ゎ厾鍏橀悰顔跨疀濞戞瑥鈧鏌ら幁鎺戝姕婵炲懌鍨藉娲传閸曨偀鍋撻崼鏇炵９闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ悗瑙勬礃閿曘垽寮幘缁樻櫢闁跨噦鎷�0XFF闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋幇浣圭秷闂佹寧绋掗崝娆撳极閹剧粯鏅搁柨鐕傛嫹
  SX1278_Write_Reg(REG_LR_DIOMAPPING1,01);                       //DIO0=00, DIO1=00, DIO2=00, DIO3=01
      
  SX1278_Write_Reg(LR_RegIrqFlagsMask,0x9F);                       //Open RxDone interrupt & CRC
  SX1278_LoRaClearIrq();   
  
//  SX1278_Write_Reg(LR_RegPayloadLength,21);         //RegPayloadLength  21byte(this register must difine when the data long of one byte in SF is 6)
                                                      //婵犵數鍋犻幓顏嗗緤閽樺鐒介柨鐔哄Т閺嬩線鏌熼崜褏甯涢柛搴㈩殕閵囧嫰骞囬埡浣稿Ф闂佸摜鍠庡ú顓㈠蓟閻斿壊妲归幖绮光偓鍐茬闂備浇妗ㄩ悞锕傚箲閸パ屽殨妞ゆ帒瀚洿闂佺硶鍓濋〃蹇斿閹剧粯鈷掑ù锝堝Г绾爼鏌涢悩鍐插鐎殿喖鎲＄换婵嗩潩椤掑偆鍞甸梻浣虹帛閸ㄥ吋鎱ㄩ妶澶婄柧闁归棿鐒﹂悡娆撴煟閻斿憡绶叉俊鎻掝煼楠炴牜鈧稒顭囬惌鎺擃殽閻愯尙绠婚柡灞诲妿閳ь剨缍嗛崑鎺懳ｉ鐐粹拻濞达綀濮ょ涵鍫曟煕閻樺啿濮嶇€殿喖鎲＄换婵嗩潩椤掑偆鍞甸梻浣圭湽閸娿倝宕归悡骞熸椽鏁愭径瀣ф嫼闁荤姵浜介崝宀勫几閹达附鐓涢悗锝庝憾閻撳吋顨ラ悙鑼闁诡喗绮撻幊鐐哄Ψ閿旂瓔浠ч梻鍌欑劍濡炲潡宕ｆ惔銊ョ；闁规儳澧庣粈濠囨煕閵夘喖澧紒鐙€鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋幇浣圭秷闂佺懓鍢查幊妯虹暦缁嬭鏃€鎷呯憴鍕典户闂傚倷鐒﹀鍧楀矗鎼淬劌纾诲鑸靛姦閺佸﹪鏌熼悧鍫熺凡缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堝醇閺囩啿鎷洪柣鐘充航閸斿矂寮搁幋锔界厸閻庯綆浜堕悡鍏碱殽閻愯尙绠婚柟顔界矒閹崇偤濡烽敂绛嬩户闂傚倷绀侀幖顐﹀磹閸洖纾归柡宥庡亐閸嬫挸顫濋悙顒€顏�
    
  addr = SX1278_Read_Reg(LR_RegFifoRxBaseAddr);           				//Read RxBaseAddr
  SX1278_Write_Reg(LR_RegFifoAddrPtr,addr);                        //RxBaseAddr -> FiFoAddrPtr闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻銊╂偆閸屾稑顏� 
  SX1278_Write_Reg(LR_RegOpMode,0x8d);                        		//Continuous Rx Mode//Low Frequency Mode
  //SX1278_Write_Reg(LR_RegOpMode,0x05);                          //Continuous Rx Mode//High Frequency Mode
	while(t--)
	{		
        if((SX1278_Read_Reg(LR_RegModemStat)&0x04)==0x04)   //Rx-on going RegModemStat
			break;
	}
    return 1;
}
/*************************************************************************
 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡灞稿墲瀵板嫭绻濋崒婊呮晨缂傚倸鍊哥粔鏉懨洪銏㈠祦闊洦绋掗崐濠氭煠閹帒鍔滄繛鍛灲濮婃椽宕崟顐熷亾閸洖纾归柡宥庡亐閸嬫挸顫濋鍌溞ㄩ悗瑙勬穿缂嶄礁顕ｉ棃娑辩€堕柡鍛箻闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柡鍥ュ灩缁€鍫濃攽閸屾碍鍟為柛瀣墵閹鈽夊▍顓т邯椤㈡棃宕滄担鎻掍壕婵炲牆鐏濋弸鐔镐繆閻愭壆鐭欑€殿噮鍋婇獮妯兼嫚閸欏妫熼梻渚€娼ч悧鍡椢涘Δ鍜佹晜闁跨噦鎷�
 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堟晸閿燂拷
 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀€鍠栭獮鏍敇濠靛洤鈧絿绱撻崒姘毙㈡繛宸弮閻涱喛绠涘☉娆忊偓濠氭煠閹帒鍔滄繛鍛灲濮婃椽鎮℃惔锝嗘喖濠殿喗菧閸斿孩绂嶉幖浣肝╅柍杞拌兌椤斿棝鎮楅悷鏉款仾婵犮垺顭囬幑銏＄瑹閳ь剙顫忛搹瑙勫磯闁靛ǹ鍎查悗楣冩⒑閸濆嫷鍎忔い顓犲厴閻涱喛绠涘☉娆愭闂佽法鍣﹂幏锟� 127闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡灞诲姂閹倝宕掑☉姗嗕紦
***************************************************************************/
uint8_t SX1278_LoRaReadRSSI(void)
{
  u16 temp=10;
  temp=SX1278_Read_Reg(LR_RegRssiValue);                  //Read RegRssiValue闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨崇槐鎺斺偓锝庡墰娴犳硞i value
  temp=temp+127-137;                                       //127:Max RSSI, 137:RSSI offset
  return (uint8_t)temp;
}

uint8_t SX1278_LoRaRxPacket(uint8_t *valid_data, uint8_t* packet_length,u16 timeout)
{
    uint8_t addr,irq_flag;
    uint8_t packet_size;
    SX1278_LoRaEntryRx(); //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悗闈涙憸閹虫繈姊洪棃娑辨▓闁搞劋鍗抽獮鍐疀濞戞瑧鍙嗛梺缁樻礀閸婂湱鈧熬鎷�
    timeout = timeout;
    while(timeout != 1)  //timeout 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋悙顒€顏�1闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡灞剧洴楠炴鎲撮崟顓溾偓濠傤渻閵堝棗绗氶柣鎿勭節閻涱喛绠涘☉娆愭闂佽法鍣﹂幏锟� 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮妶澶婄＜婵炴垶锕╂导锟�0 闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋幇浣圭秷濠电偠澹堥褔銈导鏉戦唶闁绘柨鎲￠鐔兼⒒娴ｅ憡鎯堥柛濠傜埣瀹曟劙寮介鐔蜂壕婵﹩鍓涚粔娲煛鐏炶濮傞柟顔哄灲瀹曘劍绻濋崟顏嗙？闂傚倷绀佺紞濠囁夐幘璇茬婵せ鍋撶€殿噮鍋婇獮妯兼嫚閼碱剦妲堕梺鑽ゅС鐠€锕傛偂閸儱绀夋繝濠傜墛閻撴瑩鏌涢幋娆忊偓鏍偓姘炬嫹
    { 
        TimerDelay(10); 
        if(timeout > 1) timeout --;
        if(READ_SX1278_NIRQ())
        {
//            Debug_Printf("SX1278_RX_NIRQ()\r\n");
            irq_flag=SX1278_Read_Reg(LR_RegIrqFlags);
            if((irq_flag & RFLR_IRQFLAGS_PAYLOADCRCERROR) == RFLR_IRQFLAGS_PAYLOADCRCERROR) //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈嗗€绘俊顖涗憾濮濆闂傚倷绀侀幖顐ょ矙閸曨厽宕叉繝闈涱儏閺嬩線鏌熼幑鎰靛殭婵☆偅锕㈤弻鏇㈠醇濠靛浂妫炲銈呯箰閻栫厧顫忛搹瑙勫磯闁靛ǹ鍎查悗楣冩⒑閸濆嫷鍎忔い顓犲厴閻涱喛绠涘☉娆忊偓濠氭煠閹帒鍔滄繛鍛灲濮婃椽宕崟顐熷亾閸洖纾归柡宥庡亐閸嬫挸顫濋鍌溞ㄩ悗瑙勬穿缁绘繈鐛弽銊﹀閻熸瑥瀚鐔兼⒒娴ｈ姤纭堕柛锝忕畵楠炲繘鏁撻敓锟�
            {
                SX1278_LoRaClearIrq(); 
                SX1278_Sleep();  //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺璇″灠閻楁捇寮幘缁樺亹闁惧浚鍋呴鐔兼⒒娴ｅ憡鎯堥柛濠傜埣瀹曟劙寮介鐔蜂壕婵ǹ椴搁崵鈧柧浼欑秮閺屽秹濡烽妷褝绱炵紒缁㈠幐閸嬫捇姊婚崒娆愮グ婵炲娲熷畷浼村箛閺夊灝鍤戞繝鐢靛У閼瑰墽绮婚鐐寸叄婵﹩鍓欓埀顒傛櫕缁牊鍒婂畡鐥璮ig闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨抽埀顒傛嚀鐎氼厼顭垮Ο琛℃瀺婵炲棙鎸婚埛鎴︽偣閹帒濡奸柡瀣灴閺岋紕鈧綆浜堕悡鍏碱殽閻愯尙绠婚柡浣规崌閺佹捇鏁撻敓锟�
                return (2);
            }
            addr = SX1278_Read_Reg(LR_RegFifoRxCurrentaddr);      //last packet addr
            SX1278_Write_Reg(LR_RegFifoAddrPtr,addr);                      //RxBaseAddr -> FiFoAddrPtr    
            packet_size = SX1278_Read_Reg(LR_RegRxNbBytes);     //Number for received bytes   
            SX1278_Burst_Read(0x00, valid_data, packet_size);
            *packet_length = packet_size;
            SX1278_LoRaClearIrq();  
            SX1278_Sleep();  //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺璇″灠閻楁捇寮幘缁樺亹闁惧浚鍋呴鐔兼⒒娴ｅ憡鎯堥柛濠傜埣瀹曟劙寮介鐔蜂壕婵ǹ椴搁崵鈧柧浼欑秮閺屽秹濡烽妷褝绱炵紒缁㈠幐閸嬫捇姊婚崒娆愮グ婵炲娲熷畷浼村箛閺夊灝鍤戞繝鐢靛У閼瑰墽绮婚鐐寸叆闁绘洖鍊圭€氾拷
            return(0);
        }
    }
    SX1278_Sleep(); //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺璇″灠閻楁捇寮幘缁樺亹闁惧浚鍋呴鐔兼⒒娴ｅ憡鎯堥柛濠傜埣瀹曟劙寮介鐔蜂壕婵ǹ椴搁崵鈧柧浼欑秮閺屽秹濡烽妷褝绱炵紒缁㈠幐閸嬫捇姊婚崒娆愮グ婵炲娲熷畷浼村箛閺夊灝鍤戞繝鐢靛У閼瑰墽绮婚鐐寸叆闁绘洖鍊圭€氾拷
    return(1);
}

uint8_t SX1278_LoRaEntryTx(uint8_t packet_length)
{
    uint8_t addr = 0,temp;
    uint8_t t = 3;     
    SX1278_Standby();                           //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悗闈涙憸閹虫繈姊洪棃娑辨▓闁搞劋鍗抽獮鍐疀濞戞瑧鍘搁梺绋挎湰閿氶柍褜鍓氶悧婊堝焵椤掍浇澹樼紓宥咃工閻ｇ兘顢曢敂钘変簻闂佺ǹ绻愰崥瀣枍閵忋倖鈷戦柛婵嗗閳ь剙鐖煎畷鎰板冀椤愮喎浜炬慨姗嗗墰缁夋椽鏌＄仦璇插闁诡喓鍨介獮鍡涘级濞嗘儳娈濋梻鍌欑劍鐎笛勭瑹濡や胶顩叉繝闈涱儏閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺鍝勮閸旀垿骞冮姀銈呭窛濠电姴瀚槐鏇㈡⒒娴ｅ摜绉烘い銉︽崌瀹曟顫滈埀顒€顕ｉ锕€绠婚悹鍥у级椤ユ繈姊洪棃娑氬婵☆偅顨婇、鏃堝醇閺囩啿鎷洪柣鐘充航閸斿矂寮稿澶嬬厾闁绘挸閰ｉ崣鍕煛娴ｅ摜效妞ゃ垺姊归幆鏃堝閻欌偓娴硷拷
    SX1278_Write_Reg(REG_LR_PADAC,0x87);                                   //Tx for 20dBm闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ悗娈垮櫘閸ｏ絽鐣锋總绋垮嵆闁绘柨鎲￠鐔兼⒒娴ｅ憡鎯堥柛濠傜埣瀹曟劙寮介鐔蜂壕婵﹩鍓氱拹锟犳煙閼碱剙顣肩紒妤冨枛閸┾偓妞ゆ帒瀚弸渚€鏌熼幑鎰靛殭婵☆偅锕㈤弻鏇㈠醇濠靛浂妫炲銈呯箰閻栫厧顫忛搹瑙勫磯闁靛ǹ鍎查悘渚€姊虹紒妯绘儎闁稿妫濆畷鐗堢節閸愩劎绐為柣搴秵娴滆埖瀵奸幘缁樷拺缂佸绨辨惔鐑芥煙閸戙倖瀚�
    SX1278_Write_Reg(LR_RegHopPeriod,0x00);                               //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濇潏銊ユ畬闁句紮缍侀弻宥夊Ψ閵壯嶇礊缂佺虎鍘搁崑鎾绘⒒閸屾瑦绁版繛澶嬫礋瀹曚即骞囬弶鍨殤婵犵數濮甸懝鍓х不椤栫偞鐓熼柟瀵稿Х閹藉倿鏌￠崱鎰姦婵﹨娅ｉ幑鍕Ω閵夛妇鈧箖姊洪崫鍕靛剰妞ゎ厾鍏橀悰顔跨疀濞戞瑥鈧鏌ら幁鎺戝姕婵炲懌鍨藉娲传閸曨偀鍋撻崼鏇炵９闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ悗瑙勬礃閿曘垽寮幘缁樻櫢闁跨噦鎷�0X00闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋幇浣圭秷闂佹寧绋掗崝娆撳极閹剧粯鏅搁柨鐕傛嫹
    SX1278_Write_Reg(REG_LR_DIOMAPPING1,0x41);                       //DIO0=01, DIO1=00, DIO2=00, DIO3=01

    
    SX1278_Write_Reg(LR_RegIrqFlagsMask,0xF7);                       //Open TxDone interrupt
    SX1278_LoRaClearIrq();
    
    SX1278_Write_Reg(LR_RegPayloadLength,packet_length);             

    addr = SX1278_Read_Reg(LR_RegFifoTxBaseAddr);           //RegFiFoTxBaseAddr
    SX1278_Write_Reg(LR_RegFifoAddrPtr,addr);               //RegFifoAddrPtr
    while(t --)
    {
        temp=SX1278_Read_Reg(LR_RegPayloadLength);
        if(temp==packet_length) break; 
    }
    return 0;
}

uint8_t SX1278_LoRaTxPacket(uint8_t *valid_data, uint8_t packet_length)
{ 
    uint8_t timeout = 100;
    SX1278_LoRaEntryTx(packet_length);
	SX1278_Burst_Write(0x00, valid_data, packet_length);
	SX1278_Write_Reg(LR_RegOpMode,0x8b);  //Tx Mode           
	while(-- timeout)
	{
        TimerDelay(10);
		if(READ_SX1278_NIRQ())                      //Packet send over
		{    
//            Debug_Printf("SX1278_TX_NIRQ\r\n");
			SX1278_Read_Reg(LR_RegIrqFlags);
			SX1278_LoRaClearIrq();                                //Clear irq			
//			SX1278_Standby();                                     //Entry Standby mode 
            SX1278_Sleep();                                      //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺璇″灠閻楁捇寮幘缁樺亹闁惧浚鍋呴鐔兼⒒娴ｅ憡鎯堥柛濠傜埣瀹曟劙寮介鐔蜂壕婵ǹ椴搁崵鈧柧浼欑秮閺屽秹濡烽妷褝绱炵紒缁㈠幐閸嬶拷
			return 0;
		}
	} 
    SX1278_Sleep();                                      //闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柟鐗堟緲閸戠姴鈹戦悩瀹犲缂佺媭鍨堕弻锝夊箣閿濆憛鎾绘煛閸涱喗鍊愰柡宀嬬節瀹曟帒螣鐞涒€充壕闁哄稁鍋€閸嬫挸顫濋鍌溞ㄩ梺璇″灠閻楁捇寮幘缁樺亹闁惧浚鍋呴鐔兼⒒娴ｅ憡鎯堥柛濠傜埣瀹曟劙寮介鐔蜂壕婵ǹ椴搁崵鈧柧浼欑秮閺屽秹濡烽妷褝绱炵紒缁㈠幐閸嬶拷
    return 1;
}

uint8_t SX1278_ReadRSSI(void)
{
  uint8_t temp=0xff;
	
  temp=SX1278_Read_Reg(0x11);
  temp>>=1;
  temp=127-temp;                                           //127:Max RSSI
  return temp;
}

void SX1278_Init(void)
{
    SX1278_GPIO_Init();
    SX1278_SPI_Init();
    printf("SX1278 Init\t\tOK\n");
    CLR_SX1278_RST();
    TimerDelay(10);
    SET_SX1278_RST();
    TimerDelay(10);
    SX1278_Config(); 
}
