class Sequencer
{
  public:
  
  uint16_t voct[16]  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint16_t gateLen[16]  = {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};

  bool gates[16]  = {false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false};
  unsigned long timeCounter = 0;
  uint8_t stepCounter = 0;
  uint16_t bpm = 125;
  uint8_t stepMode = 0;
  uint8_t editMode = 0;

  Sequencer()
  {
    for(int i=0;i<16;i++)
    {
      voct[i] = 100.0+random(-50,50);
      gates[i] = random(0,100)<70;
    }
  }

  void switchStepMode() //Single, dual, pong
  {
    stepMode++;
    if(stepMode>2)
      stepMode = 0;
  }

  void switchEditMode()
  {
    editMode++;
    if(editMode>1)
      editMode = 0;
  }

  bool progress(unsigned long dt)
  {
    timeCounter+=dt;
    
    Serial.println(timeCounter);
    if(timeCounter>=(1.0/(bpm/60000.0)))
    {
      stepCounter++;
      timeCounter = 0;
      if(stepCounter>16)
        stepCounter = 0;
      return true;
    }
    else
      return false;
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
    gates[idx] = (gates[idx] == true ? false : true);
  }

  void setMode()
  {
    
  }
  
};
