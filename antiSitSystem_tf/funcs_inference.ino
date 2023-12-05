void setup_tensorflow()
{
    tflite::InitializeTarget();
    memset(tensor_arena, 0, kTensorArenaSize * sizeof(uint8_t));

    // ロギング設定.
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    // モデルを使用可能なデータ構造にマッピングする
    model = tflite::GetModel(model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        Serial.println("Model provided is schema version " + String(model->version()) + " not equal " + "to supported version " + String(TFLITE_SCHEMA_VERSION));
        return;
    }
    else
    {
        Serial.println("Model version: " + String(model->version()));
    }
    // 必要なすべての操作の実装を引っ張ってくる
    static tflite::AllOpsResolver resolver;

    // モデルを実行するためのインタプリタを構築する
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    // モデルのテンソルのためにtensor_areaからメモリを確保する
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        Serial.println("AllocateTensors() failed");
        return;
    }
    else
    {
        Serial.println("AllocateTensor() Success");
    }

    size_t used_size = interpreter->arena_used_bytes();
    Serial.println("Area used bytes: " + String(used_size));
    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("Model input:");
    Serial.println("dims->size: " + String(input->dims->size));
    for (int n = 0; n < input->dims->size; ++n)
    {
        Serial.println("dims->data[" + String(n) + "]: " + String(input->dims->data[n]));
    }

    Serial.println("Model output:");
    Serial.println("dims->size: " + String(output->dims->size));
    for (int n = 0; n < output->dims->size; ++n)
    {
        Serial.println("dims->data[" + String(n) + "]: " + String(output->dims->data[n]));
    }

    Serial.println("Completed tensorflow setup");
    digitalWrite(LED0, HIGH);
}

void setImageForPersonDetection(uint16_t *buf)
{
    int n = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            /* YUV422データから輝度データ抽出 */
            uint16_t value = buf[y * width + x];
            uint16_t y_h = (value & 0xf000) >> 8;
            uint16_t y_l = (value & 0x00f0) >> 4;
            value = (y_h | y_l); /* luminance data */
            /* グレスケデータをTensorFlowの入力バッファにセット */
            input->data.f[n++] = (float)(value) / 255.0;
        }
    }
}

bool detectPersonInImage()
{
    // 人の有無の推論処理を行い結果を返す
    Serial.println("Do inference");
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk)
    {
        Serial.println("Invoke failed");
        return;
    }

    person_score = output->data.uint8[1];
    no_person_score = output->data.uint8[0];
    Serial.print("Person = " + String(person_score) + ", ");
    Serial.println("No_person = " + String(no_person_score));

    bool result = false;
    if ((person_score > no_person_score) && (person_score > 10))
    {
        digitalWrite(LED3, HIGH);
        result = true;
    }
    else
    {
        digitalWrite(LED3, LOW);
    }

    return result;
}