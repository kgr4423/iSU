#include <Camera.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "person_detect_model.h"

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

/* カメラストリームのコールバック関数 */
/* 人認識の推論もこの関数内で行われる */ 
void CamCB(CamImage img) {
  static uint32_t last_mills = 0;

  if (!img.isAvailable()) {
    Serial.println("img is not available");
    return;
  }

  /* フレームメモリから画像データを取得 */
  uint16_t* buf = (uint16_t*)img.getImgBuff();   
  int n = 0; 
  for (int y = offset_y; y < offset_y + target_h; ++y) {
    for (int x = offset_x; x < offset_x + target_w; ++x) {
      /* YUV422データから輝度データ抽出 */
      uint16_t value = buf[y*width + x];
      uint16_t y_h = (value & 0xf000) >> 8;
      uint16_t y_l = (value & 0x00f0) >> 4;
      value = (y_h | y_l);  /* luminance data */
      /* グレスケデータをTensorFlowの入力バッファにセット */
      input->data.f[n++] = (float)(value)/255.0;
    }
  }

  Serial.println("Do inference");
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed");
    return;
  }

  

  /* 推論結果の取得 */
  bool result = false;
  int8_t person_score = output->data.uint8[1];
  int8_t no_person_score = output->data.uint8[0];
  Serial.print("Person = " + String(person_score) + ", ");
  Serial.println("No_person = " + String(no_person_score));
  if ((person_score > no_person_score) && (person_score > 10)) {
    digitalWrite(LED3, HIGH);
    result = true;
  } else {
    digitalWrite(LED3, LOW);
  }

  /* 人の有無判定 */

  /* カウンタの更新 */

  /* モードの更新 */

  /* 警告処理 */

  /* キャプチャ画像の表示 */
  disp_image(buf, offset_x, offset_y, target_w, target_h, result);
  
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
