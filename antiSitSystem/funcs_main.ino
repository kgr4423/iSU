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

double sitcountUpdater(bool personDetected)
{
    double delta;
    switch (process_mode)
    {
    case 0: // 開始点、0より小さくならないようにしている
        if (personDetected)
        {
            sitCount = 0.0;
            delta = duration;
        }
        else
        {
            sitCount = 0.0;
            delta = 0.0;
        }
        break;

    case 1: // 人が検出されればカウントプラス、そうでなければカウントマイナス
        if (personDetected)
        {
            delta = duration;
        }
        else
        {
            delta = -duration;
        }
        break;

    case 2: // 人が検出されればカウントプラス、そうでなければカウントマイナス
        if (personDetected)
        {
            delta = duration;
        }
        else
        {
            delta = -duration;
        }
        break;

    case 3: // 人が未検証のときだけカウントが進む
        if (personDetected)
        {
            delta = 0.0;
        }
        else
        {
            delta = duration;
        }
        break;

    case 4: // 終点、人未検出なら開始点に戻る
        if (personDetected)
        {
            delta = 0.0;
        }
        else
        {
            delta = 0.0;
            sitCount = 0.0;
        }
        break;
    }

    return delta;
}

void determineMode(int safe_width, int attention_width, int danger_width)
{
    last_process_mode = process_mode;

    double attention_time_sec = (double)(safe_time_sec * 0.25 / 0.7);
    double danger_time_sec = (double)(safe_time_sec * 0.05 / 0.7);
    double safe_point = safe_time_sec;
    double attention_point = safe_time_sec + attention_time_sec;
    double danger_point = safe_time_sec + attention_time_sec + danger_time_sec;
    if (sitCount <= 0)
    {
        // start mode
        process_mode = 0;
    }
    else if (0 < sitCount && sitCount <= safe_point)
    {
        // safe mode
        process_mode = 1;
    }
    else if (safe_point < sitCount && sitCount <= attention_point)
    {
        // attention mode
        process_mode = 2;
    }
    else if (attention_point < sitCount && sitCount <= danger_point)
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
