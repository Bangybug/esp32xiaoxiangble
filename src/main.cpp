/** 
 * This firmware retranslates UART Serial port via BLE. When data is available in Serial, it is sent via UART_RX_CHARACTERISTIC_UUID.
 * When incoming data is available from Bluetooth, it is taken from UART_TX_CHARACTERISTIC_UUID and sent to Serial.
 * author: Dmitry Pryadkin, drpadawan@ya.ru
 **/
 
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
 
// identifiers from xiaoxiang bms bluetooth module
#define SERVICE_UART_UUID "0000ff00-0000-1000-8000-00805f9b34fb"
#define UART_RX_CHARACTERISTIC_UUID "0000ff01-0000-1000-8000-00805f9b34fb"
#define UART_TX_CHARACTERISTIC_UUID "0000ff02-0000-1000-8000-00805f9b34fb"
 
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pRxCharacteristic;
BLECharacteristic *pTxCharacteristic;
 
// The limit is taken from BLECharacteristic.cpp/indicate/notify doc, it says:
// An indication is a transmission of up to the first 20 bytes of the characteristic value.  An indication
// will block waiting a positive confirmation from the client.
#define SERIAL_BUF_SIZE_MAX 20
uint8_t *serialReadBuffer = new uint8_t[SERIAL_BUF_SIZE_MAX];
 
class ServerConnectionCallback : public BLEServerCallbacks
{
  void onDisconnect(BLEServer *pServer)
  {
    pServer->getAdvertising()->start();
  }
};
 
class UartTxBMSCallback : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    const std::string &value = pCharacteristic->getValue();
    const size_t length = value.length();
    const char *data = value.data();
    if (length > 0)
    {
      for (size_t i = 0; i < length; ++i)
      {
        Serial.print(data[i]);
      }
    }
  }
};
 
void setup()
{
  Serial.begin(9600);
 
  BLEDevice::init("xiaoxiang BMS");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerConnectionCallback());
  pService = pServer->createService(SERVICE_UART_UUID);
  pRxCharacteristic = pService->createCharacteristic(
      UART_RX_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic = pService->createCharacteristic(
      UART_TX_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE);
 
  pTxCharacteristic->setCallbacks(new UartTxBMSCallback());
 
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UART_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}
 
void loop()
{
  int uartBytesAvailable = Serial.available();
  if (uartBytesAvailable)
  {
    if (pServer->getConnectedCount() > 0)
    {
      // allow uart buffer to accumulate (don't send byte by byte)
      if (uartBytesAvailable == 1)
      {
        delay(50);
        uartBytesAvailable = Serial.available();
      }

      const size_t sizeSerialRead = Serial.readBytes(serialReadBuffer, min(SERIAL_BUF_SIZE_MAX, uartBytesAvailable));
      pRxCharacteristic->setValue(serialReadBuffer, sizeSerialRead);
      pRxCharacteristic->notify(); // may use indicate instead but that will require client to send ack
    }
    else
    {
      // Discard UART data if no connection
      while (Serial.available())
      {
        Serial.read();
      }
    }
  }
}