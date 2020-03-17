#include <CircularBuffer.h>
#include <MAX30100.h>
#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <SoftwareSerial.h>
SoftwareSerial sim808(7,8);
PulseOximeter pox;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2);

int buttonState;
float cel;
String HeartBeat,Spo2,latitude,longitude,data[5];
uint32_t tsLastReport = 0,msLastReport=0,gsLastReport=0;


void setup() {
  lcd.init();
  lcd.backlight();
 sim808.begin(9600);
 Serial.begin(9600);
 pinMode(2,INPUT);
 sendData("AT+CSMP=17,167,0,0",500,true);  // set this parameter if empty SMS received
 sendData("AT+CMGF=1",400,true); 
 sendData("AT+CGNSPWR=1",1000,true);
 sendData("AT+CGNSSEQ=RMC",1000,true);
 sendData("AT+SAPBR=3,1,\"contype\",\"GPRS\"",1000,true);
 sendData("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"",1000,true);
 sendData("AT+SAPBR=3,1,\"USER\",\" \"",1000,true);
 sendData("AT+SAPBR=3,1,\"PWD\",\" \"",1000,true);
 sendData("AT+SAPBR=1,1",1000,true);
 sendData("AT+HTTPINIT",1000,true);
 sendData("AT+HTTPPARA=\"CID\",1",1000,true);
  Serial.println(pox.begin());
  if (!pox.begin())
  {
    Serial.println("Failed");
    }
   else
   {
    Serial.println("SUCCESS MAX30100");
    }
}
void loop() {
  buttonState=digitalRead(2);
  if(buttonState == HIGH){
    SOS();
  }
  pox.update();
  cel = ( analogRead(1) / 1023.0) * 500;
    if (millis() - tsLastReport > 1500)
    {
      HeartBeat=pox.getHeartRate();
      Serial.print("HeartRate : ");
      Serial.print(HeartBeat);
      Spo2=pox.getSpO2();
      Serial.print("  SPO2 value : ");
      Serial.print(Spo2);
      Serial.println("%");
      Serial.print("TEMPRATURE = ");
      Serial.println(cel);
      Serial.println("*C");
      if(HeartBeat.toInt() >50 and HeartBeat.toInt() < 100 and Spo2.toInt() != 0)
      {
        lcd.setCursor(0, 0);
        lcd.print("HB:");
        lcd.print(HeartBeat);
        lcd.setCursor(7, 0);
        lcd.print(", SpO2:");
        lcd.print(Spo2.toInt());
        lcd.setCursor(0, 1);
        lcd.print("Temp:");
        lcd.print(cel);
        lcd.print("*C");
      }
      tsLastReport=millis();
    }
    if(millis()-msLastReport > 20000 and HeartBeat.toInt() >50 and HeartBeat.toInt() < 100 and Spo2.toInt() != 0 )
      {
        lcd.clear();
        lcd.print("Sending Data To DB");
        SENDDATA();
        msLastReport=millis();
        pox.begin();
      }
    if(millis() - gsLastReport > 60000 )
    {
      lcd.clear();
      lcd.print("Setting GPS");
      sendTabData("AT+CGNSINF",1000,true);
      pox.begin();
      gsLastReport=millis();
    }
}

void sendTabData(String command , const int timeout , boolean debug){
  sim808.println(command);
  long int time = millis();
  int i = 0;
  while((time+timeout) > millis()){
    while(sim808.available()){
      char c = sim808.read();
      if (c != ',') {
         data[i] +=c;
         delay(100);
      } else {
        i++;  
      }
      if (i == 5) {
        delay(100);
        goto exitL;
      }
    }
  }exitL:
  if (true) {
    latitude = data[3];
    longitude =data[4];
    data[3]="";
    data[4]="";
  }
  sendData("AT+HTTPPARA=\"URL\",\"http://soldiercrate.000webhostapp.com/soldier/sol.php?lat="+latitude+"&lon="+longitude+"\"",3000,true);
  delay(1000);
  sendData("AT+HTTPACTION=0",5000,true);
}
String sendData (String command , const int timeout ,boolean debug){
  String response = "";
  sim808.println(command);
  long int time = millis();
  int i = 0;
  while ( (time+timeout ) > millis()){
    while (sim808.available()){
      char c = sim808.read();
      response +=c;
    }
  }
  if (true) {
     Serial.print(response);
     }
     return response;
}




void SENDDATA()
{
sendData("AT+HTTPPARA=\"URL\",\"http://soldiercrate.000webhostapp.com/soldier/sol.php?heart="+HeartBeat+"&spo2="+Spo2+"&tem="+cel+"\"",3000,true);
  delay(1000);
sendData("AT+HTTPACTION=0",5000,true);
}

void SOS()
{
  lcd.clear();
  lcd.print("Sending SOS");
  sendData("AT+CMGF=1",1000,true);
  delay(1000);
  sendData("AT+CMGS=\"+919381569588\"\r",1000,true);
  delay(1000);
  sim808.println("I need Help And My Position is ");
  sim808.print("http://maps.google.com/maps?q=loc:");
  sim808.print(latitude);
  sim808.print(",");
  sim808.println(longitude);
  sim808.println((char)26);
  delay(1000);
  pox.begin();
}
