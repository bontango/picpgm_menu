//picpgm_menu.c
// menu for picpgm

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <string.h>
#include <time.h>

#include <wiringPi.h>
#include <lcd.h>

//defines
#define VERSION "V0.3"



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

#define PIC_DIR "/boot/lisy/picpgm/"
#define USB_MOUNT_DIR "/pinmame/usb1/"
#define USB_DEVICE "/dev/sda1"




  // Global vars:
  static int lcdHandle ;
  char name[80]   = "PIC name:";
  char id[80]     = "Device ID:";
  char flash[80]  = "Flash:";
  char eeprom[80] = "EEPROM:";
  char cur_selection = 3; //current file selection
  static char files[100][80]; //space for 100 filenames
  char current_dir[80];  //current directory

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

//return closed switchnumber
//from an array of dip numbers with len number
//first match
//returns switchnumber or -1 if no switch pressed
int check_switches( unsigned char *pin, int number)
{

 int i,status;

 for ( i=0; i<number; i++)
 {
   status = get_switch_status(pin[i]);
   if (status) return (pin[i]);
 }

return -1;
}


//control all leds, except USB indicator
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
  break;
  case 1:
   digitalWrite ( L_READY, 1);
   digitalWrite ( L_WAITING, 1);
   digitalWrite ( L_BUSY, 1);
   digitalWrite ( L_ERROR, 1);
  break;
  case 2:
 if(status)
 {
   digitalWrite ( L_READY, 0);
   digitalWrite ( L_WAITING, 0);
   digitalWrite ( L_BUSY, 0);
   digitalWrite ( L_ERROR, 0);
   status = 0;
 }
 else
 {
   digitalWrite ( L_READY, 1);
   digitalWrite ( L_WAITING, 1);
   digitalWrite ( L_BUSY, 1);
   digitalWrite ( L_ERROR, 1);
   status = 1;
 }
 break;
}//switch
}

//set one led
//all others off
void set_led(unsigned char led)
{
  ctrl_leds(0);
  digitalWrite ( led, 1);

}

//control one led
void ctrl_led(unsigned char led, unsigned char action)
{
  digitalWrite ( led, action);

}



//do then programmin
//with the current selected file
int do_prg(void)
{


  FILE *fp;
  char cmd[256];
  char str[256];
  char message[256];
  int exit_code;

  sprintf(cmd,"/usr/bin/picpgm -p %s%s",current_dir,files[cur_selection] );
  fp = popen(cmd, "r");

   while ( fgets(str, sizeof(str)-1, fp) != NULL)
   {
     printf("%s\n",str);
   }
   exit_code = WEXITSTATUS(pclose(fp));

  switch(exit_code)
  {
   case 0: strcpy(message,"PIC succesfully programmed"); break;
   case 1: strcpy(message,"verify error occured"); break;
   case 2: strcpy(message,"no programmer interface found"); break;
   case 3: strcpy(message,"no PIC found"); break;
   case 4: strcpy(message,"invalid parameter"); break;
   case 5: strcpy(message,"HEX file has errors"); break;
   case 6: strcpy(message,"problems with loading port I/O driver"); break;
   case 7: strcpy(message,"no HEX file specified"); break;
   default: sprintf(message,"unexpected error occoured:%d ",exit_code); break;
  }

      lcdPosition (lcdHandle, 0, 0) ; lcdPrintf (lcdHandle, "%-32s",message) ;

  return exit_code;

}


//let picpgm atodetect the PIC
int do_autodetect(void) 
{
 
  FILE *fp;
  char str[256];
  char str1[256];
  char str2[256];
  int exit_code;

  fp = popen("/usr/bin/picpgm -r", "r");

   while ( fgets(str, sizeof(str)-1, fp) != NULL)
   {
     //look for the key words
     if ( strncmp(str,name,9) == 0)
      {
        sscanf(str,"%s %s %s",str1,str2,name);
      }
     if ( strncmp(str,id,10) == 0)
      {
        sscanf(str,"%s %s %s",str1,str2,id);
      }
     if ( strncmp(str,flash,6) == 0)
      {
        sscanf(str,"%s %s",str1,flash);
      }
     if ( strncmp(str,eeprom,7) == 0)
      {
        sscanf(str,"%s %s",str1,eeprom);
      }
   }

   exit_code = WEXITSTATUS(pclose(fp));

   if (exit_code == 0)
     {
      lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, " detected PIC:  ") ;
      lcdPosition (lcdHandle, 0, 1) ; lcdPrintf (lcdHandle, "%16s",name) ;
     }
   else
     {
      lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, " cannot identify") ;
      lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, "  PIC in socket ") ;
     }

 return exit_code;
}

//show_selection
//0 - initial read of directory give back number of files
//1 - step one down
//-1 - step one up
void show_selection(int cmd)
{
  int i = 0;
  DIR *d;
  char str[255];
  struct dirent *dir;
  static int no_of_files = 0;

 switch (cmd)
 {
  case 0:
  //read the whole dir
  d = opendir(current_dir);

  if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
             {
		//check for .hex extension
		if (strlen(dir->d_name) >= 5)
                  {
                    strcpy(str,dir->d_name);
		    printf("check: %s\n",str);
		    if (strncmp( &str[strlen(str)-4],".hex",4) == 0)
			{
                	 strcpy(files[i],dir->d_name);
			 printf("match: %s\n",files[i]);
		         ++i;
			}//match
                  } //strlen >= 5
             }
        }
        no_of_files = i -1;
        closedir(d);
     }
   cur_selection = 2;
   break;
  case 1:
   if ( cur_selection < no_of_files ) cur_selection++;
   break;
  case -1:
   if ( cur_selection > 0 ) cur_selection--;
   break;
 }

