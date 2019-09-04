//picpgm_menu.c
// menu for picpgm


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <string.h>
#include <time.h>

#include <wiringPi.h>
#include <lcd.h>

//defines
#define VERSION "V1.0"



#define S_LEFT 14  //S1
#define S_UP 21  //S2
#define S_ENTER 22  //S3
#define S_DOWN 23  //S4
#define S_RIGHT 24  //S5
#define S_USB 25  //S6
#define S_DETECT 8  //S7
#define S_PRG 9  //S8

#define L_READY 28 //D1
#define L_WAITING 27 //D2
#define L_BUSY 26 //D3
#define L_ERROR 11 //D4
#define L_USB 29 //D5



// Global lcd handle:
static int lcdHandle ;

//simple debounce and 'long hold' routine
//returns:
//0 - switch not pressed or bounce detected
#define PIN_NOT_PRESSED 0
//1 - switch pressed short
#define PIN_PRESSED_SHORT 1
//2 - switch pressed long period
#define PIN_PRESSED_LONG 2
int get_switch_status( int pin)
{

unsigned int time_pressed, time_passed;

//switch closed?
if (digitalRead(pin)) return PIN_NOT_PRESSED; //we have a pull up resistor, so status!=0 means switch open

//switch is closed, lets do a debounce
delay(50);
if (digitalRead(pin)) return PIN_NOT_PRESSED; //closed to short
time_pressed = millis(); //store the time

//OK, switch is closed, lets wait for open again
while ( digitalRead(pin) == PIN_NOT_PRESSED);

//lookhow long the switch was pressed
time_passed = millis() - time_pressed;

if ( time_passed > 800) return PIN_PRESSED_LONG; else return PIN_PRESSED_SHORT;


}


//control all leds
//0 -> OFF
//1 -> ON
//2 -> toggle
//will set to 0 with first call
void ctrl_leds(unsigned char cmd)
{
 static unsigned char status = 1;

switch(cmd)
{
  case 0:
   digitalWrite ( L_READY, 0);
   digitalWrite ( L_WAITING, 0);
   digitalWrite ( L_BUSY, 0);
   digitalWrite ( L_ERROR, 0);
   digitalWrite ( L_USB, 0);
  break;
  case 1:
   digitalWrite ( L_READY, 1);
   digitalWrite ( L_WAITING, 1);
   digitalWrite ( L_BUSY, 1);
   digitalWrite ( L_ERROR, 1);
   digitalWrite ( L_USB, 1);
  break;
  case 2:
 if(status)
 {
   digitalWrite ( L_READY, 0);
   digitalWrite ( L_WAITING, 0);
   digitalWrite ( L_BUSY, 0);
   digitalWrite ( L_ERROR, 0);
   digitalWrite ( L_USB, 0);
   status = 0;
 }
 else
 {
   digitalWrite ( L_READY, 1);
   digitalWrite ( L_WAITING, 1);
   digitalWrite ( L_BUSY, 1);
   digitalWrite ( L_ERROR, 1);
   digitalWrite ( L_USB, 1);
   status = 1;
 }
 break;
}//switch
}

//interrupt routines
void myInterrupt1 (void) { printf("switch 1 pressed\n"); }
void myInterrupt2 (void) { printf("switch 2 pressed\n"); }
void myInterrupt3 (void) { printf("switch 3 pressed\n"); }
void myInterrupt4 (void) { printf("switch 4 pressed\n"); }
void myInterrupt5 (void) { printf("switch 5 pressed\n"); }
void myInterrupt6 (void) { printf("switch 6 pressed\n"); }
void myInterrupt7 (void) { printf("switch 7 pressed\n"); }
void myInterrupt8 (void) { printf("switch 8 pressed\n"); }
 
int main (int argc, char *argv[])
{


  wiringPiSetup () ;

//init switches
/*
pinMode ( S_LEFT, INPUT);
pinMode ( S_UP, INPUT);
pinMode ( S_ENTER, INPUT);
pinMode ( S_DOWN, INPUT);
pinMode ( S_RIGHT, INPUT);
pinMode ( S_USB, INPUT);
pinMode ( S_DETECT, INPUT);
pinMode ( S_PRG, INPUT);
pullUpDnControl ( S_LEFT, PUD_UP);
pullUpDnControl ( S_UP, PUD_UP);
pullUpDnControl ( S_ENTER, PUD_UP);
pullUpDnControl ( S_DOWN, PUD_UP);
pullUpDnControl ( S_RIGHT, PUD_UP);
pullUpDnControl ( S_USB, PUD_UP);
pullUpDnControl ( S_DETECT, PUD_UP);
pullUpDnControl ( S_PRG, PUD_UP);
*/
unsigned char switches[8] = { S_LEFT,S_UP,S_ENTER,S_DOWN,S_RIGHT,S_USB,S_DETECT,S_PRG};
  wiringPiISR (switches[0], INT_EDGE_FALLING, &myInterrupt1) ;
  wiringPiISR (switches[1], INT_EDGE_FALLING, &myInterrupt2) ;
  wiringPiISR (switches[2], INT_EDGE_FALLING, &myInterrupt3) ;
  wiringPiISR (switches[3], INT_EDGE_FALLING, &myInterrupt4) ;
  wiringPiISR (switches[4], INT_EDGE_FALLING, &myInterrupt5) ;
  wiringPiISR (switches[5], INT_EDGE_FALLING, &myInterrupt6) ;
  wiringPiISR (switches[6], INT_EDGE_FALLING, &myInterrupt7) ;
  wiringPiISR (switches[7], INT_EDGE_FALLING, &myInterrupt8) ;


//init LEDs
pinMode ( L_READY, OUTPUT);
pinMode ( L_WAITING, OUTPUT);
pinMode ( L_BUSY, OUTPUT);
pinMode ( L_ERROR, OUTPUT);
pinMode ( L_USB, OUTPUT);
ctrl_leds(0);


//    int  lcdInit (int rows, int cols, int bits, int rs, int strb,
//            int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7) ;

  lcdHandle = lcdInit (2, 16, 4, 7,0, 2,3,12,13,0,0,0,0) ;

  if (lcdHandle < 0)
  {
    fprintf (stderr, "%s: lcdInit failed\n", argv [0]) ;
    return -1 ;
  }


  //lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "picpgm Menu V1.0") ;
  lcdPosition (lcdHandle, 0, 0) ; lcdPrintf (lcdHandle, "picpgm Menu %s",VERSION) ;
  lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, "  by bontango   ") ;
  sleep(1); ctrl_leds(1);
  sleep(1); ctrl_leds(0);
  sleep(1); ctrl_leds(1);
  sleep(1); ctrl_leds(0);
  sleep(1); ctrl_leds(1);
  sleep(1); ctrl_leds(0);
  lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, "    READY!      ") ;
  digitalWrite ( L_READY, 1);

  //loop for checking all switches
  while(1) {

  };
}

