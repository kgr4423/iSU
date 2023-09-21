#include <SDHCI.h>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <cstring>
#include <Camera.h>
#include <RTC.h>

#define TIME_HEADER 'T' // Header tag for serial time sync message
#define BAUDRATE                (115200)
#define TOTAL_PICTURE_COUNT     (5)

SDClass  theSD;
int take_picture_count = 0;

//
// @brief 初期化
//
void setup()
{  
  SerialPortSetup();
  SdSetup();
  SdUsbMscSetup();
  RtcSetup();
  CameraSetup(); 
}

//
// @brief JPEG形式の写真を毎秒撮影する
//
void loop()
{
  RtcSynchronize();

  //静止画像を撮影するまで1秒待つ
  sleep(1);  

  //このサンプルコードは開始から毎秒1枚画像を撮影できる
  if (take_picture_count < TOTAL_PICTURE_COUNT)
    {
      Serial.println("call takeAndSavePicture()");
      takeAndSavePicture();
    }
  else if (take_picture_count == TOTAL_PICTURE_COUNT)
    {
      Serial.println("End.");
      theCamera.end();
    }
  
  take_picture_count++;
}