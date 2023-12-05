
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <cstdint>
#include <SPI.h>
#include <EEPROM.h>
#include <DNNRT.h>
#include <SDHCI.h>
// カメラの撮影・撮影画像の表示関連
#include <Camera.h>
const int offset_x = 16;
const int offset_y = 16;
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


SDClass theSD;

// 撮影画像サイズ
#define CAM_IMG_W 96
#define CAM_IMG_H 96
// 推論画像サイズ
#define DNN_IMG_W 96
#define DNN_IMG_H 96
// 推論画像の左上角座標
#define IMG_DRAW_OFFSET_X 16
#define IMG_DRAW_OFFSET_Y 16

DNNRT dnnrt;
DNNVariable input(DNN_IMG_W *DNN_IMG_H);

static uint8_t const label[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// LCDモニタ関連の関数
#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
uint16_t disp[160 * 40];


// カメラストリームのコールバック関数
// カメラで画像がキャプチャされる度にこの関数が呼ばれる
// 人認識の推論もこの関数内で行われる
void CamCB(CamImage img)
{
    static uint32_t last_mills = 0;

    if(display_mode == "main"){
        // 人の有無判定
        bool personDetected = detectPersonInImage(img);

        // カウンタの更新
        double d = sitcountUpdater(personDetected);
        sitCount += d;
        // モードの更新
        determineMode(safe_width, attention_width, danger_width);

        // キャプチャ画像の表示
        display_main(getImageData(img), personDetected); 
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

    while (!theSD.begin())
    {
        Serial.println("insert sd card");
    }

    File nnbfile = theSD.open("model.nnb");
    int ret = dnnrt.begin(nnbfile);
    if (ret < 0)
    {
        Serial.println("dnnrt.begin failed");
        return;
    }

    setup_display();
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

// 推論処理
bool detectPersonInImage(CamImage img){
    //YUVをRGBに変更
    img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    uint16_t *tmp = (uint16_t *)img.getImgBuff();
    //RGBのうちGを抽出
    float *dnnbuf = input.data();
    for (int n = 0; n < DNN_IMG_H * DNN_IMG_W; ++n)
    {
        dnnbuf[n] = (float)((tmp[n] & 0x07E0) >> 5);
    }
    // 推論処理
    dnnrt.inputVariable(input, 0);
    dnnrt.forward();
    DNNVariable output = dnnrt.outputVariable(0);
    int index = output.maxIndex();

    bool personDetected;
    if(index == 0){
        personDetected = false;
    }else{
        personDetected = true;
    }

    return personDetected;
}