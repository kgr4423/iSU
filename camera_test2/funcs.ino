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
// Cameraの初期化
//
void CameraSetup(){
  CamErr err;

  //パラメータなしのbegin()は、次のことを意味する
  //バッファの数 = 1、30FPS、QVGA、YUV 4:2:2 フォーマット
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  //ホワイトバランスを自動調整
  // Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }
 
  //静止画像に関するパラメータを設定する
  //次の場合ではQUADVGA（解像度）でJPEG（形式）となる
  // Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
     CAM_IMGSIZE_QUADVGA_H,
     CAM_IMGSIZE_QUADVGA_V,
     CAM_IMAGE_PIX_FMT_JPG);
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

  Serial.println("Camera Prepared");
}

//
// シリアルポートの初期化
//
void SerialPortSetup(){
  //シリアル通信を開始し、ポートが開くのを待つ
  Serial.begin(BAUDRATE);
  while (!Serial)
    {
      //シリアルポートが接続するのを待つ、Native USB Portのみ可
      ;
    }
  Serial.println("SerialPort Prepared");
}

//
// SDの初期化
//
void SdSetup(){
  while (!theSD.begin()) 
    {
      //SDカードが挿入されるまで待つ
      Serial.println("Insert SD card!");
    }
  Serial.println("SD Prepared");
}

//
// USB MSCの開始
//
void SdUsbMscSetup(){
  if (theSD.beginUsbMsc()) {
    Serial.println("USB MSC Failure!");
  } else {
    Serial.println("USB MSC Prepared");
  }
}

//
// RTCの初期化
//
void RtcSetup(){
  RTC.begin();
  RtcTime compiledDateTime(__DATE__, __TIME__);
  RTC.setTime(compiledDateTime);
  Serial.println("RTC Prepared");
}

//
// PCからのシリアル入力による時刻同期
//
void RtcSynchronize(){
  if (Serial.available()) {
    if(Serial.find(TIME_HEADER)) {
      uint32_t pctime = Serial.parseInt();
      RtcTime rtc(pctime);
      RTC.setTime(rtc);
    }
  }
}

//
// 現在の時刻を取得し、char型の配列に格納する関数
//
char* getCurrentTimeAsChar() {
    // 現在の時刻を取得
    RtcTime now = RTC.getTime();

    // 時刻を文字列に変換
    char timeStr[40]; // 十分なサイズのバッファを確保
    snprintf(timeStr, sizeof(timeStr), "%04d_%02d_%02d_%02dh%02dm%02ds",
             now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

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
// 静止画像の撮影と保存
//
void takeAndSavePicture(){
  //カメラデバイスからの画像データの受信待機
  CamImage img = theCamera.takePicture();

  //imgインスタンスが利用可能か確認
  if (img.isAvailable())
    {
      //ファイル名の生成
      char* filename = getCurrentTimeAsChar();
      filename = appendString(filename, ".JPG");
      filename = appendString("images/", filename);

      //新しく作るファイルと同じ名前の古いファイルを消去し、新しいファイルを作る
      theSD.remove(filename);
      File myFile = theSD.open(filename, FILE_WRITE);
      myFile.write(img.getImgBuff(), img.getImgSize());
      myFile.close();
      Serial.print("picture saved: ");
      Serial.println(filename);

      writeLogFile(filename);
    }
  else
    {
      Serial.println("Failed to take picture");
    }
}

//
// シリアルポートへの出力とログファイルへの書き込み
//
void writeLogFile(char* message){

  char* now = getCurrentTimeAsChar();

  File logFile = theSD.open("log.txt", FILE_WRITE);
  logFile.print(now);
  logFile.print(" : ");
  logFile.println(message);
  logFile.flush();
  logFile.close();

  delete[] now;

}