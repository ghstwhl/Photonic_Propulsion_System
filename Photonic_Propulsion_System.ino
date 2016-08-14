/*****************************************************************************
 * Photonic Propulsion System
 *
 * With this simple project you can make your vehicle float above the desert 
 * in the dark of night! OK... you can make it appear as if your vehicle 
 * floats above the desert in the dark of night. Yeah, it's just a super 
 * fancy programmable ground effects lighting kit. I am sorry if you feel 
 * misled, but I really wanted to win the Make It Glow contest and "Stick 
 * some LEDs under your car" wasn't going to go viral.
 * 
 * To be fair, this isn't your typical auto-parts store lighting kit. This 
 * isn't a couple of tubes of low-voltage neon, or a string of single-color
 * LEDs. This project gives you dynamic lighting, and the power to create 
 * your own animation sequences. It has multiple knobs and switches for 
 * altering the behavior on the fly, and it has strips of LEDs on your 
 * console to let you preview the action under your car without having to 
 * jump out and look every time you make a change.
 * 
 * The parts list for this project is included in the following Instructable:
 * 
 * 
 * 
 * This code requires the following libraries:
 *   - Arduino Total Control Lighting Library
 *     https://bitbucket.org/devries/arduino-tcl
 *   - Adafruit NeoPixel library
 *     https://github.com/adafruit/Adafruit_NeoPixel
 * 
 * 
 *
 * FINAL hardware tweak:  Wire up a CdS photocell to A5, with a 10k pulldown 
 * to ground for auto-dimming of the preview display
 * 
 *
 *
 * Mode selection:
 * 
 *  Switched modes:
 *
 *   Rainbling
 *    * -  (d6 switch on)
 *    * -  (d7 switch on)
 *
 *   Fire
 *    - *  (d6 switch off)
 *    - *  (d7 switch off)
 *
 *   Color Picker
 *    * -  (d6 switch on)
 *    - *  (d7 switch off)
 *
 *   Cylon Eue
 *    - *  (d7 switch off)
 *    * -  (d6 switch on)
 * 
 * 
 *  Momentary modes: 
 * 
 *   DO NOT ACTIVATE, UNDER PENALTY OF LAW
 *    Seriously, if you push the two buttons at the same time, it's on you.
 * 
 * 
 * Copyright 2016 Chris O'Halloran - cknight __ ghostwheel _ com
 * Copyright Chris O'Halloran
 * License: Attribution Non-commercial Share Alike (by-nc-sa) (CC BY-NC-SA 4.0)
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
****************************************************************************/


//Uncomment the below line to enable serial debugging and memory testing.
//#define DEBUG 1


//Uncomment the below lines if you have added a WS2811/WS2812 preview strip
//#define PREVIEW 1
//#define PREVIEWLEDS 10
//#define PREVIEWPIN 8
//#define PREVIEWCDS 5


//Uncomment this line ONLY if having flashing red and blue lights on your vehicle would not be against the law.
//Really, just uncommenting this line could be illegal.  Do you really want to risk it?
//#define LEOMODE 1


#include <SPI.h>
#include <TCL.h>

#ifdef PREVIEW
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel preview = Adafruit_NeoPixel(PREVIEWLEDS, PREVIEWPIN, NEO_GRB + NEO_KHZ800);
#endif


#define R 0
#define G 1
#define B 2


// How many pixels in our strand.
#define PIXELS 50

// Custom defines and globals just for rainbling mode...
#define HINTERVAL_MAX 10.0
#define V_MAX 0.99
#define SAT_MAX 1.0
#define flash_prob_max 20480
float hval = 0.0;

// Define the min and max delay between iterations
#define DELAYLOW 10
#define DELAYHIGH 150


// This is for new code that tries to determine whether the TCL Developer Shield is installed, or the Simple Shield
// Assume simple shield, unless proven otherwise
int DevSheildInstalled = 0;
int MOMENTARY1_Initial_State;   // Initial state of switch, required as there was inconsistant placement at the factory
int MOMENTARY2_Initial_State;   // Initial state of switch, required as there was inconsistant placement at the factory
int TCL_SWITCH1_Initial_State;  // Initial state of switch, required as there was inconsistant placement at the factory
int TCL_SWITCH2_Initial_State;  // Initial state of switch, required as there was inconsistant placement at the factory


