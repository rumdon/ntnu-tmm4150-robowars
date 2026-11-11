#include <shared.h>
#include <ELRS.h>
#include <ble.h>
#include <Adafruit_MCP3008.h>
// #include <Servo.h>
#include <ESP32Servo.h>

int activePWMChannels = 0;
//
//  FUNCTIONS 
//

float readChannelRangeMapped(int channel, int expectedMin, int expectedMiddle, int expectedMax){ //@Brief maps the channel range to a 0.0-1.0 range, middle=-1 for no snapping to centre
  float channelRaw = crsf.getChannel(channel);
  if(channelRaw <= expectedMin + 10){
    return 0.0;
  }else if(expectedMiddle != -1 && channelRaw >= expectedMiddle - 10 && channelRaw <= expectedMiddle + 10 ){
    return 0.5;
  }else if(channelRaw >= expectedMax - 10){
    return 1.0;
  }else{
    return ((float)channelRaw - (float)expectedMin)/((float)expectedMax - (float)expectedMin);
  }
}
int average(int *array, int size){
  int sum = 0;
  for(int i = 0; i < size;i++){ 
    sum += array[i];
  }
  return (int)(sum/size);
};



//
//  CLASSES
//

class powerConsumer {
  public:
    virtual boolean isRunning(){return false;};
};

class motor : public powerConsumer{
  private:
    int8_t motorType;
    int pin[3];
    boolean isActive;
    Servo *servo;
    ulong lastServoChangeTime;
  public:
    motor(int8_t type_of_motor, int pin1, int pin2 = -1, int pin3 = -1)
      : motorType(type_of_motor), pin{pin1,pin2,pin3}, isActive(false)
    {    
      if(motorType == DC_M){
        // pinMode(pin[0],OUTPUT);
        // pinMode(pin[1],OUTPUT);
        ledcAttachChannel(pin[0],20000,8, activePWMChannels++);
        ledcAttachChannel(pin[1],20000,8,activePWMChannels++);
      }
      if(motorType == SERVO_M){
        // pinMode(pin[0],OUTPUT);
        servo = new Servo();
        // servo->setPeriodHertz(50);
        // servo->attach(pin[0]);
        ledcAttachChannel(pin[0], 50, 14, activePWMChannels++);
      }
    }
    
    /*
   @brief Range: 
   DC:      [-255, 255],  +forward, -backwards.
   SERVO:   [   0, 180],  degrees
    */ 
    boolean driveMotor(int speed){
      if(motorType==DC_M){
        if(speed>0){
          isActive = true;
          ledcWrite(pin[0],0);
          ledcWrite(pin[1],min(speed,255));
        }else if(speed<0){
          isActive = true;
          ledcWrite(pin[0],min(-speed,255));
          ledcWrite(pin[1],0);
        }else{
          isActive = false;
          ledcWrite(pin[0],0);
          ledcWrite(pin[1],0);
          return false;
        }
        return true;
      }else if(motorType==SERVO_M){
        if(pin[2] != speed && abs(pin[2]-speed) >3){ // using undef pin[2] as servoLastAngle, speed is new angle
          
          // servo->write(min(speed,180));
          pin[2] = speed;
          speed = constrain(speed, 0, 180);
          uint32_t us = map(speed, 0, 180, 500, 2500);   // pulse width
          ledcWrite(pin[0], us * 16383UL / 20000UL);          // 20000 us period


          lastServoChangeTime == millis();
          isActive = true;
          return true;
        }
        if(millis() - lastServoChangeTime > 1000){ // Assumes a servo can finish turning after 1s, might need to be updated
          isActive = false;
        }
        return false;
      }
      return false;
    }

    boolean isRunning() override{
      return isActive;
    }
};

class powerManager{
  private:
    int bufSize;
    int batteryVoltageCh=0;
    Adafruit_MCP3008 adc;
    int *adcBatteryBuffer;
    int adcReadInterval;
    int adcBatteryBufferPointer=0;
    unsigned long adcLastRead = 0;
    
    powerConsumer *powerConsumers[16];
    int powerConsumersCount = 0;
    unsigned long powerConsumersLastActive = 0;

    int batteryReadConsts[5] = {-1};

    void measureBattery(){
      if(millis()-powerConsumersLastActive>250){
        adcBatteryBuffer[adcBatteryBufferPointer++%bufSize] = adc.readADC(batteryVoltageCh);
        // adcBatteryBuffer[adcBatteryBufferPointer++%bufSize] = analogRead(5);
      }
    }
    void checkPowerConsumersActive(){
      boolean isActive = false;
      for (int i = 0; i < powerConsumersCount; i++)
      {
        isActive = isActive || powerConsumers[i]->isRunning();
      }
      if(isActive){
        powerConsumersLastActive = millis();
      }
    }
    float getBatteryVoltageOffset(){
      if(batteryReadConsts[0] == -1) return 0;
      float read = (float) analogRead(batteryReadConsts[0]);
      read = min((float)batteryReadConsts[2], read); // make sure it is capped by expected read max
      read = (max((float)batteryReadConsts[1], read) - float(batteryReadConsts[1])) /((float)batteryReadConsts[2] - (float)batteryReadConsts[1]); // map it to a 0.0 to 1.0 range

      read = read*((float)batteryReadConsts[4] - (float)batteryReadConsts[3])+(float)batteryReadConsts[3];

      return read;
    }

