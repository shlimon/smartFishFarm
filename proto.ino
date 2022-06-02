#include <Wire.h>     //I2C for LCD
#include "Waveshare_LCD1602_RGB.h"  //waveshare library for lcd
#include <Servo.h>
#include <SPI.h>
#include <WiFiNINA.h>

/* wifi set up*/
char ssid[] = "DODO-4FE0";
char pass[] = "XE4EQPZ28L";
int keyIndex = 0;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

String readString;

/* pH level sensor*/
int SensorPin = A1;        // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue;  //Store the average value of the sensor feedback
int buf[10],temp;             //for storing the average reading of the pH
float phValue;
unsigned long previousPHMesurement =0;
unsigned long pHMesurementInterval =6000;

/* LCD*/
Waveshare_LCD1602_RGB lcd(16,2); //16 characters and 2 lines of show
int g,b,t=0;
int r = 255;

/* FISH FEEDER*/
int servoPin =9;
int closeFeeder=0;
int openFeeder=180;
Servo fishFeeder;
unsigned long previousFeedingTime =0;
unsigned long fishFeedingInterval =6000;


void setup()
{  
  fishFeeder.attach(servoPin);
  lcd.init();
  Serial.begin(9600);  
  Serial.println("Ready");    //Test the serial monitor

   //connecting to the wifi & establishing server
   while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  server.begin();

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  
}

void loop()
{
  unsigned long currentTime = millis();

  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("new client");

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (readString.length() < 100)
        {
          readString += c;
          Serial.write(c);
          
          if (c == '\n') {
            client.println("<a href=\"/?feed\"\">Feed the fishes</a>");

            delay(1);
            
            if(readString.indexOf("?feed") > 0)
            {
              fishFeed();
              delay(1);
            }
                       
            readString="";

            delay(1);
            client.stop();
            Serial.println("client disonnected");
          }
        }
      }
    }
  }

  

  if(currentTime - previousPHMesurement >= pHMesurementInterval)  //interval for pH mesurement measurement
  {
     get_pH();
    Serial.print("    pH:");  
    Serial.print(phValue);
    Serial.println(" ");
    previousPHMesurement = currentTime;
    display(phValue);
    
  }
  
  if(currentTime - previousFeedingTime >= fishFeedingInterval)  //interval for feeding fish
  {
    fishFeed();
    previousFeedingTim = currentTime;
  }


  
}

void fishFeed(){
   fishFeeder.write(openFeeder);
   delay(1000);
   fishFeeder.write(closeFeeder);
   delay(1000);
}

void display(float phValue){
  String result = String(phValue);  //converting the float value to char
  char charResult[6];
  result.toCharArray(charResult,6);
  Serial.print(charResult);
  
  lcd.setCursor(0,0);
  lcd.send_string("pH level:");
  
  lcd.setCursor(10,0);
  lcd.send_string(charResult);
  lcd.setRGB(r,g,b);
}

void get_pH(){
  delay(100);
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  
   
}
