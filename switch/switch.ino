const int buttonPin = 7;  // the number of the pushbutton pin
 
void setup()
{
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT_PULLUP);  
  pinMode(LED0, OUTPUT);
}

void loop()
{
  int buttonState = digitalRead(buttonPin);
  if (buttonState == LOW) {
   digitalWrite(LED0, HIGH);
  } else {
   digitalWrite(LED0, LOW);
  }
}
