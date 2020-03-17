#include <SoftwareSerial.h>
#include <CircularBuffer.h>
#include <MAX30100.h>
#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>
#include<Wire.h>
long int HeartBeat,Spo2,preHeart,preSpo2;


SoftwareSerial sim808(7,8);

#define REPORTING_TIME 1000
#define Reporting_GPS_Time 10000

PulseOximeter pox;

uint32_t tsLastReport = 0;
uint32_t tsLastReportGps = 0;

void onBeatDetected(){
  Serial.println("Beat");
  }
void setup(){
  sim808.begin(115200);
    sim808.println("AT+SAPBR=3,1,\"contype\",\"GPRS\"");
    delay(1000);
    sim808.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
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
  Serial.begin(9600);
  Serial.println("Initilizating pulse Oximeter ... ");
  Serial.println(pox.begin());
  if (!pox.begin())
  {
    Serial.println("Failed");
    }
   else
   {
    Serial.println("SUCCESS");
    }

    pox.setOnBeatDetectedCallback(onBeatDetected);

  }

  void loop()
  {
    pox.update();
    if (millis() - tsLastReport > REPORTING_TIME)
    {
      Serial.print("HeartRate : ");
      
      HeartBeat=pox.getHeartRate();
      Serial.print(pox.getHeartRate());
      Serial.print("  SPO2 value : ");
      Serial.print(pox.getSpO2());
      Spo2=pox.getSpO2();
      Serial.println("%");
      tsLastReport=millis();
      }
      if( HeartBeat!=0 and Spo2 !=0 and (millis()-tsLastReportGps >Reporting_GPS_Time))
      {
        Serial.print("Went in if loop");
        
         sim808.print("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/update?api_key=FKTVF16UC9J15OEJ&field1=");
         sim808.print(HeartBeat); //>>>>>>  variable 1 (HeartBeat)
         sim808.print("&field2=");
  sim808.print(Spo2);
  sim808.print("\"");
  sim808.write(0x0d);
  sim808.write(0x0a);
  delay(5000);
  sim808.println("AT+HTTPACTION=0");
  delay(10000);
  tsLastReportGps=millis();
  pox.begin();
      }     
    }
