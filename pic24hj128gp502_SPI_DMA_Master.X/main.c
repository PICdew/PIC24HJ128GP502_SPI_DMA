/*****************************************************************************
  FileName:        main.c
  Processor:       PIC24HJ128GP502
  Compiler:        XC16 ver 1.30
  Created on:      12 grudnia 2017, 09:37
  Description:     SPI DMA MASTER
 ******************************************************************************/

#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /*definicje typu uint8_t itp*/
#include <string.h>
#include "ustaw_zegar.h" /*tutaj m.in ustawione FCY*/
#include <libpic30.h> /*dostep do delay-i,musi byc po zaincludowaniu ustaw_zegar.h*/
#include "dogm204.h"

#define LED1_TOG PORTA ^= (1<<_PORTA_RA1_POSITION) /*zmienia stan bitu na przeciwny*/

void config_DMA0_SPI1(void);
void config_DMA1_SPI1(void);
void config_SPI_MASTER(void);

char BuforTX[] __attribute__((space(dma)))="ALAMAKOTA";
char BuforRX[20] __attribute__((space(dma)));
volatile uint16_t SPI_receive_data ;

int main(void) {
    ustaw_zegar(); /*odpalamy zegar wewnetrzny na ok 40MHz*/
    __delay_ms(50); /*stabilizacja napiec*/
    /*
     * wylaczamy ADC , wszystkie piny chcemy miec cyfrowe
     * pojedynczo piny analogowe wylaczamy w rejestrze AD1PCFGL 
     * Po resecie procka piny oznaczone ANx sa w trybie analogowych wejsc.
     */
    PMD1bits.AD1MD = 1; /*wylaczamy ADC*/
    /* 
     * ustawiamy wszystkie piny analogowe (oznacznone ANx) jako cyfrowe
     * do zmiany mamy piny AN0-AN5 i AN9-AN12 co daje hex na 16 bitach = 0x1E3F
     */
    AD1PCFGL = 0x1E3F;
    TRISAbits.TRISA1 = 0 ; /*RA1 jako wyjscie tu mamy LED*/
    
    /*remaping pinow na potrzeby SPI
    SDO --> pin 11
    SDI --> pin 14
    SCK --> pin 15
    */
    RPOR2bits.RP4R = 7;     /*inaczej _RP4R = 7*/
    RPINR20bits.SDI1R = 5;  /*inaczej _SDI1R = 5*/
    RPOR3bits.RP6R = 8;     /*inaczej _RP6R = 8*/
    
    WlaczLCD(); /*LCD Init*/
    UstawKursorLCD(1,1);
    WyswietlLCD("TEST SPI DMA :");
         
    /*SPI init with DMA*/
    config_SPI_MASTER();
    config_DMA1_SPI1();
    config_DMA0_SPI1();
        
               
    while (1) {
               
        DMA0REQbits.FORCE = 1;          /*Manual Transfer Start*/
        while(DMA0REQbits.FORCE==1);    /*po zakonczonym transferze kanaly DMA zostaja zamkniete*/
        DMA1CONbits.CHEN = 1;			/*Enable DMA1 Channel*/	
        DMA0CONbits.CHEN = 1;           /*Enable DMA0 Channel*/
                              
        __delay_ms(1000) ;              /*bee*/
                          
    }

}

void config_DMA0_SPI1(void){
/*=============================================================================
 Konfiguracja DMA kanal 0 do transmisji SPI w trybie One_Shot (bez powtorzenia)
===============================================================================
 DMA0 configuration
 Direction: Read from DMA RAM and write to SPI buffer
 AMODE: Register Indirect with Post-Increment mode
 MODE: OneShot, Ping-Pong Disabled*/
 
/* Rejestr DMA0CON
 * bit15    -0 chen/chanel --> disable
 * bit14    -1 size --> Byte    
 * bit13    -1 dir --> Read from DMA RAM address write to peripheral address
 * bit12    -0 half --> Initiate block transfer complete interrupt when all of the data has been moved
 * bit11    -0 nullw --> Normal operation
 * bit10-6  -Unimplemented raed as 0
 * bit5-4   -00 amode  --> Register Indirect with Post_Incerement mode
 * bit3-2   -Unimplemented read as 0
 * bit1-0   -01 mode --> OneShot, Ping-Pong modes disabled*/
DMA0CON = 0x6001 ;/*0x6001 - wartosc wynika z ustawienia bitow jak wyzej*/
DMA0CNT = strlen(BuforTX);/*ustal ile znaków do przeslania max 1024 bajty*/
/*IRQ Select Register,wskazujemy SPI1*/
DMA0REQ = 0x000A ; /*SPI1*/
/*Peripheral Adress Register*/
DMA0PAD =  (volatile unsigned int)&SPI1BUF ; /*rzutowanie typu i pobranie adresu rejestru SPI1BUF*/
/*wewnetrzna konstrukcja/funkcja kompilatora*/
DMA0STA = __builtin_dmaoffset(BuforTX) ;/*taka jest konstrukcja wskazania na bufor z danymi*/

IFS0bits.DMA0IF = 0 ; /*clear DMA Interrupt Flag */
IEC0bits.DMA0IE = 1 ; /*enable DMA Interrupt */

/*Wazne :kanal DMA moze byc otwarty dopiero po wpisaniu danych do rejestru DMASTA i DMAxCNT*/
DMA0CONbits.CHEN  = 1; /*Canal DMA0 enable*/

  /*po zakonczonym transferze automatycznie kanal DMA zostaje wylaczony, zmienia
   *sie wartosc rejestru DMAxCON na 16-tym bicie z "1" na "0". Aby ponownie odpalic transfer
   *nalezy wlaczyc ten bit DMAxCON.bits.CHEN=1 i odpalic transfer DMA0REQbits.FORCE = 1
   */
}

