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

//wifi/internet globals
String kerberos = "cnord";        // UPDATE WITH YOUR KERBEROS
String MAC = "";
String resp = "";
uint32_t tLastIotReq = 0;       // time of last send/pull
uint32_t tLastIotResp = 0;      // time of last response
String domain = "iesc-s2.mit.edu";
int port = 80;

//game globals
uint32_t tQuestionStart = 0; //used for determining delta
uint32_t tQuestionEnd = 0; //used for determining delta
float delta = 0; //time it takes for player to answer
int id = 0; //question ID, different for each question
bool isCorrect = 0;
//int currentScore = 0; //+10 for every correct answer
int roundNum = 0; //this gets incremented every loop
int gameID = 0; //this gets changed later
int MAX_ROUNDS = 5; //game ends after MAX_ROUNDS questions

#define SSID "MIT GUEST" // network SSID and password
#define PASSWORD ""

ESP8266 wifi = ESP8266(true);  //Change to "true" or nothing for verbose serial output

//define buttons
int a_button = 4; //black
int b_button = 2; //black
int c_button = 8; //yellow
int d_button = 6; //blue

void setup() {
  Serial.begin(115200);
  //button setup
  pinMode(a_button, INPUT); //set pin to "listen" to signals
  pinMode(b_button, INPUT);
  pinMode(c_button, INPUT);
  pinMode(d_button, INPUT);
  pinMode(a_button, INPUT_PULLUP); //set input to be normally "HI"
  pinMode(b_button, INPUT_PULLUP);
  pinMode(c_button, INPUT_PULLUP);
  pinMode(d_button, INPUT_PULLUP);

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
    Serial.print("WIFI CONNECTING.....");
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
  menu();
}

void loop() {
  Serial.println("----------begin loop-----------------");
  wifi.clearRequest();
  if(roundNum==MAX_ROUNDS){
    endGame();
  }
  roundNum += 1;
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
    //currentScore += 10; //10 points per correct question
    Serial.println("Correct answer!");
    updateDisplay("Correct answer!\nPosting to DB...");
  }
  else {
    Serial.println("Sorry, wrong answer\n:(");
    updateDisplay("Sorry, wrong answer\n:(\nPosting to DB...");
  }
  tQuestionEnd = millis();
  delta = (tQuestionEnd - tQuestionStart) / 1000.;
  postData(id, gameID, roundNum, delta, isCorrect);
  String lead = getLeaderboard();
  updateDisplay(lead);
  delay(2000); //show leaderboard for 2 seconds
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
  String menuText = "Teensy Trivia\n6.S08 Final Project\nJenny Xu and Claire Nord\nA. New Game [Solo]\nB. Join Game [Alpha]\nC. Leaderboard";
  updateDisplay(menuText);
  while (digitalRead(a_button) && digitalRead(b_button) && digitalRead(c_button)) {
    //delay until you press A, B, or C
    delay(50);
  }
  if (!digitalRead(a_button)) {
    gameID = random(1,999);
    String disp = "Creating game #" + String(gameID) + "...\nTell other players to select [B. Join game]!\n\nPRESS A TO BEGIN GAME\npress B to cancel";
    updateDisplay(disp);
    while(digitalRead(a_button) && digitalRead(b_button)){delay(50);} //wait until they press A or B
    if(!digitalRead(b_button)){menu();}
  }
  if (!digitalRead(c_button)) {
    String lead = getLeaderboard();
    updateDisplay(lead);
    delay(2000);
    menu();
  }
  if (!digitalRead(b_button)) {
    updateDisplay("Waiting for leader...\nJK, feature yet to be implemented!\nSorry!");
    delay(2000);
    menu();
  }
  //if they select the A button, the method will just end and loop will begin, which is the game
}

void endGame() {
  String menuText = "Game over! Here are your final results:";
  updateDisplay(menuText);
  delay(3000);
  String lead = getLeaderboard();
  updateDisplay(lead);
  delay(2000);
  menuText = "Play again?\nA. YES\nB. NO";
  updateDisplay(menuText);
  while (digitalRead(a_button) && digitalRead(b_button)) {
    //delay until you press A or B
    delay(50);
  }
  if (!digitalRead(a_button)) {
    roundNum = 0;
    menu();
  }
  if (!digitalRead(b_button)) {
    menuText = "GAME OVER";
    updateDisplay(menuText);
    while(true){delay(9000);} //delay forever
  }
  //if they select the A button, the game will restart
}

String getQuestion() {
  //gets a question from sb1.py.
  String response = "";
  if (wifi.isConnected()) { //&& !wifi.isBusy()
    Serial.print("Getting question at t=");
    Serial.println(millis());
    String getPath = "/student_code/" + kerberos + "/dev1/sb1.py";
    String getParams = "sender=" + kerberos + "&deviceType=teensy";
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

String getLeaderboard() {
  //gets the leaderboard from sb2.py.
  String response = "";
  if (wifi.isConnected() && !wifi.isBusy()) {
    Serial.print("Getting leaderboard at t=");
    Serial.println(millis());
    String getPath = "/student_code/" + kerberos + "/dev1/sb2.py";
    String getParams = "sender=" + kerberos + "&deviceType=teensy";
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      Serial.print("Got response at t=");
      Serial.println(millis());
      Serial.println(response);
      response = response.substring(response.indexOf("<html>")+6,response.indexOf("</html>"));
    } else {
      Serial.println("No timely response");
    }
  }
  else{
    Serial.println("either wifi is disconnected or wifi is busy");
  }
  return response;
}

void postData(String questionID, int gameID, int roundNum, float deltaT, int correct) {
  //posts the user's answer, etc. to sb3.py.
  if (wifi.isConnected() && !wifi.isBusy()) {
    Serial.print("Posting data at t=");
    Serial.println(millis());
    String postPath = "/student_code/" + kerberos + "/dev1/sb3.py";
    //TODO: complete these get params
    String postParams = "sender=" + kerberos +
                        "&questionID=" + questionID +
                        "&deviceType=teensy&gameID=" + String(gameID) +
                        "&roundNum=" + String(roundNum) +
                        "&delta=" + String(deltaT, 3) +
                        "&isCorrect=" + correct;  //deltaT is set to 3 decimal places
                        // does not include currentScore
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
