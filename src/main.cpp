#define EN_DEBUG
#if defined(EN_DEBUG)
#define debug Serial
#define DB_LOG(x) debug.print(x)
#define DB(x) debug.print("__DEBUG: " + (String)x)
#define DB_LN(x) debug.println("__DEBUG: " + (String)x)
#define DB_BEGIN(x) debug.begin(x)
#else
#define DB_BG(...)
#define DB_LN(...)
#define DB(...)
#define DB_LOG(...)
#endif
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet2.h>
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>
#include <SHT3x.h>
#define lan_RST 10
#define lan_SS 53

uint8_t coils[16] = {37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22};
uint8_t input[16] = {A8, A9, A10, A11, A12, A13, A14, A15, A0, A1, A2, A3, A4, A5, A6, A7};

EthernetServer ethServer(502);
ModbusTCPServer modbusTCPServer;
SHT3x Sensor;
byte __mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress __ip = {192, 168, 1, 25};
void inOutInit();
void pollingInOut(); 
void updateSensor();
void setup()
{
  Serial.begin(9600);
  inOutInit();
  Sensor.Begin();
  Ethernet.init(lan_SS);
  pinMode(lan_RST, OUTPUT);
  digitalWrite(lan_RST, LOW);
  delay(500);
  digitalWrite(lan_RST, HIGH);
  DB_LN("Ethernet Modbus TCP");
  Ethernet.begin(__mac, __ip);
  ethServer.begin();
  Serial.println(Ethernet.localIP());
  if (!modbusTCPServer.begin())
  {
    DB_LN("Failed to start Modbus TCP Server!");
    while (1)
      ;
  }
  modbusTCPServer.configureHoldingRegisters(0x00, 100);
  modbusTCPServer.configureCoils(0x00, 16);
  modbusTCPServer.configureInputRegisters(0x00, 16);
  modbusTCPServer.configureDiscreteInputs(0x00, 16);
}
void loop()
{
  EthernetClient client = ethServer.available();
  if (client)
  {
    modbusTCPServer.accept(client);
    if (client.connected())
    {
      modbusTCPServer.poll();
      pollingInOut();
      updateSensor();
    }
  }
}
void inOutInit()
{
  for (int i = 0; i < 16; i++)
  {
    pinMode(coils[i], OUTPUT);
    pinMode(input[i], INPUT_PULLUP);
  }
}
void pollingInOut()
{
  for (int i = 0; i < 16; i++)
  {
    digitalWrite(coils[i], modbusTCPServer.coilRead(i));
    modbusTCPServer.discreteInputWrite(i, digitalRead(input[i]));
  }
}
void updateSensor()
{
  static unsigned long timeSampling = 0;
  Sensor.UpdateData();
  if (millis() - timeSampling > 10)
  {
    uint16_t tem = Sensor.GetTemperature() * 100;
    uint16_t humi = Sensor.GetRelHumidity() * 100;
    modbusTCPServer.inputRegisterWrite(0, tem);
    modbusTCPServer.inputRegisterWrite(1, humi);
    timeSampling = millis();
  }
}