#include <CircularBuffer.h>
#include <MAX30100.h>
#include <MAX30100_BeatDetector.h>
#include <MAX30100_Filters.h>
#include <MAX30100_PulseOximeter.h>
#include <MAX30100_Registers.h>
#include <MAX30100_SpO2Calculator.h>

#include <SoftwareSerial.h>
SoftwareSerial sim808(7,8);
PulseOximeter pox;

String data[5],URL;
bool instate=true;
#define DEBUG true
String HeartBeat,Spo2;
uint32_t tsLastReport = 0;
uint32_t msLastReport=0;
String state,timegps,latitude,longitude;
#define REPORTING_TIME 1000

void setup() {
 sim808.begin(9600);
 Serial.begin(9600);
 delay(50);
 sim808.print("AT+CSMP=17,167,0,0");  // set this parameter if empty SMS received
 delay(100);
 sim808.print("AT+CMGF=1\r"); 
 delay(400);
 sendData("AT+CGNSPWR=1",1000,DEBUG);
 delay(50);
 sendData("AT+CGNSSEQ=RMC",1000,DEBUG);
 delay(150);
 sendData("AT+SAPBR=3,1,\"contype\",\"GPRS\"",1000,DEBUG);
 sendData("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"",1000,DEBUG);
 sendData("AT+SAPBR=3,1,\"USER\",\" \"",1000,DEBUG);
 sendData("AT+SAPBR=3,1,\"PWD\",\" \"",1000,DEBUG);
 sendData("AT+SAPBR=1,1",1000,DEBUG);
 sendData("AT+HTTPINIT",1000,DEBUG);
 sendData("AT+HTTPPARA=\"CID\",1",1000,DEBUG);
  Serial.println("Initilizating pulse Oximeter ... ");
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
      if(millis()-msLastReport > 10000)
      {
        Serial.print("sending data");
        SENDGPS(HeartBeat,Spo2);
        msLastReport=millis();
        pox.begin();
      }
//  sendTabData("AT+CGNSINF",1000,DEBUG);
//  if (state !=0) {
//    Serial.println("State  :"+state);
//    Serial.println("Time  :"+timegps);
//    Serial.println("Latitude  :"+latitude);
//    Serial.println("Longitude  :"+longitude);
//    sim808.println("AT+CMGF=1");
//    sim808.print("AT+CMGS=\"");
//    delay(10000);
//    sim808.flush();
//    pox.begin();
//  } else {
//    Serial.println("GPS Initialising...");
//  }
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
  if (debug) {
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude =data[4];  
  }
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
  if (debug) {
     Serial.print(response);
     }
     return response;
}




void SENDGPS(String HeartBeat,String Spo2)
{

//  if(instate==true)
//  {
//    sim808.println("AT+SAPBR=3,1,\"contype\",\"GPRS\"");
//    delay(1000);
//    sim808.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");
//    delay(1000);
//    sim808.println("AT+SAPBR=3,1,\"USER\",\" \"");
//    delay(1000);
//    sim808.println("AT+SAPBR=3,1,\"PWD\",\" \"");
//    delay(1000);
//    sim808.println("AT+SAPBR=1,1");
//    delay(1000);
//    sim808.println("AT+HTTPINIT");
//    delay(1000);
//    sim808.print("AT+HTTPPARA=\"CID\",1");
//    delay(1000);
//    instate=false;
//  }
//  URL="http://maps.google.com/maps?q=loc:";
Serial.print("this");
Serial.println(Spo2);
sendData("AT+HTTPPARA=\"URL\",\"http://soldiercrate.000webhostapp.com/sol.php?heart="+HeartBeat+"&spo2="+Spo2+"\"",1000,DEBUG);
// sim808.print("AT+HTTPPARA=\"URL\",\"http://soldiercrate.000webhostapp.com/sol.php?tem=");
// sim808.print(HeartBeat); //>>>>>>  variable 1 (HeartBeat)
// sim808.print("&spo2=");
// sim808.print(Spo2);
//  sim808.print("\"");
//  sim808.write(0x0d);
//  sim808.write(0x0a);
  delay(1000);
  sim808.println("AT+HTTPACTION=0");
  delay(10000);
  Serial.print("SENT");
}
