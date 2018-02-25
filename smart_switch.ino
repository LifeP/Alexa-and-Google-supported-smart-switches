#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <functional>
#include "switch.h"
#include "UpnpBroadcastResponder.h"
#include "CallbackFunction.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
boolean connectWifi();
//Functions for alexa
void Front_bed_fun_on();
void Front_bed_fun_off();
void Rear_bed_fun_on();
void Rear_bed_fun_off();
void fan_fun_on();
void fan_fun_off();
void Socket01_fun_on();
void Socket01_fun_off();
void Socket02_fun_on();
void Socket02_fun_off();
void All_fun_on();
void All_fun_off();

//SSID & PASSWORD
const char* ssid = "Your SSID";
const char* password = "Your Password";
boolean wifiConnected = false;


//For Alexa
UpnpBroadcastResponder upnpBroadcastResponder;
Switch *Front_light_alx = NULL;
Switch *Rear_light_alx = NULL;
Switch *Fan_alx = NULL;
Switch *Socket01_alx = NULL;
Switch *Socket02_alx = NULL;
Switch *All_dev_alx = NULL;

//For MQTT setup
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Ashish_8284" //"...your AIO username (see https://accounts.adafruit.com)..."
#define AIO_KEY         "bc80745918fb440fbba258562c70241d"
//For GPIO identificaiton
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
static const uint8_t SD3  = 10;
/************ Global State (you don't need to change this!) ******************/
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
/****************************** Feeds ***************************************/
Adafruit_MQTT_Subscribe Front_light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Front_light");
Adafruit_MQTT_Subscribe Back_light = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Back_light");
Adafruit_MQTT_Subscribe Fan = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Fan");
Adafruit_MQTT_Subscribe Socket01 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Socket01");
Adafruit_MQTT_Subscribe All_devices = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/All_devices");
Adafruit_MQTT_Subscribe Socket02 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Socket02");
/*************************** internal data ********************************/
char* MQTT_CMD_01 = "OFF";
char* MQTT_CMD_02 = "OFF";
char* MQTT_CMD_03= "OFF";
char* MQTT_CMD_04= "OFF";
char* MQTT_CMD_05= "OFF";
char* MQTT_CMD_06= "OFF";
void MQTT_connect();
void setup()
{
  pinMode(D0, OUTPUT);
  digitalWrite(D0,1);
  pinMode(D1, OUTPUT);
  digitalWrite(D1,1);
  pinMode(D2, OUTPUT);
  digitalWrite(D2,1);
  pinMode(D3, OUTPUT);
  digitalWrite(D3,1);
  pinMode(D4, OUTPUT);
  digitalWrite(D4,1);
  pinMode(D8, OUTPUT);
  digitalWrite(D8,1);
  pinMode(D5,INPUT_PULLUP);
  pinMode(D6,INPUT_PULLUP);
  pinMode(D7,INPUT_PULLUP); 
  Serial.begin(115200);
// Initialise wifi connection
    wifiConnected = connectWifi();
    if(wifiConnected){
    upnpBroadcastResponder.beginUdpMulticast();
    Front_light_alx = new Switch("Firs light", 80, Front_bed_fun_on, Front_bed_fun_off);
    Rear_light_alx= new Switch("Second light", 81, Rear_bed_fun_on, Rear_bed_fun_off);
    Fan_alx  = new Switch("Fan", 82, fan_fun_on, fan_fun_off);
    Socket01_alx  = new Switch("First Socket", 83, Socket01_fun_on, Socket01_fun_off);
    All_dev_alx  = new Switch("All devices", 84, All_fun_on, All_fun_off);
    Socket02_alx  = new Switch("Second Socket", 85, Socket02_fun_on, Socket02_fun_off);
    Serial.println("Adding switches upnp broadcast responder");
    upnpBroadcastResponder.addDevice(*Front_light_alx);
    upnpBroadcastResponder.addDevice(*Rear_light_alx);
    upnpBroadcastResponder.addDevice(*Fan_alx);
    upnpBroadcastResponder.addDevice(*Socket01_alx);
    upnpBroadcastResponder.addDevice(*All_dev_alx);
    upnpBroadcastResponder.addDevice(*Socket02_alx);
  }
  mqtt.subscribe(&Front_light);
  mqtt.subscribe(&Back_light);
  mqtt.subscribe(&Fan);
  mqtt.subscribe(&Socket01);
  mqtt.subscribe(&All_devices);
  mqtt.subscribe(&Socket02);
}
uint32_t x=0;
void loop()
{ 
	 if(wifiConnected){
      upnpBroadcastResponder.serverLoop();
      Front_light_alx->serverLoop();
      Rear_light_alx->serverLoop();
      Fan_alx->serverLoop();
      Socket01_alx->serverLoop();
      All_dev_alx->serverLoop();
      Socket02_alx->serverLoop();
      MQTT_connect();
      Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.  readSubscription(1000))) {
    if (subscription == &Front_light) {
          MQTT_CMD_01 = (char *)Front_light.lastread;    
          if ( MQTT_CMD_01[1] == 'N' ){
          Front_bed_fun_on();
          Serial.println("First bed light on from google");
          }
          if ( MQTT_CMD_01[1] == 'F'){
          Front_bed_fun_off();
          Serial.println("First bed light off from google");
          }
    } 
    if (subscription == &Back_light) {
          MQTT_CMD_02 = (char *)Back_light.lastread;    
          if ( MQTT_CMD_02[1] == 'N' ){
             Rear_bed_fun_on();
             Serial.println(" Second bed light on from google");
             }
          if ( MQTT_CMD_02[1] == 'F'){
            Rear_bed_fun_off();
            Serial.println(" Second bed light off from google");
          }
          }
    if (subscription == &Fan) {
          MQTT_CMD_03 = (char *)Fan.lastread;    
          if ( MQTT_CMD_03[1] == 'N' ){
             fan_fun_on();
             Serial.println(" Fan on from google");
             }
          if ( MQTT_CMD_03[1] == 'F'){
            fan_fun_off();
            Serial.println(" Fan off from google");
          }
    }
    if (subscription == &Socket01) {
          MQTT_CMD_04 = (char *)Socket01.lastread;    
          if ( MQTT_CMD_04[1] == 'N' ){
             Socket01_fun_on();
             Serial.println(" Socket01 on from google");
             }
          if ( MQTT_CMD_04[1] == 'F'){
            Socket01_fun_off();
            Serial.println(" Socket01 off from google");
          }
    }
    if (subscription == &All_devices) {
          MQTT_CMD_05 = (char *)All_devices.lastread;    
          if ( MQTT_CMD_05[1] == 'N' ){
            All_fun_on();
            Serial.println(" All on from google");
             }
          if ( MQTT_CMD_05[1] == 'F'){
            All_fun_off();
            Serial.println(" All off from google");
            }
          }
    if (subscription == &Socket02) {
          MQTT_CMD_06 = (char *)Socket02.lastread;    
          if ( MQTT_CMD_06[1] == 'N' ){
            Socket02_fun_on();
            Serial.println(" All Socket02 from google");
             }
          if ( MQTT_CMD_06[1] == 'F'){
            Socket02_fun_off();
            Serial.println(" All Socket02 from google");
            }
          }          
       }
}
//if(wifiConnected){ if (mqtt.connected()){ digitalWrite(D0,LOW);}} else digitalWrite(D0,HIGH);
//Serial.print("Inputs:- "); 
//Serial.print(digitalRead(D5));Serial.print(digitalRead(D6));Serial.print(digitalRead(D7));Serial.println(digitalRead(D8));
}
void Front_bed_fun_on() {
    Serial.println("First bed light on alexa");
    digitalWrite(D1,0);   
}
void Front_bed_fun_off() {
    Serial.print("First bed light off alexa");
   digitalWrite(D1,1);
}
void Rear_bed_fun_on() {
    Serial.print("Second bed light from alexa");
   digitalWrite(D2,0);
}
void Rear_bed_fun_off() {
  Serial.print("Second bed light off from alexa");
  digitalWrite(D2,1);
}
void fan_fun_on() {
    Serial.print("Fan on from alexa");
       digitalWrite(D3,0);
}
void fan_fun_off() {
    Serial.print("Fan off from alexa");
      digitalWrite(D3,1);
   }
