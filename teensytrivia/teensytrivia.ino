#include <Wire.h>
#include <SPI.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266.h>

Adafruit_SSD1306 display(4);

#define wifiSerial Serial1          // for ESP chip
#define buzzerPin 10

//wifi/internet globals
String kerberos = "cnord";      //update with player's kerb/username
String MAC = "";
String resp = "";
uint32_t tLastIotReq = 0;       //time of last send/pull
uint32_t tLastIotResp = 0;      //time of last response
String domain = "iesc-s2.mit.edu";
int port = 80;

//game globals
uint32_t tQuestionStart = 0;   //used for determining delta
uint32_t tQuestionEnd = 0;     //used for determining delta
float delta = 0;               //time it takes for player to answer
int id = 0;                    //question ID, different for each question
bool isCorrect = 0;            //this gets changed for each question
int roundNum = 0;              //this gets incremented every loop
int gameID = 0;                //this gets changed later
int MAX_ROUNDS = 5;            //game ends after MAX_ROUNDS questions
String totalPlayers = "";
String playersAnswered = "";

#define SSID "MIT GUEST" // network SSID and password
#define PASSWORD ""

ESP8266 wifi = ESP8266(true);  //Change to "true" or nothing for verbose serial output
bool debug = false; //if debug is TRUE, then Serial monitor is required to open
                    //if debug is FALSE, then Serial monitor is not required (can just use battery)

//define buttons
int a_button = 4; //black
int b_button = 2; //black
int c_button = 8; //yellow
int d_button = 6; //blue

void setup() {
  if (debug) {
    Serial.begin(115200);
  }
  //button setup
  pinMode(a_button, INPUT);        //set pin to "listen" to signals
  pinMode(b_button, INPUT);
  pinMode(c_button, INPUT);
  pinMode(d_button, INPUT);
  pinMode(a_button, INPUT_PULLUP); //set input to be high normally
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

  //wifi setup
  if (debug) {
    Serial.println("TRYING");
  }
  wifi.begin();
  if (debug) {
    Serial.print("WIFI CONNECTING.....");
  }
  wifi.connectWifi(SSID, PASSWORD);
  while (!wifi.isConnected()); //wait for connection
  MAC = wifi.getMAC();
  if (debug) {
    Serial.println("CONNECTED");
  }
  //get extra message if there is one and clear it out
  if (wifi.hasResponse()) {
    resp = wifi.getResponse();
  }

  display.setCursor(0, 0);
  randomSeed(analogRead(0));//seed random number
  display.setTextSize(2);
  resetLeaderboard();       //DB is now empty
  menu();
}

void loop() {
  if (debug) {
    Serial.println("-------------begin loop-----------------");
  }
  wifi.clearRequest();
  if (roundNum == MAX_ROUNDS) {
    endGame(); //end the game if we've already played MAX_ROUNDS rounds
  }
  roundNum += 1;
  resp = getQuestion(); //get a question from our JSON file
  if (resp.equals("") && debug == true) {
    Serial.println("Response is empty.  Wifi connection was probably lost?");
  }

  int correct_pin = parseResponse(resp); //resp is now nicely formatted
  updateDisplay(resp);                   //display the question
  tQuestionStart = millis();             //start timer
  while (digitalRead(a_button) && digitalRead(b_button) && digitalRead(c_button) && digitalRead(d_button)) {
    //delay until you press a button again
    delay(50); //this means all deltas will be in 50-ms increments (adjust the delay to make shorter delta)
  }
  isCorrect = 0;
  if (!digitalRead(correct_pin)) {
    isCorrect = 1;
    tone(buzzerPin, 600, 250);           //happy buzzer tone
    delay(100);
    tone(buzzerPin, 600, 100);
    delay(100);
    tone(buzzerPin, 600, 100);
    tone(buzzerPin, 670, 700);
    updateDisplay("Correct answer!\nWaiting for others to finish...");
  }
  else {
    tone(buzzerPin, 440, 250);           //sad buzzer tone
    delay(100);
    tone(buzzerPin, 200, 900);
    updateDisplay("Sorry, wrong answer\n:(\nWaiting for others to finish...");
  }
  tQuestionEnd = millis();               //stop the timer and calculate delta
  delta = (tQuestionEnd - tQuestionStart) / 1000.;
  postData(id, gameID, roundNum, delta, isCorrect); //post the results of the question to the database
  while (getStatus() != "Y") //Wait until everyone has answered. getStatus also populates playersAnswered and totalPlayers
  {
    updateDisplay("Waiting for others to finish..." + playersAnswered + "/" + totalPlayers);
    delay(500); //just check every half second or so to see if everyone responded
  }
  String winner = getWinner(); //gets the username of the winner
  updateDisplay(winner + " won this round! Get ready for the next round...");
  delay(2000); //show winner for 2 seconds
}

