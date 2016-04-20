#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include <SparkFunLSM9DS1.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266.h>

Adafruit_SSD1306 display(4);

#define IOT 1
#define IOT_UPDATE_INTERVAL 1000 //snce want to get every 1000 seconds and alternate posting/getting so need 1/2
#define wifiSerial Serial1          // for ESP chip
#define LSM9DS1_M  0x1E // 
#define LSM9DS1_AG  0x6B //

String kerberos = "jennycxu";        // UPDATE WITH YOUR KERBEROS
String MAC = "";
String resp = "";
uint32_t tLastIotReq = 0;       // time of last send/pull
uint32_t tLastIotResp = 0;      // time of last response


String lastRequest = "get";
String postOrBroadcast = "post";
#define SSID "6S08C"       // network SSID and password
#define PASSWORD "6S086S08"

ESP8266 wifi = ESP8266(true);  //Change to "true" or nothing for verbose serial output

int led = 13;

void setup() {
  Serial.begin(115200);
  delay(2500);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2500);
  display.clearDisplay();
  display.setTextColor(WHITE);
  Serial.println("TRYING");

  if (IOT) {
    wifi.begin();
    Serial.println("TRYING");
    wifi.connectWifi(SSID, PASSWORD);
    while (!wifi.isConnected()); //wait for connection
    MAC = wifi.getMAC();
  }
  Serial.println("CONNECTED");
  //get extra message if there is one and clear it out
  if ((IOT && wifi.hasResponse())) {
    resp = wifi.getResponse();
  }

  display.setCursor(0, 0);
  randomSeed(analogRead(0));//seed random number
  display.setTextSize(2);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
}

void loop() {
  if (millis() - tLastIotReq >= IOT_UPDATE_INTERVAL) {
    if (wifi.isConnected() && !wifi.isBusy()) { //Check if we can send request
      Serial.print("Sending request at t=");
      Serial.println(millis());

      String domain = "iesc-s2.mit.edu";
      int port = 80;

      String path = "/student_code/" + kerberos + "/dev1/sb1.py";
      String getParams;

      wifi.sendRequest(GET, domain, port, path, getParams);
      /*if (lastRequest == "get") {
        getParams = "&recipient=jenny&source=teensey";

        wifi.sendRequest(GET, domain, port, path, getParams);
        if(postOrBroadcast == "post"){
          lastRequest = "post";
          postOrBroadcast = "broadcast";
        } else {
          lastRequest = "broadcast";
          postOrBroadcast = "post";//every other time post something
        }
        } else if(lastRequest == "post"){
        Serial.println("HI IM POSTING");
          getParams = "&sender=jenny&recipient=person1&message=Hello!workinkg";
           lastRequest = "get";
        wifi.sendRequest(POST, domain, port, path, getParams);
        } else {
         getParams = "&sender=jenny&recipient=BROADCAST&message=Hello!brodacasting";
           lastRequest = "get";
           wifi.sendRequest(POST, domain, port, path, getParams);
        }*/
      tLastIotReq = millis();



    }

  }


  if ((IOT && wifi.hasResponse())) {
    Serial.println("HAVE IOT");
    resp = wifi.getResponse();
    tLastIotResp = millis();
    updateDisplay();

    Serial.println(resp);
    delay(1000);//can read for 5 seconds before reset

  }

  //}
  delay(50);
}
void updateDisplay()
{
  Serial.println("HI IM DISPLAING");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(resp);
  display.println("HI");
  display.display();
}
