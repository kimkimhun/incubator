
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <Wire.h>
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#include "DHT.h"
#include <time.h>
#include <math.h>
// Set Server and Firebase
#define DHTPIN D3
#define DHTTYPE DHT21
#define FIREBASE_HOST "veckenz-smartfarm.firebaseio.com"
#define FIREBASE_AUTH "bRs74l5xbjNeGTu9lSwmHsMRuVigjQE7rtHkkA9o"
#define WIFI_SSID "HELLO_WORLD"
#define WIFI_PASSWORD "kimuchi999"

// initial Icon
#include "temp.h"

// intial variable

byte setting = 0;

// D0 -> SDA
// D1 -> SCL
SSD1306 display(0x3c, D1, D2);
// dht sensor
DHT dht(DHTPIN, DHTTYPE);
// keypad set up
//const int buttonPin[] = {D4, D5, D6, D7};
//const int btn1 = D5;
//const int btn2 = D4;
//const int btn3 = D7;
//const int btn4 = D6;
//int buttonState = 0;
//int start = 1;

// Relay setup
const int relayCH1 = D6;
const int relayCH2 = D7;

// timezone
int timezone = 7;
int dst = 0;
int count = 0;
struct tm *p_tm;
boolean internetSetup = true;
boolean online = false;
boolean setupDisplay = true;
boolean reConnect = false;
// time rotate
const int timeRotate = 7200;
int timeHoldRotate = 0;

// temperature
const int tempMin = 37.5;
const int tempMax = 39;
void wifiConnect()
{
  // Initial Server
  int countTime = 0;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  drawSetUp("connecting");
  delay(1000);
  while (WiFi.status() != WL_CONNECTED)
  {
    if (countTime >= 30)
    {
      offlineMode();
    }
    else
    {
      if (countTime % 3 == 0)
      {
        drawSetUp(".");
      }
      else if (countTime % 3 == 1)
      {
        drawSetUp("..");
      }
      else if (countTime % 3 == 2)
      {
        drawSetUp("...");
      }
      else
      {
        drawSetUp("....");
      }
      countTime++;
      delay(1000);
    }
  }
  drawSetUp("Connected");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  internetSetup = false;
  delay(1000);
}

void setup()
{
  pinMode(relayCH1, OUTPUT);
  pinMode(relayCH2, OUTPUT);
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);

  // Initial Pin Deley out
  pinMode(relayCH1, OUTPUT);

  dht.begin();
  // wifiConnect();
  delay(2000);
}

void loop()
{
  //  if(WiFi.status() != WL_CONNECTED){
  //    WiFi.
  //  } else {
  //   onlineMode();
  //  }

  autoMode();
}

void autoMode()
{
  if (setupDisplay)
  {
    drawSetUp("AUTO");
    delay(3000);
    digitalWrite(relayCH1, HIGH);
    digitalWrite(relayCH2, HIGH);
    setupDisplay = false;
  }
  drawMainDisplay();
}

void onlineMode()
{
  if (setupDisplay)
  {
    drawSetUp("Online");
    delay(3000);
    setupDisplay = false;
  }
  online = true;
  drawMainDisplay();
}

void offlineMode()
{
  if (setupDisplay)
  {
    drawSetUp("Offline");
    delay(3000);
    setupDisplay = false;
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    wifiConnect();
  }
  // reConnectWifi();
  online = false;
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
  float humduty = dht.readHumidity();
  float temp = dht.readTemperature();
  int seconds =  (timeRotate - timeHoldRotate) % 60 ;  
  int minutes =  (timeRotate - timeHoldRotate) / 60;
  int hours = minutes / 60;
  minutes = minutes % 60;
  control(temp,humduty);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 0, String(hours) + "H " + String(minutes) + "M " + String(seconds) + "S");
  display.drawString(64, 22, String(temp) + " C");
  display.drawString(64, 44, String(humduty) + " %");
  display.display();
  timeHoldRotate++;
  delay(1000);
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
