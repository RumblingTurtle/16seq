#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/Picopixel.h>

#define EB_FAST 100
#define EB_DEB 10   
#include <EncButton.h>

#include <MedianFilterLib2.h>
#include "Sequencer.h"
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define WINDOW_SIZE 8
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define MCP4725_ADDR 0x61
#define STEP_START_X 23
#define STEP_START_Y 24
#define FAST_TURN_SPEED 10
#define HOLD_TIME_MS 500

int buttonVals[] = {699, 709, 720, 731, 749, 761, 774, 787, 807, 821, 836, 852, 876, 893, 910, 929};
EncButton<EB_CALLBACK, 2,3,4> enc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Sequencer seq;
uint8_t selectedMenu = 0;
unsigned long lastTime = millis();

unsigned long holdTime = 0;
int lastButton = -1;
int currentButton = -1;
bool buttonHold = false;

bool updateScreen = false;
MedianFilter2<int> filter(WINDOW_SIZE);

int getButton()
{ 
  int val = filter.AddValue(analogRead(A2));
  if(val<500)
    return -1;
  for(int i=0;i<16;i++)
    if(val<buttonVals[i]+5)
      return i;
}

void handleButtons(unsigned long dt)
{
  currentButton = getButton();
  if(currentButton == lastButton)
  {
    if(currentButton!=-1)
    {
      holdTime+=dt;
      if(holdTime>HOLD_TIME_MS)
      {
        updateScreen = true;
        buttonHold = true;
      }
    }
  }
  else
  {
    if(currentButton==-1)
    { 
      holdTime=0;
      if(buttonHold)
      {
        buttonHold = false;
      } 
      else
      {
        seq.toggleStepGate(lastButton);
      }
      
      updateScreen = true;
    }
  }
  lastButton = currentButton;
}

void setup()
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }
  pinMode(A2,INPUT);
  Serial.begin(9600);
  display.setFont(&Picopixel);
  enc.attach(TURN_HANDLER, encTurnCallback);
  enc.attach(CLICK_HANDLER, encClickCallback);
}

void writeToDac(int val)
{
  Wire.beginTransmission(MCP4725_ADDR);
  Wire.write(64);               
  Wire.write(val >> 4);        
  Wire.write((val & 15) << 4); 
  Wire.endTransmission();
}

void drawUI()
{
  //Drawing steps
  uint8_t cStep = seq.currentStep();
  for(int i = 0; i<4; i++)
  {
    for(int j = 0; j<4; j++)
    {
      uint8_t idx = i*4+j;
      display.drawRect(STEP_START_X-1+9*j, STEP_START_Y-1+9*i, 8, 8, WHITE);

      if(seq.gates[idx])
        display.fillRect(STEP_START_X+1+9*j, STEP_START_Y+1+9*i, 4, 4, WHITE);

      if(idx==cStep)
        display.fillRect(STEP_START_X+2+9*j, STEP_START_Y+2+9*i, 2, 2, seq.gates[idx] ? BLACK : WHITE);
    }
  }

  //Drawing menu
  display.fillRect(69,19+10*selectedMenu,58,7,WHITE);

  display.setTextSize(1);

  display.setTextColor(WHITE);
  if(selectedMenu==0)
    display.setTextColor(BLACK);

  display.setCursor(70,24);
  display.print("BPM: ");
  display.print(seq.bpm);

  display.setTextColor(WHITE);
  if(selectedMenu==1)
    display.setTextColor(BLACK);

  display.setCursor(70,34);
  display.print("Mode: ");
  switch (seq.stepMode)
  {
  case 0:
    display.print("Single");
    break;
  
  case 1:
    display.print("Dual");
    break;
  case 2:
    display.print("Pong");
    break;
  default:
    break;
  }

  display.setTextColor(WHITE);
  if(selectedMenu==2)
    display.setTextColor(BLACK);
  display.setCursor(70,44);
  display.print("Edit: ");
  switch (seq.editMode)
  {
  case 0:
    display.print("Voct");
    break;
  
  case 1:
    display.print("Gate len");
    break;
  default:
    break;
  }

  display.setTextColor(WHITE);
  display.setCursor(70,54);
  display.print("Val: ");
  if(buttonHold)
  {
    if(seq.editMode == 0)
      display.print(seq.voct[currentButton]);
    else
      display.print(seq.gateLen[currentButton]);
  }

}

void encTurnCallback()
{
  if(buttonHold)
  {
    updateScreen = true;
    int increment;
    if(enc.fast())
      increment = FAST_TURN_SPEED*enc.dir();
    else
      increment = enc.dir();

    if(seq.editMode==0)
      seq.voct[currentButton]+=increment;
    else
      seq.gateLen[currentButton]+=increment;
  }
  else
  {
    
    updateScreen = true;
    if(selectedMenu==0) //BPM
    {
      if(enc.fast())
        seq.bpm += FAST_TURN_SPEED*enc.dir();
      else
        seq.bpm += enc.dir();
      if(seq.bpm<1)
        seq.bpm  = 1;
      if(seq.bpm>300)
        seq.bpm = 300;
    }
    
    if(selectedMenu==1) //Mode
      seq.switchStepMode();

    if(selectedMenu==2) //Edit
      seq.switchEditMode();
  }
}

void encClickCallback()
{
  updateScreen = true;

  selectedMenu++;
  if(selectedMenu>2)
  {
    selectedMenu=0;
  }
}

void loop()
{
  enc.tick();
  unsigned long dt = millis()-lastTime;
  lastTime = millis();
  
  updateScreen = updateScreen || seq.progress(dt);
  handleButtons(dt);
  
  if(seq.currentGate())
    writeToDac(seq.currentVoct());
  else
    writeToDac(0);
  
  if(updateScreen)
  {
    display.clearDisplay();
    drawUI();
    display.display();
    updateScreen = false;
  }
}
