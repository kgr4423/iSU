void setup()
{
    pinMode(14, OUTPUT);
    
}

void loop()
{
    tone(14, 440);
    delay(500);
    noTone(14);
    delay(500);

}