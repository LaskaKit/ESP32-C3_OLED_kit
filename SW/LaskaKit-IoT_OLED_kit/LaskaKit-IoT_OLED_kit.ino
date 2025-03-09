/*
* Example code for ESP32-C3_OLED_kit 
*
* ESP32-C3-LPKit reads data from:
* * SCD41:  It shows the CO2, temperature and humidity
* or
* * BME280: It shows the pressure, temperature and humidity
* or
* * SHT40:  It shows the temperature and humidity
* on OLED display
*
* Hardware:
* LaskaKit ESP32-C3-LPKit                                   - https://www.laskakit.cz/laskkit-esp-12-board/
* LaskaKit SCD41 Senzor CO2, teploty a vlhkosti vzduchu     - https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/
* LaskaKit BME280 Senzor tlaku, teploty a vlhkosti vzduchu  - https://www.laskakit.cz/arduino-senzor-tlaku--teploty-a-vlhkosti-bme280/
* LaskaKit SHT40 Senzor teploty a vlhkosti vzduchu          - https://www.laskakit.cz/laskakit-sht40-senzor-teploty-a-vlhkosti-vzduchu/
* LaskaKit OLED displej 128x64 1.3" I²C                     - https://www.laskakit.cz/laskakit-oled-displej-128x64-1-3--i2c/
*
* Library:
* https://github.com/sparkfun/SparkFun_SCD4x_Arduino_Library
* https://github.com/adafruit/Adafruit_BME280_Library
* https://github.com/adafruit/Adafruit_SHT4X
* https://github.com/adafruit/Adafruit_SH110x
*
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Math.h>

// Choose your version | Vyber svou verze kitu
#define SCD41
//#define BME280
//#define SHT4x

// OLED 
// uncomment only one of them, check the solderbridge on LaskaKit OLED
#define OLED_Address 0x3c //initialize with the I2C addr 0x3C
//#define OLED_Address 0x3d  //initialize with the I2C addr 0x3D
#define POWER 2               // Power pin 

#ifdef SCD41
  #include <SparkFun_SCD4x_Arduino_Library.h>
  SCD4x SSCD41;
#elif defined (BME280)
  #include <Adafruit_BME280.h> 
  Adafruit_BME280 bme;
#else SHT4x
  #include <Adafruit_SHT4x.h>
  Adafruit_SHT4x sht4 = Adafruit_SHT4x();
#endif

// fonts
#include "DSEG14_Classic_Bold_32px.h"
#include "DSEG14_Classic_Bold_12px.h"

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

int co2 = 0;
float temperature = 0.0;
int pressure = 0;
int humidity = 0;

void setup() {
  // Speed of Serial
  Serial.begin(115200);
  pinMode(POWER, OUTPUT); 
  digitalWrite(POWER, HIGH); // enable power supply for uSup
  delay(500);   

  Serial.println("Setup start");
  // set dedicated I2C pins 8 - SDA, 10 SCL for ESP32-C3-LPKit
  Wire.begin();

  /*----- OLED sequence ------*/
  delay(250);                        // wait for the OLED to power up
  display.begin(OLED_Address, true);
  //display.setContrast (0); // dim display

  // Clear the buffer.
  display.clearDisplay();

  #ifdef SCD41
    /*----- SCD41 sequence ------*/
    //             begin, autoCalibrate
    //               |      |
    if (SSCD41.begin(false, true) == false) {
      Serial.println("SCD41 was not set correctly. Check the connection.");
      displayMessage("SCD41", "NOK, was it correct defined?");
      while (1) ;
    }
    displayMessage("SCD41", "OK, waiting for measurement");
    if (SSCD41.startLowPowerPeriodicMeasurement() == true) {
      Serial.println("Low power mode enabled.");
    }
  #elif defined BME280
    unsigned status;
    
    // default settings
    status = bme.begin();  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
  #elif defined SHT4x
    /*----- SHT4x sequence ------*/
    if (! sht4.begin()) {
      Serial.println("SHT4x not found");
      displayMessage("SHT4x", "NOK, was it defined correct?");
      while (1) delay(1);
    }
    displayMessage("SHT4x", "OK, waiting for measurement");
    sht4.setPrecision(SHT4X_HIGH_PRECISION); // highest resolution
    sht4.setHeater(SHT4X_NO_HEATER); // no heater
  #else
    Serial.print("Error, no sensor defined!");
    display.setFont(&DSEG14_Classic_Bold_12);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 40);
    display.println("Error, no sensor defined!");
  #endif

  Serial.println("Setup done");
}


