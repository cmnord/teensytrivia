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
int id = 0; //question ID, different for each question

#define SSID "6S08B"       // network SSID and password
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
  Serial.println("-----begin loop-----------------");
  wifi.clearRequest();
  //if (millis() - tLastIotReq >= IOT_UPDATE_INTERVAL) {
    resp = getQuestion();
  //  tLastIotReq = millis();
  //}
  if (resp.equals("")){
    Serial.println("Response is empty :((((");
  }
  int correct_pin = parseResponse(resp);
  //resp is now nicely formatted
  updateDisplay(resp);
  tQuestionStart = millis();
  while (digitalRead(a_button) && digitalRead(b_button) && digitalRead(c_button) && digitalRead(d_button)) {
    //delay until you press a button again
    delay(50); //this means all deltas will be in 50-ms increments (adjust the delay to make shorter delta)
  }
  isCorrect = 0;
  if (!digitalRead(correct_pin)) {
    isCorrect = 1;
    Serial.println("Correct answer!");
    updateDisplay("Correct answer!");
  }
  else {
    Serial.println("Sorry, wrong answer\n:(");
    updateDisplay("Sorry, wrong answer\n:(");
  }
  tQuestionEnd = millis();
  delta = (tQuestionEnd - tQuestionStart) / 1000.;
  postData(id, 0, 1, delta, isCorrect, 10);
  //String lead = getLeaderboard();
  //updateDisplay(lead);
  //delay(2000); //show leaderboard for 2 seconds
}

void updateDisplay(String text) {
  Serial.println("HI IM DISPLAYING");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(text);
  display.display();
}

void menu() {
  String menuText = "Teensy Trivia\n6.S08 Final Project\nJenny Xu and Claire Nord\nA. Start Game\nB. Leaderboard";
  while (digitalRead(a_button) && digitalRead(b_button)) {
    //delay until you press A or B
    delay(50);
  }
  if (!digitalRead(b_button)) {
    //B button selected, so display leaderboard
    //display leaderboard by GET from results.py
  }
  //if they select the A button, the method will just end and loop will begin, which is the game
}

String getQuestion() {
  //gets a question from sb1.py.
  String domain = "iesc-s2.mit.edu";
  int port = 80;
  String response = "";
  if (wifi.isConnected()) { //&& !wifi.isBusy()
    Serial.print("Getting question at t=");
    Serial.println(millis());
    String getPath = "/student_code/" + kerberos + "/dev1/sb1.py";
    String getParams = "recipient=" + kerberos + "&source=teensey";
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      Serial.print("Got response at t=");
      Serial.println(millis());
      Serial.println(response);
    } else {
      Serial.println("No timely response");
    }
  }
  else{
    Serial.println("either wifi is disconnected or wifi is busy");
  }
  return response;
}

void postData(String questionID, int gameID, int roundNum, float deltaT, int correct, int score) {
  //posts the user's answer, etc. to sb3.py.
  String domain = "iesc-s2.mit.edu";
  int port = 80;
  if (wifi.isConnected() && !wifi.isBusy()) {
    Serial.print("Posting data at t=");
    Serial.println(millis());
    String postPath = "/student_code/" + kerberos + "/dev1/sb3.py";
    //TODO: complete these get params
    String postParams = "sender=" + kerberos +
                        "&questionID=" + questionID +
                        "&deviceType=teensey&gameID=" + String(gameID) +
                        "&roundNum=" + String(roundNum) +
                        "&delta=" + String(deltaT, 3) +
                        "&isCorrect=" + correct +
                        "&currentScore=" + String(score);  //deltaT is set to 3 decimal places
    wifi.sendRequest(POST, domain, port, postPath, postParams);
    String junk = wifi.getResponse();
    Serial.println("This data was posted yay");
    isCorrect = -1;//if it has an answer needed to be pushed, push it and then restart
    unsigned long t = millis();
    while (wifi.isBusy() && millis() - t < 10000); //wait for response
  }
}

int parseResponse(String response) {
  //updates resp to be nicely formatted text for teensy
  //returns the int value of the button w/ correct answer
  //should this be two different methods?
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
  String question = "";
  String ans_a = "";
  String ans_b = "";
  String ans_c = "";
  String ans_d = "";
  String correct_ans = "";
  
  id = response.substring(response.indexOf("<h1>") + 4, response.indexOf("<h1>") + 7).toInt();
  question = response.substring(response.indexOf("<h1>") + 9, response.indexOf("</h1>"));
  ans_a = response.substring(response.indexOf("<A>") + 3, response.indexOf("</A>"));
  ans_b = response.substring(response.indexOf("<B>") + 3, response.indexOf("</B>"));
  ans_c = response.substring(response.indexOf("<C>") + 3, response.indexOf("</C>"));
  ans_d = response.substring(response.indexOf("<D>") + 3, response.indexOf("</D>"));
  correct_ans = response.substring(response.indexOf("<h4>") + 4, response.indexOf("</h4>"));
  
  int ans_pin = a_button;
  if (correct_ans.equals(ans_b)) {
    ans_pin = b_button;
  }
  else if (correct_ans.equals(ans_c)) {
    ans_pin = c_button;
  }
  else if (correct_ans.equals(ans_d)) {
    ans_pin = d_button;
  }
  resp = question + "\n";
  resp += "A. " + ans_a + "\nB. " + ans_b + "\nC. " + ans_c + "\nD. " + ans_d;
  return ans_pin;
}
