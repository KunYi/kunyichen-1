#include <windows.h>
#include "s2450addr.h"
#include "option.h"
#include "utils.h"

//***************************[ PORTS ]****************************************************
void Port_Init(void)
{
}


void Led_Display(int data)
{
	// Active is low.(LED On)
	// GPF7  GPF6   GPF5   GPF4
	// nLED_8 nLED4 nLED_2 nLED_1
	//
	rGPFDAT = (rGPFDAT & ~(0xf<<4)) | ((data & 0xf)<<4);
}

// Do-nothing delay loop.
//
void Delay(void)
{
    volatile int i;

    for(i=0 ; i < 1000 ; i++)
    {
    }
}



//***************************[ UART ]******************************
void Uart_Init_org(void)
{
    int i;
    rGPHCON = rGPHCON & ~((3 << 4)|(3 << 6));
    rGPHCON = rGPHCON | ((2 << 4)|(2 << 6));

    // Disable pull-up on TXD1 and RXD1.
    //
#if (BSP_TYPE == BSP_SMDK2443)
#ifdef EVT1
    rGPHUDP = rGPHUDP | ((1 << 4)|(1 << 6));
#else
	rGPHUDP = rGPHUDP | ((2 << 4)|(2 << 6));
#endif
#elif (BSP_TYPE == BSP_SMDK2450)
    rGPHUDP = rGPHUDP | ((1 << 4)|(1 << 6));
#endif

    rUFCON1 = 0x0;      // FIFO disable
    rUMCON1 = 0x0;      // AFC disable

    rULCON1 = 0x3;      // Normal,No parity,1 stop,8 bits
    rUCON1  = 0x005;

    rUBRDIV1=( (int)(PCLK/16./115200) -1 );

    for(i=0;i<100;i++);

}

int g_DebugUart=0;
void Uart_Init(int nUartx) // edited by junon 060530
{
	int ch;
	float DIV_VAL;
	int UDIV_SLOT;
 	volatile UART_REGS *pUartRegs;
 	unsigned int temp;
	unsigned int baud = 115200;
 		
	// select UART function
	rUCON0 &= ~(0x3);//receive disable before gpio set to prevent dummy data input
	rUCON1 &= ~(0x3);//receive disable 
	rUCON2 &= ~(0x3);//receive disable 
	rUCON3 &= ~(0x3);//receive disable 
	
	temp = (rGPHCON & ~0xffff ) | 0xaaaa;
	rGPHCON = temp;//set uart mode, keep uart mode if it was to prevent gpio set to input, uart
				     //it will cause cause rx fifo dummy data input.
 
//	for (ch=0;ch<4;ch++)
	ch = nUartx;
	 g_DebugUart = ch;
	{
		pUartRegs = (UART_REGS *)(UART_REG_BASE+UART_REG_OFFSET*ch);	
		pUartRegs->rUlCon = 0x3; //Line control register : Normal,No parity,1 stop,8 bits
	     //  11]  [10]       [9]     [8]        [7]         [6]         [5]          [4]              [3:2]               [1:0]
	     // Clock Sel,  Tx Int,  Rx Int, Rx Time Out, Rx err, Loop-back, Send break,  Transmit Mode, Receive Mode
	     //  1   0          0       0    ,     0            1           0               0     ,       01                  01
	     //   PCLK2      Pulse  Pulse    Disable    Generate  Normal      Normal        Interrupt or Polling
/*
		pUartRegs->rUCon = 0x245; 
		pUartRegs->rUbrDiv = ((int)(PCLK/16./baud+0.5)-1);   //Baud rate divisior register 0 
		pUartRegs->rUfCon = 0x0; //UART FIFO control register, FIFO disable
		pUartRegs->rUmCon = 0x0; //UART MODEM control register, AFC disable
		pUartRegs->rUdivSlot= 0x54AA; //UART MODEM control register, AFC disable
*/
#if 1
		DIV_VAL = ((float)PCLK/16./baud)-1;

		pUartRegs->rUCon = 0x845;
		pUartRegs->rUbrDiv = (int)DIV_VAL;   //Baud rate divisior register 0 
		pUartRegs->rUfCon = 0x6; //UART FIFO control register, FIFO disable, tx, rx fifo reset
		pUartRegs->rUmCon = 0x0; //UART MODEM control register, AFC disable

		UDIV_SLOT = (int)( (DIV_VAL-(int)DIV_VAL)*16 );
#else

		DIV_VAL = BSP_UART0_UBRDIV;

		pUartRegs->rUCon = 0x845;
		pUartRegs->rUbrDiv = (int)DIV_VAL;   //Baud rate divisior register 0 
		pUartRegs->rUfCon = 0x6; //UART FIFO control register, FIFO disable, tx, rx fifo reset
		pUartRegs->rUmCon = 0x0; //UART MODEM control register, AFC disable

		UDIV_SLOT = (int)( (DIV_VAL-(int)DIV_VAL)*16 );
	
#endif		
		
		switch (UDIV_SLOT) {
		case 0 : pUartRegs->rUdivSlot = 0; break;
		case 1 : pUartRegs->rUdivSlot = 0x0080; break;
		case 2 : pUartRegs->rUdivSlot = 0x0808; break;
		case 3 : pUartRegs->rUdivSlot = 0x0888; break;
		case 4 : pUartRegs->rUdivSlot = 0x2222; break;
		case 5 : pUartRegs->rUdivSlot = 0x4924; break;
		case 6 : pUartRegs->rUdivSlot = 0x4a52; break;
		case 7 : pUartRegs->rUdivSlot = 0x54aa; break;
		case 8 : pUartRegs->rUdivSlot = 0x5555; break;
		case 9 : pUartRegs->rUdivSlot = 0xd555; break;
		case 10 : pUartRegs->rUdivSlot = 0xd5d5; break;
		case 11 : pUartRegs->rUdivSlot = 0xddd5; break;
		case 12 : pUartRegs->rUdivSlot = 0xdddd; break;
		case 13 : pUartRegs->rUdivSlot = 0xdfdd; break;
		case 14 : pUartRegs->rUdivSlot = 0xdfdf; break;
		case 15 : pUartRegs->rUdivSlot = 0xffdf; break;
		}
	}
//	printf("DIV_VAL:%f , UDIV_SLOT:%d\n", DIV_VAL, UDIV_SLOT);
}

