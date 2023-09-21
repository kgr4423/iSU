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
// エラーメッセージの出力
//
void printError(enum CamErr err)
{
  Serial.print("Error: ");
  switch (err)
    {
      case CAM_ERR_NO_DEVICE:
        Serial.println("No Device");
        break;
      case CAM_ERR_ILLEGAL_DEVERR:
        Serial.println("Illegal device error");
        break;
      case CAM_ERR_ALREADY_INITIALIZED:
        Serial.println("Already initialized");
        break;
      case CAM_ERR_NOT_INITIALIZED:
        Serial.println("Not initialized");
        break;
      case CAM_ERR_NOT_STILL_INITIALIZED:
        Serial.println("Still picture not initialized");
        break;
      case CAM_ERR_CANT_CREATE_THREAD:
        Serial.println("Failed to create thread");
        break;
      case CAM_ERR_INVALID_PARAM:
        Serial.println("Invalid parameter");
        break;
      case CAM_ERR_NO_MEMORY:
        Serial.println("No memory");
        break;
      case CAM_ERR_USR_INUSED:
        Serial.println("Buffer already in use");
        break;
      case CAM_ERR_NOT_PERMITTED:
        Serial.println("Operation not permitted");
        break;
      default:
        break;
    }
}

//
// ビデオフレームをキャプチャしたときのCameraライブラリからのコールバック
//
void CamCB(CamImage img)
{

  //imgインスタンスが利用可能かどうか確認
  if (img.isAvailable())
    {

      //RGB565のデータが必要な場合は、画像データ形式をRGB565に変換する
      img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);

      //画像をディスプレイに表示したいときなどは、
      //getImgSize()やgetImgBuff()を使用することで、画像データを直接使用することができる
      Serial.print("Image data size = ");
      Serial.print(img.getImgSize(), DEC);
      Serial.print(" , ");

      Serial.print("buff addr = ");
      Serial.print((unsigned long)img.getImgBuff(), HEX);
      Serial.println("");
    }
  else
    {
      Serial.println("Failed to get video stream image");
    }
}

//
// 現在の時刻を取得し、char型の配列に格納する関数
//
char* getCurrentTimeAsChar() {
    // 現在の時刻を取得
    std::time_t now = std::time(nullptr);
    std::tm* timeInfo = std::localtime(&now);

    // 時刻を文字列に変換
    char timeStr[20]; // 十分なサイズのバッファを確保
    std::strftime(timeStr, sizeof(timeStr), "%Y_%m%d_%H-%M-%S", timeInfo);

    // 文字列をコピーしてchar型の配列に格納
    char* result = new char[strlen(timeStr) + 1]; // null終端文字も考慮
    std::strcpy(result, timeStr);

    return result;
}

//
// char型の配列に文字列を連結する関数
//
char* appendString(char* str, const char* suffix) {
    // 新しいサイズを計算
    size_t len1 = strlen(str);
    size_t len2 = strlen(suffix);
    char* result = new char[len1 + len2 + 1]; // null終端文字も考慮

    // 文字列を連結
    strcpy(result, str);
    strcat(result, suffix);

    return result;
}

//
// @brief カメラの初期化
//
void setup()
{

  CamErr err;

  //シリアル通信を開始し、ポートが開くのを待つ
  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      //シリアルポートが接続するのを待つ、Native USB Portのみ可
      ;
    }

  //SDの初期化
  while (!theSD.begin()) 
    {
      //SDカードが挿入されるまで待つ
      Serial.println("Insert SD card.");
    }

  //パラメータなしのbegin()は、次のことを意味する
  //バッファの数 = 1、30FPS、QVGA、YUV 4:2:2 フォーマット
  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  //ビデオストリームを開始
  //カメラデバイスからビデオストリームデータを受信した場合、カメラライブラリからCamCBを呼び出す
  // Serial.println("Start streaming");
  // err = theCamera.startStreaming(true, CamCB);
  // if (err != CAM_ERR_SUCCESS)
  //   {
  //     printError(err);
  //   }

  //ホワイトバランスを自動調整
  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }
 
  //静止画像に関するパラメータを設定する
  //次の場合ではQUADVGA（解像度）でJPEG（形式）となる
  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H,
     CAM_IMGSIZE_QUADVGA_V,
     CAM_IMAGE_PIX_FMT_JPG);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  /* Initialize SD */
  while (!theSD.begin()) {
    Serial.println("mount sd");
    ; /* wait until SD card is mounted. */
  }

  /* Start USB MSC */
  if (theSD.beginUsbMsc()) {
    Serial.println("USB MSC Failure!");
  } else {
    Serial.println("*** USB MSC Prepared! ***");
    Serial.println("Insert SD and Connect Extension Board USB to PC.");
  }

  // Initialize RTC at first
  RTC.begin();

  // Set the temporary RTC time
  RtcTime compiledDateTime(__DATE__, __TIME__);
  RTC.setTime(compiledDateTime);
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
          // char filename[16] = {0};
          // sprintf(filename, "PICTex%03d.JPG", take_picture_count);
          // Serial.print("Save taken picture as ");
          // Serial.print(filename);
          // Serial.println("");
          char* currentTime = getCurrentTimeAsChar();
          char* filename = appendString(currentTime, ".JPG");
          Serial.print(filename);

          //新しく作るファイルと同じ名前の古いファイルを消去し、新しいファイルを作る
          // char filename[16] = "images/c.JPG";
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