/*################################# An example to connect thingcontro.io MQTT over TLS 1.2 ###############################
 * Using thingcontrol board V 1.0 read ambient temperature and humidity values from an FS200-SHT 1X sensor via I2C send to 
 * thingcontrol.io by MQTT over TLS 1.2 via WiFi (WiFi Manager)
 *########################################################################################################################*/
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <ADS1X15.h>
#include <Arduino.h>

#include <ArduinoOTA.h>
#include <WiFi.h>
#include <Wire.h>

//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial
 

/**********************************************  WIFI Client 注意编译时要设置此值 *********************************
   wifi client
*/
const char* ssid = "greenio"; //replace "xxxxxx" with your WIFI's ssid
const char* password = "green7650"; //replace "xxxxxx" with your WIFI's password

//WiFi&OTA 参数
String HOSTNAME = "EGAT";
#define PASSWORD "green7650" //the password for OTA upgrade, can set it in any char you want

#define WIFI_AP ""
#define WIFI_PASSWORD ""

String deviceToken = "BsEoft9JX0jD7F1VNzGA"; //TV4em3w1NA21n6AyOWkO
char thingsboardServer[] = "mqtt.thingcontrol.io";

String json = "";

unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long startTeleMillis;
unsigned long starSendTeletMillis;
unsigned long currentMillis;
const unsigned long periodSendTelemetry = 1;  //the value is a number of milliseconds

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

int status = WL_IDLE_STATUS;
int PORT = 8883;



ADS1015 adsV(0x48);     /* Use this for the 12-bit version */
ADS1015 adsA(0x49);     /* Use this for the 12-bit version */
int16_t adc0, adc1, adc2, adc3;
int16_t adc4, adc5, adc6, adc7;


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}
 
void setupOTA()
{
  //Port defaults to 8266
  //ArduinoOTA.setPort(8266);

  //Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(HOSTNAME.c_str());

  //No authentication by default
  ArduinoOTA.setPassword(PASSWORD);

  //Password can be set with it's md5 value as well
  //MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  //ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]()
  {
    Serial.println("Start Updating....");

    Serial.printf("Start Updating....Type:%s\n", (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem");
  });

  ArduinoOTA.onEnd([]()
  {

    Serial.println("Update Complete!");

    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    String pro = String(progress / (total / 100)) + "%";
    int progressbar = (progress / (total / 100));
    //int progressbar = (progress / 5) % 100;
    //int pro = progress / (total / 100);



    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
     
  });

  ArduinoOTA.onError([](ota_error_t error)
  {
    Serial.printf("Error[%u]: ", error);
    String info = "Error Info:";
    switch (error)
    {
      case OTA_AUTH_ERROR:
        info += "Auth Failed";
        Serial.println("Auth Failed");
        break;

      case OTA_BEGIN_ERROR:
        info += "Begin Failed";
        Serial.println("Begin Failed");
        break;

      case OTA_CONNECT_ERROR:
        info += "Connect Failed";
        Serial.println("Connect Failed");
        break;

      case OTA_RECEIVE_ERROR:
        info += "Receive Failed";
        Serial.println("Receive Failed");
        break;

      case OTA_END_ERROR:
        info += "End Failed";
        Serial.println("End Failed");
        break;
    }


    Serial.println(info);
    ESP.restart();
  });

  ArduinoOTA.begin();
}

 
void setup() {
  Serial.begin(115200); // Open serial connection to report values to host
  Serial.println(F("Starting... Ambient Temperature/Humidity Monitor"));
  Serial.println();
  Serial.println(F("***********************************"));

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect("@Thingcontrol_AP")) {
    Serial.println("failed to connect and hit timeout");
    delay(1000);
  }
  client.setServer( thingsboardServer, PORT );
  reconnectMqtt();

  Serial.print("Start.....");
  startMillis = millis();  //initial start time


  uint64_t chipider;

  uint64_t chipid = ESP.getEfuseMac();
  unsigned long long1 = (unsigned long)((chipid & 0xFFFF0000) >> 16 );
  unsigned long long2 = (unsigned long)((chipid & 0x0000FFFF));

  String hex = String(long1, HEX) + String(long2, HEX);


  HOSTNAME.concat(hex);
  
 
  Serial.println("Initialize...OTA");



   Serial.println("Initialize...ADS");
  adsV.begin();

  adsV.setDataRate(7);
  adsV.setWireClock(400000);
  adsA.begin();
  adsA.setDataRate(7);
  adsA.setWireClock(400000);

//   setupWIFI();
  setupOTA();

}

