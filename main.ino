#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// ------------------------------------------------------------
// Include WiFi Libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Establish network credentials
const char* ssid     = "DruidMagic";
const char* password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;
// -------------------------------------------------------------------

const int AirValue = 847;   //you need to replace this value with Value_1
const int WaterValue = 450;  //you need to replace this value with Value_2
const int SensorPin = A0;

int soilMoistureValue = 0;
int soilMoisturePercent = 0;

int soilMoistValue = 0;
int soilMoistHigh = 0;
int soilMoistLow = 0;

int counter = 0;
bool isTemp = true;

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

float temperatureC;
float temperatureF;

// --------------------- THIS IS HIGH LOW TEMP VARIABLES
float tempValue = 72;
float tempHighValue = 75;
float tempLowValue = 70;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 0;     

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// LED Light
const int ledPin = 13;
uint8_t ledPercent = 0;

void setup()
{
  Wire.begin(PIN_WIRE_SDA,PIN_WIRE_SCL);
  // Start the Serial Monitor
  Serial.begin(9600);
  // Start the DS18B20 sensor
  sensors.begin();
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  pinMode(15, OUTPUT);

  //LED 
  pinMode(ledPin, OUTPUT); // LED 

  //MOIST - MOTOR
  pinMode(12, OUTPUT); 

  // WEB SERVER SETUP
  Serial.print("Establishing Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

void loop()
{
  sensors.requestTemperatures(); 
  temperatureC = sensors.getTempCByIndex(0);
  temperatureF = sensors.getTempFByIndex(0);

  // Serial Check
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");
  Serial.println(soilMoistureValue);

  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    //TURN OFF EVERYTHING
    digitalWrite(LED_BUILTIN,HIGH);         // Settings 
    digitalWrite(15, LOW);                  // Turn OFF Channel
    analogWrite(13, 0);                     // Turn OFF Light
    analogWrite(12, 0);                     // Turn OFF Motor
    update_LCD_Settings(lcd);               // Print LCD - Settings
    
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the LED on and off
            if (header.indexOf("GET /TEMP/plus") >= 0) {
              if (tempValue < 90)
                tempValue += 1;
              updateTemp();
              Serial.println((String)"Current Temp: "+tempValue);
            } else if (header.indexOf("GET /TEMP/minus") >= 0) {
              if (tempValue > 60)
                tempValue -= 1;
              updateTemp();
              Serial.println((String)"Current Temp: "+tempValue);
            } else if (header.indexOf("GET /LIGHT/plus") >= 0) {
              if (ledPercent < 100)
                ledPercent += 10;
              Serial.println((String)"Current Light: "+ledPercent);
            } else if (header.indexOf("GET /LIGHT/minus") >= 0) {
              if (ledPercent > 0)
                ledPercent -= 10;
              Serial.println((String)"Current Light: "+ledPercent);
            } else if (header.indexOf("GET /MOIST/plus") >= 0) {
              if (soilMoistValue < 100)
                soilMoistValue += 10;
              updateMoist();
              Serial.println((String)"Current Light: "+ledPercent);
            } else if (header.indexOf("GET /MOIST/minus") >= 0) {
              if (soilMoistValue > 0)
                soilMoistValue -= 10;
              updateMoist();
              Serial.println((String)"Current Light: "+ledPercent);
            } else if (header.indexOf("GET /FINISH") >= 0) {
              client.stop();
              break; // FIX THIS SECTION
            }
         
            
            // HTML Page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // Style Sheet
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".btn-group button { background-color: #04AA6D; border: 1px solid green; color: white; padding: 10px 24px; cursor: pointer; float: center;}");
            client.println(".btn-group button:not(:last-child) { border-right: none; }");
            client.println(".btn-group after { content:""; clear: both; display: table; }");
            client.println(".btn-group button:hover { background-color: #3e8e41; }");

            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}");
            
            client.println("</style></head>");
         
            // Web Page Heading
            client.println("<body><h1>Germination Chamber Settings</h1>");
            
            // Display Current Temperature State
            client.println((String)"<p>Current Temp:  " + tempValue + "</p>");

            client.println("<div class=\"btn-group\">");
            client.println("<a href=\"/TEMP/minus\"><button class=\"button_small\">-</button></a>");
            client.println("<a href=\"/TEMP/plus\"><button class=\"button_small\">+</button></a>");
            client.println("</div>");

            // Display Current Light State
            client.println("<br>");
            client.println("<br>");
            client.println((String)"<p>Current Light:  " + ledPercent + "</p>");

            client.println("<div class=\"btn-group\">");
            client.println("<a href=\"/LIGHT/minus\"><button class=\"button_small\">-</button></a>");
            client.println("<a href=\"/LIGHT/plus\"><button class=\"button_small\">+</button></a>");
            client.println("</div>");

            // Display Current Moist State
            client.println("<br>");
            client.println("<br>");
            client.println((String)"<p>Current Moisture:  " + soilMoistValue + "</p>");

            client.println("<div class=\"btn-group\">");
            client.println("<a href=\"/MOIST/minus\"><button class=\"button_small\">-</button></a>");
            client.println("<a href=\"/MOIST/plus\"><button class=\"button_small\">+</button></a>");
            client.println("</div>");

            // Close Button 
            client.println("<br>");
            client.println("<br>");
            client.println("<div class=\"btn-group\">");
            client.println("<a href=\"/FINISH\"><button class=\"button_small\">FINISH</button></a>");
            client.println("</div>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    lcd.clear(); // IDK maybe this will help
  }
  else {
    if (temperatureF < tempLowValue)
    {
      digitalWrite(15, HIGH);
    }
    else if (temperatureF > tempHighValue)
    {
      digitalWrite(15, LOW);
    }
    delay(500);
      
    soilMoisture();
    
    analogWrite(ledPin, ledPercent * 2.5);

    //Output Moisture
    if (soilMoistValue == 0){
      analogWrite(12, 0);
    }
    else if (soilMoisturePercent < soilMoistLow)
    {
      analogWrite(12, 25);  // Sketchy Value
    }
    else if (soilMoisturePercent > soilMoistHigh)
    {
      analogWrite(12, 0);
    }
    update_LCD(lcd, temperatureC, temperatureF);
    // SOME ISSUE WHEN LIGHTS ARE OFF AND MOISTURE ON = LCD IS FUCKED
  }
  
}

