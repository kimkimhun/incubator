
#include <Wire.h>
#include "SSD1306.h"

// intial variable

byte setting = 0;

// D0 -> SDA
// D1 -> SCL
SSD1306 display(0x3c, D1, D2);

// keypad set up
const int buttonPin[] = {D4, D5, D6, D7};
const int btn1 = D5;
const int btn2 = D4;
const int btn3 = D7;
const int btn4 = D6;
int buttonState = 0;
int start = 1;

// Relay setup
const int relayCH1 = 10;

void setup()
{

    // Initialising the UI will init the display too.
    display.init();

    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_24);

    // initialize key pad
    Serial.begin(115200);
    for (int x = 0; x < 4; x++)
    {
        pinMode(buttonPin[x], INPUT_PULLUP);
    }

    pinMode(relayCH1, OUTPUT);

}

void loop()
{
    if (start)
    {
        drawStartDisplay();
        start = 0;
    }
    keypadFunction();
}

void keypadFunction()
{
    for (int x = 0; x < 4; x++)
    {
        buttonState = digitalRead(buttonPin[x]);
        if (buttonState == LOW && buttonPin[x] == btn1)
        {
            if (setting == 0)
            {
                
            }
            else
            {

            }
            drawDisplay("Up");
            digitalWrite(relayCH1, HIGH);

        }
        if (buttonState == LOW && buttonPin[x] == btn2)
        {
            Serial.print("2");
            drawDisplay("Down");
            digitalWrite(relayCH1, LOW);

        }
        if (buttonState == LOW && buttonPin[x] == btn3)
        {
            Serial.print("3");
            drawDisplay("Cancel");
        }
        if (buttonState == LOW && buttonPin[x] == btn4)
        {
            Serial.print("4");
            drawDisplay("Ok");
        }
        delay(50);
    }
}

void drawStartDisplay()
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(65, 15, "Select Pin");
    display.display();
}

void drawDisplay(char menuMgs[])
{
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(65, 15, menuMgs);
    display.display();
}

void setSetting()
{
    setting = 1;
}

void clearSetting()
{
    setting = 0;
}