#include<SoftwareSerial.h>
SoftwareSerial gsm(7,8);

void setup()
{
  delay(10000);
  Serial.begin(9600);
  gsm.begin(9600);
  gsm.println("AT+CMGF=1");
  delay(1000);
  gsm.println("AT+CMGS=\"+919381569588\"\r"); //replace x by your number
  delay(1000);
  gsm.println("hello is test");
  delay(100);
  gsm.println((char)26);
  delay(1000);
}
void loop()
{

}