void updateDisplay(String text) {
  //just displays a string of text on the OLED.
  if(debug){Serial.println("DISPLAYING");}
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(text);
  display.display();
}

void menu() {
  String menuText = "Teensy Trivia\n6.S08 Final Project\nJenny Xu and Claire Nord\n[A] Play\n[B] Leaderboard";
  updateDisplay(menuText);
  while (digitalRead(a_button) && digitalRead(b_button)) {
    //delay until you press A or B
    delay(50);
  }
  if (!digitalRead(a_button)) { //person selected A
    delay(50);                  //prevent switch debouncing
    gameID = random(1, 999);    //create a game ID
    String disp = "Joining game...Others can join at this time...\n[A] Ready to start\n[B] Return to main menu";
    updateDisplay(disp);
    roundNum = 0;              //start game off at 0 round
    delta = 0;                 //nonexistent delta
    isCorrect = 0;             //no score yet
    id = 1;                    //placeholder value
    postData(id, gameID, roundNum, delta, isCorrect);        //post these placeholder values to db so we know this person wants to play
    while (digitalRead(a_button) && digitalRead(b_button)) { //wait until they press A or B
      delay(50);
      if (!digitalRead(a_button)) {
        delay(50);                                           //prevent switch debouncing
        String players = getPlayers();                       //get a string of all the current players
        updateDisplay("Here are the \ncompetitors: " + players + "\n[A] Continue");
        while (digitalRead(a_button)) {
          delay(50);
        }
        updateDisplay("3....2....1... GO!!");                //3...2...1...GO sound
        tone(buzzerPin, 440, 200);
        delay(1000);
        tone(buzzerPin, 440, 200);
        delay(1000);
        tone(buzzerPin, 440, 200);
        delay(1000);
        tone(buzzerPin, 880, 1000);
        delay(1000);
        break;                                               //exit the while loop
      }
      if (!digitalRead(b_button)) {                          //pressed B
        delay(50);                                           //prevent switch debouncing
        menu();                                              //go back to menu
        break;                                               //exit the while loop
      }
    }
  }
  if (!digitalRead(b_button)) {                              //pressed B
    String lead = getLeaderboard();
    lead = lead.substring(0, lead.indexOf("<b>"));           //parse leaderboard
    updateDisplay(lead);                                     //display leaderboard for 2 seconds
    delay(2000);
    menu();
  }
  //if they select the A button, the method will just end and loop will begin, which is the game
}

void endGame() {
  //shows some stats at the end of the game.
  String menuText = "Game over!\nGetting final leaderboard...\n(press A to advance)";
  updateDisplay(menuText);
  while (digitalRead(a_button)) {
    delay(50); //show menuText until they press A
  }
  delay(50);                                                  //prevent switch debouncing
  String lead = getLeaderboard();
  String leaderboard = lead.substring(0, lead.indexOf("<b>")); //parse leaderboard
  updateDisplay(leaderboard);                                 //show leaderboard until they press A
  while (digitalRead(a_button)) {
    delay(50);
  }
  delay(50);                                                  //prevent switch debouncing
  menuText = "And the winner is....\n(Press A to advance)";
  updateDisplay(menuText);
  while (digitalRead(a_button)) {
    delay(50); //show menuText until they press A
  }
  delay(50);                                                  //prevent switch debouncing
  String winner = lead.substring(lead.indexOf("<w>") + 3, lead.indexOf("</w>")); //parse out winner
  String winScore = lead.substring(lead.indexOf("<s>") + 3, lead.indexOf("</s>")); //parse out score of winner
  updateDisplay(winner + " wins with a score of " + winScore + "!!\nCongrats!!\n(Press A to advance)");
  while (digitalRead(a_button)) {
    delay(50); //show winner score until they press A
  }
  delay(50);                                                  //prevent switch debouncing
  menuText = "Play again?\nA. YES\nB. NO";
  updateDisplay(menuText);
  while (digitalRead(a_button) && digitalRead(b_button)) {    //wait until they press A or B
    delay(50);
  }
  if (!digitalRead(a_button)) {                               //pressed A
    delay(50);                                                //prevent switch debouncing
    resetLeaderboard(); 
    menu();                                                   //go back to menu
  }
  if (!digitalRead(b_button)) {                               //pressed B
    delay(50);                                                //prevent switch debouncing
    menuText = "GAME OVER";
    updateDisplay(menuText);
    resetLeaderboard();                                       //DB is now empty
    while (true) {
      delay(9000);
    } //basically delay forever
  }
  //if they select the A button, this method will exit and the game will restart
}

