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
  // Synchronize with the PC time
  if (Serial.available()) {
    if(Serial.find(TIME_HEADER)) {
      uint32_t pctime = Serial.parseInt();
      RtcTime rtc(pctime);
      RTC.setTime(rtc);
    }
  }

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
          Serial.print(filename);

          //新しく作るファイルと同じ名前の古いファイルを消去し、新しいファイルを作る
          theSD.remove(filename);
          File myFile = theSD.open(filename, FILE_WRITE);
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();

          delete[] currentTime;
          delete[] filename;
        }
      else
        {
          //写真のサイズが割り当てられたメモリーサイズを超える場合がある
          //その場合より大きなメモリーを確保し、画像サイズを小さくする必要がある
          //より大きなメモリーを確保する方法
          //・setStillPictureImageFormat() で指定された jpgbufsize_divisor を減少させる
          //・Arduino IDEのツールメニューからメモリサイズを大きくする
          //画像のサイズを小さくする方法
          //・setJPEGQuality()でJPEGの画質を落とす
          Serial.println("Failed to take picture");
        }
    }
  else if (take_picture_count == TOTAL_PICTURE_COUNT)
    {
      Serial.println("End.");
      theCamera.end();
    }
  else
    {
      exit(0);
    }

  
  take_picture_count++;
}