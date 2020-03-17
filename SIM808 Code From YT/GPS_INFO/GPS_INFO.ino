#include <SoftwareSerial.h>
#define DEBUG true


SoftwareSerial sim808(7,8); 
        
void setup()
{
  Serial.begin(9600);
 sim808.begin(9600); 
}

void loop()
{
   getgps();
   while(1)
   {
        sendData( "AT+CGNSINF",1000,DEBUG);   
        delay(1000);
        
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
