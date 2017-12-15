/*****************************************************************************
  FileName:        dogm204.h
  Processor:       PIC24HJ128GP502
  Compiler:        XC16 ver 1.30
******************************************************************************/
#ifndef DOGM204_H
#define	DOGM204_H

/*_TRISB3 --> TRISBbits.TRISB3*/
#define TRIS_RESET   _TRISA0
#define TRIS_RW      _TRISB3
#define TRIS_RS      _TRISA2
#define TRIS_E       _TRISB2
#define TRIS_DB4     _TRISB12
#define TRIS_DB5     _TRISB13
#define TRIS_DB6     _TRISB14
#define TRIS_DB7     _TRISB15
/*_RB3 --> PORTBbits.RB3*/
#define RESET   _RA0
#define RW      _RB3
#define RS      _RA2
#define E       _RB2
#define DB4     _RB12
#define DB5     _RB13
#define DB6     _RB14
#define DB7     _RB15

/* przyporzadkowanie adresow pamieci DD-RAM do pol wyswietlacza*/
#define LCD_Line1 0x00 /*adres 1 znaku 1 wiersza */
#define LCD_Line2 0x20 /*adres 1 znaku 2 wiersza */
#define LCD_Line3 0x40 /*adres 1 znaku 3 wiersza */
#define LCD_Line4 0x60 /*adres 1 znaku 4 wiersza */

#define CGRAM_SET 0x40

 void Wyslij_do_LCD(unsigned char bajt);
 void WlaczLCD();
 void WyswietlLCD(char *napis);
 void lcd_int(uint16_t val);
 void UstawKursorLCD(uint8_t y, uint8_t x);
 void CzyscLCD();
 void WpiszSwojeZnaki(void);
 void DefineCharacter(int8_t nr, char *znak);
#endif	/* DOGM204_H */