String getStatus() {
  //checks sb3.py to see if the round is over
  String response = "";
  if (wifi.isConnected()) {
    if(debug){
      Serial.print("Getting status at t=");
      Serial.println(millis());
    }
    String getPath = "/student_code/" + kerberos + "/dev1/sb3.py";
    String getParams = "";
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      if(debug){
        Serial.print("Got response at t=");
        Serial.println(millis());
        Serial.println(response);
      }
      //populate playersAnswered and totalPlayers, then parse response
      playersAnswered = response.substring(response.indexOf("Round Status") + 14, response.indexOf("Round Status") + 15);
      totalPlayers = response.substring(response.indexOf("Round Status") + 16, response.indexOf("Round Status") + 17);
      response = response.substring(response.indexOf("<R>") + 3, response.indexOf("</R>"));
    }
    else if(debug) {
      Serial.println("No timely response");
    }
  }
  else if(debug){
    Serial.println("either wifi is disconnected or wifi is busy");
  }
  return response;
}

String getQuestion() {
  //gets a question from sb1.py.
  String response = "";
  if (wifi.isConnected()) {
    if(debug){
      Serial.print("Getting question at t=");
      Serial.println(millis());
    }
    String getPath = "/student_code/" + kerberos + "/dev1/sb1.py";
    String getParams = "sender=" + kerberos + "&deviceType=teensy";
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      if(debug){
        Serial.print("Got response at t=");
        Serial.println(millis());
        Serial.println(response);
      }
    }
    else if(debug) {
      Serial.println("No timely response");
    }
  }
  else if(debug){
    Serial.println("either wifi is disconnected or wifi is busy");
  }
  return response;
}

String getPlayers() {
  //gets the players of the game from sb2.py.
  //return format: "jennycxu cnord cnordy jennytest "
  String response = "";
  String players = "";
  if (wifi.isConnected() && !wifi.isBusy()) {
    if(debug){
      Serial.print("Getting players at t=");
      Serial.println(millis());
    }
    String getPath = "/student_code/" + kerberos + "/dev1/sb2.py";
    String getParams = "sender=" + kerberos; //does NOT include deviceType so that we get <tags></tags>
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      if(debug){
        Serial.print("Got response at t=");
        Serial.println(millis());
        Serial.println(response);
      }
      //take players out of response one by one
      int numPlayers = (response.substring(response.indexOf("Total players: ") + 15, response.indexOf("</h3>"))).toInt();
      String alph = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      for (int j = 0; j < numPlayers; j++) {
        String st = "<" + String(alph[j]) + ">";
        String nd = "</" + String(alph[j]) + ">";
        String player = response.substring(response.indexOf(st) + 3, response.indexOf(nd));
        players += player + " ";
      }
    }
    else if(debug) {
      Serial.println("No timely response.");
    }
  }
  else if(debug){
    Serial.println("Either wifi is disconnected or wifi is busy.");
  }
  return players;
}

