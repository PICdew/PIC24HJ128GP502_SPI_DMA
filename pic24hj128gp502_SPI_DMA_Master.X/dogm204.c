/*****************************************************************************
  FileName:        dogm204.c
  Processor:       PIC24HJ128GP502
  Compiler:        XC16 ver 1.30
******************************************************************************/
#include "xc.h" /* wykrywa rodzaj procka i includuje odpowiedni plik*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> /*dyrektywy uint8_t itp*/
#define FCY 40000000UL /* podajemy wartosc ustawionego zegara (40 MHz), wazne 
aby przed includowaniem <libpic30.h>, potrzebne to jest to wyliczania delay-i*/
#include <libpic30.h> /*biblioteka dajaca dostepp do delay-i.*/
#include "dogm204.h"

/*definicje funkcji*/
void Wyslij_do_LCD(unsigned char bajt)
{
	/*ustaw linie EN, przed wysylka danych*/

    E = 1;
  	/*wyslanie 4 najstarszych bitow danych*/
	if(bajt & 0x80)	DB7	= 1; else DB7 = 0;
	if(bajt & 0x40)	DB6	= 1; else DB6 = 0;
	if(bajt & 0x20)	DB5	= 1; else DB5 = 0;
	if(bajt & 0x10)	DB4	= 1; else DB4 = 0;
	__delay_us(1);
	/*potwierdzenie wyslania danych (opadajacym zboczem EN)*/
	E = 0;
		
	/*ustawienie EN*/
	__delay_us(1);
    E = 1;
  	/*wyslanie 4 najmlodszych bitow danych*/	
	if(bajt & 0x08)	DB7	= 1; else DB7 = 0;
	if(bajt & 0x04)	DB6	= 1; else DB6 = 0;
	if(bajt & 0x02)	DB5	= 1; else DB5 = 0;
	if(bajt & 0x01)	DB4	= 1; else DB4 = 0;
	__delay_us(1);
	/*potwierdz wysylke danych opadajacym zboczem EN*/
	E = 0;

	__delay_us(16);
    
}	


void WlaczLCD()
{
	/*ustawienie kierunku wyjsciowego linii podlaczonych do LCD*/
	TRIS_RESET = 0 ;
    TRIS_RW = 0 ;
    TRIS_RS = 0;
	TRIS_E = 0;
	TRIS_DB7 = 0;
	TRIS_DB6 = 0;
	TRIS_DB5 = 0;
	TRIS_DB4 = 0;

	/*zerowanie linii*/
	RESET = 1 ; /* 0 - Stan aktywny*/
    RW = 0 ;
    RS = 0; /* 0 - wskazuje na rejestr rozkazow / 1 - wskazuje na rejestr danych*/
	E = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;

    /*Start Inicjalizacji DOGM204 tryb 4-bity*/
	/*zaczekaj co najmniej 5 ms na ustabilizowanie sie napiecia*/
	__delay_ms(5);
    /*Hardware Reset 10ms*/
    RESET = 0 ;
    __delay_ms(10);
    RESET = 1 ;
    __delay_ms(1);
    
  /*Sekwencja startowa dla trybu 4-bit, patrz mini-datasheet str 5*/
  Wyslij_do_LCD(0x33);//wysylamy instrukcje do rejestru rozkazow
  Wyslij_do_LCD(0x32);
  Wyslij_do_LCD(0x2A);
  Wyslij_do_LCD(0x09);
  Wyslij_do_LCD(0x06);
  Wyslij_do_LCD(0x1E);
  Wyslij_do_LCD(0x29);
  Wyslij_do_LCD(0x1B);
  Wyslij_do_LCD(0x6E);
  Wyslij_do_LCD(0x57);
  Wyslij_do_LCD(0x72);
  Wyslij_do_LCD(0x28);
  Wyslij_do_LCD(0x0F); /*Display on, cursor on, blink on*/
  CzyscLCD();
RS = 1 ; /*przelacz na rejestr danych*/  
        
 /*Koniec inicjalizacji i ustawien wyswietlacza DOGM204*/      
}

void WyswietlLCD(char *napis)
{
    while(*napis){
    Wyslij_do_LCD(*napis++);
    }
         
}
// wyslanie liczby dziesietnej
    void lcd_int(uint16_t val)
    {
    char bufor[17];
    sprintf(bufor,"%i",val);
    WyswietlLCD(bufor);
    }

void UstawKursorLCD(uint8_t y, uint8_t x)
{
	uint8_t n ;
    /*y (wiersze) = 1 do 4*/
    /*x (kolumna) = 1 do 20*/
    /*ustal adres pocz?tku znaku w wierszu*/
	switch(y)
    {
        case 1: y = LCD_Line1 ;break;
        case 2: y = LCD_Line2 ;break;
        case 3: y = LCD_Line3 ;break;
        case 4: y = LCD_Line4 ;break;
    
    }
    /*ustal nowy adres pami?ci DD RAM*/
	/*ustaw bajt do Set DDRAM adres*/
    /* x odejmujemy jeden aby przekonwertowa? z 0-19 na 1-20 */
	n = 0b10000000 + y + (x-1) ;
	
	/*wy?lij rozkaz ustawienia nowego adresu DD RAM*/
	RS = 0; /*stan niski na lini? RS, wybieramy rejestr instrukcji*/
	Wyslij_do_LCD(n);
    RS = 1;  /*prze??cz na rejestr danych */ 
}