  public:
    powerManager(int size_of_buffers)
    : bufSize(size_of_buffers){
      adcBatteryBuffer = (int*) malloc(bufSize*sizeof(int));
      if (adcBatteryBuffer == NULL) {
        Serial.println("Memory allocation failed!");
        while(1); // Halt execution
      }
    }

    boolean initADC(int SCLK, int D_IN, int D_OUT, int CS, int read_interval){
      boolean success = adc.begin(SCLK, D_IN, D_OUT, CS);
      adcReadInterval = read_interval;
      memset(adcBatteryBuffer,adc.readADC(batteryVoltageCh),sizeof(adcBatteryBuffer));
      return success;
    }
    void setBatteryCh(int ch){
      batteryVoltageCh = ch; 
    }
    void setBatteryOffset(int pin, int readMin, int readMax, int mapMin, int mapMax){
      pinMode(pin, INPUT);
      
      batteryReadConsts[0] = pin;
      batteryReadConsts[1] = readMin;
      batteryReadConsts[2] = readMax;
      batteryReadConsts[3] = mapMin;
      batteryReadConsts[4] = mapMax;
    }
    int readADCChannel(int ch){
      return adc.readADC(ch);
    }

    float getBatteryVoltage(){
      return 0.023214285714 * (float)(average(adcBatteryBuffer, bufSize)) + 0.38;
      // return 0.023214285714 * (float)(average(adcBatteryBuffer, bufSize)) + getBatteryVoltageOffset();
      // return (float)(average(adcBatteryBuffer, bufSize));
    }

    int getLastRawBatteryRead(){
      return adc.readADC(batteryVoltageCh);
      if(adcBatteryBufferPointer > 0 ){
        return adcBatteryBuffer[(adcBatteryBufferPointer-1)%bufSize];
      }else{
        return -1;
      }
    }

    void task(){
      int now = millis();
      if(adcLastRead + adcReadInterval < now){
        adcLastRead = now;
        measureBattery();
      }
      checkPowerConsumersActive();
    }
    void addPowerConsumer(powerConsumer *consumer){
      powerConsumers[powerConsumersCount++] = consumer;
    }
};


//
//  PROGRAM START
//

powerManager pm(50);

motor rMotor(DC_M, 15,23);
motor lMotor(DC_M, 3,2);
motor sMotor(SERVO_M, 1);    



void setup() {
  Serial.begin(115200);
  setupELRS();
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  pm.addPowerConsumer(&lMotor);
  pm.addPowerConsumer(&rMotor);
  pm.addPowerConsumer(&sMotor);
  pm.initADC(11, 10, 20, 8,50);
  pinMode(5, INPUT);
  pm.setBatteryOffset(0,400,3600,-2,2);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  setupBLE();
}

void loop() {
  bleTask();
  ELRSTask();
  pm.task();
  
  // if(INTERVAL(250)){
  //   Serial.println(analogRead(7));
  // }

  if(INTERVAL(50) && crsf.isLinkUp() && millis()>3000){
    /*
      1: RStick   Hori      |      5: LE 	on/off        
      2: RStick   Vert      |      6: SA
      3: LStick   VERT      |      7: SB
      4: LStick   Hori      |      8: SC
    */
    int right_speed, left_speed, servo_angle;
    
    // ::::::: DC CONTROL :::::::
    right_speed = left_speed = (255.0*(readChannelRangeMapped(2,992,1500,2011)-0.5)*2.0);
    right_speed += (255.0*(readChannelRangeMapped(1,992,1500,2011)-0.5)*2.0);
    left_speed -= (255.0*(readChannelRangeMapped(1,992,1500,2011)-0.5)*2.0);
    if(INTERVAL(125)){
      sendLeftBelt(left_speed);
      sendRightBelt(right_speed);
    }
    lMotor.driveMotor(left_speed);
    rMotor.driveMotor(right_speed);

    // ::::::: SERVO CONTROL :::::::
    servo_angle = (180.0*(readChannelRangeMapped(3,992,-1,2011)));
    servo_angle =  servo_angle - servo_angle%3;
    if(sMotor.driveMotor(servo_angle)){
      sendServo(servo_angle);
    };
  }
  if(INTERVAL(500)){
    sendCharge(pm.getBatteryVoltage()*1000);
    Serial.println(pm.getBatteryVoltage());
    // Serial.println(pm.getLastRawBatteryRead());
    // Serial.print("\n");
    // Serial.println("-----------------------");

    // for (size_t i = 0; i < 8; i++)
    // {
    //   Serial.print(pm.readADCChannel(i));
    //   Serial.print(" ");
    // }
    // Serial.println(analogRead(5));
    // Serial.println("-----------------------");
  }
}


