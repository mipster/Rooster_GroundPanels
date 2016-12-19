#include "FastLED.h"                                          // FastLED library. Please use the latest development version.
#include <MPR121.h>
#include <Wire.h>

// Fixed definitions cannot change on the fly.
#define LED_DT 13                                             // Data pin to connect to the strip.
#//define LED_CK 11                                             // Clock pin for WS2801 or APA102.
#define COLOR_ORDER GRB                                       // It's GRB for WS2812 and BGR for APA102.
#define LED_TYPE WS2812B                                     // Using APA102, WS2812, WS2801. Don't forget to change LEDS.addLeds.
#define panel_NUM_LEDS 172                                           // Number of LED's.
#define rooster_NUM_LEDS    114
#define PANELS 1        //number of panels
#define FPS 60
int panel[PANELS] = {0};

#define ELECTRODE1 0                                          //Electrode to monitor
#define ELECTRODE2 5                                       //Electrode to monitor
#define ELECTRODE3 9                                       //Electrode to monitor

// Global variables can be changed on the fly.
uint8_t max_bright = 255;                                      // Overall brightness definition. It can be changed on the fly.

struct CRGB leds[panel_NUM_LEDS+rooster_NUM_LEDS];                                   // Initialize our LED array.
//struct CRGB rooster_leds[rooster_NUM_LEDS];

int thresholdOn[PANELS]={20};       //threshold for touching
int thresholdOff[PANELS]={6};         //threshold for releasing

int  rledpos[PANELS][2]={
  {0+panel_NUM_LEDS,38+panel_NUM_LEDS-1},
};

int  pledpos[PANELS][2]={
  {0,172},
};

void setup() {
  delay(1000);                                                // Power-up safety delay.
  MPR121.begin(0x5C);
  MPR121.setInterruptPin(4);
  MPR121.updateTouchData();
  Serial.begin(9600);
  if(!MPR121.begin(0x5C)){ 
    Serial.println("error setting up MPR121");  
    switch(MPR121.getError()){
      case NO_ERROR:
        Serial.println("no error");
        break;  
      case ADDRESS_UNKNOWN:
        Serial.println("incorrect address");
        break;
      case READBACK_FAIL:
        Serial.println("readback failure");
        break;
      case OVERCURRENT_FLAG:
        Serial.println("overcurrent on REXT pin");
        break;      
      case OUT_OF_RANGE:
        Serial.println("electrode out of range");
        break;
      case NOT_INITED:
        Serial.println("not initialised");
        break;
      default:
        Serial.println("unknown error");
        break;      
    }
    while(1);
  }

  //LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);  // Use this for WS2801 or APA102
  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, panel_NUM_LEDS+rooster_NUM_LEDS);  // Use this for WS2812
  
  FastLED.setBrightness(max_bright);
 // set_max_power_in_volts_and_milliamps(5, 500);               // FastLED Power management set at 5V, 500mA.

  // slow down some of the MPR121 baseline filtering to avoid 
  // filtering out slow hand movements
  MPR121.setRegister(NHDF, 0x01); //noise half delta (falling)
  MPR121.setRegister(FDLF, 0x3F); //filter delay limit (falling)
  
} // setup()

void loop () {                                              


 detectTouch();
 if(panel[0] == 0) {cooldown();}
 Serial.println(panel[0]);
FastLED.show();
delay(1000/FPS);
}


void detectTouch(){
  static int last_reading[PANELS] = {0};

int reading[PANELS];       //initialize reading array
MPR121.updateAll();               // update all of the data from the MPR121

for (int i=0;i<PANELS;i++){
  
    // read the difference between the measured baseline and the measured continuous data
  reading[i]=MPR121.getBaselineData(i)-MPR121.getFilteredData(i);
 int pstrt=pledpos[i][0];
 int pstp=pledpos[i][1];
  int rstrt=rledpos[i][0];
 int rstp=rledpos[i][1];

/*   DEBUG INFO
 
 Serial.print("last reading  :");
  Serial.print(last_reading[0]);
  Serial.print("      reading  :");
  Serial.print(reading[0]);
  Serial.print("       baseline  :");
  Serial.print(MPR121.getBaselineData(0));
  Serial.print("     filtered  :");
  Serial.println(MPR121.getFilteredData(0));


  */
      if(reading[i] >= thresholdOn[i] && last_reading[i] < thresholdOn[i]) {       //turn on leds when electrode is touched
        fill_gradient_RGB(leds, pstrt, CRGB::Red,pstp/2, CRGB::Orange);
        fill_gradient_RGB(leds, pstp/2+1, CRGB::Orange,pstp, CRGB::Red);
        //fill_gradient_RGB(leds, rstrt, CHSV(HUE_YELLOW,255,80),rstp, CHSV(HUE_RED,255,80));
         Serial.print("On");
        FastLED.show();
      panel[i] = 1;
    }
      
   if(reading[i] <= thresholdOff[i] && last_reading[i] > thresholdOff[i]) {                   //shut off leds on release
    // fill_gradient_RGB(leds, pstrt, CRGB::Black,pstp, CRGB::Black);
    //   fill_gradient_RGB(leds, rstrt, CRGB::Black,rstp, CRGB::Black);
   //  FastLED.show();
    //  Serial.print(i);
    Serial.println("  Off");
          panel[i] = 0;
    }
     last_reading[i] = reading[i]; 
}
}

void cooldown(){
  for (int i=0;i<panel_NUM_LEDS;i++){
  int fade = map(random8(32),0,32,0,256);
  leds[i].fadeToBlackBy( fade);   //cool LEDS down by 64/256
  }
}


