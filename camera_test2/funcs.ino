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
}

//
// SDの初期化
//
void SdSetup(){
  while (!theSD.begin()) 
    {
      //SDカードが挿入されるまで待つ
      Serial.println("Insert SD card.");
    }
}

//
// Cameraの初期化
//
void CameraSetup(){
  CamErr err;

  //パラメータなしのbegin()は、次のことを意味する
  //バッファの数 = 1、30FPS、QVGA、YUV 4:2:2 フォーマット
  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS)
    {
      printError(err);
    }

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
}