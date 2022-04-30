#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#define MCP4725_ADDR 0x61
class Sequencer
{
  public:
  
  uint16_t voct[16]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  bool gates[16]  = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
  unsigned long timeCounter = 0;
  uint8_t stepCounter = 0;
  uint8_t bpm = 125;
  
  Sequencer()
  {
    for(int i=0;i<16;i++)
    {
      voct[i] = 100.0+random(-50,50);
      gates[i] = true;
    }
  }

  void progress(unsigned long dt)
  {
    timeCounter+=dt;
    if(timeCounter>(1.0/(bpm/60000.0)))
    {
      stepCounter++;
      timeCounter = 0;
      if(stepCounter>16)
        stepCounter = 0;
    }

  }

  uint8_t currentStep()
  {
    return stepCounter;
  }
  
  uint16_t currentVoct()
  {
    return voct[stepCounter];
  }
  
  uint8_t currentStepTime()
  {
    return timeCounter;
  }
  
  bool currentGate()
  {
    return gates[stepCounter];
  }

  void setBpm(uint8_t newBpm)
  {
    bpm = newBpm;
  }
  
  void toggleStepGate(uint8_t idx)
  {
    gates[stepCounter] = gates[stepCounter] == true ? false : true;
  }

  void setMode()
  {
    
  }
  
};

Sequencer seq;
unsigned long lastTime = millis();

void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();
}

void setup()
{
  Serial.begin(9600);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  testdrawline();      // Draw many lines

}

void writeToDac(int val)
{
  Wire.beginTransmission(MCP4725_ADDR);
  Wire.write(64);                     // cmd to update the DAC
  Wire.write(val >> 4);        // the 8 most significant bits...
  Wire.write((val & 15) << 4); // the 4 least significant bits...
  Wire.endTransmission();
}

void loop()
{
  unsigned long dt = millis()-lastTime;
  lastTime = millis();
  seq.progress(dt);

  if(seq.currentGate())
  {
    writeToDac(seq.currentVoct());
  } 
  else
  {
    writeToDac(0);
  }
}
