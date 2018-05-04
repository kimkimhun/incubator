
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#include "DHT.h"
#include <EEPROM.h>

// Set Server and Firebase
#define FIREBASE_HOST "veckenz-smartfarm.firebaseio.com"
#define FIREBASE_AUTH "bRs74l5xbjNeGTu9lSwmHsMRuVigjQE7rtHkkA9o"
#define WIFI_SSID "F1"
#define WIFI_PASSWORD "12345678"
#define DHTPIN D3
#define DHTTYPE DHT21

// D0 -> SDA
// D1 -> SCL
SSD1306 display(0x3c, D1, D2);
// dht sensor
DHT dht(DHTPIN, DHTTYPE);

// Relay setup
const int relayCH1 = D6;
const int relayCH2 = D7;

// internet
boolean online = false;
boolean setupDisplay = true;
boolean setupFirebase = true;

int timeMonitor = 3;

// time rotate
int timeRotate = 7200;
int timeHoldRotate = 0;

// temperature
float tempMin = 37.5;
float tempMax = 39;
float humduty = 0;
  float temp = 0;
// EEPROM
int EEaddress = 0;

void setup()
{
  EEPROM.begin(32);
  pinMode(relayCH1, OUTPUT);
  pinMode(relayCH2, OUTPUT);
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);

  // Initial Pin Deley out
  pinMode(relayCH1, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  dht.begin();
   readData();
    digitalWrite(relayCH1, HIGH);
    digitalWrite(relayCH2, HIGH);
    drawSetUp("SETUP");
    delay(3000);
}

void loop()
{
  wConnect();
}

void wConnect() {
    if(WiFi.status() != WL_CONNECTED) {
      setupFirebase = true;
      offlineMode();
    } else {
      if(setupFirebase){
        Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
        setupFirebase = false;
        // readDatabase();
      }
      onlineMode();
    } 
   timeHoldRotate++;
   delay(1000);
}

void onlineMode()
{
  drawMainDisplay();
  if (WiFi.status() == WL_CONNECTED && timeHoldRotate % timeMonitor == 0) {
      writeDatabase(temp,humduty); 
      checkController();
  }
}

void offlineMode()
{
  drawMainDisplay();
}

// Display Code

void drawSetUp(char msg[])
{
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(65, 15, msg);
  display.display();
}

void drawMainDisplay()
{
  display.clear();
  humduty = dht.readHumidity();
  temp = dht.readTemperature();
  int seconds =  (timeRotate - timeHoldRotate) % 60 ;  
  int minutes =  (timeRotate - timeHoldRotate) / 60;
  int hours = minutes / 60;
  minutes = minutes % 60;
  control(temp,humduty);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 0, String(hours) + "H " + String(minutes) + "M " + String(seconds) + "S");
  display.drawString(64, 22, String(temp) + " C");
  display.drawString(64, 44, String(humduty) + " % " + String( WiFi.status() == WL_CONNECTED ? "Online":"Offline"));
  display.display();
  writeData();
}

void checkController()
{
  if(WiFi.status() == WL_CONNECTED){
    boolean check = Firebase.getBool("setting/contact/changeData");
    if (check) {
      check = false;
      tempMin = Firebase.getFloat("setting/tempmin/");
      tempMax = Firebase.getFloat("setting/tempmax/");
      timeRotate = Firebase.getInt("setting/rotate/") * 3600;
      if (tempMin == 0 || tempMax == 0 || timeRotate == 0) {
        readData();
      } else {
        timeHoldRotate = 0;
        Firebase.setBool("setting/contact/changeData", false);
        display.clear();
        display.drawString(64, 22, String("Update"));
        display.display();
        delay(2000); 
      }
    } 
   } 
}

void control(float temp, float humduty)
{
  if (temp < tempMax && temp > tempMin)
  {
    digitalWrite(relayCH1, HIGH);
  }
  else
  {
    digitalWrite(relayCH1, LOW);
  }
  rotate();
}

void rotate()
{
  if (timeHoldRotate == timeRotate)
  {
    drawSetUp("Rotate");
    digitalWrite(relayCH2, LOW);
    timeHoldRotate = 0;
    delay(5 * 1000);
    display.clear();
    // 7200
  }
  else
  {
    digitalWrite(relayCH2, HIGH);
  }
}

void writeDatabase(float temp, float humduty) {
  Firebase.setFloat("display/temperature/", temp);
  Firebase.setFloat("display/humduty/", humduty);
}

void readDatabase () {
   tempMin = Firebase.getFloat("setting/tempmin/");
   tempMax = Firebase.getFloat("setting/tempmax/");
   timeRotate = Firebase.getInt("setting/rotate/") * 3600;
}

void writeData() {
 EEPROM.put(EEaddress, timeHoldRotate);
 EEPROM.put(EEaddress + 4, timeRotate);
 EEPROM.put(EEaddress + 8, tempMin);
 EEPROM.put(EEaddress + 12, tempMax);
 EEPROM.commit();
}

void readData() {
  Serial.print("EEPROM contents at Address=0 is  : ");
  timeHoldRotate = 0;
  EEPROM.get(EEaddress,timeHoldRotate);
  EEPROM.get(EEaddress + 4,timeRotate);
  EEPROM.get(EEaddress + 8,tempMin);
  EEPROM.get(EEaddress + 12,tempMax);
  Serial.println(timeHoldRotate);
}

