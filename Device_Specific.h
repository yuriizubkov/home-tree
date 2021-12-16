//////////////////////////////////
//   DEVICE-SPECIFIC SERVICES   //
//////////////////////////////////

#include "extras/PwmPin.h"
#include "WS2812FX.h"

#define LED_COUNT 49
#define LED_PIN 5 // GPIO-5

uint8_t brightness = 1;

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setupWS2812FX()  {
  ws2812fx.init();
  ws2812fx.setBrightness(brightness);
  ws2812fx.setColor(255,255,255);
  ws2812fx.setMode(FX_MODE_STATIC);
}

void setBrightness(uint8_t bright) {
  brightness = bright;
  ws2812fx.setBrightness(brightness);
}

struct DEV_Identify : Service::AccessoryInformation {

  int nBlinks;                    // number of times to blink built-in LED in identify routine
  SpanCharacteristic *identify;   // reference to the Identify Characteristic
  
  DEV_Identify(const char *name, const char *manu, const char *sn, const char *model, const char *version, int nBlinks) : Service::AccessoryInformation(){
    
    new Characteristic::Name(name);                   // create all the required Characteristics with values set based on above arguments
    new Characteristic::Manufacturer(manu);
    new Characteristic::SerialNumber(sn);    
    new Characteristic::Model(model);
    new Characteristic::FirmwareRevision(version);
    identify = new Characteristic::Identify();          // store a reference to the Identify Characteristic for use below

    this->nBlinks = nBlinks;                            // store the number of times to blink the LED
  }

  boolean update(){

    uint8_t currentBrightness = brightness;
    uint8_t currentMode = ws2812fx.getMode();
    ws2812fx.setMode(FX_MODE_STATIC);
       
    for(int i=0; i<nBlinks; i++){
      ws2812fx.setBrightness(0);
      delay(250);
      ws2812fx.setBrightness(255);
      delay(250);
    }

    ws2812fx.setBrightness(currentBrightness);
    ws2812fx.setMode(currentMode);

    return(true);                               // return true
    
  } // update
  
};

struct DEV_RgbLamp : Service::LightBulb {       // RGB Lamp
  SpanCharacteristic *power;                   // reference to the On Characteristic
  SpanCharacteristic *H;                       // reference to the Hue Characteristic
  SpanCharacteristic *S;                       // reference to the Saturation Characteristic
  SpanCharacteristic *V;                       // reference to the Brightness Characteristic
  
  DEV_RgbLamp() : Service::LightBulb(){       // constructor() method
    power = new Characteristic::On(0);                    
    H = new Characteristic::Hue(0);              // instantiate the Hue Characteristic with an initial value of 0 out of 360
    S = new Characteristic::Saturation(0);       // instantiate the Saturation Characteristic with an initial value of 0%
    V = new Characteristic::Brightness(brightness);     // instantiate the Brightness Characteristic with an initial value of 50%
    V->setRange(1,100,1);                      // sets the range of the Brightness to be from a min of 1%, to a max of 100%, in steps of 1%
  } // end constructor

  boolean update(){                         // update() method

    boolean p;
    float v, h, s, r, g, b;

    h = H->getVal<float>();                      // get and store all current values.  Note the use of the <float> template to properly read the values
    s = S->getVal<float>();
    v = V->getVal<float>();                      // though H and S are defined as FLOAT in HAP, V (which is brightness) is defined as INT, but will be re-cast appropriately
    p = power->getVal();

    if(power->updated()){
      p = (boolean)power->getNewVal();
      if(p) {
        ws2812fx.setBrightness(brightness);
        ws2812fx.start();
      } else {
        ws2812fx.setBrightness(0);
        ws2812fx.stop();
      }
    } 
      
    if(H->updated()){
      h=H->getNewVal<float>();
    } 

    if(S->updated()){
      s=S->getNewVal<float>();
    }

    if(V->updated()){
      v = V->getNewVal<float>();
      setBrightness((uint8_t)((v/100.0) * 255)); // LED brightness is 0..255, HomeKit brightness is 1..100%
    }

    // Here we call a static function of LedPin that converts HSV to RGB.
    // Parameters must all be floats in range of H[0,360], S[0,1], and V[0,1]
    // R, G, B, returned [0,1] range as well

    //LedPin::HSVtoRGB(h,s/100.0,v/100.0,&r,&g,&b);   // since HomeKit provides S and V in percent, scale down by 100
    LedPin::HSVtoRGB(h,s/100.0,1,&r,&g,&b); // don't need to scale power down here
    
    int R, G, B;

    R = p*r*100;                                      // since LedPin uses percent, scale back up by 100, and multiple by status fo power (either 0 or 1)
    G = p*g*100;
    B = p*b*100;

    ws2812fx.setColor(R,G,B);
      
    return(true);                               // return true
  
  } // update
};

struct DEV_LightFX : Service::Switch { 
  SpanCharacteristic *on;
  int fx;
  uint16_t speed;
  DEV_LightFX **switches;

  DEV_LightFX(int fx, uint16_t speed, DEV_LightFX **switches) : Service::Switch() {
    on = new Characteristic::On(0);
    this->fx = fx;
    this->speed = speed;
    this->switches = switches;
  };

  boolean update(){                         // update() method
    if(on->getNewVal()) {
      switchOffOthers();
      ws2812fx.setSpeed(this->speed);
      ws2812fx.setMode(this->fx);
    } else {
      ws2812fx.setMode(FX_MODE_STATIC);
    }
    
    return(true);                               // return true
  } // update

  void switchOffOthers() {
    for(unsigned int a = 0; a < 9; a = a + 1 ){
      if(switches[a] != this && switches[a]->on->getVal()){
        switches[a]->on->setVal(0);
        LOG1("Switching FX OFF: ");
        LOG1(switches[a]->fx);
        LOG1("\n");
      }
    }
  }
};
