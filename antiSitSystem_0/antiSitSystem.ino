#include <SDHCI.h>
#include <stdio.h>
#include <iostream>
#include <ctime>
#include <cstring>
#include <Camera.h>
#include <RTC.h>

#define TIME_HEADER 'T' // Header tag for serial time sync message
#define BAUDRATE                (115200)

SDClass  theSD;
int camera_interval = 60;
int sitCount = 0;
char mode = "safe"

void setup()
{  
  SerialPortSetup();
  SdSetup();
  SdUsbMscSetup();
  RtcSetup();
  CameraSetup();

  writeLogFile("Setup done");
}

void loop()
{
  //静止画像を撮影するまでX秒待つ
  sleep(camera_interval);

  //静止画像の撮影と保存、画像の保存先パスを返す
  char* imagePath = takeAndSavePicture();

  //画像中の人の有無識別
  bool personDetected = detectPersonInImage(imagePath);

  //カウンタの更新
  int delta = sitCountUpdater(mode, personDetected);
  sitCount = sitCount + delta;

  //カウンタに応じたモード判定
  mode = determineMode(sitCount);

  //モードに応じた警告など
  alert(mode);

}