void soilMoisture()
{
  soilMoistureValue = analogRead(SensorPin);
  soilMoisturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if (soilMoisturePercent < 0)
  {
    soilMoisturePercent = 0;
  }
  else if (soilMoisturePercent > 100)
  {
    soilMoisturePercent = 100;
  }
}

void update_LCD(LiquidCrystal_I2C lcd, float v1, float v2)
{
  char display_string[40] = "";
  char float_str1[16] = "";
  char float_str2[16] = "";

  counter = counter + 1;
  if (counter > 10)
  {
    isTemp = !isTemp;
    counter = 0;
  }
  if (isTemp)
  {
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Temperature:");
    dtostrf(v1,4,2,float_str1);
    dtostrf(v2,4,2,float_str2);
    lcd.setCursor(0,1);
    sprintf(display_string, "%s F %s C", float_str2, float_str1);
    lcd.print(display_string);
  }
  else
  {
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Moisture:");
    lcd.setCursor(0,1);
    sprintf(display_string, "%d Percent", soilMoisturePercent);
    lcd.print(display_string);
  }
  
}

void update_LCD_Settings(LiquidCrystal_I2C lcd)
{
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print("Changing");
  lcd.setCursor(0,1);
  lcd.print("Settings");
}

void updateTemp()
{
  tempLowValue = tempValue - 2.0;
  tempHighValue = tempValue + 2.0;
}

void updateMoist()
{
  soilMoistLow = soilMoistValue - 2.0;
  soilMoistHigh = soilMoistValue + 2.0;
}
