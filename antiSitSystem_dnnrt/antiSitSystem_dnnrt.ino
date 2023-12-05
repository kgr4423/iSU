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
const int offset_y = 24;
const int width = 96;
const int height = 96;
// const int target_w = 110;
// const int target_h = 110;
const int pixfmt = CAM_IMAGE_PIX_FMT_YUV422;

// その他
const int button_pin_4 = 4;
const int button_pin_5 = 5;
const int button_pin_6 = 6;
const int button_pin_7 = 7;
const int beep_pin = 14; // 音声出力ピンの設定
double sitCount = 0.0;
int last_process_mode = 4;
int process_mode = 4;
int safe_width = 10;
int attention_width = 20;
int danger_width = 30;
int person_score;
int no_person_score;
int safe_time_min = 1;
double safe_time_sec = safe_time_min * 60.0;
double time_from_start = 0;
char* display_mode = "main"; 
double duration;


// カメラストリームのコールバック関数
// カメラで画像がキャプチャされる度にこの関数が呼ばれる
// 人認識の推論もこの関数内で行われる
void CamCB(CamImage img)
{
    static uint32_t last_mills = 0;

    if(display_mode == "main"){
        // キャプチャ画像データの取得
        CamImage small;
        CamErr err = img.clipAndResizeImageByHW(small
                            , 0  , 0
                            , 191, 191
                            , 96 , 96);

        uint16_t* tmp = (uint16_t*)small.getImgBuff();
        // 人認識用に画像データを整形しTensorFlowの入力バッファにセット
        setImageForPersonDetection(tmp);
        // 人の有無判定
        bool personDetected = detectPersonInImage();

        // カウンタの更新
        double d = sitcountUpdater(personDetected);
        sitCount += d;
        // モードの更新
        determineMode(safe_width, attention_width, danger_width);

        // キャプチャ画像の表示
        display_main(img, personDetected); 
        // 警告処理
        alert();
    }else{
        // キャプチャ画像の表示
        display_setting();
    }

    // 各種パラメータの表示
    // displayText();
    
    // 処理時間の測定と表示
    uint32_t current_mills = millis();
    duration = (double)(current_mills - last_mills) / 1000;
    Serial.println("duration = " + String(duration));
    last_mills = current_mills;
    if(display_mode == "main"){
        time_from_start += duration;
        Serial.println("time = " + String(time_from_start));
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(button_pin_4, INPUT_PULLUP);
    pinMode(button_pin_5, INPUT_PULLUP);
    pinMode(button_pin_6, INPUT_PULLUP);
    pinMode(button_pin_7, INPUT_PULLUP);
    pinMode(beep_pin, OUTPUT);

    setup_display();
    setup_tensorflow();
    setup_camera();
}

void loop()
{
    int button_pin_4_state = digitalRead(button_pin_4);
    int button_pin_5_state = digitalRead(button_pin_5);
    int button_pin_6_state = digitalRead(button_pin_6);
    int button_pin_7_state = digitalRead(button_pin_7);

    if (button_pin_5_state == LOW && display_mode == "setting"){
        safe_time_min += 1;
        safe_time_sec = safe_time_min * 60.0;
        output_beep(beep_pin, 440, 50);
        resetRect(0, 70, 224, 170, 0xFFFF);
    }
    if (button_pin_6_state == LOW && display_mode == "setting"){
        safe_time_min -= 1;
        safe_time_sec = safe_time_min * 60.0;
        output_beep(beep_pin, 440, 50);
        resetRect(0, 70, 224, 170, 0xFFFF);
    }
    if (button_pin_7_state == LOW)
    {
        //スイッチ押したときの処理
        if(display_mode == "main"){
            display_mode = "setting";
        }else{
            display_mode = "main";
        }

        output_beep(beep_pin, 440, 100);
        delay(10);
        output_beep(beep_pin, 440, 100);

        resetRect(0, 0, 320, 240, 0xFFFF);
    }
    
    
}
