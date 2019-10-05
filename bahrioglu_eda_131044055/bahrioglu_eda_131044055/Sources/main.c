/****************************************/
/*            Eda BAHRÝOGLU             */
/*              131044055               */
/*           CSE 334 PROJECT            */          
/****************************************/

/*                            NOTLAR                                */
/********************************************************************/
/* Programda 1 ms lik delayler kullanilmistir.                      */
/* Proje için kullandýklarim :                                      */
/* 1-) PORTB LED Duraklar arasi mesafeler gösterilmistir.           */
/* 2-)7 SEGMENT ile trenin gelis zamaný ekranda gösterilmistir.     */
/* 3-)Dip switcler ile trenin yönü belirtilmistir.1 oldugunda düz   */
/*    0 oldugunda tersten gitme islemi yapar.                       */
/* 4-)TimerOverflow ile programin calýsma süres, hesaplanmýstir.    */
/* 5-)Output compare kullanilmistir.                                */
/* 6-)Buzzer kapi acilmasi ve kapanmasi sýrasýnda ötmesi icin       */
/*    kullanilmistir.                                               */
/* 7-)interrupt TOF ile birlikte kullanilmistir.                    */
/* 8-)lcd mesajlar icin kullanilmistir.(Durak ve msjlar)            */
/* 9-) Serial comunication lcd kodu icinde msj gönderirken          */
/*      kullanilmistir.                                             */ 
/* 10-)Keypad acil durum butonu icin kullanilmistir.                */
/* Keypad,lcd  icin kitaptan yararlanilmistir.                       */
/********************************************************************/



#include <hidef.h>      /* common defines and macros */
#include "derivative.h" /* derivative-specific definitions */

#define MAX 9

#define A5  880     
#define B5  987
#define C5  523
#define D5  587
#define E5  659
#define F5  698
#define G5  783

#define LCD_DAT PORTK   /* Port K drives LCD data pins, E, and RS */
#define LCD_DIR DDRK    /* Direction of LCD port */ 
#define LCD_E 0x02      /* LCD E signal */ 
#define LCD_RS 0x01     /* LCD Register Select signal */ 
#define CMD 0           /* Command type put2lcd */ 
#define DATA 1          /* Data type put2lcd */

unsigned int array[]= { A5,B5,C5,F5,A5,G5,B5,G5,D5
};

unsigned int count;
                      
void openlcd(void);               /* Init LCD display */ 
void put2lcd(char c, char type);  /* Write command or data to LCD controller */ 
void puts2lcd (char *ptr);        /* Write a string to the LCD display */ 
void ms_delay(int n);             /* Delay  1 ms  */ 
void lcdMainStraight(void);
void lcdMainBack(void);
void dipSwitchFunc(void);
void ledFunc(void);
void GateAlarmFunc(unsigned int array[]);
void TOFInit(void);
void TimerProgramDisplay(void);
void keypad(void);

void main(void) {
  /* put your own code here */
 
  TOFInit();  
  TimerProgramDisplay();
  dipSwitchFunc();
  keypad();  
  __asm CLI;
  
 
	EnableInterrupts;


  for(;;) {
    _FEED_COP(); /* feeds the dog */
  } /* loop forever */
  /* please make sure that you never leave main */
}



void openlcd(void) {

   LCD_DIR = 0xFF;     /* make LCD_DAT output */
   ms_delay(100);     /* Wait LCD to be ready */ 
   put2lcd(0x28, CMD); /* set 4-bit data, 2-line display, 5x7 font */ 
   put2lcd(0x0F, CMD); /* turn on display, cursor, blinking */ 
   put2lcd(0x06, CMD); /* move cursor right */
   put2lcd(0x01, CMD);  /* clear screen, move cursor to home */ 
   ms_delay(150);       /* wait until "clear display" command complete */
  
}                  

 /*******************************************************************/
/*used with Serial port */
/*msjý bit bit alir.*/
void puts2lcd (char *ptr) {

   SCI0BDH = 0x00;
   SCI0BDL = 0x0D;
   SCI0CR1 = 0x00;
   SCI0CR2 = 0x0C;
   
   while (*ptr) { 
    
     put2lcd(*ptr, DATA); /* Write data to LCD */ 
     ms_delay(1);       /* Wait for data to be written */ 
    
     while(!(SCI0SR1 & SCI0SR1_TDRE_MASK));
     SCI0DRL = *ptr;
    
     ptr++;              
     
   } 
   
}

/***********************************************************************/
/*Kitaptan alinmistir.*/