void loop()
{
 
  adc0 = adsV.readADC(0);
  adc1 = adsV.readADC(1);
  adc2 = adsV.readADC(2);
  adc3 = adsV.readADC(3);
  adc4 = adsA.readADC(0);
  //  if ((adc0 > 5) || (adc1 < 5) || (adc2 > 5) || (adc3 > 5)) {
  //    Serial.print(millis());  Serial.print(":");  Serial.print(adc0);  Serial.print("\t");  Serial.print(adc1);  Serial.print("\t"); Serial.print(adc2);  Serial.print("\t");  Serial.print(adc3);  Serial.print("\t");  Serial.println(adc4);
  sendVtelemetry();

  Serial.println(millis());
  //  }

  unsigned long ms = millis();
  if (ms % 60000 == 0) {
    status = WiFi.status();
    if ( status == WL_CONNECTED)
    {
      if ( !client.connected() )
      {
        reconnectMqtt();
      }

    }


  }
  if (ms % 6000 == 0) {


    adc5 = adsA.readADC(1);
    adc6 = adsA.readADC(2);
    adc7 = adsA.readADC(3);

    Serial.print(millis());  Serial.print(":");  Serial.print(adc5);  Serial.print("\t"); Serial.print(adc6);  Serial.print("\t");  Serial.println(adc7);
    sendAtelemetry();
    Serial.println(millis());

  }

    ArduinoOTA.handle();

    
 
}

void setWiFi()
{
  Serial.println("OK");
  Serial.println("+:Reconfig WiFi  Restart...");
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  if (!wifiManager.startConfigPortal("ThingControlCommand"))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
   
  }
  Serial.println("OK");
  Serial.println("+:WiFi Connect");
  client.setServer( thingsboardServer, PORT );  // secure port over TLS 1.2
  reconnectMqtt();
}

void processTele(char jsonTele[])
{
  char *aString = jsonTele;
  Serial.println("OK");
  Serial.print(F("+:topic v1/devices/me/ , "));
  Serial.println(aString);
  client.publish( "v1/devices/me/telemetry", aString);
}

void reconnectMqtt()
{
  if ( client.connect("EGAT.SubStation", deviceToken.c_str(), NULL) )
  {
    Serial.println( F("Connect MQTT Success."));
    client.subscribe("v1/devices/me/rpc/request/+");
  }
}
void sendVtelemetry()
{
  String json = "";
  json.concat("{\"adc0\":");
  json.concat(adc0);

  json.concat(",\"adc1\":");
  json.concat(adc1);
  json.concat(",\"adc2\":");
  json.concat(adc2);
  json.concat(",\"adc3\":");
  json.concat(adc3);
  json.concat(",\"adc4\":");
  json.concat(adc4);
  json.concat("}");

  Serial.println(json);

  client.publish( "v1/devices/me/telemetry",  json.c_str());
}

void sendAtelemetry()
{
  String json = "";
  json.concat("{\"adc5\":");
  json.concat(adc5);
  json.concat(",\"adc6\":");
  json.concat(adc6);
  json.concat(",\"adc7\":");
  json.concat(adc7);
  json.concat("}");

  Serial.println(json);

  client.publish( "v1/devices/me/telemetry",  json.c_str());
}
  
 
