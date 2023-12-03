// LCDモニタ関連の関数

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <cstdint>

#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
uint16_t disp[160 * 40];

void setup_display()
{
    tft.begin();
    tft.setRotation(3);
    tft.fillRect(0, 0, 320, 240, 0xFFFF);
}

void resetRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c)
{
    tft.fillRect(x, y, w, h, c);
}

void displayText()
{
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.setCursor(175, 54);
    tft.print(" #Person");
    tft.setCursor(175, 72);
    tft.print(" True :");
    tft.print(person_score);
    tft.setCursor(175, 90);
    tft.print(" False:");
    tft.print(no_person_score);
    tft.setCursor(175, 126);
    tft.print(" count:");
    tft.print(sitCount);
    tft.setCursor(175, 144);
    tft.print("  mode:");
    tft.print(process_mode);
}

void display_main(uint16_t *buf, bool result)
{
    // 警告枠表示
    uint16_t pixel_color;
    if(process_mode == 0 || process_mode == 1){
        pixel_color = ILI9341_BLACK;
    }else if(process_mode == 2){
        pixel_color = ILI9341_YELLOW;
    }else{
        pixel_color = ILI9341_RED;
    }
    tft.fillRect(0, 0, 224, 16, pixel_color);
    tft.fillRect(0, 0, 16, 240, pixel_color);
    tft.fillRect(0, 224, 224, 16, pixel_color);
    tft.fillRect(208, 0, 16, 240, pixel_color);
    //タイムバー
    display_timebar();
    //ボタン表示
    switch_box();
    tft.setTextColor(ILI9341_BLACK);
    setText("SETTING ", 267, 210, 1);
    //画像表示
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            // YUV422形式の画像データから輝度成分を抽出しRGB565へ変換
            uint16_t value = buf[y*width + x];
            uint16_t y_h = (value & 0xf000) >> 8;
            uint16_t y_l = (value & 0x00f0) >> 4;
            value = (y_h | y_l);
            uint16_t value6 = (value >> 2);
            uint16_t value5 = (value >> 3);
            value = (value5 << 11) | (value6 << 5) | value5;
            //ピクセル
            //左右反転のため「width*2 - 」としている
            int LU = width*2 - x*2;
            int RU = width*2 - x*2 + 1;
            int LD = width*2 - x*2     + width*2;
            int RD = width*2 - x*2 + 1 + width*2;
            // 撮影画像表示部分
            disp[LU] = value; disp[RU] = value;
            disp[LD] = value; disp[RD] = value;
            if (result && (10 <= y) && (y <= 20) && (10 <= x) && (x <= 20))
            {
                // 判定アイコン表示部分
                disp[LU] = ILI9341_RED; disp[RU] = ILI9341_RED;
                disp[LD] = ILI9341_RED; disp[RD] = ILI9341_RED;
            }
        }
        // 画像の表示
        tft.drawRGBBitmap(offset_x, offset_y + y*2, disp, width*2, 2);
    }
}

void display_setting(){
    resetRect(0, 70, 60, 50, 0xFFFF);
    resetRect(180, 70, 30, 50, 0xFFFF);
    //タイムバー
    display_timebar();
    //ボタン表示
    switch_box();
    tft.setTextColor(ILI9341_BLACK);
    setText(" > ", 270, 105, 2);
    setText(" < ", 270, 155, 2);
    setText("END ", 270, 205, 2);
    //設定画面表示
    setText("TIME SETTING", 5, 16, 3);
    setText(" Set 'safe-time'.", 10, 50, 2);
    tft.setTextSize(3);
    tft.setCursor(65, 90);
    tft.print(safe_time_min);
    setText("   min", 65, 90, 3);
    setText("<", 30, 90, 3);
    setText(">", 190, 90, 3);
    //パラメータ表示
    tft.fillRect(5, 140, 50, 26, ILI9341_GREEN);
    tft.fillRect(5, 170, 50, 26, ILI9341_YELLOW);
    tft.fillRect(5, 200, 50, 26, ILI9341_RED);
    setText("safe", 65, 140, 2);
    setText("atten", 65, 165, 2);
    setText("-tion", 65, 185, 2);
    setText("danger", 65, 205, 2);
    setText("min", 180, 140, 2);
    setText("min", 180, 175, 2);
    setText("min", 180, 205, 2);
    tft.setTextSize(2);
    tft.setCursor(145, 140);
    tft.print(safe_time_min);
    tft.setCursor(145, 175);
    tft.print(safe_time_min);
    tft.setCursor(145, 205);
    tft.print(safe_time_min);

}

void display_timebar(){
    tft.fillRect(224, 0, 32, 168, ILI9341_GREEN);
    tft.fillRect(224, 168, 32, 48, ILI9341_YELLOW);
    tft.fillRect(224, 216, 32, 24, ILI9341_RED);
}
void switch_box(){
    tft.drawRect(257, 40, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 90, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 140, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 190, 62, 45, ILI9341_BLACK);
}
void setText(char* text, uint16_t x, uint16_t y, uint8_t size){
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
}

