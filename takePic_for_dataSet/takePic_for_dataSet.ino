#include <SDHCI.h>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <cstring>
#include <Camera.h>
#include <RTC.h>

#define TIME_HEADER 'T' // Header tag for serial time sync message
#define BAUDRATE (115200)
#define TOTAL_PICTURE_COUNT (1)

#define CAMIMAGE_SIZE 110

const int offset_x = 25;
const int offset_y = 5;
const int width = 160;
const int height = 120;
const int target_w = 110;
const int target_h = 110;
const int pixfmt = CAM_IMAGE_PIX_FMT_YUV422;

SDClass theSD;
int take_picture_count = 0;

const int takePicButtonPin = 4; // the number of the pushbutton pin
const int endButtonPin = 7;     // the number of the pushbutton pin
const int beep_pin = 14;        // 音声出力ピンの設定

void setup()
{
    SerialPortSetup();
    SdSetup();
    RtcSetup();
    CameraSetup();
    setup_display();

    writeLogFile("Setup done");

    pinMode(takePicButtonPin, INPUT_PULLUP);
    pinMode(endButtonPin, INPUT_PULLUP);
    pinMode(LED0, OUTPUT);
}

void loop()
{
    int takePicButtonState = digitalRead(takePicButtonPin);
    int endButtonState = digitalRead(endButtonPin);

    if (takePicButtonState == LOW)
    {
        tone(beep_pin, 440);
        delay(100);
        noTone(beep_pin);
        delay(10);
        tone(beep_pin, 440);
        delay(100);
        noTone(beep_pin);

        digitalWrite(LED0, HIGH);
        takeAndSavePicture();

        sleep(1);
    }
    else
    {
        digitalWrite(LED0, LOW);
    }

    if (endButtonState == LOW)
    {
        digitalWrite(LED0, LOW);
        theCamera.end();

        tone(beep_pin, 300);
        delay(100);
        noTone(beep_pin);
        delay(10);
        tone(beep_pin, 300);
        delay(100);
        noTone(beep_pin);

        writeLogFile("End");
        SdUsbMscStart();
    }
}