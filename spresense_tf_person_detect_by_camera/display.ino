#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define TFT_RST 8
#define TFT_DC  9
#define TFT_CS  10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS ,TFT_DC ,TFT_RST);
uint16_t disp[target_w*target_h];

/* indicator box */
int box_sx = 80;
int box_ex = 90;
int box_sy = 5;
int box_ey = 15;

void setup_display() {
  tft.begin(); 
  tft.setRotation(3);  
}

void disp_image(uint16_t* buf, int offset_x, int offset_y
              , int target_w, int target_h, bool result) {
  int n = 0; 
  for (int y = offset_y; y < offset_y + target_h; ++y) {
    for (int x = offset_x; x < offset_x + target_w; ++x) {
      uint16_t value = buf[y*width + x];
      uint16_t y_h = (value & 0xf000) >> 8;
      uint16_t y_l = (value & 0x00f0) >> 4;
      value = (y_h | y_l);       
      uint16_t value6 = (value >> 2);
      uint16_t value5 = (value >> 3);
      disp[n] = (value5 << 11) | (value6 << 5) | value5;      
      if (result && (y >= (offset_y + box_sy)) && (y <= (offset_y + box_ey)) 
        && (x >= (offset_x + box_sx)) && (x <= (offset_x + box_ex))) {
        disp[n] = ILI9341_RED;
      }
      ++n;
    }
  }
  tft.drawRGBBitmap(0, 0, disp, target_w, target_h); 
}
