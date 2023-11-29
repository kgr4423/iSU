void setup_camera(){
  CamErr err = theCamera.begin(1, CAM_VIDEO_FPS_15, width, height, pixfmt);
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("camera begin err: " + String(err));
    return;
  }
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("start streaming err: " + String(err));
    return;
  }
}

uint16_t* getImageData(CamImage img){
    if (!img.isAvailable()) {
        Serial.println("img is not available");
        return;
    }
    uint16_t* buf = (uint16_t*)img.getImgBuff();

    return buf;
}

int sitcountUpdater(bool personDetected){
    int delta;
    switch(mode){
        case 0:
            if(personDetected){
                delta = 1;
            }else{
                delta = 0;
            }
            break;

        case 1:
            if(personDetected){
                delta = 1;
            }else{
                delta = -1;
            }
            break;

        case 2:
            if(personDetected){
                delta = 1;
            }else{
                delta = -1;
            }
            break;

        case 3:
            if(personDetected){
                delta = 0;
            }else{
                delta = 1;
            }
            break;

        case 4:
            if(personDetected){
                delta = 0;
            }else{
                delta = 0;
                sitCount = 0;
            }
            break;
    }

    return delta;
}

void determineMode(int safe_width, int attention_width, int danger_width){
    last_mode = mode;
    if(sitCount == 0){
        mode = 0;
    }else if(1 <= sitCount && sitCount <= safe_width){
        mode = 1;
    }else if(safe_width + 1 <= sitCount && sitCount <= attention_width){
        mode = 2;
    }else if(attention_width + 1 <= sitCount && sitCount <= danger_width){
        mode = 3;
    }else{
        mode = 4;
    }
}

void alert(){
    if(mode == 0 && last_mode == 4){
        tone(beep_pin, 300);
        delay(100);
        noTone(beep_pin);
        delay(10);
        tone(beep_pin, 300);
        delay(100);
        noTone(beep_pin);
    }
    if(mode == 2 && last_mode == 1){
        tone(beep_pin, 440);
        delay(100);
        noTone(beep_pin);
        delay(10);
        tone(beep_pin, 440);
        delay(100);
        noTone(beep_pin);
    }
    if(mode == 3 && last_mode == 2){
        tone(beep_pin, 600);
        delay(500);
        noTone(beep_pin);
        delay(500);
        tone(beep_pin, 600);
        delay(500);
        noTone(beep_pin);
        delay(500);
        tone(beep_pin, 600);
        delay(500);
        noTone(beep_pin);
    }
}