void loop() {

#ifdef SCD41
  if(!SSCD41.readMeasurement()) { // wait for a new data (approx 30s)
    Serial.println("SCD41 not ready");
    delay(1000);
    return;
  } else {
    Serial.println("SCD41 read mesurement");
    co2 = SSCD41.getCO2();
    temperature = SSCD41.getTemperature();
    humidity = SSCD41.getHumidity();

    Serial.println();
    Serial.print("CO2(ppm):\t");
    Serial.println(co2);
    Serial.print("Temperature(C):\t");
    Serial.println(temperature);
    Serial.print("Humidity(%RH):\t");
    Serial.println(humidity);

    display.clearDisplay();
    // CO2
    display.setFont(&DSEG14_Classic_Bold_32);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 40);
    display.println(co2);
    // Temperature
    display.setFont(&DSEG14_Classic_Bold_12);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(20, 60);
    display.print((round(temperature * 10) / 10.0), 1);
    display.println("C");
    // Humidity
    display.setFont(&DSEG14_Classic_Bold_12);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(80, 60);
    display.print(humidity);
    display.println("%");
  }
#elif defined BME280
  
  Serial.println("SCD41 read mesurement");
  temperature = bme.readTemperature();
  humidity    = bme.readHumidity();
  pressure    = bme.readPressure() / 100.0F;  

  Serial.println();
  Serial.print("Pressure:\t");
  Serial.print(pressure);
  Serial.println(" kPa");
  Serial.print("Temperature:\t");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity:\t");
  Serial.print(humidity);
  Serial.println("% rH");

  display.clearDisplay();

  // Pressure
  display.setFont(&DSEG14_Classic_Bold_32);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 40);
  display.println(pressure);
  // Temperature
  display.setFont(&DSEG14_Classic_Bold_12);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(20, 60);
  display.print((round(temperature * 10) / 10.0), 1);
  display.println("C");
    // Humidity
  display.setFont(&DSEG14_Classic_Bold_12);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(80, 60);
  display.print(humidity);
  display.println("%");

#elif defined SHT4x
  sensors_event_t hum, temp; // temperature and humidity variables
 
  sht4.getEvent(&hum, &temp);
  temperature = temp.temperature;
  humidity    = hum.relative_humidity;

  Serial.println();
  Serial.print("Temperature:\t");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity:\t");
  Serial.print(humidity);
  Serial.println("% rH");

  display.clearDisplay();

  // Temperature
  display.setFont(&DSEG14_Classic_Bold_32);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 40);
  display.print((round(temperature * 10) / 10.0), 1);
  display.println("C");
    // Humidity
  display.setFont(&DSEG14_Classic_Bold_12);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(50, 60);
  display.print(humidity);
  display.setCursor(80, 60);
  display.println("% rH");
#else
  Serial.print("Error, no sensor defined!");
  display.setFont(&DSEG14_Classic_Bold_12);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 40);
  display.println("Error, no sensor defined!");
#endif

  // update display
  display.display();
  
  Serial.println("End of loop");
  delay(1000);
}

void displayMessage(char* sensor, char* message){
    
    display.setFont(NULL);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(5, 5);
    display.print("Senzor ");
    display.print(sensor);
    display.print(" is ");
    display.setCursor(5, 20);
    display.println(message);
      // update display
    display.display();
}