void put2lcd(char c, char type) {

 char c_lo, c_hi;
 c_hi = (c & 0xF0) >> 2; /* Upper 4 bits of c */ 
 c_lo = (c & 0x0F) << 2; /* Lower 4 bits of c */
 
 LCD_DAT &= (~LCD_RS);   /* select LCD command register */
 
 
   if (type == DATA)
    
    LCD_DAT = c_hi | LCD_RS; /* output upper 4 bits, E, RS high */ 
    
   else 
   
    LCD_DAT = c_hi; /* output upper 4 bits, E, RS low */
    
   
 
 LCD_DAT |= LCD_E; /* pull E signal to high */
 
 __asm(nop); /* Lengthen E */ 
 __asm(nop); 
 __asm(nop);


LCD_DAT &= (~LCD_E); /* pull E to low */

if (type == DATA)
 
  LCD_DAT = c_lo | LCD_RS; /* output lower 4 bits, E, RS high */
  
else LCD_DAT = c_lo; /* output lower 4 bits, E, RS low */

  LCD_DAT |= LCD_E; /* pull E to high */
  
__asm(nop); /* Lengthen E */ 
__asm(nop); 
__asm(nop);

LCD_DAT &= (~LCD_E); /*pull E to low */

ms_delay(1); /* Wait for command to execute */ 

}

/******************************************************************************/

/*1 ms delay*/
void ms_delay(int n){
  
  int i, j;
  
  for(i=0; i<n; i++)
    for(j=0; j<329; j++) ;
  
}


/******************************************************************************/
/*Duraklarýn düz istikamette ilerlemesini saglayan fonksiyondur.*/
/*Buzzer, delay ve ledler kullanilmistir.                       */

void lcdMainStraight(void){
   
  char *msg1 = "Kadikoy";
  char *msg2 = "Ayrilik cesmesi";
  char *msg3 = "Acibadem";
  char *msg4 = "Unalan";
  char *msg5 = "Goztepe";
  char *msg6 = "Bostanci";
  char *msg7 = "Kucukyali";
  char *msg8 = "Maltepe";
  char *msg9 = "Kartal"; 
  char *msg10 ="Pendik";
  char *msg ="Sonraki Durak";
   
  openlcd();  // Initialize LCD disp
  
 
    
  puts2lcd(msg1); // Send first line
  ms_delay(20000);   
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg2); // Send second line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array); 
   
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg3); // Send third line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg4); // Send four line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
   
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg5); // Send five line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg6); // Send six line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg7); // Send seven line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg8); // Send eight line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg9); // Send nine line
  ms_delay(20000);
  ledFunc();
 
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg10); // Send ten line
  ms_delay(20000); 
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  __asm(swi);
  
}
/****************************************************************************/
/*Duraklarýn ters istikamette ilerlemesini saglayan fonksiyondur.*/
/*Buzzer, delay ve ledler kullanilmistir.                        */

void lcdMainBack(void){
 
  char *msg1 = "Kadikoy";
  char *msg2 = "Ayrilik cesmesi";
  char *msg3 = "Acibadem";
  char *msg4 = "Unalan";
  char *msg5 = "Goztepe";
  char *msg6 = "Bostanci";
  char *msg7 = "Kucukyali";
  char *msg8 = "Maltepe";
  char *msg9 = "Kartal"; 
  char *msg10 ="Pendik";
  char *msg ="Sonraki Durak";
  
  openlcd();  // Initialize LCD disp
   
  puts2lcd(msg10); // Send first line
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg9); // Send second line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
    
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
 
  puts2lcd(msg8); // Send third line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg7); // Send four line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  put2lcd(0x01, CMD);
  ms_delay(10000);
   
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg6); // Send five line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg5); // Send six line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg4); // Send seven line
  ms_delay(20000);
  ledFunc();
 
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg3); // Send eight line
  ms_delay(20000);
  ledFunc();

  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg2); // Send nine line
  ms_delay(20000);
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  puts2lcd(msg);
  ms_delay(20000);
  put2lcd(0x01, CMD); 
  ms_delay(10000);
  
  puts2lcd(msg1); // Send ten line
  ms_delay(20000); 
  ledFunc();
  
  GateAlarmFunc(array);
      
  PORTB = 0x01;
  ms_delay(10000);
  PORTB = 0x03;
  ms_delay(10000);
  PORTB = 0x07;
  ms_delay(10000);
  
  GateAlarmFunc(array);
  put2lcd(0x01, CMD);
  ms_delay(10000);
  
  __asm(swi);
  
}


/*********************************************************************************/
/*Dippswitch fonksiyonudur*/
/*Trenin gidiþ yönünü belirtir.*/

void dipSwitchFunc(void){

 DDRH = 0x00;
 
 if(PTH == 1) 
 
  lcdMainStraight();
  
 else
 
  lcdMainBack(); 
  
}

/**************************************************************/
/*Ledlerin yanmasini saglayarak duraklar arasi mesafeleri ve */ 
/*kapýnýn acýlýp kapanma süresini belirtir                   */
void ledFunc(void) {
 
   DDRB = 0xFF;
   
   PORTB = 0x01;
   ms_delay(10000);
   PORTB = 0x03;
   ms_delay(10000);
   PORTB = 0x07;
   ms_delay(10000);
   PORTB = 0x0F;
   ms_delay(10000);
   PORTB = 0x1F;
   ms_delay(10000);
   PORTB = 0x3F;
   ms_delay(10000);
   PORTB = 0x7F;
   ms_delay(10000);
   PORTB = 0xFF;
   ms_delay(10000); 
  
}


