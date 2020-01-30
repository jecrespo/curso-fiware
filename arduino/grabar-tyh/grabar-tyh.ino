#define DEBUG 1

#include "secrets.h"

/* secrets.h file
  #define SECRET_MAC {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}
  #define SECRET_IP  {192, 168, 1, 2}
  #define SECRET_GW  {192, 168, 1, 1}
  #define SECRET_SERVER {192, 168, 1, 3}
*/

#include <Ethernet.h>
#include <SPI.h>
#include <SimpleDHT.h>

byte mac[] = SECRET_MAC;
byte ip[] = SECRET_IP;
byte DNS[] = {8, 8, 8, 8};
byte gateway[] = SECRET_GW;
byte subnet[] = {255, 255, 255, 0};
byte server[] = SECRET_SERVER;

EthernetClient client;
int pinDHT22 = 2;
SimpleDHT22 dht22(pinDHT22);

void setup()
{
  Ethernet.begin(mac, ip, DNS, gateway, subnet);
#if DEBUG
  Serial.begin(250000);
#endif
  delay(1000);
}

void loop()
{
  grabaDatos();

  String webString = "";
  if (client.available()) {
#if DEBUG
    Serial.println("Respuesta del Servidor---->");
#endif
    while (client.available()) {
      char c = client.read();
      webString += c;
    }
#if DEBUG
    Serial.println(webString);
#endif
    client.stop();
  }

  delay(5000);
}

void grabaDatos() {
#if DEBUG
  Serial.println("leyendo temperatura y humedad... ");
#endif

  float temperature = 0;
  float humidity = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht22.read2(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
#if DEBUG
    Serial.print("Read DHT22 failed, err="); Serial.println(err); delay(2000);
#endif
    return;
  }

#if DEBUG
  Serial.print("Sample OK: ");
  Serial.print((float)temperature); Serial.print(" *C, ");
  Serial.print((float)humidity); Serial.println(" RH%");
#endif

  String datos = "{\"t\": {\"type\": \"string\",\"value\": " + String(temperature) + "}, \"h\": {\"type\": \"string\", \"value\": " + String(humidity) + "}}";

#if DEBUG
  Serial.println("connecting to server...");
#endif

  if (client.connect(server, 1026)) {
    client.println("PATCH /v2/entities/sensoroficina:ard001/attrs HTTP / 1.1");
    client.println("Content-Type: application/json");
    client.println("fiware-service: demooficina");
    client.println("fiware-ServicePath: /oficina/");
    client.print("Content-Length: ");
    client.println(datos.length());
    client.println("Connection: close");
    client.println();
    client.println(datos);

#if DEBUG
    Serial.println("PATCH /v2/entities/sensoroficina:ard001/attrs HTTP / 1.1");
    Serial.println("Content - Type: application/json");
    client.println("fiware-service: demooficina");
    client.println("fiware-ServicePath: /oficina/");
    Serial.print("Content-Length: ");
    Serial.println(datos.length());
    Serial.println("Connection: close");
    Serial.println();
    Serial.println(datos);
#endif
  }
  else {
#if DEBUG
    Serial.println("connection failed");
#endif
  }
}
