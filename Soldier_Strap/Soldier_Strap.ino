#include <SoftwareSerial.h>
#include <CircularBuffer.h>
#include <MAX30100.h>
#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>
#include <Wire.h>

#define TimeForSending 20000
#define REPORTING_PERIOD_MS 1000
#define TimeForGPS 40000
int busy;
uint32_t tsLastReport = 0;
uint32_t senddatatime = 0;
uint32_t gpstime;
long int LastReported;
int flag;
int keeper;
String Latitude,Longitude;
long int HeartBeat,Spo2;
String data[5];
int buttonState = 0;
int uday = 1;
int instate =0 ;

// 2 is For SOS

SoftwareSerial sim808(7,8); // RX,TX
PulseOximeter pox;

void onBeatDetected()
{
    Serial.println("Beat!");
}


void MAX30100()
{
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        tsLastReport = millis();
}

void SOS()
{
  sim808.println("AT+CMGF=1");
  delay(1000);
  sim808.println("AT+CMGS=\"+919381569588\"\r"); //replace x by your number
  delay(1000);
  sim808.println("I need Help And My Position is ");
  sim808.print("Latitude : ");
  sim808.println(Latitude);
  sim808.print("Longitude : ");
  sim808.println(Longitude);
  sim808.print("http://maps.google.com/maps?q=loc:");
  sim808.print(Latitude);
  sim808.print(",");
  sim808.println(Longitude);
  if (HeartBeat!=0 and Spo2 !=0)
  {
    sim808.print("Heart Rate : ");
    sim808.println(HeartBeat);
//    sim808.print("");
  }
  delay(100);
  sim808.println((char)26);
  delay(1000);
}


void GPS()
{
  sim808.println("AT+CGNSPWR=1");
  delay(500);
  Serial.println("GPS Connection Successful");
  sim808.println("AT+CGNSSEQ=RMC");
  Serial.println("GPS on");
  sim808.println("AT+CGNSINF");
  long int time = millis();
  int i = 0;
  while((time+3000) > millis())
  {
    while(sim808.available())
    {
      char c = sim808.read();
      if (c != ',')
      {
         data[i] +=c;
         delay(100);
      } 
      else
      {
        i++;  
      }
      if (i == 5) {
        delay(100);
        goto exitL;
      }
    }
  }
  exitL:
  if (true)
  {
    Latitude = data[3];
    Longitude =data[4];  
  }
  Serial.print("Latitude : ");
  Serial.print(Latitude);
  Serial.print("Longitude : ");
  Serial.print(Longitude);
  
}


void SendData()
{
  sim808.print("AT+HTTPPARA=\"URL\",\"http://soldiercrate.000webhostapp.com/sol.php?heart=");
  sim808.print(HeartBeat); //>>>>>>  variable 1 (HeartBeat)
  sim808.print("&spo2=");
  sim808.print(Spo2);
  sim808.print("\"");
  sim808.write(0x0d);
  sim808.write(0x0a);
  delay(5000);
  sim808.println("AT+HTTPACTION=0");
  delay(10000);
  pox.begin();
}

void setup()
{
  Serial.begin(9600);
  sim808.begin(115200);
  pinMode(2,INPUT);
  Serial.println("Sim Initiation Successful");
  Serial.println("Initializing pulse oximeter..");
  if (!pox.begin())
  {
    Serial.println("Failed To Initialize Pulse Oximeter");
  }
  else
  {
    Serial.println("Succesfully initialized Pulse Oximeter");
  }
    //pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    
    pox.setOnBeatDetectedCallback(onBeatDetected);
    
    sim808.println("AT+SAPBR=3,1,\"contype\",\"GPRS\"");
    delay(1000);
    
//    sim808.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
//    delay(1000);

    sim808.println("AT+SAPBR=3,1,\"APN\",\"imis\"");
    delay(1000);
    
    sim808.println("AT+SAPBR=3,1,\"USER\",\" \"");
    delay(1000);
    
    sim808.println("AT+SAPBR=3,1,\"PWD\",\" \"");
    delay(1000);
    
    sim808.println("AT+SAPBR=1,1");
    delay(1000);
    
    sim808.println("AT+HTTPINIT");
    delay(1000);
    
    sim808.print("AT+HTTPPARA=\"CID\",1");
    delay(1000);
    
  Serial.println("Initilizating pulse Oximeter ... ");
  pox.begin();

}



void loop() {

  pox.update();
  LastReported=millis();
  if (flag == 1)
  {
    senddatatime=LastReported;
    uday=0;
  }
  if (keeper == 1)
  {
    gpstime=LastReported;
    keeper=0;
  }
  
  buttonState = digitalRead(2);
  if (buttonState == 1)
  {
    Serial.println("Button Was Pressed");
    delay(1000);
    instate=1;
  }
  if(instate == 1)
  {
    SOS();
    delay(7000);
    Serial.println("SOS has been sent");
    instate=0;
    pox.begin();
  }

  if (millis() - tsLastReport > REPORTING_PERIOD_MS and (busy ==0)) 
  {
    busy=1;
    MAX30100();
    busy=0;
    
  }

  if(HeartBeat >=60 and HeartBeat <=100 and Spo2 !=0 and (millis()-senddatatime > TimeForSending) )
  {
    busy=1;
    Serial.print("Sending Data");
    SendData();
    delay(20000);
    senddatatime=millis();
    pox.begin(); 
    busy=0;   
  }
//  if (millis()-gpstime >TimeForGPS and (busy ==0))
//  {
//    busy=1;
//    GPS();
//    gpstime=millis();
//    busy=0;
//  }
  
    
  
}
