// 推論処理関連
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "person_detect_model.h" //使用する推論モデル

tflite::ErrorReporter *error_reporter = nullptr;
const tflite::Model *model = nullptr;
tflite::MicroInterpreter *interpreter = nullptr;
TfLiteTensor *input = nullptr;
TfLiteTensor *output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 100000;
uint8_t tensor_arena[kTensorArenaSize];

// カメラの撮影・撮影画像の表示関連
#include <Camera.h>
const int offset_x = 16;
const int offset_y = 16;
const int width = 96;
const int height = 104;
// const int target_w = 110;
// const int target_h = 110;
const int pixfmt = CAM_IMAGE_PIX_FMT_YUV422;

// その他
const int beep_pin = 14; // 音声出力ピンの設定
int sitCount = 0;
int last_mode = 4;
int mode = 4;
int safe_width = 10;
int attention_width = 20;
int danger_width = 30;
int person_score;
int no_person_score;

// カメラストリームのコールバック関数
// カメラで画像がキャプチャされる度にこの関数が呼ばれる
// 人認識の推論もこの関数内で行われる
void CamCB(CamImage img)
{
    static uint32_t last_mills = 0;

    // キャプチャ画像データの取得
    uint16_t *buf = getImageData(img);

    // 人認識用に画像データを整形しTensorFlowの入力バッファにセット
    setImageForPersonDetection(buf);
    // 人の有無判定
    bool personDetected = detectPersonInImage();

    // カウンタの更新
    int delta = sitcountUpdater(personDetected);
    sitCount = sitCount + delta;
    // モードの更新
    determineMode(safe_width, attention_width, danger_width);

    // キャプチャ画像の表示
    display_image(buf, personDetected);
    // 各種パラメータの表示
    // displayText();
    // 警告処理
    alert();

    // 処理時間の測定と表示
    uint32_t current_mills = millis();
    uint32_t duration = current_mills - last_mills;
    Serial.println("duration = " + String(duration));
    last_mills = current_mills;
}

void setup()
{
    Serial.begin(115200);
    pinMode(beep_pin, OUTPUT);

    setup_display();
    setup_tensorflow();
    setup_camera();
}

void loop()
{
}
