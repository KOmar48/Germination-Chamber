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

// Auxiliar variables to store the current output state
String outputLEDState = "off";          //LED
String outputGPIO5State = "off";        //D1 
String outputGPIO4State = "off";        //D2

// Assign output variables to GPIO pins
const int outRelay1 = 5;                //D1
const int outRelay2 = 4;                //D2

// --------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  // Initialize the variables as outputs
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(outRelay1, OUTPUT);
  pinMode(outRelay2,OUTPUT);
  
  // Assign output variables appropriate values
  digitalWrite(LED_BUILTIN, HIGH);      // Cathode
  digitalWrite(outRelay1, LOW);
  digitalWrite(outRelay2, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Establishing Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

// -------- WHILE ------------
void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
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
            if (header.indexOf("GET /LED/on") >= 0) {
              Serial.println("LED on");
              outputLEDState = "on";
              digitalWrite(LED_BUILTIN, LOW);
            } else if (header.indexOf("GET /LED/off") >= 0) {
              Serial.println("LED off");
              outputLEDState = "off";
              digitalWrite(LED_BUILTIN, HIGH);
            } 
            // turn the GPIO pins on and off
            else if (header.indexOf("GET /Relay1/on") >= 0) {
              Serial.println("Relay1 on");
              outputGPIO5State = "on";
              digitalWrite(outRelay1, HIGH);
            }
            else if (header.indexOf("GET /Relay1/off") >= 0) {
              Serial.println("Relay1 off");
              outputGPIO5State = "off";
              digitalWrite(outRelay1, LOW);
            }
            else if (header.indexOf("GET /Relay2/on") >= 0) {
              Serial.println("Relay2 on");
              outputGPIO4State = "on";
              digitalWrite(outRelay2, HIGH);
            }
            else if (header.indexOf("GET /Relay2/off") >= 0) {
              Serial.println("Relay2 off");
              outputGPIO4State = "off";
              digitalWrite(outRelay2, LOW);
            }
            
            
            // HTML Page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // Style Sheet
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Druid Magic Server</h1>");
            
            // Display current state, and ON/OFF buttons for LED  
            client.println("<p>LED - State " + outputLEDState + "</p>");
            // If the LED is off, it displays the ON button       
            if (outputLEDState=="off") {
              client.println("<p><a href=\"/LED/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/LED/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            // Display current state, and ON/OFF buttons for LED  
            client.println("<p>RELAY1 - State " + outputGPIO5State + "</p>");
            // If the PIN5 is off, it displays the ON button       
            if (outputGPIO5State=="off") {
              client.println("<p><a href=\"/Relay1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/Relay1/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            // Display current state, and ON/OFF buttons for LED  
            client.println("<p>RELAY2 - State " + outputGPIO4State + "</p>");
            // If the PIN4 is off, it displays the ON button       
            if (outputGPIO4State=="off") {
              client.println("<p><a href=\"/Relay2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/Relay2/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            
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
  }
}
