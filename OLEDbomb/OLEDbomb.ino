/*
* Name: övningsprojekt
* Author: Tor H
* Date: 2025-10-13
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display, 
* Further, it measures temprature with ds3231 and displays a mapped value to a 9g-servo-motor.
* In addition, the measured temperature value gets converted into a usable integer for the Light Ring and lights up the ring accordingly with according colours(green,yellow,red)
*/

// Include Libraries
#include "U8glib.h"
#include <RTClib.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6        // Data pin connected to DIN
#define NUMPIXELS 32 // Number of LEDs in your ring (change if needed)

// Forward declarations (optional but nice when functions are below loop)
String getTime();
float getTemp();
void oledWrite(String text, float temp);
void servoWrite(float value);

// Init constants
const int MinTemp = 18;
const int MaxTemp = 28;

Servo myServo;
const int SERVO_PIN = 9;
// Init global variables
int ServoAngle = 0;  //Current angle of the Servo

// Construct objects
RTC_DS3231 rtc;
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send AC
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // init communication
  Serial.begin(9600);
  Wire.begin();

  // Init Hardware
  rtc.begin();
  myServo.attach(SERVO_PIN);  // attach servo to the pin
  myServo.write(90); 
  pixels.begin();      // Initialize the NeoPixel ring
  pixels.clear();      // Turn off all pixels
  
  // Settings
  u8g.setFont(u8g_font_8x13); // liten, läsbar

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}


/*
Main Loop - Writes the Time of day and temperature as well as the angle the servo is at the moment.
Remakes the given temperature to a value for the Light Ring, Lights up the light ring accordingly
*/
int count1 = 0;  //Set variable before loop
void loop() {
  oledWrite(getTime(), getTemp());
  servoWrite(getTemp());

  

  float t = getTemp();  
  oledWrite(getTime(), t);
  servoWrite(t);

  int count = (int)((t - 18.0f) * 32.0f / (28.0f - 18.0f) + 0.5f); // round to nearest
  count = constrain(count, 0, 32);  // clamp to 0..32

  if (count1 > count) {
    // temperature went DOWN: turn OFF pixels from old-1 down to new
    for (int e = count1 - 1; e >= count; e--) {
      pixels.setPixelColor(e, 0);           // off
      pixels.show();
      delay(50);
    }
  } else if (count1 < count) {
    // temperature went UP: turn ON pixels from old up to new-1
    for (int i = count1; i < count; i++) {
      if (i < 8){
        pixels.setPixelColor(i, pixels.Color(0, 255, 0));
        pixels.show();
        delay(50);
      } else if (i < 15){
        pixels.setPixelColor(i, pixels.Color(255, 255, 0));
        pixels.show();
        delay(50);
      } else{
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // red
      pixels.show();
      delay(50);
      }
    }
  }
  count1 = count;
}


//This function reads time from an ds3231 module and packages the time as a String
//Returns: time in hh:mm:ss as String
String getTime() {
  DateTime now = rtc.now();
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  return String(buf);
}

/*
* This function takes temperature from ds3231 and returns as a float
*Returns: temperature as float 
*/
float getTemp() {
  return rtc.getTemperature();
}

/*
* This function takes a string and draws it to an oled display
*Parameters: - text: String to write to display
*/
void oledWrite(String text, float temp) {
  u8g.firstPage();
  do {
    String s = String(temp, 1);      // one decimal
    String a = String(ServoAngle);
    u8g.drawStr(10, 10, text.c_str());
    u8g.drawStr(10, 20, s.c_str());
    u8g.drawStr(10, 30, a.c_str());
  } while (u8g.nextPage());
}

/*
* takes a temperature value and maps it to corresponding degree on a servo
* Parameters: value = Temperature measued from getTemp()
* Writes the mapped value as an angle for the Servo to adjust to.
*/
void servoWrite(float value) {
  int angle = map((long)value, MinTemp, MaxTemp, 10, 170);
  angle = constrain(angle, 0, 180);
  ServoAngle = angle;
  myServo.write(angle);
}