void CzyscLCD()
{
	RS = 0; /*przelacz na rejestr rozkazow*/
	Wyslij_do_LCD(1);
	RS = 1; /*przelacz na rejestr danych*/
	/*czekaj ??*/
	__delay_us(1);
} 
/*funkcja definiuje jeden znak*/
void WpiszSwojeZnaki(void) {
    /*definicja wlasnych znaków maks 8 szt*/
    char znak1[]= {0,0,14,17,31,16,14,2}; /* definicja literki e z ogonkiem */
    char znak2[]= {0x0C,0x12,0x12,0x0C,0,0,0,0};/*definicja stC*/
    int i; 
    /* adresy poczatku definicji znaku to wielokrotnosc osmiu DEC(0,8,16,24,32,40,48,56)
     * ale uwaga wazne ! adresy kodowane sa na 6 mlodszych bitach dwa najstarsze bity
     * to zawsze  01 (01AAAAAA-gdzie A adres).Uwzgledniajac wartosc calego bajtu
     * adresy poczatku beda wygladal tak HEX(0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78)
     * Aby wpisac do pamieci wyswietlacza zdefiniowany znak nalezy najpierw wyslac 
     * do rejestru rozkazów (RS na 0) adres poczatku definicji znaku 
     * a w drugim kroku przesylamy dane (RS=1) 8 x bajt (tablica) definjujace obraz znaku*/
    
    RS = 0 ;/*stan niski na linii RS, wybieramy rejestr instrukcji*/
     /*wysylamy instrukcje do rejestru rozkazow (ustaw adres poczatkowy w CGRAM 
      na nasz znak w tym przypadku znak na pozycji drugiej) */
    Wyslij_do_LCD(0x48);/*wysylamy instrukcje do rejestru rozkazow 
     (ustaw adres poczatkowy w CGRAM na nasz znak w tym przypadku znak na pozycji drugiej) */
    
    RS = 1 ;/*stan wysoki na linii RS, wybieramy rejestr danych*/
    /*wysylamy 8 x bajt zdefiniowanego w tablicy znak1[] znaku*/
    for(i=0;i<=7;i++)
    {
       Wyslij_do_LCD(znak1[i]);
    }
   
    RS = 0 ;/*stan niski na lini RS, wybieramy rejestr instrukcji*/
    /*ustawiamy adres DDRAM na pierwszy znak w pierwszej linii, nie zapomnijmy
     o tym poniewaz inaczej zostaniemy w pamieci CGRAM*/
    Wyslij_do_LCD(0x80);
    RS = 1 ; /*stan wysoki na linii RS, wybieramy rejestr danych*/
}
/*funkcja uniwersalna do definiowania znaku*/
void DefineCharacter(int8_t nr, char *znak)/*nr 0...7, *znak to wskaznik na tablice z danymi*/
{
    int i; 
    /* adresy poczatku definicji znaku to wielokrotnosc osmiu DEC(0,8,16,24,32,40,48,56)
     * ale uwaga wazne ! adresy kodowane sa na 6 mlodszych bitach dwa najstarsze bity
     * to zawsze  01 (01AAAAAA-gdzie A adres).Uwzgledniajac wartosc calego bajtu
     * adresy poczatku beda wygladal tak HEX(0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78)
     * Aby wpisac do pamieci wyswietlacza zdefiniowany znak nalezy najpierw wyslac 
     * do rejestru rozkazów (RS na 0) adres poczatku definicji znaku 
     * a w drugim kroku przesylamy dane (RS=1) 8 x bajt (tablica) definjujace obraz znaku*/
    
    RS = 0 ;/*stan niski na linii RS, wybieramy rejestr instrukcji*/
     /*wysylamy instrukcje do rejestru rozkazow*/
    Wyslij_do_LCD((nr*8)|CGRAM_SET);/*ustaw adres poczatkowy w CGRAM na nasz znak*/
    
    RS = 1 ;/*stan wysoki na linii RS, wybieramy rejestr danych*/
    /*wysylamy 8 x bajt zdefiniowanego w tablicy znak[] znaku*/
    for(i=0;i<=7;i++)
    {
       Wyslij_do_LCD(*znak++);
    }
   
    RS = 0 ;/*stan niski na lini RS, wybieramy rejestr instrukcji*/
    /*ustawiamy adres DDRAM na pierwszy znak w pierwszej linii, nie zapomnijmy
     o tym poniewaz inaczej zostaniemy w pamieci CGRAM*/
    Wyslij_do_LCD(0x80);
    RS = 1 ; /*stan wysoki na linii RS, wybieramy rejestr danych*/ 
    
    /*definiowanie znaku DefineCharacter(1,tablica) gdzie nr 0...7 a tablica to 
     np char tablica[]= {0x0C,0x12,0x12,0x0C,0,0,0,0} definicja stC*/
    
    /*wywolanie zdefiniowanego znaku Wyslij_do_LCD(nr) gdzie nr to 0...7 lub
     WyswietlLCD("\nr"*/
}