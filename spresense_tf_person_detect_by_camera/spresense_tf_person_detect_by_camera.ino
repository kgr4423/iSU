#include <Camera.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "person_detect_model.h" //使用する推論モデル

tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 100000;
uint8_t tensor_arena[kTensorArenaSize];

/* 切り取り・拡大縮小のパラメータ */
const int offset_x = 32;
const int offset_y = 12;
const int width    = 160;
const int height   = 120;
const int target_w = 96;
const int target_h = 96;
const int pixfmt   = CAM_IMAGE_PIX_FMT_YUV422;

int sitCount = 0;
int mode = 0;
int safe_width = 30;
int attention_width = 40;
int danger_width = 50;

/* カメラストリームのコールバック関数 */
/* カメラで画像がキャプチャされる度にこの関数が呼ばれる */
/* 人認識の推論もこの関数内で行われる */ 
void CamCB(CamImage img) {
  static uint32_t last_mills = 0;

  /* キャプチャ画像の取得 */
  if (!img.isAvailable()) {
      Serial.println("img is not available");
      return;
  }
  uint16_t* imgBuff = getImageData(img);

  /* 人の有無判定 */ 
  bool personDetected = detectPersonInImage();

  /* カウンタの更新 */
  int delta = sitcountUpdater(mode,personDetected);
  sitCount = sitCount + delta;

  /* モードの更新 */
  mode = determineMode(sitCount, safe_width, attention_width, danger_width);

  /* 警告処理 */
  //alert(mode); //未実装

  /* キャプチャ画像の表示 */
  disp_image(imgBuff, offset_x, offset_y, target_w, target_h, personDetected);
  
  /* 処理時間の測定と表示 */
  uint32_t current_mills = millis();
  uint32_t duration = current_mills - last_mills;
  Serial.println("duration = " + String(duration));
  last_mills = current_mills; 
}


void setup() {
  Serial.begin(115200);
  setup_display();
  setup_tensorflow();
  setup_camera();
}

void loop() {
}
