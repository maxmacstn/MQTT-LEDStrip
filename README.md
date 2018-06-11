

# HomeBridge-MQTT LED-Strip

[![forthebadge](https://forthebadge.com/images/badges/built-with-love.svg)](https://forthebadge.com)

A code for ESP8266 microcontroller to control LED Strip via [homebridge-mqtt plugin for homebridge](https://github.com/cflurin/homebridge-mqtt) which can contron LED Strip from iOS Devices.
Implemented from https://github.com/ArnieX/esp8266_dimmer_mqtt

![gif](images/videotogif_2018.06.09_21.57.40.gif?raw=true)
https://www.youtube.com/watch?v=2EnSU2nZBkw

# Functions
 - Turn on/off
 - Brightness control 
 - Manual control with momentary push button. (Still have bugs, more info in known bugs section)

# Hardware
I duplicate DIY Dimmer module circuit from https://github.com/ArnieX/esp8266_dimmer_mqtt but I added momentary push button and connected it to D3 and ground (No pull-up resistor needed because I uses built-in pull-up)
It control 12V LED Strip using PWM Signal that generated from ESP8266 which can control brightness of the LED Strip
Please visit original project link for schematic.
Note : Controlling LED from this circuit is invert, 100% PWM Duty cycle is off, 0% duty cycle is full brightness. (You can change invert setting with ISINVERT boolean parameter)

# Add accessories to Homebridge-mqtt plugin
According to homebride-mqtt plugin, you can add accessories for LED-Strip to homebridge by sending message(payload) to topic below.

 * topic
```url
homebridge/to/set
```

 * payload
```json
{
    "name": "LED Strip",
    "service_name": "led_strip",
    "service": "Lightbulb",
    "Brightness": "default"
  }
```

# Usage
 1. Install homebridge-mqtt plugin to your homebridge instance and add accessories to it using topic and payload above.
 2. Feel free to edit source file to suits your requirements and don't forget to change your mqtt server ip to match yours (Mine is 192.168.1.120)
 3. Install dependencies
  - WiFiManager
  - PubSubClient
  - ESP8266wifi
 4. Flash main.cpp to your microcontroller. You can use Aruino IDE or PlatformIO
 5. For the first time, you'll need to connect to your Wifi. First, ESP8266 will create wifi named "ESP8266 LED_Light", connect to it using password 12345678, you'll see login redirect to wifi config page(if not, enter 192.168.4.1 in browser), connect to your wifi.
 6. If everything works, your ESP8266 should followed the command from HomeBridge.


# Tested on
* NodeMCU
* Wemos D1 Mini

# Known bugs
 - Manual light control with button from ESP8266 may consequence to stopped sync data with HomeBridge. It may caused by PubSubCilent stopped subscribe to MQTT topic. I tried to add delay and boolean to lock the critical section in the code while the process is still running to prevent it to take action without finished previous action but this bug still occurs.


If you got any problem or suggestions, don't hesitate to contact me, enjoy!


