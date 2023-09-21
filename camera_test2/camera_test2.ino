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

      //静止画像を撮影
      //ビデオストリームとは異なり、カメラデバイスからの画像データの受信を待ちます
      Serial.println("call takePicture()");
      CamImage img = theCamera.takePicture();

      //imgインスタンスが利用可能か確認
      //もしエラーが発生したらimgは利用できない
      if (img.isAvailable())
        {
          //ファイル名の生成
          char* currentTime = getCurrentTimeAsChar();
          char* filename = appendString(currentTime, ".JPG");

          //新しく作るファイルと同じ名前の古いファイルを消去し、新しいファイルを作る
          theSD.remove(filename);
          File myFile = theSD.open(filename, FILE_WRITE);
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();
          Serial.print("picture saved: ");
          Serial.println(filename);

          delete[] currentTime;
          delete[] filename;
        }
      else
        {
          Serial.println("Failed to take picture");
        }
    }
  else if (take_picture_count == TOTAL_PICTURE_COUNT)
    {
      Serial.println("End.");
      theCamera.end();
    }
  
  take_picture_count++;
}