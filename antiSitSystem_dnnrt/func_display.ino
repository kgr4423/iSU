// LCDモニタ関連の関数
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

void display_main(CamImage disp_img, bool isPerson)
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
    tft.fillRect(0, 0, 208, 24, pixel_color);
    tft.fillRect(0, 0, 16, 240, pixel_color);
    tft.fillRect(0, 216, 224, 24, pixel_color);
    tft.fillRect(208, 0, 16, 240, pixel_color);
    //タイムバー
    display_timebar();
    //ボタン表示
    switch_box();
    tft.setTextColor(ILI9341_BLACK);
    setText("SETT ", 267, 198, 2);
    setText("-ING ", 267, 213, 2);
    //画像表示
    disp_img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);
    tft.drawRGBBitmap(offset_x, offset_y, (uint16_t *)disp_img.getImgBuff(), width*2, height*2);
    //認識アイコン表示
    if(isPerson){
        tft.fillRect(267, 5, 42, 30, ILI9341_RED);
    }else{
        tft.fillRect(267, 5, 42, 30, ILI9341_WHITE);
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

    int attention_time_min = (int)(safe_time_min * 0.25 / 0.7);
    int danger_time_min = (int)(safe_time_min * 0.05 / 0.7);
    tft.setTextSize(2);
    tft.setCursor(145, 140);
    tft.print(safe_time_min);
    tft.setCursor(145, 175);
    tft.print(attention_time_min);
    tft.setCursor(145, 205);
    tft.print(danger_time_min);

}

void display_timebar(){
    int all_time_sec = safe_time_sec / 0.7;
    uint16_t now_icon_pos = 240 * sitCount / all_time_sec;
    tft.fillRect(224, 0, 32, 168, ILI9341_GREEN);
    tft.fillRect(224, 168, 32, 60, ILI9341_YELLOW);
    tft.fillRect(224, 228, 32, 12, ILI9341_RED);
    tft.setTextSize(1);
    tft.setCursor(224, now_icon_pos);
    tft.print(" now");
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

