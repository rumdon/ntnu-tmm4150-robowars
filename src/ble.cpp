#include <shared.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
const char *serviceUUID = "ef285d93-51b4-4996-b0d1-eade88331410";
const char *chargeCharacteristicUUID = "9e7e0c71-af3e-4e37-8ee1-16692a5a69c5";
const char *turnCharacteristicUUID = "f8757a98-ba9e-49e9-bfc7-7885ee900dce";
const char *leftBeltCharacteristicUUID = "ff7999e2-68d2-49f3-9c56-9b97d91706c9";
const char *rightBeltCharacteristicUUID = "54117d2f-40e8-4ef5-a6e0-aa55bc6f9b04";
const char *servoCharacteristicUUID = "2ae8065a-53b1-4ee7-8427-0db91b7d9458";

BLEServer *pServer;
BLECharacteristic *pCharacteristicCharge;
BLECharacteristic *pCharacteristicLeftBelt;
BLECharacteristic *pCharacteristicRightBelt;
BLECharacteristic *pCharacteristicTurn;
BLECharacteristic *pCharacteristicServo;
BLEAdvertising *pAdvertising;

int prevBleConnectionCount = 0;


void setupBLE(){
  delay(100);
  BLEDevice::init("Vinglepingle");
  
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(serviceUUID);

  pCharacteristicCharge = pService->createCharacteristic(
    chargeCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristicLeftBelt = pService->createCharacteristic(
    leftBeltCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristicRightBelt = pService->createCharacteristic(
    rightBeltCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristicServo = pService->createCharacteristic(
    servoCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
    
  pCharacteristicTurn = pService->createCharacteristic(
    turnCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);

  // pCharacteristicCharge->addDescriptor(new BLE2902());
  // pCharacteristicLeftBelt->addDescriptor(new BLE2902());
  // pCharacteristicRightBelt->addDescriptor(new BLE2902());
  pService->start();
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x12); // For iPhone
  pAdvertising->start();
};

void sendCharge(int value) {
  pCharacteristicCharge->setValue((uint8_t*)&value, sizeof(value));
  pCharacteristicCharge->notify();
}
void sendLeftBelt(int value) {
  pCharacteristicLeftBelt->setValue((uint8_t*)&value, sizeof(value));
  pCharacteristicLeftBelt->notify();
}
void sendRightBelt(int value) {
  pCharacteristicRightBelt->setValue((uint8_t*)&value, sizeof(value));
  pCharacteristicRightBelt->notify();
}
void sendServo(int value){
  pCharacteristicServo->setValue((uint8_t*)&value, sizeof(value));
  pCharacteristicServo->notify();
}

void bleTask(){
  int connectioncount = pServer->getConnectedCount();
  if (connectioncount != prevBleConnectionCount) {
      Serial.println("Connection count changed");
      prevBleConnectionCount = connectioncount;
    if (connectioncount == 0) {
      BLEDevice::startAdvertising();
    }
  }
  float charge = *(float *)pCharacteristicCharge->getValue().c_str();
  float turn = *(float *)pCharacteristicTurn->getValue().c_str();


};