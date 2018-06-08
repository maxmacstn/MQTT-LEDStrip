#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#define ISINVERT true
#define FADESPEED 5

// //Function prototype
// void setOn(bool);
// void setBrightness(int);
// void blink();

// Constants
const char *autoconf_ssid = "ESP8266 LED_Light"; //AP name for WiFi setup AP which your ESP will open when not able to connect to other WiFi
const char *autoconf_pwd = "12345678";           // AP password so noone else can connect to the ESP in case your router fails
const char *mqtt_server = "192.168.1.120";       //MQTT Server IP, your home MQTT server eg Mosquitto on RPi, or some public MQTT
const int mqtt_port = 1883;                      //MQTT Server PORT, default is 1883 but can be anything.
const int pwmpin = D2;

// MQTT Constants
const char *mqtt_device_value_get_topic = "homebridge/from/set";
const char *device_name = "LED Strip";

// Global
int current_brightness = 100; // LED STRIP OFF (100), LED STRIP ON (0) My dimmer module is driven LOW so 100% is 0, 0% is 100
byte state = 1;
bool previousOn = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_ota()
{

  // Set OTA Password, and change it in platformio.ini
  ArduinoOTA.setPassword("ESP8266_PASSWORD");
  ArduinoOTA.onStart([]() {});
  ArduinoOTA.onEnd([]() {});
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {});
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR)
      ; // Auth failed
    else if (error == OTA_BEGIN_ERROR)
      ; // Begin failed
    else if (error == OTA_CONNECT_ERROR)
      ; // Connect failed
    else if (error == OTA_RECEIVE_ERROR)
      ; // Receive failed
    else if (error == OTA_END_ERROR)
      ; // End failed
  });
  ArduinoOTA.begin();
}

void reconnect()
{

  // Loop until we're reconnected
  while (!client.connected())
  {

    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect

    if (client.connect(clientId.c_str()))
    {

      // Once connected, resubscribe.
      client.subscribe(mqtt_device_value_get_topic);
    }
    else
    {

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setOn(bool isOn)
{
  if (!ISINVERT)
  {
    // Turn on : fade to previous brightness from off
    if (isOn && current_brightness != 0 && !previousOn)
    {
      for (int i = 0; i <= current_brightness; i++)
      {
        Serial.println(i, DEC);
        analogWrite(pwmpin, i);
        delay(10);
      }
      previousOn = true;
    }
    // Turn on: fade to 100 from off (if previous brightness is 0) : NOT INVERT
    else if (isOn && current_brightness == 0)
    {

      current_brightness = 100;
      for (int i = 0; i <= 100; i++)
      {
        Serial.println(i, DEC);
        analogWrite(pwmpin, i);
        delay(FADESPEED);
      }
      previousOn = true;
    }

    // Turn off
    if (isOn == false)
    {

      for (int i = current_brightness; i >= 0; i--)
      {
        Serial.println(i, DEC);
        analogWrite(pwmpin, i);
        delay(FADESPEED);
      }
      previousOn = false;
    }
  }
  if(ISINVERT){
    // Turn on : fade to previous brightness from off
    if (isOn && current_brightness != 100 && !previousOn)
    {
      for (int i = 100; i >= current_brightness; i--)
      {
        Serial.println(i, DEC);
        analogWrite(pwmpin, i);
        delay(10);
      }
      previousOn = true;
    }
    // Turn on: fade to 100 from off (if previous brightness is 0) :  INVERT
    else if (isOn && current_brightness == 100)
    {

      current_brightness = 0;
      for (int i = 100; i >= 0; i--)
      {
        Serial.println(i, DEC);
        analogWrite(pwmpin, i);
        delay(FADESPEED);
      }
      previousOn = true;
    }

    // Turn off
    if (isOn == false)
    {

      for (int i = current_brightness; i <=  100; i++)
      {
        Serial.println(i, DEC);
        analogWrite(pwmpin, i);
        delay(FADESPEED);
      }
      previousOn = false;
    }
  }
}

void setBrightness(int newbrightness)
{

  // This function will animate brightness change from last known brightness to the new one
  // It takes already inverted value so 100 is OFF, 0 is full brightness

  if (newbrightness > current_brightness)
  {
    for (int i = current_brightness; newbrightness >= i; i++)
    {
      Serial.println(i, DEC);
      analogWrite(pwmpin, i);
      delay(FADESPEED);
      current_brightness = i;
    }
  }
  else if (newbrightness < current_brightness)
  {
    for (int i = current_brightness; newbrightness <= i; i--)
    {
      Serial.println(i, DEC);
      analogWrite(pwmpin, i);
      delay(FADESPEED);
      current_brightness = i;
    }
  }
  else if (newbrightness == current_brightness)
  {
    analogWrite(pwmpin, newbrightness);
  }
}

void blink()
{

  //Blink on received MQTT message
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

void callback(char *topic, byte *payload, unsigned int length)
{

  char c_payload[length];
  memcpy(c_payload, payload, length);
  c_payload[length] = '\0';

  String s_topic = String(topic);
  String s_payload = String(c_payload);

  Serial.println(s_payload + "\0");

  StaticJsonBuffer<200> jsonBuffer;

  JsonObject &root = jsonBuffer.parseObject(s_payload);

  const char *name = root["name"];

  Serial.println(name);
  if (strcmp(name, device_name) != 0)
  {
    return;
  }

  blink();
  const char *characteristic = root["characteristic"];

  if (strcmp(characteristic, "On") == 0)
  {
    bool value = root["value"];

    // if (ISINVERT)
    //   value = !value;

    if (value)
      Serial.println("On = true");
    else
      Serial.println("On = false");

    setOn(value);
  }
  if (strcmp(characteristic, "Brightness") == 0)
  {
    int value = root["value"];

    if (ISINVERT)
      value = 100 - value;

    Serial.print("Brightness = ");
    Serial.println(value, DEC);
    setBrightness(value);
  }
        }

      } else if (s_payload == "0") {

        if (state != 0) {

          // Turn OFF function

          client.publish(mqtt_dimlightstatus_set_topic, "0");
          state = 0;
          blink();

          setBrightness(100);


        }

      }

    } else if ( s_topic == mqtt_dimlightbrightness_get_topic ) {

      int receivedBrightness = s_payload.toInt();

      if (receivedBrightness <= 100) {

        int brightness = 100-receivedBrightness;  // Invert brightness received to reflect that the module I have is driven LOW

        setBrightness(brightness);
        client.publish(mqtt_dimlightbrightnessstatus_set_topic,c_payload);

      }

    } 
    */
}

void setup()
{

  pinMode(pwmpin, OUTPUT);      //Setup pin for MOSFET
  analogWriteRange(100);        //This should set PWM range not 1023 but 100 as is %
  analogWrite(pwmpin, 100);     //Turn OFF by default
  pinMode(LED_BUILTIN, OUTPUT); //Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect(autoconf_ssid, autoconf_pwd);
  setup_ota();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  digitalWrite(LED_BUILTIN, HIGH); //Turn off led as default
  Serial.begin(115200);
  Serial.print("Connected");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
}