if(no_of_files >= 1)
  {
  lcdPosition (lcdHandle, 0, 0);
  lcdPrintf (lcdHandle, "%-32s",files[cur_selection]) ;
  }
else
  {
  lcdPuts (lcdHandle, "no .hex files   ") ;
  lcdPosition (lcdHandle, 0, 1);
  lcdPuts (lcdHandle, "  found         ") ;
  }

}


//try to mount th usb_stick
//set led and current_dir accordantly
int do_usb(void)
{
  FILE *fp;
  char cmd[80];
  char str[255];
  int exit_code;
  static char mounted = 0;

  if(mounted) //already mounted, do unmount
  {

  printf("USB mounted, try to unmount\n");

  //change current directory to be able to unmount
  //sprintf(cmd,"cd %s",PIC_DIR);
  //system(cmd);

  sprintf(cmd,"/bin/umount %s",USB_DEVICE);
  fp = popen(cmd, "r");

   while ( fgets(str, sizeof(str)-1, fp) != NULL)
   {
     printf("%s\n",str);
   }
   exit_code = WEXITSTATUS(pclose(fp));

  printf("exit code mount was:%d\n",exit_code);

  if ( exit_code == 0 )
   {
     lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "USB unmount OK  ") ;
     lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, "press OK to sel.") ;
     ctrl_led(L_USB,0);
     strcpy(current_dir,PIC_DIR);
     mounted = 0;
   }
  else { lcdPosition (lcdHandle, 0, 0) ; lcdPrintf (lcdHandle, "%-32s",str) ; }
  }
  else //do the mount
  {
  printf("try to mount USB\n");

  sprintf(cmd,"/bin/mount %s %s -o ro",USB_DEVICE,USB_MOUNT_DIR);
  fp = popen(cmd, "r");

   while ( fgets(str, sizeof(str)-1, fp) != NULL)
   {
     printf("%s\n",str);
   }
   exit_code = WEXITSTATUS(pclose(fp));

  printf("exit code mount was:%d\n",exit_code);

  if ( exit_code ==0 )
   {
     lcdPosition (lcdHandle, 0, 0) ; lcdPuts (lcdHandle, "USB dev mount OK") ;
     lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, "press OK to sel.") ;
     ctrl_led(L_USB,1);
     strcpy(current_dir,USB_MOUNT_DIR);
     mounted = 1;
   }
  else { lcdPosition (lcdHandle, 0, 0) ; lcdPrintf (lcdHandle, "%-32s",str) ; }
   }

  return exit_code;

}

int main (int argc, char *argv[])
{

 int switch_pressed;
 unsigned char switches[] = { S_LEFT,S_UP,S_ENTER,S_DOWN,S_RIGHT,S_USB,S_DETECT,S_PRG};

  wiringPiSetup () ;

//init switches
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

//init LEDs
pinMode ( L_READY, OUTPUT);
pinMode ( L_WAITING, OUTPUT);
pinMode ( L_BUSY, OUTPUT);
pinMode ( L_ERROR, OUTPUT);
pinMode ( L_USB, OUTPUT);
ctrl_leds(0);
ctrl_led(L_USB,0);



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
  sleep(1); ctrl_leds(1); ctrl_led(L_USB,1);
  sleep(1); ctrl_leds(0); ctrl_led(L_USB,0);
/*
  sleep(1); ctrl_leds(1);
  sleep(1); ctrl_leds(0);
  sleep(1); ctrl_leds(1);
  sleep(1); ctrl_leds(0);
*/
  lcdPosition (lcdHandle, 0, 1) ; lcdPuts (lcdHandle, "press OK to sel.") ;
  digitalWrite ( L_READY, 1);

  //state machine
  //state = 0;

  //init vars
  strcpy(current_dir,PIC_DIR);

  //loop for checking all switches
  while(1) {

    if ( ( switch_pressed = check_switches(switches, 8)) >= 0)
    {
    switch(switch_pressed) {
      case S_DETECT:
        set_led ( L_BUSY);
	if ( do_autodetect() == 0) set_led ( L_READY); else set_led ( L_ERROR);
	break;
      case S_ENTER:
           show_selection(0);
	break;
      case S_DOWN:
           show_selection(1);
	break;
      case S_UP:
           show_selection(-1);
	break;
      case S_PRG:
        set_led ( L_BUSY);
	if ( do_prg() == 0) set_led ( L_READY); else set_led ( L_ERROR);
	break;
      case S_USB:
        set_led ( L_BUSY);
	if ( do_usb() == 0) set_led ( L_READY); else set_led ( L_ERROR);
	break;
      default:
          printf("switch %d pressed\n",switch_pressed);
	break;
     }
  }//if switch pressed

  };
}

