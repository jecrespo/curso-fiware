/*
  SENT DATA FROM ARDUINO TO ORION by using ethernet shield
*/

#include "secrets.h"

/* secrets.h file
  #define SECRET_MAC {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}
  #define SECRET_IP  {192, 168, 1, 2}
  #define SECRET_GW  {192, 168, 1, 1}
  #define SECRET_SERVER {192, 168, 1, 3}
*/

// libraries
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <Ethernet.h>

// SENSOR Config .....................................
#define DHTPIN 2
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
unsigned long delayTime;
//....................................................

// ETHERNET Config .....................................
// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = SECRET_MAC;
byte ip[] = SECRET_IP;
byte DNS[] = {8, 8, 8, 8};
byte gateway[] = SECRET_GW;
byte subnet[] = {255, 255, 255, 0};

// initialize the library instance:
EthernetClient client;

IPAddress FIWARE_SERVER = SECRET_SERVER;
int FIWARE_PORT = 7896;

// DEVICE config ......................................
//  VARIABLES
const int numsensors = 2;
String measures[numsensors][2];
char DEVICE_apikey[] = "4jggokgpepnvsb2uvd59ob"; // Arduino key
char DEVICE_id[] = "ard001"; // Arduino id
// ....................................................

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 1L * 1000L; // delay between updates, in milliseconds. Time to sent HTTP REQUEST

void setup() {
  // start serial port:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // Initialize device.
  dht.begin();
  // start the Ethernet connection:
  Serial.println("inicializando red...");
  Ethernet.begin(mac, ip, DNS, gateway, subnet);
  Serial.println(FIWARE_SERVER);

  // give the Ethernet shield two seconds to initialize:
  delay(2000);

  delayTime = 5000;  // time to refresh data

}

void loop() {

  printValues();

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }

  delay(delayTime);

}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  String payload;

  // if there's a successful connection:
  if (client.connect(FIWARE_SERVER, FIWARE_PORT)) {
    Serial.println("connecting...");

    for (int i = 0; i < numsensors; i++) {
      payload = payload + String(measures[i][0]) + "|" + String(measures[i][1]);
      if (i != numsensors - 1) {
        payload = payload + "|";
      }
    }

    // send the HTTP GET request:
    //client.println("GET /version HTTP/1.1");
    client.println("POST /iot/d?i=" + String(DEVICE_id) + "&k=" + String(DEVICE_apikey) + "&getCmd=1 HTTP/1.1");
    client.println("Host:" + String(FIWARE_SERVER) + ":" + String(FIWARE_PORT));
    client.println("Content-Length: " + String(payload.length()));
    client.println("Connection: close");
    client.println();
    client.println(payload);
    Serial.println(payload);

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}


void printValues() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float temp_meas = event.temperature;
  measures[0][0] = "t";
  measures[0][1] = String(temp_meas);
  Serial.print(F("Temperature: "));
  Serial.print(event.temperature);
  Serial.println(F("°C"));
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  float hum_meas = event.relative_humidity;
  measures[1][0] = "h";
  measures[1][1] = String(hum_meas);
  Serial.print(F("Humidity: "));
  Serial.print(event.relative_humidity);
  Serial.println(F("%"));

  Serial.println();

}