String getWinner() {
  //gets the current winner from sb3.py.
  String response = "";
  if (wifi.isConnected() && !wifi.isBusy()) {
    if(debug){
      Serial.print("Getting winner at t=");
      Serial.println(millis());
    }
    String getPath = "/student_code/" + kerberos + "/dev1/sb3.py";
    String getParams = "sender=" + kerberos + "&deviceType=teensy";
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      if(debug){
        Serial.print("Got response at t=");
        Serial.println(millis());
        Serial.println(response);
      }
      //winner is bounded <w> </w> tags
      response = response.substring(response.indexOf("<W>") + 3, response.indexOf("</W>"));
    }
    else if(debug){
      Serial.println("No timely response.");
    }
  }
  else if(debug){
    Serial.println("either wifi is disconnected or wifi is busy.");
  }
  return response;
}

String resetLeaderboard() {
  //Clears the database using sb2.py.
  String response = "";
  if (wifi.isConnected() && !wifi.isBusy()) {
    if(debug){
      Serial.print("Getting leaderboard at t=");
      Serial.println(millis());
    }
    String getPath = "/student_code/" + kerberos + "/dev1/sb2.py";
    String getParams = "sender=" + kerberos + "&shouldDelete=T";
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      if(debug){
        Serial.print("Got response at t=");
        Serial.println(millis());
        Serial.println(response);
      }
      response = response.substring(response.indexOf("<html>") + 6, response.indexOf("</html>"));
    }
    else if(debug) {
      Serial.println("No timely response.");
    }
  }
  else if(debug){
    Serial.println("either wifi is disconnected or wifi is busy.");
  }
  return response;
}

String getLeaderboard() {
  //gets the leaderboard from sb2.py.
  String response = "";
  if (wifi.isConnected() && !wifi.isBusy()) {
    if(debug){
      Serial.print("Getting leaderboard at t=");
      Serial.println(millis());
    }
    String getPath = "/student_code/" + kerberos + "/dev1/sb2.py";
    String getParams = "sender=" + kerberos + "&deviceType=teensy";
    wifi.sendRequest(GET, domain, port, getPath, getParams);
    unsigned long t = millis();
    while (!wifi.hasResponse() && millis() - t < 10000); //wait for response
    if (wifi.hasResponse()) {
      response = wifi.getResponse();
      if(debug){
        Serial.print("Got response at t=");
        Serial.println(millis());
        Serial.println(response);
      }
      response = response.substring(response.indexOf("<html>") + 6, response.indexOf("</html>"));
    }
    else if(debug){
      Serial.println("No timely response.");
    }
  }
  else if(debug) {
    Serial.println("either wifi is disconnected or wifi is busy.");
  }
  return response;
}

void postData(String questionID, int gameID, int roundNum, float deltaT, int correct) {
  //posts the user's answer, etc. to sb3.py.
  if (wifi.isConnected() && !wifi.isBusy()) {
    if(debug){
      Serial.print("Posting data at t=");
      Serial.println(millis());
    }
    String postPath = "/student_code/" + kerberos + "/dev1/sb3.py";
    String postParams = "sender=" + kerberos +
                        "&questionID=" + questionID +
                        "&deviceType=teensy&gameID=" + String(gameID) +
                        "&roundNum=" + String(roundNum) +
                        "&delta=" + String(deltaT, 3) +
                        "&isCorrect=" + correct;  //deltaT is set to 3 decimal places
    wifi.sendRequest(POST, domain, port, postPath, postParams);
    String junk = wifi.getResponse();
    if(debug){ Serial.println("This data has been posted!");}
    isCorrect = -1;//if it has an answer needed to be pushed, push it and then restart
    unsigned long t = millis();
    while (wifi.isBusy() && millis() - t < 10000); //wait for response
  }
}

int parseResponse(String response) {
  //updates resp to be nicely formatted text for teensy
  //returns the int value of the button w/ correct answer
  //also shuffles order of answers
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

  //Shuffle all the answers
  String shuffledAnswers[4] = {
    ans_a, ans_b, ans_c, ans_d
  };
  for (int a = 0; a < 4; a += 1)
  {
    String tempString;
    int r = int(random(a, 3)); //random number from a to 3
    tempString = shuffledAnswers[a];
    shuffledAnswers[a] = shuffledAnswers[r];
    shuffledAnswers[r] = tempString;
  }
  ans_a = shuffledAnswers[0];
  ans_b = shuffledAnswers[1];
  ans_c = shuffledAnswers[2];
  ans_d = shuffledAnswers[3];
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