/*************************************************************************/
/*Alarrmalar icin kullanýlan outout compare ile buzzer fonksiyonudur.*/
void GateAlarmFunc(unsigned int array[]){

  int i,j;
    
  DDRT = DDRT | 0x20; /*pin 5 kullanilmistir.*/
  TSCR1 = 0x80; /*enable timer*/
  TSCR2 = 0x00; /*no prescaler*/
  TIOS = 0x20; /*select channel 5 output comp*/
  TCTL1 = 0x04;

  for(j=0; j<4000; ++j){
      
    for(i=0; i<MAX; ++i) {
      
     count = TCNT;
     count = count + array[i];
     TC5 = count;
     while(!(TFLG1 & TFLG1_C5F_MASK)); 
     TFLG1 = TFLG1 | TFLG1_C5F_MASK;
   
    }
  }
  
  TIOS = 0x00; /*Buzzerin sesinin kesilmesini saglar.*/ 
}


/***************************************************************************/
/*TOF icin initiaize fonksiyonudur.                                        */
void TOFInit(void){

  DDRP= 0xFF;
  DDRB = 0xFF;
  count = 0;
  TSCR1 = 0x80;
  TSCR2 = 0x86;
  TFLG2 = TFLG2 | TFLG2_TOF_MASK;
      
  
   
}


/*************************************************************************/
/*TOF icin interrup yapilmistir.*/
interrupt (((0x10000-Vtimovf)/2)-1) void TOF_ISR(void) {

   count++;
   PORTB = count;
   TFLG2 = TFLG2 | TFLG2_TOF_MASK ;

}

/****************************************************************************/



/*Trenin gelis zamaný kadar 7 segmentte gösterir.*/
void TimerProgramDisplay(void){
  
    int data,result;
    data = count*3; //her timeroverflowda 2.73 ile carp ms 
    result = data/1000;
    
     
    DDRB = 0xFF;
    DDRP = 0xFF;
    
     
    PORTB = 0x30;
    PTP = 0x0E;   
    ms_delay(20000);
   
    PORTB = 0x5B;
    PTP = 0x0D;
    ms_delay(20000);              
  
    PORTB = 0x4F;
    PTP = 0x0B;
    ms_delay(20000);
    
    PORTB = 0x72; 
    PTP = 0x07;    /*portb ledinde 0 yanar*/  
  
   
}

/*****************************************************************************/

void keypad(void){
  
   const unsigned char keypad[4][4] =
   {
    '1','2','3','A',
    '4','5','6','B',
    '7','8','9','C',
    '*','0','#','D'
    
   };
   unsigned char column,row;
   
   DDRB = 0xFF;                           //MAKE PORTB OUTPUT
   DDRJ |=0x02; 
   PTJ &=~0x02;                           
   DDRP |=0x0F;                          
   PTP |=0x0F;                            //TURN OFF 7SEG LED
   DDRA = 0x0F;                           //MAKE ROWS INPUT AND COLUMNS OUTPUT
   
 
   while(1){                              
      do{                                 
         PORTA = PORTA | 0x0F;            
         row = PORTA & 0xF0;              //READ ROWS
      }while(row == 0x00);                //WAIT UNTIL KEY PRESSED //CLOSE do1



      do{                                 
         do{                              
            ms_delay(15);                  
            row = PORTA & 0xF0;           //READ ROWS
         }while(row == 0x00);             //CHECK FOR KEY PRESS //CLOSE do3
         
         ms_delay(180);                     
         row = PORTA & 0xF0;
      }while(row == 0x00);                

      while(1){                           
         PORTA &= 0xF0;                   
         PORTA |= 0x01;                   //COLUMN 0 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 0
            column = 0;
            break;                       
         }
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x02;                   //COLUMN 1 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 1
            column = 1;
            break;                        
         }

         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x04;                   //COLUMN 2 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 2
            column = 2;
            break;                        
         }
         PORTA &= 0xF0;                   //CLEAR COLUMN
         PORTA |= 0x08;                   //COLUMN 3 SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
         if(row != 0x00){                 //KEY IS IN COLUMN 3
            column = 3;
            break;                        
         }
         row = 0;                         //KEY NOT FOUND
      break;                              //step out of while(1) loop to not get stuck
      }                                   //end while(1)

      if(row == 0x10){
         PORTB=keypad[0][column];         //OUTPUT TO PORTB LED
          GateAlarmFunc(array);
      }
      else if(row == 0x20){
         PORTB=keypad[1][column];
          GateAlarmFunc(array);
      }
      else if(row == 0x40){
         PORTB=keypad[2][column];
          GateAlarmFunc(array);
      }
      else if(row == 0x80){
         PORTB=keypad[3][column];
         GateAlarmFunc(array);
      }

      do{
         ms_delay(180);
         PORTA = PORTA | 0x0F;            //COLUMNS SET HIGH
         row = PORTA & 0xF0;              //READ ROWS
      }while(row != 0x00);                //MAKE SURE BUTTON IS NOT STILL HELD
   }                                      
  
}

/*****************************************************************************************/
/**********************************    END    ********************************************/