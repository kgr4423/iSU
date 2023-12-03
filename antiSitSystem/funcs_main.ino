void setup_camera()
{
    CamErr err = theCamera.begin(1, CAM_VIDEO_FPS_15, width, height, pixfmt);
    if (err != CAM_ERR_SUCCESS)
    {
        Serial.println("camera begin err: " + String(err));
        return;
    }
    err = theCamera.startStreaming(true, CamCB);
    if (err != CAM_ERR_SUCCESS)
    {
        Serial.println("start streaming err: " + String(err));
        return;
    }
}

uint16_t *getImageData(CamImage img)
{
    if (!img.isAvailable())
    {
        Serial.println("img is not available");
        return;
    }
    uint16_t *buf = (uint16_t *)img.getImgBuff();

    return buf;
}

int sitcountUpdater(bool personDetected)
{
    int delta;
    switch (process_mode)
    {
    case 0: // 開始点、0より小さくならないようにしている
        if (personDetected)
        {
            delta = 1;
        }
        else
        {
            delta = 0;
        }
        break;

    case 1: // 人が検出されればカウントプラス、そうでなければカウントマイナス
        if (personDetected)
        {
            delta = 1;
        }
        else
        {
            delta = -1;
        }
        break;

    case 2: // 人が検出されればカウントプラス、そうでなければカウントマイナス
        if (personDetected)
        {
            delta = 1;
        }
        else
        {
            delta = -1;
        }
        break;

    case 3: // 人が未検証のときだけカウントが進む
        if (personDetected)
        {
            delta = 0;
        }
        else
        {
            delta = 1;
        }
        break;

    case 4: // 終点、人未検出なら開始点に戻る
        if (personDetected)
        {
            delta = 0;
        }
        else
        {
            delta = 0;
            sitCount = 0;
        }
        break;
    }

    return delta;
}

void determineMode(int safe_width, int attention_width, int danger_width)
{
    last_process_mode = process_mode;
    if (sitCount == 0)
    {
        // start mode
        process_mode = 0;
    }
    else if (1 <= sitCount && sitCount <= safe_width)
    {
        // safe mode
        process_mode = 1;
    }
    else if (safe_width + 1 <= sitCount && sitCount <= attention_width)
    {
        // attention mode
        process_mode = 2;
    }
    else if (attention_width + 1 <= sitCount && sitCount <= danger_width)
    {
        // danger mode
        process_mode = 3;
    }
    else
    {
        // end mode
        process_mode = 4;
    }
}

void alert()
{
    if (process_mode == 0 && last_process_mode == 4)
    {
        // ﾌﾞﾌﾞ
        tone(beep_pin, 300);
        delay(100);
        noTone(beep_pin);
        delay(10);
        tone(beep_pin, 300);
        delay(100);
        noTone(beep_pin);
    }
    if (process_mode == 2 && last_process_mode == 1)
    {
        // ﾋﾟﾋﾟ
        tone(beep_pin, 440);
        delay(100);
        noTone(beep_pin);
        delay(10);
        tone(beep_pin, 440);
        delay(100);
        noTone(beep_pin);
    }
    if (process_mode == 3 && last_process_mode == 2)
    {
        // ﾋﾟｰﾋﾟｰﾋﾟｰ
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

void output_beep(uint8_t pin, unsigned int freq, unsigned long delay_time){
    tone(pin, freq);
    delay(delay_time);
    noTone(pin);
}