void Socket01_fun_on() {
    Serial.print("Socket01 on from alexa");
       digitalWrite(D4,0);
}
void Socket01_fun_off() {
    Serial.print("Socket01 off from alexa");
      digitalWrite(D4,1);
   }  
void Socket02_fun_on() {
    Serial.print("Socket02 on from alexa");
       digitalWrite(D8,0);
}
void Socket02_fun_off() {
    Serial.print("Socket02 , off from alexa");
      digitalWrite(D8,1);
   } 
void All_fun_on() {
   Serial.print("All devices on from alexa");
     digitalWrite(D1,0);
     delay(50);
        digitalWrite(D2,0);
        delay(50);
           digitalWrite(D3,0);
           delay(20);
             digitalWrite(D4,0);
             delay(20);
               digitalWrite(D8,0);
  }
void All_fun_off() {
   Serial.print("All devices off from alexa");
     digitalWrite(D1,1);
     delay(20);
      digitalWrite(D2,1);
      delay(20);
        digitalWrite(D3,1);
        delay(20);
          digitalWrite(D4,1);
          delay(20);
            digitalWrite(D8,1); 
  }
boolean connectWifi(){
  boolean state = true;
  int i = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi :- ");
  Serial.println(ssid);
  Serial.println("Connecting ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(i);
    Serial.println(" Attampt");
    Serial.print(".");
    if (i > 10){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
  Serial.println("Connection failed ");
  }
  return state;
}
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
      return;
     }
       Serial.print("Connecting to MQTT... ");
       uint8_t retries = 3;
       while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(1000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}