void config_DMA1_SPI1(void)
{
/*=============================================================================
 Konfiguracja DMA kanal 1 do odbioru SPI w trybie One_Shot (bez powtorzenia)
===============================================================================
 DMA1 configuration
 Direction: Read from SPI buffer and write to DMA RAM
 AMODE: Register Indirect with Post-Increment mode
 MODE: OneShot, Ping-Pong Disabled*/
    
 /* Rejestr DMA1CON
 * bit15    -0 chen/chanel --> disable
 * bit14    -1 size --> Byte    
 * bit13    -0 dir --> Read from Peripheral address, write to DMA RAM address 
 * bit12    -0 half --> Initiate block transfer complete interrupt when all of the data has been moved
 * bit11    -0 nullw --> Normal operation
 * bit10-6  -Unimplemented raed as 0
 * bit5-4   -00 amode  --> Register Indirect with Post_Incerement mode
 * bit3-2   -Unimplemented read as 0
 * bit1-0   -01 mode --> OneShot, Ping-Pong modes disabled*/
    DMA1CON = 0x4001 ;          /*0x4001 - wartosc wynika z ustawienia bitow jak wyzej*/							
	DMA1CNT = strlen(BuforTX);/*ustal ile znaków do odebrania max 1024 bajty*/					
	DMA1REQ = 0x000A; /*podpinamy peryferium SPI do DMA*/					
	DMA1PAD = (volatile unsigned int) &SPI1BUF; /*rzutowanie typu i pobranie adresu rejestru SPI1BUF*/
	DMA1STA= __builtin_dmaoffset(BuforRX); /*taka jest konstrukcja wskazania na bufor z danymi*/
	IFS0bits.DMA1IF  = 0;			/*Clear DMA interrupt*/
	IEC0bits.DMA1IE  = 1;			/*Enable DMA interrupt*/
	DMA1CONbits.CHEN = 1;			/*Enable DMA Channel*/		
	
}

/*konfiguracja SPI dla Mastera*/
void config_SPI_MASTER(void) {
     
IFS0bits.SPI1IF = 0;                    /*Clear the Interrupt Flag*/
IEC0bits.SPI1IE = 0;                    /*Disable the Interrupt*/
/*Set clock SPI on SCK, 40 MHz / (4*8) = 1,250 MHz*/
//SPI1CON1bits.PPRE = 0b10;             /*Set Primary Prescaler 4:1*/
//SPI1CON1bits.SPRE = 0b000;            /*Set Secondary Prescaler 8:1*/

SPI1CON1bits.MODE16 = 0;                /*Communication is word-wide (8 bits)*/
SPI1CON1bits.MSTEN = 1;                 /*Master Mode Enabled*/
SPI1STATbits.SPIEN = 1;                 /*Enable SPI Module*/
IFS0bits.SPI1IF = 0;                    /*Clear the Interrupt Flag*/
IEC0bits.SPI1IE = 1;                    /*Enable the Interrupt SPI*/
}


/*=============================================================================
Interrupt Service Routines.
=============================================================================*/

void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void)
{
      IFS0bits.DMA0IF = 0;                /*Clear the DMA0 Interrupt Flag*/

}

void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void)
{
      LED1_TOG ;                          /*zmieniaj stan wyjscia na przeciwny*/
      UstawKursorLCD(2,1);
      WyswietlLCD(BuforRX+1);             /*wyswietlamy dane , ktore przyszly od Slave*/
      memset(BuforRX,0, strlen(BuforRX)); /*clear BuforRX, tak sprytnie sobie czyscimy bufor*/
      IFS0bits.DMA1IF = 0;                /*Clear the DMA1 Interrupt Flag*/

} 

void __attribute__((interrupt, no_auto_psv)) _SPI1Interrupt(void)
{

      IFS0bits.SPI1IF = 0;		/*Clear the DMA0 Interrupt Flag*/

}