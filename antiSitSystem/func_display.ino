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
    tft.writeFillRect(0, 0, 320, 240, 0xFFFF);
}

void resetTextArea()
{
    tft.writeFillRect(160, 0, 170, 240, 0xFFFF);
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
    tft.print(mode);
}

void display_main(uint16_t *buf, bool result)
{
    // 警告枠表示
    uint16_t pixel_color;
    if(mode == 0 || mode == 1){
        pixel_color = ILI9341_BLACK;
    }else if(mode == 2){
        pixel_color = ILI9341_YELLOW;
    }else{
        pixel_color = ILI9341_RED;
    }
    tft.fillRect(0, 0, 224, 16, pixel_color);
    tft.fillRect(0, 0, 16, 240, pixel_color);
    tft.fillRect(0, 224, 224, 16, pixel_color);
    tft.fillRect(208, 0, 16, 240, pixel_color);
    //タイムバー
    tft.fillRect(224, 0, 32, 80, ILI9341_GREEN);
    tft.fillRect(224, 80, 32, 80, ILI9341_YELLOW);
    tft.fillRect(224, 160, 32, 80, ILI9341_RED);
    //ボタン表示
    tft.drawRect(257, 40, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 90, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 140, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 190, 62, 45, ILI9341_BLACK);
    tft.setCursor(267, 210);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("SETTING");
    //画像表示
    for (int y = 0; y < height; ++y)
    {
        for (int x = width; x > 0; --x)
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
            int LU = x*2;
            int RU = x*2 + 1;
            int LD = x*2     + width*2;
            int RD = x*2 + 1 + width*2;
            // 撮影画像表示部分
            disp[LU] = value; disp[RU] = value;
            disp[LD] = value; disp[RD] = value;
            if (result && (10 <= y) && (y <= 20) && (110 <= x) && (x <= 130))
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
    //タイムバー
    tft.fillRect(224, 0, 32, 80, ILI9341_GREEN);
    tft.fillRect(224, 80, 32, 80, ILI9341_YELLOW);
    tft.fillRect(224, 160, 32, 80, ILI9341_RED);
    //ボタン表示
    tft.setTextColor(ILI9341_BLACK);
    tft.drawRect(257, 40, 62, 45, ILI9341_BLACK);
    tft.drawRect(257, 90, 62, 45, ILI9341_BLACK);
    tft.setCursor(267, 110);
    tft.print("   UP   ");
    tft.drawRect(257, 140, 62, 45, ILI9341_BLACK);
    tft.setCursor(267, 160);
    tft.print("  DOWN   ");
    tft.drawRect(257, 190, 62, 45, ILI9341_BLACK);
    tft.setCursor(267, 210);
    tft.print("  END  ");
}
