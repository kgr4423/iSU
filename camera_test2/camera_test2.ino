#include <SDHCI.h>
#include <stdio.h>

#include <Camera.h>

#define BAUDRATE                (115200)
#define TOTAL_PICTURE_COUNT     (10)




//
// @brief カメラの初期化
//
void setup()
{
  int take_picture_count = 0;
  SerialPortSetup();
  SdSetup();
  CameraSetup();
  
}

//
// @brief JPEG形式の写真を毎秒撮影する
//
void loop()
{
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
          char filename[16] = {0};
          sprintf(filename, "PICT%03d.JPG", take_picture_count);
    
          Serial.print("Save taken picture as ");
          Serial.print(filename);
          Serial.println("");

          //新しく作るファイルと同じ名前の古いファイルを消去し、新しいファイルを作る
          theSD.remove(filename);
          File myFile = theSD.open(filename, FILE_WRITE);
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();
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

  take_picture_count++;
}