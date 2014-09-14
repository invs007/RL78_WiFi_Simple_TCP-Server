// RDKRL78_spi.c
#include <stdbool.h>
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "r_cg_serial.h"
#include "r_cg_it.h"
#include "RDKRL78_spi.h"
#include "WIFIApi.h"

extern volatile uint8_t G_SPI21_SendingData; //spi busy flag
extern volatile uint8_t G_SPI21_ReceivingData; //spi busy flag

extern volatile uint8_t G_SPI31_SendingData; //spi busy flag
extern volatile uint8_t G_SPI31_ReceivingData; //spi busy flag

extern uint8_t  G_WIFI_ReceiveBuffer;
extern uint32_t G_WIFI_BufferIndex;

uint8_t WIFI_ReceiveByte(char*);



unsigned char *SPI_CS_Port[] = {
    (unsigned char *)&P8,   // EINK-CS#       P80
    (unsigned char *)&P14,  // SD-CS          P142
    (unsigned char *)&P14,  // LCD-CS         P145
    (unsigned char *)&P8,   // PMOD2-CS       P83
    (unsigned char *)&P5    // WIFI-CS        P55
};

uint8_t SPI_CS_Pin[] = {
    0,   // EINK-CS#       P80
    2,   // SD-CS          P142
    5,   // LCD-CS         P145
    3,   // PMOD2-CS       P83
    5    // WIFI-CS        P55
};


/*---------------------------------------------------------------------------*
 * Routine:  SPI_SetBitRate
 *---------------------------------------------------------------------------*
 * Description:
 *      Set the I2C speed in kHz.
 * Inputs:
 *      uint16_t aSpeed -- kHz speed of I2C bus.
 * Outputs:
 *      void
 *---------------------------------------------------------------------------*/
void SPI_SetBitRate(uint32_t bitsPerSecond)
{
    uint16_t fCLK_devisor;
    uint32_t baud_devisor;
    
    /*  Calculate division ratio of the operation clock to be stored in bits 15:9 
    of the SDRmn register. */
    
    baud_devisor = ((RL78_MAIN_SYSTEM_CLOCK / bitsPerSecond / 2) - 1);
    
    /* increase the fCLK devisor each time the baud rate is divided until it fits */
    for(fCLK_devisor = 0; fCLK_devisor<12; fCLK_devisor++)
    {
        /* check if baud_devisor is greater than 7 bits */
        if(baud_devisor > 127)
        {
            baud_devisor = baud_devisor/2;
        }
        else
        {
            SPS1  = fCLK_devisor & 0xF;   /* Serial clock select register */
            SDR13 = baud_devisor<<9;
            break;
        }
    }
}


void ConfigureOutputPorts(void)
{
    /* Set Initial Output Value */
    P14 = 0xDF; /* LCD_CS 0xFE */   /*G14 -> P145  */
    /* Set Output Port Modes */
    PM14 = 0xDF; /* LCD CS 0xFE */   /*G14 -> P145 */
    ADPC = 0x01; /* GV: Fixes LED12 issue */
}


void SPI_Init()
{
    IO_Reset();
 //   ConfigureOutputPorts();
    LCD_CREATE();
    WIFI_CREATE();
    SPI_SetBitRate(11500);
    LCD_START();
    WIFI_START();
}

void IO_Reset()
{
    volatile int i = 0;
    
		//#warning RESET-IO must be inverted for actual HW
		RESET_IO_PORT |= (1<<RESET_IO_BIT_POS);
		// P13 |= (1<<0); // Assert #RESET-IO
    for (i=0;i<10000;i++)
			;
		RESET_IO_PORT &= !(1<<RESET_IO_BIT_POS);
		//    P13 &= ~(1<<0);  // Deassert #RESET-IO
    for (i=0;i<10000;i++)
			;
}

void SPI_CS_Start(uint8_t aDevice)
{
	*SPI_CS_Port[aDevice] &= ~(1<<SPI_CS_Pin[aDevice]);
}

void SPI_CS_End(uint8_t aDevice)
{
	*SPI_CS_Port[aDevice] |= (1<<SPI_CS_Pin[aDevice]);
}

void SPI_Send(uint8_t aDevice, uint8_t *aData, uint32_t aLength)
{
		volatile uint16_t d;

    uint8_t noRXData;	
		SPI_CS_Start(aDevice);

		for (d=100; d>0; d--) // delay
			;

    switch(aDevice)
    {
      case SPI_LCD  :
                G_SPI21_SendingData = 1;
		G_SPI21_ReceivingData = 0;
                LCD_SEND_RECEIVE(aData, aLength, &noRXData);  
                while(G_SPI21_SendingData);
                break;
      case SPI_WIFI :
                G_SPI31_SendingData = 1;
		G_SPI31_ReceivingData = 0;
                WIFI_SEND_RECEIVE(aData, aLength, &noRXData);  
                while(G_SPI31_SendingData);
                break;
    }
   
    
    SPI_CS_End(aDevice);
}

void SPI_SendReceive(uint8_t aDevice, uint8_t *aTXData, uint32_t aTXLength, uint8_t *aRXData)
{
		volatile uint16_t d;
     SPI_CS_Start(aDevice);
	
		for (d=100; d>0; d--) // delay
			;
    switch(aDevice)
    {
      case SPI_LCD  :
                G_SPI21_SendingData = 1;
		G_SPI21_ReceivingData = 1;
                LCD_SEND_RECEIVE(aTXData, aTXLength, aRXData);  
                while(G_SPI21_SendingData || G_SPI21_ReceivingData);
                break;
      case SPI_WIFI :
                G_SPI31_SendingData = 1;
		G_SPI31_ReceivingData = 1;
                WIFI_SEND_RECEIVE(aTXData, aTXLength, aRXData);  
                while(G_SPI31_SendingData || G_SPI31_ReceivingData);
                break;
    }
    SPI_CS_End(aDevice);
}


               