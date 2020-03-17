int buttonState;
void setup()
{
  Serial.begin(9600);
  pinMode(2,INPUT);
}
void loop()
{
  buttonState=digitalRead(2);
  if(buttonState == HIGH)
  {
    Serial.print("Flag Keeper");
  }
}
