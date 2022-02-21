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

#define lan_RST 10
#define lan_SS 53

uint8_t coils[16]={};
uint8_t input[16]={};

EthernetServer ethServer(502);
ModbusTCPServer modbusTCPServer;
byte __mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress __ip = {192, 168, 1, 25};

void setup()
{
  Serial.begin(9600);
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

        }
    }
}