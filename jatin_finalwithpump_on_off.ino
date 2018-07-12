/* This code is developed by Jatin Kumar
 * For the Project (IOT Based Smart Irrigation System)
 * This project is built using Arduino uno, DHT22 Temperature and Humidity sensor Soil Moisture sensor, 
 * Flame sensor, Rain sensor and a relay module  
 * Project has a feature to send a text message on registered mobile no using twilio Api 
 * Thingspeak Cloud is used to plot the readings on the graph 
 */

#include <DHT_U.h>
#include <DHT.h>;
#include <SoftwareSerial.h>
//const int sensorMin = 0;     // sensor minimum
//const int sensorMax = 1024;  // sensor maximum
#define DHTPIN 8     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

// replace with your channel's thingspeak API key
String apiKey = "UBDMJ8SPIONV0P3D";

SoftwareSerial ser(2,4); // RX, TX using software serial for esp8266
int relayoutput=9;
int ledPin = 13;

//READINGS VARIABLE
float hum;  //Stores humidity value
float temp; //Stores temperature value
float rainsensor;
float Flamesensor;
float moisturesensor;
void readsensorsdata();
int pump=0;
int FLAG=0;

//function declaration
void readsensorsdata();
void esp_8266();
void readpump();

// this runs once
void setup() {                
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);    
  pinMode(relayoutput,OUTPUT);   

  // enable debug serial
  Serial.begin(9600); 
  // enable software serial
  ser.begin(115200);
  ser.println("AT+RST\r\n");
  delay(5000);
}


// the loop 

void loop() {
  
  // blink LED on board
  digitalWrite(ledPin, HIGH);   
  delay(200);               
  digitalWrite(ledPin, LOW);
  readsensorsdata();
  delay(200);
  readpump();
  delay(200); 
  esp_8266(); 
    
}

void readsensorsdata()
{
   moisturesensor=analogRead(A2);
   if(FLAG==0)
   {
   if(moisturesensor<500.00)
   {
     Serial.println("pump on");
     digitalWrite(relayoutput,LOW);
     pump=1;
   }
   else
   {
     Serial.println("pump off");
     digitalWrite(relayoutput,HIGH);
     pump=0;
   } 
   }
   rainsensor = analogRead(A0);
   // Serial.println("rain ");
  //  Serial.println(rainsensor);

   Serial.print("Rain Sensor: ");
  Serial.println(rainsensor);
  int range = map(rainsensor, sensorMin, sensorMax, 0, 3);
  
            // range value:
           switch (range) {
             case 0:    // Sensor getting wet
    //             Serial.println("Flood");
                      break;
             case 1:    // Sensor getting wet
                 Serial.println("Rain Warning");
                      break;
             case 2:    // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
                 Serial.println("Not Raining");
                      break;
                       }
    Serial.println(""); 
    
    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    
    //Print temp and humidity values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    Serial.println("");
    
    //Detect Flame
    Flamesensor= analogRead(A1);
   
    Serial.println("Flame Sensor");
    if(Flamesensor<990){
          Serial.println("Fire ");
                       }
    else{
          Serial.println("No fire Normal"); 
        }
          Serial.println("");
          Serial.println("");
 
  }
    


void readpump()
{
    int i=0;
    char c[100]; // returned data storage
  
   // TCP connection to thingspeak.com api server;
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
  if(ser.find("ERROR")){
    Serial.println("AT+CIPSTART error");
    ser.println("AT+CIPCLOSE\r\n");
    ser.println(cmd);
    return;
  }
  else if(ser.find("OK"))
  {
    Serial.println("connected to Thing speak API");
  }
  else if(ser.find("ALREA"))
  {
    Serial.println("Already connected");
  }
  //
  // prepare GET string
  String getStr = "GET /channels/432054/fields/7/last.json";
  getStr += "\r\n";


  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);
 
  if(ser.find(">")){
    ser.print(getStr);
    delay(1000);
    if(ser.find("+IPD"))
    {
      Serial.println("+IPD found");
      while(ser.available())
      { 
         Serial.println(ser.read());
        if(ser.find("field7"))
        { Serial.println("found field7: ");
          while(ser.available())  
         {
      c[i]=ser.read();
      i++;
         }
        }
      }
      if(c[3]=='1')
      {
        Serial.println("pump on by user ");
        FLAG=1;
        pump=1;
        digitalWrite(relayoutput,LOW);
      }
      
      else if(c[3]=='0')
      {
        
        FLAG=0;
        pump=0;
        Serial.println("pump off by user");
        digitalWrite(relayoutput,HIGH);
      }
   //   Serial.println(i);
    //  Serial.println(c);
      c[3]='/0';
      ser.flush();
    }
    else
    {
      Serial.println("+IPD not found");
      ser.flush();
    }
  }
  else{
    ser.println("AT+CIPCLOSE\r\n");
    // alert user
    Serial.println("AT+CIPCLOSE");
    Serial.println("read pump failed");
    
    ser.flush();
  }
  delay(2000);
}

void esp_8266()
{
 // convert to string
  char buf1[10];
  String tempp = dtostrf( temp, 4, 1, buf1);
  char buf2[10];
  String humm = dtostrf( hum, 4, 1, buf2);
  char buf3[10];
  String flame = dtostrf( Flamesensor, 4, 1, buf3);
  char buf4[10];
  String rain = dtostrf( rainsensor, 4, 1, buf4);
  char buf5[10];
  String moist = dtostrf( moisturesensor, 4, 1, buf5);
 // Serial.print(tempp);
 // Serial.println(" degre");
  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
   
  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  
  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(tempp);
  getStr +="&field2=";
  getStr += String(humm);
  getStr +="&field3=";
  getStr += String(flame);
  getStr +="&field4=";
  getStr += String(rain);
  getStr +="&field5=";
  getStr += String(moist);
  getStr +="&field6=";
  getStr += String(pump);
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);

  if(ser.find(">")){
    ser.print(getStr);
  }
  else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }
  // thingspeak needs 15 sec delay between updates
  delay(30000);  
}


