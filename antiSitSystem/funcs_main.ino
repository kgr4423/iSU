// コールバック関数内の関数まとめ
uint16_t* getImageData(CamImage img){
    if (!img.isAvailable()) {
        Serial.println("img is not available");
        return;
    }
    uint16_t* buf = (uint16_t*)img.getImgBuff();

    return buf;
}

void setImageForPersonDetection(uint16_t* buf){
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
}

bool detectPersonInImage(){
    Serial.println("Do inference");
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        Serial.println("Invoke failed");
        return;
    }

    person_score = output->data.uint8[1];
    no_person_score = output->data.uint8[0];
    Serial.print("Person = " + String(person_score) + ", ");
    Serial.println("No_person = " + String(no_person_score));

    bool result = false;
    if ((person_score > no_person_score) && (person_score > 10)) {
        digitalWrite(LED3, HIGH);
        result = true;
    } else {
        digitalWrite(LED3, LOW);
    }

    return result;
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
                sitCount = 0;
            }
            break;
    }

    return delta;
}

void determineMode(int safe_width, int attention_width, int danger_width){
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