//=====================================================================
void Uart_SendByte_org(int data)
{
        if(data=='\n')
        {
            while(!(rUTRSTAT1 & 0x2));
            Delay();                 //because the slow response of hyper_terminal
            WrUTXH1('\r');
        }

        while(!(rUTRSTAT1 & 0x2));   //Wait until THR is empty.
        Delay();
        WrUTXH1(data);
}


void Uart_SendByte(int data)
{

	int data2=data;
        if(data=='\n')    		data2='\r';

	if(0==  g_DebugUart) 
	{     	
        while(!(rUTRSTAT0 & 0x2));   //Wait until THR is empty.
        Delay();
        WrUTXH0(data2);
	}else 
	if(1==  g_DebugUart){
	        while(!(rUTRSTAT1 & 0x2));   //Wait until THR is empty.
	        Delay();
	        WrUTXH1(data2);
	}
	if(2==  g_DebugUart){
	        while(!(rUTRSTAT2 & 0x2));   //Wait until THR is empty.
	        Delay();
	        WrUTXH2(data2);
	}
	if(3==  g_DebugUart){
	        while(!(rUTRSTAT3 & 0x2));   //Wait until THR is empty.
	        Delay();
	        WrUTXH3(data2);
	}
	
	
}

//====================================================================
void Uart_SendString(char *pt)
{
    while(*pt)
        Uart_SendByte(*pt++);
}



//====================================================================
void Uart_SendDWORD(DWORD d, BOOL cr)
{
    Uart_SendString("0x");
    Uart_SendString(hex2char((d & 0xf0000000) >> 28));
    Uart_SendString(hex2char((d & 0x0f000000) >> 24));
    Uart_SendString(hex2char((d & 0x00f00000) >> 20));
    Uart_SendString(hex2char((d & 0x000f0000) >> 16));
    Uart_SendString(hex2char((d & 0x0000f000) >> 12));
    Uart_SendString(hex2char((d & 0x00000f00) >> 8));
    Uart_SendString(hex2char((d & 0x000000f0) >> 4));
    Uart_SendString(hex2char((d & 0x0000000f) >> 0));
    if (cr)
        Uart_SendString("\n");
}
void Uart_SendBYTE(BYTE d, BOOL cr)
{
	//Uart_SendString("0x");
	Uart_SendString(hex2char((d & 0x000000f0) >> 4));
	Uart_SendString(hex2char((d & 0x0000000f) >> 0));
	Uart_SendString(" ");
	if (cr)
		Uart_SendString("\n");
}

 //[david.modify] 2008-04-28 10:39
void Uart_Print(unsigned char *pAddr, unsigned int nBytes)
 {
 	int i=0;
	Uart_SendString("\r\n");
	Uart_SendString("pAddr=");
	Uart_SendDWORD((DWORD)pAddr, 0);
	Uart_SendString(" nBytes=");
	Uart_SendDWORD((DWORD)nBytes, 1);	
	for(i=0;i<nBytes;i++) {
		if(i%16==0) {
			Uart_SendString("\r\n");
			Uart_SendDWORD(pAddr+i, 0);
			Uart_SendString(": ");						
		}
		
		Uart_SendBYTE(pAddr[i], 0);
	}
 }

//====================================================================
char *hex2char(unsigned int val)
{
    static char str[2];

    str[1]='\0';

    if(val<=9)
        str[0]='0'+val;
    else
        str[0]=('a'+val-10);

    return str;
}
