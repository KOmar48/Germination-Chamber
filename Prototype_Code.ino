
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define analogPin A0 /* ESP8266 Analog Pin ADC0 = A0 */

LiquidCrystal_I2C lcd(0x27, 16, 2);


const int trigPin = 12;
const int echoPin = 14;

const int motorPin = 2;

const int dutyCycle = 255; 

//define sound velocity in cm/uS
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

const int AirValue = 847;   //you need to replace this value with Value_1
const int WaterValue = 450;  //you need to replace this value with Value_2
const int SensorPin = A0;
int soilMoistureValue = 0;
int soilmoisturepercent=0;

void setup() {
  Serial.begin(115200); // Starts the serial communication
  Wire.begin();
  while (!Serial);
  
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("ADC Value =      ");
}

void loop() {


  soilMoistureValue = analogRead(SensorPin);  //put Sensor insert into soil
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if(soilmoisturepercent > 100)
{
  Serial.println("100 %");
  lcd.setCursor(0,0);
  lcd.print("Moisture:      "); 
  lcd.setCursor(12, 0);
  lcd.print("100 %");
  analogWrite(motorPin, 0);
}

else if(soilmoisturepercent <0)
{
  Serial.println("0 %");
  lcd.setCursor(0,0);
  lcd.print("Moisture:      "); 
  lcd.setCursor(12, 0);
  lcd.print("0 %");
  analogWrite(motorPin, 0);
}

else if(soilmoisturepercent <80 && soilmoisturepercent >30 )
{
  Serial.println("0 %");
  lcd.setCursor(0,0);
  lcd.print("Moisture:      "); 
  lcd.setCursor(12, 0);
  lcd.print(soilmoisturepercent);
  lcd.print("%");
  
  analogWrite(motorPin, dutyCycle);
  delay(1000);
}


else if(soilmoisturepercent >=0 && soilmoisturepercent <= 100)
{
  Serial.print(soilmoisturepercent);
  Serial.println("%");
  lcd.setCursor(0,0);
  lcd.print("Moisture:      "); 
  lcd.setCursor(12, 0);
  lcd.print(soilmoisturepercent);
  lcd.print("%");
  analogWrite(motorPin, 0);
}


  


  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_VELOCITY/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance on the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);
  
  lcd.setCursor(0, 1);
  lcd.print("Water Level: ");
  lcd.print(distanceCm);
  
  
  delay(100);
}