const uint8_t PROGMEM gamma_table[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

byte pixel_color[PIXELS][3];


int CheckSwitches();

void setup() {

#ifdef DEBUG
  Serial.begin(9600); // Start serial communication at 9600 baud
#endif
  
  TCL.begin();
  TCL.setupDeveloperShield();

#ifdef PREVIEW
    preview.begin(); // This initializes the NeoPixel library.
#endif
  
  DevBoardDetect() ;
  zap_black();
}

void loop() {
  zap_black();

  switch (CheckSwitches()) {
    case 3:
      FireStrand();
      break;
    case 2:
      cylon_eye();
      break;
    case 1:
      color_picker();
      break;
    default:  // AKA SWITCHSTATE==0
      rainbling();
      break;
    }

}


void FireStrand() {
  int i;
  int red;
  int green;
  float intensity;
  float chromatography;
  int delaytime;
  int strandlength;

  while ( 3 == CheckSwitches() ) {
    intensity=(float)map(analogRead(TCL_POT4), 0, 1023, 0, 100)/100;
    chromatography=(float)map(analogRead(TCL_POT3), 0, 1023, 0, 50)/100;
    delaytime=(int)map(analogRead(TCL_POT1), 0, 1023, 150, 0);

    for(i=0;i<PIXELS;i++) {
      red=(int)(random(0,256) * intensity);
      green=(int)(random(0,(red * chromatography +1)) * intensity);
      pixel_color[i][R]= red;
      pixel_color[i][G]= green;
      pixel_color[i][B]=0;
    }
    update_strand_no_gamma(); 
    delay(delaytime);
  }
}




void color_picker() {
  int i; // A variable for looping

  while ( 1 == CheckSwitches() ) {
    for(i=0;i< PIXELS;i++) {
      pixel_color[i][R]=(int)((float)map(analogRead(TCL_POT1),0,1020,0,255) * ( (float)map(analogRead(TCL_POT4),0,1020,0,255) / 255 ));
      pixel_color[i][G]=(int)((float)map(analogRead(TCL_POT2),0,1020,0,255) * ( (float)map(analogRead(TCL_POT4),0,1020,0,255) / 255 ));
      pixel_color[i][B]=(int)((float)map(analogRead(TCL_POT3),0,1020,0,255) * ( (float)map(analogRead(TCL_POT4),0,1020,0,255) / 255 ));
#ifdef DEBUG
      Serial.println( ( (float)map(analogRead(TCL_POT4),0,1020,0,255) / 255 )  );
      Serial.print( map(analogRead(TCL_POT1),0,1020,0,255) );
      Serial.print( "   " );
      Serial.print(    (int)((float)map(analogRead(TCL_POT1),0,1020,0,255) * ( (float)map(analogRead(TCL_POT4),0,1020,0,255) / 255 ))        );
      Serial.println( "" );
#endif

    }
    update_strand(); // Send all the pixels out
    
  }
}



void rainbling() {
/**************************************************************
* An optimized version of rainbling that shaved 654 bytes off 
* the program and 185 bytes off global storage.
* Requires these to be set globally at the top:
* 
* #include <SPI.h>
* #include <TCL.h>
* 
* #define R 0
* #define G 1
* #define B 2
* 
* // How many pixels in our strand.
* #define PIXELS 50
* 
* // Custom defines and globals just for rainbling mode...
* #define HINTERVAL_MAX 10.0
* #define V_MAX 0.99
* #define SAT_MAX 1.0
* #define flash_prob_max 20480
* float hval = 0.0;
***************************************************************/
  int i;
  float local_h;
  int speed_pot;
  int brightness_pot;
  int saturation_pot;
  int flash_pot;
  float sat;
  float v;
  float totem_interval = 360.0/PIXELS;

  while ( 0 == CheckSwitches()) {

    speed_pot = analogRead(TCL_POT1);
    brightness_pot = analogRead(TCL_POT2);
    saturation_pot = analogRead(TCL_POT3);
    flash_pot = analogRead(TCL_POT4);
    sat = SAT_MAX/1023.0*saturation_pot;
    v = V_MAX/1023.0*brightness_pot;

    for(i=0;i<PIXELS;i++) {
      local_h = hval+i*totem_interval;
      while(local_h>=360.0) {
        local_h-=360.0;
      }

      if(random(flash_prob_max)<flash_pot) {
        pixel_color[i][R]=255;
        pixel_color[i][G]=255;
        pixel_color[i][B]=255;
      }
      else {
        HSVtoRGB(local_h,sat,v,&pixel_color[i][R],&pixel_color[i][G],&pixel_color[i][B]);
      }
    }
  
    update_strand_reverse();
    delay(25);
    hval+=(float)(HINTERVAL_MAX/1023.0*speed_pot);
    while(hval>=360.0) {
      hval-=360.0;
    }
  }  
}


void zap_black() {
  int i;
  for(i=0;i<PIXELS;i++) {
    pixel_color[i][R]=0x00;
    pixel_color[i][G]=0x00;
    pixel_color[i][B]=0x00;
  }
  update_strand();
}


void update_strand_no_gamma() {
  int i;
  
  TCL.sendEmptyFrame();
  for(i=0;i<PIXELS;i++) {
    TCL.sendColor(pixel_color[i][R],pixel_color[i][G],pixel_color[i][B]);
#ifdef DEBUG
      Serial.print( "I=" );
      Serial.print( i );
      Serial.print( ", R=" );
      Serial.print( pixel_color[i][R] );
      Serial.print( ", G=" );
      Serial.print( pixel_color[i][G] );
      Serial.print( ", B=" );
      Serial.print( pixel_color[i][B] );
      Serial.println( "" );
#endif

  }
  TCL.sendEmptyFrame();

#ifdef PREVIEW
  int j = (PIXELS / PREVIEWLEDS);
  int l,m;
  float brightness = (float)map(analogRead(5), 1020, 0, 25, 100) / 100;

  for ( l = 0; l < PREVIEWLEDS ; l++) {
    int r_tmp = 0;
    int g_tmp = 0;
    int b_tmp = 0;
    for ( m = 0; m < j; m++) {
      r_tmp += pixel_color[((l*j) + m)][R];
      g_tmp += pixel_color[((l*j) + m)][G];
      b_tmp += pixel_color[((l*j) + m)][B];
    }
    r_tmp = (int)((float)r_tmp / (float)j * brightness);
    g_tmp = (int)((float)g_tmp / (float)j * brightness);
    b_tmp = (int)((float)b_tmp / (float)j * brightness);
    preview.setPixelColor(l, preview.Color(r_tmp,g_tmp,b_tmp));
  }
  preview.show();
#endif

  
}


void update_strand() {
  int i;
  
  TCL.sendEmptyFrame();
  for(i=0;i<PIXELS;i++) {
    TCL.sendColor(pgm_read_byte(&gamma_table[pixel_color[i][R]]),pgm_read_byte(&gamma_table[pixel_color[i][G]]),pgm_read_byte(&gamma_table[pixel_color[i][B]]));
#ifdef DEBUG
      Serial.print( "I=" );
      Serial.print( i );
      Serial.print( ", R=" );
      Serial.print( pgm_read_byte(&gamma_table[pixel_color[i][R]]) );
      Serial.print( ", G=" );
      Serial.print( pgm_read_byte(&gamma_table[pixel_color[i][G]]) );
      Serial.print( ", B=" );
      Serial.print( pgm_read_byte(&gamma_table[pixel_color[i][B]]) );
      Serial.println( "" );
#endif

  }
  TCL.sendEmptyFrame();

#ifdef PREVIEW
  int j = (PIXELS / PREVIEWLEDS);
  int l,m;
  float brightness = (float)map(analogRead(5), 1020, 0, 25, 100) / 100;

  for ( l = 0; l < PREVIEWLEDS ; l++) {
    int r_tmp = 0;
    int g_tmp = 0;
    int b_tmp = 0;
    for ( m = 0; m < j; m++) {
      r_tmp += pixel_color[((l*j) + m)][R];
      g_tmp += pixel_color[((l*j) + m)][G];
      b_tmp += pixel_color[((l*j) + m)][B];
    }
    r_tmp = (int)((float)r_tmp / (float)j * brightness);
    g_tmp = (int)((float)g_tmp / (float)j * brightness);
    b_tmp = (int)((float)b_tmp / (float)j * brightness);
    preview.setPixelColor(l, preview.Color(pgm_read_byte(&gamma_table[r_tmp]),pgm_read_byte(&gamma_table[g_tmp]),pgm_read_byte(&gamma_table[b_tmp])));
  }
  preview.show();
#endif

  
}

void update_strand_reverse() {
  int i;
  
  TCL.sendEmptyFrame();
  for(i=(PIXELS-1);i>=0;i--) {
    TCL.sendColor(pgm_read_byte(&gamma_table[pixel_color[i][R]]),pgm_read_byte(&gamma_table[pixel_color[i][G]]),pgm_read_byte(&gamma_table[pixel_color[i][B]]));
  }
  TCL.sendEmptyFrame();

#ifdef PREVIEW
  int j = (PIXELS / PREVIEWLEDS);
  int l,m;
  int x = 0;
  float brightness = (float)map(analogRead(5), 1020, 0, 25, 100) / 100;

  for ( l = (PREVIEWLEDS-1); l >= 0 ; l--) {
    int r_tmp = 0;
    int g_tmp = 0;
    int b_tmp = 0;
    for ( m = 0; m < j; m++) {
      r_tmp += pixel_color[((l*j) + m)][R];
      g_tmp += pixel_color[((l*j) + m)][G];
      b_tmp += pixel_color[((l*j) + m)][B];
    }
    r_tmp = (int)((float)r_tmp / (float)j * brightness);
    g_tmp = (int)((float)g_tmp / (float)j * brightness);
    b_tmp = (int)((float)b_tmp / (float)j * brightness);
    preview.setPixelColor(x, preview.Color(pgm_read_byte(&gamma_table[r_tmp]),pgm_read_byte(&gamma_table[g_tmp]),pgm_read_byte(&gamma_table[b_tmp])));
    x++;
  }
  preview.show();
#endif


}

    
/* Convert hsv values (0<=h<360, 0<=s<=1, 0<=v<=1) to rgb values (0<=r<=255, etc) */
void HSVtoRGB(float h, float s, float v, byte *r, byte *g, byte *b) {
  int i;
  float f, p, q, t;
  float r_f, g_f, b_f;

  if( s < 1.0e-6 ) {
    /* grey */
    r_f = g_f = b_f = v;
  }
  
  else {
    h /= 60.0;              /* Divide into 6 regions (0-5) */
    i = (int)floor( h );
    f = h - (float)i;      /* fractional part of h */
    p = v * ( 1.0 - s );
    q = v * ( 1.0 - s * f );
    t = v * ( 1.0 - s * ( 1.0 - f ) );

    switch( i ) {
      case 0:
        r_f = v;
        g_f = t;
        b_f = p;
        break;
      case 1:
        r_f = q;
        g_f = v;
        b_f = p;
        break;
      case 2:
        r_f = p;
        g_f = v;
        b_f = t;
        break;
      case 3:
        r_f = p;
        g_f = q;
        b_f = v;
        break;
      case 4:
        r_f = t;
        g_f = p;
        b_f = v;
        break;
      default:    // case 5:
        r_f = v;
        g_f = p;
        b_f = q;
        break;
    }
  }
  
  *r = (byte)floor(r_f*255.99);
  *g = (byte)floor(g_f*255.99);
  *b = (byte)floor(b_f*255.99);
}


#ifdef LEOMODE
void DoNotPushThoseButtons() {
  int i;
  int j = 1;
  int k;
  
  
  while ( (digitalRead(TCL_MOMENTARY1) != MOMENTARY1_Initial_State) && (digitalRead(TCL_MOMENTARY2) != MOMENTARY2_Initial_State) ) {
    for ( k=0; k < 5; k++) {
      for ( i=0; i < PIXELS; i++) {
        if  ( i < (PIXELS/2) ) {
            pixel_color[i][R] = 255;
            pixel_color[i][G] = 0;
            pixel_color[i][B] = 0;
        }
        else if ( i > (PIXELS/2) ) {
            pixel_color[i][R] = 0;
            pixel_color[i][G] = 0;
            pixel_color[i][B] = 255;
        }
      }
      update_strand();
      delay(50);     
      zap_black();
      delay(50);
    
    }
    for ( k=0; k < 5; k++) {
      for ( i=0; i < PIXELS; i++) {
        if  ( i < (PIXELS/2) ) {
            pixel_color[i][R] = 0;
            pixel_color[i][G] = 0;
            pixel_color[i][B] = 255;
        }
        else if ( i > (PIXELS/2) ) {
            pixel_color[i][R] = 255;
            pixel_color[i][G] = 0;
            pixel_color[i][B] = 0;
        }
      }
      update_strand();
      delay(50);     
      zap_black();
      delay(50);
      
    }
  }
}
#endif

int CheckSwitches() {

  // This alows Simple Shield Mode to be disabled if a switch change is detected.
  // This helps defend against false positives in the shield detection code.
  if ( 0 == DevSheildInstalled ) {
    if ( (TCL_SWITCH1_Initial_State != digitalRead(TCL_SWITCH1)) || (TCL_SWITCH2_Initial_State != digitalRead(TCL_SWITCH2)) || (digitalRead(TCL_MOMENTARY1) != MOMENTARY1_Initial_State) || (digitalRead(TCL_MOMENTARY2) != MOMENTARY2_Initial_State) ) {
      DevSheildInstalled = 1;
      zap_black();
    }
    else {
      return(0);
    }
  }

#ifdef LEOMODE
  while ( (digitalRead(TCL_MOMENTARY1) != MOMENTARY1_Initial_State) && (digitalRead(TCL_MOMENTARY2) != MOMENTARY2_Initial_State) ) {
     DoNotPushThoseButtons();
  }
#endif

  // Read the two slide sswitches and assign SWITCHSTATE a value based on their positions
  if (digitalRead(TCL_SWITCH1) == 0 && digitalRead(TCL_SWITCH2) == 0){
    return(3);
  }
  else if (digitalRead(TCL_SWITCH1) == 0 && digitalRead(TCL_SWITCH2) == 1){
    return(2);
  }
  else if (digitalRead(TCL_SWITCH1) == 1 && digitalRead(TCL_SWITCH2) == 0){
    return(1);
  }
  else{
    return(0);
  }

}

void DevBoardDetect() {
  int i;
  int DevSheldTestCount = 0;
  int DevSheldTest;
  DevSheldTest = analogRead(TCL_POT1);
  for(i=0; i<10; i++) {
    if ( (analogRead(TCL_POT1) < (DevSheldTest - 1) ) || (analogRead(TCL_POT1) > (DevSheldTest + 1) )    ) {
      DevSheldTestCount++;
    }
  }
  if ( 5 >= DevSheldTestCount ) {
    DevSheildInstalled = 1;
  }

}


void cylon_eye() {
  int i;
  int j; // The lag counter
  int pos;

  while ( 2 == CheckSwitches() ) {

    // Forward color sweep
    for(i=0; i<PIXELS;i++){
      pixel_color[i][R]=map(analogRead(TCL_POT1),0,1020,0,255);
      pixel_color[i][G]=map(analogRead(TCL_POT2),0,1020,0,255);
      pixel_color[i][B]=map(analogRead(TCL_POT3),0,1020,0,255);
      for(j=1;j<=10;j++) {
        pos=i-j;
        if(pos>=0) {
          pixel_color[pos][R] = (int)((float)pixel_color[pos][R] * .75);
          pixel_color[pos][G] = (int)((float)pixel_color[pos][G] * .75);
          pixel_color[pos][B] = (int)((float)pixel_color[pos][B] * .75);
        }
      } 

      // Empty out all trailing LEDs.  This prevents 'orphans' when dynamically shortening the tail length.
      for(pos=i-j; pos>=0;pos--){
        pixel_color[pos][R] = 0;
        pixel_color[pos][G] = 0;
        pixel_color[pos][B] = 0;
      }

      update_strand(); // Send all the pixels out
      delay(map(analogRead(TCL_POT4), 0, 1020, DELAYLOW, DELAYHIGH));

      CheckSwitches();
      if ( 2 != CheckSwitches() ) {
        break;
      }
    }

    CheckSwitches();
    if ( 2 != CheckSwitches() ) {
      break;
    }

    // Reverse color sweep
    for(i=PIXELS-1; i>=0;i--){
      pixel_color[i][R]=map(analogRead(TCL_POT1),0,1020,0,255);
      pixel_color[i][G]=map(analogRead(TCL_POT2),0,1020,0,255);
      pixel_color[i][B]=map(analogRead(TCL_POT3),0,1020,0,255);
      for(j=1;j<=10;j++) {
        pos=i+j;
        if(pos<PIXELS) {
          pixel_color[pos][R] = (int)((float)pixel_color[pos][R] * .75);
          pixel_color[pos][G] = (int)((float)pixel_color[pos][G] * .75);
          pixel_color[pos][B] = (int)((float)pixel_color[pos][B] * .75);
        }
      }

    // Empty out all trailing LEDs.  This prevents 'orphans' when dynamically shortening the tail length.
      for(pos=i+j; pos<PIXELS;pos++){
        pixel_color[pos][R] = 0;
        pixel_color[pos][G] = 0;
        pixel_color[pos][B] = 0;
      }

      update_strand(); // Send all the pixels out
      delay(map(analogRead(TCL_POT4), 0, 1020, DELAYLOW, DELAYHIGH));

      CheckSwitches();
      if ( 2 != CheckSwitches() ) {
        break;
      }
    }
    
  }
}
