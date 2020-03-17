#include <LiquidCrystal.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
SoftwareSerial sim808(10,11);
char phone_no[] = "8500356218"; // replace with your phone no.
String data[5];
#define DEBUG true
String state,timegps,latitude,longitude;
String textMessage;
const int relay = 12;
const int flexPin = A2; //pin A0 to read analog input
const int buzzer = 9;
int value;
// Data wire is plugged into digital pin 2 on the Arduino
#define ONE_WIRE_BUS 2
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);
OneWire oneWire(ONE_WIRE_BUS);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

 int vi = A0;
int vib = 0;
int c = 0;
const int LED=13;

const int GSR=A1;
int threshold=0;
int sensorValue;

#define REPORTING_PERIOD_MS     1000
 
PulseOximeter pox;
uint32_t tsLastReport = 0,msLastReport = 0,gsrLastReported = 0,fsLastReported = 0,gsLastReported=0;
 

void onBeatDetected()
{
    Serial.println("Beat!");
}





void setup(){
  long sum=0;
    Serial.begin(115200);
     sim808.begin(19200);
      sensors.begin();  // Start up the library
pox.begin();
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
      pinMode(buzzer, OUTPUT);  //Set pin 3 as 'output'
  delay(1000);
  
  for(int i=0;i<500;i++)
  {
  sensorValue=analogRead(GSR);
  sum += sensorValue;
  delay(5);
  }
  threshold = sum/500;
   Serial.print("threshold =");
   Serial.println(threshold);

    Serial.print("Initializing pulse oximeter..");
    lcd.begin(16,2);
    lcd.print("Initializing...");
    delay(3000);
    lcd.clear();
 
    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
     pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
 
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
delay(2000);
  Serial.println("SIM900 ready...");
 sim808.print("AT+CSMP=17,167,0,0");  // set this parameter if empty SMS received
 delay(100);
 sim808.print("AT+CMGF=1\r"); 
 delay(400);

 sendData("AT+CGNSPWR=1",1000,DEBUG);
 delay(50);
 sendData("AT+CGNSSEQ=RMC",1000,DEBUG);
 delay(150);
 sim808.print("AT+CMGF=1\r"); 
  delay(100);
  // Set module to send SMS data to serial out upon receipt 
  sim808.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
 
  delay(2500);
  lcd.clear();
       //gpssms();

  }

void loop(){
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("BPM : ");
        lcd.print(pox.getHeartRate());
        lcd.setCursor(0,1);
        lcd.print("SpO2: ");
        lcd.print(pox.getSpO2());
        lcd.print("%");
        tsLastReport = millis();
    }
    if (millis() - msLastReport > 10000)
    {
      sensors.requestTemperatures(); 
       //print the temperature in Celsius
      Serial.print("Temperature: ");
      Serial.print(sensors.getTempCByIndex(0));
      Serial.print((char)176);//shows degrees character
      Serial.print("C  |  ");
      //print the temperature in Fahrenheit
      Serial.print((sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0);
      Serial.print((char)176);//shows degrees character
      Serial.println("F");
      delay(500);
      msLastReport = millis();
      pox.begin();
    }
    if (millis() - gsrLastReported > 10000)
    {
          gsr();
          pox.begin();
          gsrLastReported=millis();
    }

    if (millis() - fsLastReported >5000)
    {
          flex();
          fsLastReported = millis();
    }
    if (millis() - gsLastReported > 40000)
    {
      getgps();
      sendData( "AT+CGNSINF",1000,DEBUG);   
      delay(1000);
      pox.begin();
      gsLastReported = millis();
    }
    
}
  void gsr()
  {
    int temp;
  sensorValue=analogRead(GSR);
   vib = analogRead(A0);
   Serial.println("vib");
   Serial.print(vib);
  Serial.print("GSR sensorValue=");
  Serial.println(sensorValue);
  temp = threshold - sensorValue;
 
  if(abs(temp)>60)
  {
    sensorValue=analogRead(GSR);
    temp = threshold - sensorValue;
    if(abs(temp)>60){
    digitalWrite(LED,HIGH);
    Serial.println("heart attack alret");
    delay(3000);
    digitalWrite(LED,LOW);
    delay(1000);
    }
  }
  if (vib < 800)
  {
    c = c + 1;
  }
  if (c == 15)
  {
    Serial.println("pidisu vachindhi ro");
   /*oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(0, 0);
    oled.println("pidusu");*/
    c=0;
    delay(1500);
    
  }
  }
void gpssms()
{
sendTabData("AT+CGNSINF",1000,DEBUG);
  if (state !=0) {
    Serial.println("State  :"+state);
    Serial.println("Time  :"+timegps);
    Serial.println("Latitude  :"+latitude);
    Serial.println("Longitude  :"+longitude);

    sim808.print("AT+CMGS=\"");
    sim808.print(phone_no);
    sim808.println("\"");
    
    delay(300);

    sim808.print("http://maps.google.com/maps?q=loc:");
    sim808.print(latitude);
    sim808.print(",");
    sim808.print (longitude);
    delay(200);
    sim808.println((char)26); // End AT command with a ^Z, ASCII code 26
    delay(200);
    sim808.println();
    delay(20000);
    sim808.flush();
    
  } else {
    Serial.println("GPS Initialising...");
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
  if (debug) {
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude =data[4];  
  }
}


void getgps(void)
{
   sendData( "AT+CGNSPWR=1",1000,DEBUG); 
   sendData( "AT+CGNSSEQ=RMC",1000,DEBUG); 
}


String sendData(String command, const int timeout, boolean debug)
{
    String response = "";    
    sim808.println(command); 
    long int time = millis();   
    while( (time+timeout) > millis())
    {
      while(sim808.available())
      {       
        char c = sim808.read(); 
        response+=c;
      }  
    }    
    if(debug)
    {
      Serial.print(response);
    }    
    return response;
    
}
void flex()
{
  value = analogRead(flexPin);         //Read and save analog value from potentiometer
 // Serial.println(value);               //Print value
  value = map(value, 700, 900, 0, 255);//Map value 0-1023 to 0-255 (PWM)
  delay(100);
    Serial.println("flex=");
//Small delay
    lcd.clear();

  Serial.println(value);
  lcd.print("flex=");
  lcd.setCursor(0,1);
  lcd.print(value);
    delay(1000);

  if (value >= 400)
  {
    lcd.clear();
    Serial.println("ok");
      Serial.println("   ACTIVE");

digitalWrite(relay, HIGH); 
            digitalWrite(buzzer, HIGH); 
lcd.print("Shock Mode ");
  lcd.setCursor(0,1);
  lcd.print("   ACTIVE");

delay(2000);

      digitalWrite(relay, LOW); 
            digitalWrite(buzzer, LOW); 
         gpssms();

}
pox.begin();
}
