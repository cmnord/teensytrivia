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

uint32_t tQuestionStart = 0; //used for determining delta
uint32_t tQuestionEnd = 0; //used for determining delta
float delta = 0; //time it takes for player to answer

String lastRequest = "get";
#define SSID "6S08C"       // network SSID and password
#define PASSWORD "6S086S08"

ESP8266 wifi = ESP8266(true);  //Change to "true" or nothing for verbose serial output

//define buttons
int a_button = 4; //black
int b_button = 2; //black
int c_button = 8; //yellow
int d_button = 6; //blue
int led = 13;

bool isCorrect = 0;

void setup() {
  Serial.begin(115200);
  //button setup
  pinMode(a_button, INPUT); //set pin to "listen" to signals
  pinMode(b_button, INPUT); //set pin to "listen" to signals
  pinMode(c_button, INPUT); //set pin to "listen" to signals
  pinMode(d_button, INPUT); //set pin to "listen" to signals
  pinMode(led, OUTPUT); //set LED to be output
  pinMode(a_button, INPUT_PULLUP); //set input to be normally "HI"
  pinMode(b_button, INPUT_PULLUP); //""
  pinMode(c_button, INPUT_PULLUP); //""
  pinMode(d_button, INPUT_PULLUP); //""
  digitalWrite(led, LOW); //Set LED to be off at beginning

  //display setup
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

      String path = "/student_code/cnord/dev1/sb1.py";
      String pathPost = "/student_code/" + kerberos + "/dev1/sb3.py";
      String getParams;

      if (lastRequest == "get") {
        getParams = "&recipient=jenny&source=teensey";
        wifi.sendRequest(GET, domain, port, path, getParams);
        //lastRequest = "post";
      } else if (lastRequest == "post") { //if it is set to post that means they selected an answer
        Serial.println("HI IM POSTING:");
        getParams = "&sender=" + kerberos + "&questionID=1&deviceType=teensey&id=0&gameID=0&roundNum=1&delta="+delta+"&isCorrect=" + isCorrect + "&currentScore=10";
        lastRequest = "get";
        wifi.sendRequest(POST, domain, port, pathPost, getParams);
        isCorrect = -1;//if it has an answer needed to be pushed, push it and then restart
      }
      tLastIotReq = millis();
    }
  }
  if ((IOT && wifi.hasResponse())) {
    Serial.println("HAVE IOT");
    resp = wifi.getResponse();
    /*
      resp format:
      <html><h1>008. What was the name of Cheerios when it was first marketed 50 years ago?</h1><ul>
      <li><a>Cheerioats</a></li>
      <li><b>Cheer Oats</b></li>
      <li><c>Cheerie's</c></li>
      <li><d>Cheerio-Loops</d></li>
      </ul><h4>Cheerioats</h4>
      </body></html>
    */
    int correct_pin = parseResponse(resp);
    //resp is now nicely formatted
    
    tLastIotResp = millis();
    updateDisplay();
    tQuestionStart = millis();
    while (digitalRead(a_button) &&
           digitalRead(b_button) &&
           digitalRead(c_button) &&
           digitalRead(d_button)) {
      //delay until you press a button again
      delay(50);
    }
    isCorrect = 0;
    if (!digitalRead(correct_pin)) {
      isCorrect = 1;
      lastRequest = "post";//to post the answer on next wifi available
      Serial.println("U R RIGHT");
    }
    else {
      lastRequest = "post";
      Serial.println("UR WRONG");
    }
    tQuestionEnd = millis();
    delta = (tQuestionEnd - tQuestionStart) / 1000.;
    Serial.print("----------- Delta= ");
    Serial.print(delta);
    Serial.println("sec ---------------");
  }
}

void updateDisplay() {
  Serial.println("HI IM DISPLAING");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(resp);
  display.display();
}

int parseResponse(String response) {
  //updates resp to be nicely formatted text for teensy
  //returns the int value of the button w/ correct answer
  //should this be two different methods?
  String question = response.substring(response.indexOf("<h1>") + 9, response.indexOf("</h1>"));
  String ans_a = response.substring(response.indexOf("<A>") + 3, response.indexOf("</A>"));
  String ans_b = response.substring(response.indexOf("<B>") + 3, response.indexOf("</B>"));
  String ans_c = response.substring(response.indexOf("<C>") + 3, response.indexOf("</C>"));
  String ans_d = response.substring(response.indexOf("<D>") + 3, response.indexOf("</D>"));

  String correct_ans = response.substring(response.indexOf("<h4>") + 4, response.indexOf("</h4>"));
  int ans_pin = a_button;
  if(correct_ans.equals(ans_b)){ans_pin = b_button;}
  else if(correct_ans.equals(ans_c)){ans_pin = c_button;}
  else if(correct_ans.equals(ans_d)){ans_pin = d_button;}
  resp = question + "\n";
  resp += "A. " + ans_a + "\nB. " + ans_b + "\nC. " + ans_c + "\nD. " + ans_d;
  return ans_pin;
}
