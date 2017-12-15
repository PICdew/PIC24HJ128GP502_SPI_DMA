/*****************************************************************************
  FileName:        main.c
  Processor:       PIC24HJ128GP502
  Compiler:        XC16 ver 1.30
  Created on:      12 grudnia 2017, 09:37
  Description:     SPI SLAVE
 ******************************************************************************/

#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>      /*dyrektywy uint8_t itp*/
#include <string.h>
#include "ustaw_zegar.h" /*tutaj m.in ustawione FCY*/
#include <libpic30.h>    /*dostep do delay-i,musi byc po zaincludowaniu ustaw_zegar.h*/

void config_SPI_SLAVE(void);

volatile uint8_t i;
volatile uint16_t SPI_receive_data ;

#define LED1_TOG PORTA ^= (1<<_PORTA_RA1_POSITION) /*zmienia stan bitu na przeciwny*/
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
    
    TRISAbits.TRISA1 = 0 ; // RA1 jako wyjscie tu mamy LED
/*remaping pinow na potrzeby SPI
 SDO --> pin 11
 SDI --> pin 14
 SCK --> pin 15
 */
    RPOR2bits.RP4R = 7;     /*inaczej _RP4R  = 7*/
    RPINR20bits.SDI1R = 5;  /*inaczej _SDI1R = 5*/
    RPINR20bits.SCK1R = 6;  /*inaczej _SCKIR = 6*/
    
    config_SPI_SLAVE();
       
    while (1)
    {
           
    }

}

/*Konfiguracja SPI dla SLAVE*/
void config_SPI_SLAVE(void) {
    
IFS0bits.SPI1IF = 0;       /*Clear the Interrupt Flag*/
IEC0bits.SPI1IE = 0;       /*Disable The Interrupt*/

SPI1CON1bits.MODE16 = 0;   /*Communication is word-wide (8 bits)*/
SPI1STATbits.SPIEN = 1;    /*Enable SPI Module*/
/*Interrupt Controller Settings*/
IFS0bits.SPI1IF = 0;       /*Clear the Interrupt Flag*/
IEC0bits.SPI1IE = 1;       /*Enable The Interrupt*/
}

/*=============================================================================
Interrupt Service Routines.
=============================================================================*/
/*Obsluga wektora przerwania dla SPI1 , przerwanie zglaszane po zakonczeniu transferu*/
void __attribute__((interrupt, no_auto_psv))_SPI1Interrupt(void)
{
LED1_TOG ; /*zmieniaj stan wyjscia na przeciwny*/
SPI_receive_data = SPI1BUF; /*pobieramy odebrane dane*/
if(SPI_receive_data) SPI1BUF = SPI_receive_data ; /*jesli Master cos przyslal zaladuj to do wyslania*/
SPI_receive_data = 0; /*zeruj*/
IFS0bits.SPI1IF = 0 ; /*Clear SPI1 Interrupt Flag*/
}

