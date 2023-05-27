/*
* Example code for ESP32-C3_OLED_kit 
*
* ESP32-C3-LPKit reads data from bme280
* It shows the pressure, temperature, humidity
* to OLED display
*
* Hardware:
* ESP32-C3-LPKit - https://www.laskakit.cz/laskkit-esp-12-board/
* LaskaKit BME280 Senzor tlaku, teploty a vlhkosti vzduchu - https://www.laskakit.cz/arduino-senzor-tlaku--teploty-a-vlhkosti-bme280/
* OLED displej 128x64 1.3" I²C - https://www.laskakit.cz/laskakit-oled-displej-128x64-1-3--i2c/
*
* Library:
* https://github.com/adafruit/Adafruit_BME280_Library
* https://github.com/adafruit/Adafruit_SH110x
*
*/


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_BME280.h>      // BME280 by Adafruit 

// fonts
#include "DSEG14_Classic_Bold_32px.h"
#include "DSEG14_Classic_Bold_12px.h"

// OLED 
// uncomment only one of them, check the solderbridge on LaskaKit OLED
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C
//#define i2c_Address 0x3d  //initialize with the I2C addr 0x3D

#define BME280_ADDRESS (0x77)

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme;

float temperature = 0.0;
int pressure =    0.0;
int humidity =        0;

void setup() {
  // Speed of Serial
  Serial.begin(115200);

  // set dedicated I2C pins 8 - SCD, 10 SCL for ESP32-C3-LPKit
  Wire.begin(8, 10);

  if (! bme.begin(BME280_ADDRESS)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  Serial.println("-- Weather Station Scenario --");
  Serial.println("forced mode, 1x temperature / 1x humidity / 1x pressure oversampling,");
  Serial.println("filter off");
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
  delay(10);

  /*----- OLED sequence ------*/
  delay(250);                        // wait for the OLED to power up
  display.begin(i2c_Address, true);  // Address 0x3C default
  //display.setContrast (0); // dim display

  // Clear the buffer.
  display.clearDisplay();
}


void loop() {

  temperature = bme.readTemperature();
  humidity    = bme.readHumidity();
  pressure    = bme.readPressure() / 100.0F;  

  Serial.println();

  Serial.print("Pressure:");
  Serial.println(pressure);

  Serial.print("\tTemperature(C):");
  Serial.println(temperature);

  Serial.print("\tHumidity(%RH):");
  Serial.println(humidity);

  display.clearDisplay();

  /*----- OLED sequence ------*/
    // CO2
  display.setFont(&DSEG14_Classic_Bold_32);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 40);
  display.println(pressure);

  // Temperature
  display.setFont(&DSEG14_Classic_Bold_12);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(20, 60);
  display.println(temperature);

    // Humidity
  display.setFont(&DSEG14_Classic_Bold_12);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(80, 60);
  display.println(humidity);

  // update display
  display.display();

  // go to sleep for 1 minute
  esp_sleep_enable_timer_wakeup(60 * 1000000);
  esp_deep_sleep_start();

  delay(500);
}
