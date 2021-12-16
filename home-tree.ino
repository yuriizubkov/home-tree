#include "HomeSpan.h"
#include "Device_Specific.h"

void setup() {
  Serial.begin(115200);
  homeSpan.enableOTA(); // OTA updates with default password "homespan-ota"
  //homeSpan.setLogLevel(1);
  homeSpan.begin(Category::Lighting, "The Tree");

  new SpanAccessory();
    new DEV_Identify("The Tree", "lensflare.dev", "Tree-001", "WS2812b LED Tree", "0.3.3", 5);
    new Service::HAPProtocolInformation();
      new Characteristic::Version("1.1.0");

    (new DEV_RgbLamp())->setPrimary();
      new Characteristic::Name("Tree's Light");

    // FX Buttons
    DEV_LightFX **switches; // Pointer to a pointer of type DEV_LightFX
    switches = new DEV_LightFX *[9];  // Declares array of pointers

    switches[0] = new DEV_LightFX(FX_MODE_RAINBOW_CYCLE, 8000, switches);
      new Characteristic::Name("Rainbow");

    switches[1] = new DEV_LightFX(FX_MODE_TWINKLEFOX, 1500, switches);
      new Characteristic::Name("TwinkleFOX");

    switches[2] = new DEV_LightFX(FX_MODE_FIREWORKS, 3000, switches);
      new Characteristic::Name("Fireworks");

    switches[3] = new DEV_LightFX(FX_MODE_FIREWORKS_RANDOM, 3000, switches);
      new Characteristic::Name("Fireworks Color");

    switches[4] = new DEV_LightFX(FX_MODE_RAIN, 1000, switches);
      new Characteristic::Name("Rain");

    switches[5] = new DEV_LightFX(FX_MODE_CHASE_RAINBOW, 5000, switches);
      new Characteristic::Name("Chase Rainbow");

    switches[6] = new DEV_LightFX(FX_MODE_MERRY_CHRISTMAS, 7000, switches);
      new Characteristic::Name("Merry Christmas");

    switches[7] = new DEV_LightFX(FX_MODE_HALLOWEEN, 7000, switches);
      new Characteristic::Name("Halloween");

    switches[8] = new DEV_LightFX(FX_MODE_COLOR_SWEEP_RANDOM, 3000, switches);
      new Characteristic::Name("Sweep Random");

  setupWS2812FX();
}

void loop() {
  homeSpan.poll();
  ws2812fx.service();
}
