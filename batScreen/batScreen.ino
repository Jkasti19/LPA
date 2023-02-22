#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LC709203F.h>

Adafruit_LC709203F lc;

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR   0x3C 
#define OLED_RESET -1 
Adafruit_SSD1306 display(OLED_WIDTH,OLED_HEIGHT,&Wire,OLED_RESET);

void setup() {
  Serial.begin(115200);
  Serial.println("LC709203F Test Program");
  lc.begin();

  lc.setThermistorB(3950);
  lc.setPackSize(LC709203F_APA_500MAH);
  lc.setAlarmVoltage(3.8);
  
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("SD405-Sp22-09");
  display.setTextSize(2);
  display.println("LPA-Light");
  display.println("Plate");
  display.println("Apparatus");

  display.display();

  delay(5000);
}

void loop() {
//  Serial.print("Batt_Voltage:");
//  Serial.print(lc.cellVoltage(), 3);
//  Serial.print("\t");
//  Serial.print("Batt_Percent:");
//  Serial.print(lc.cellPercent(), 1);
//  Serial.print("\t");
//  Serial.print("Batt_Temp:");
//  Serial.println(lc.getCellTemperature(), 1);
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Power %");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 17);
  delay(1000);
  display.println("Voltage: "); display.print(lc.cellVoltage()) ; display.println(" V");
  display.println("Charge Percent: "); display.print(lc.cellPercent()); display.println(" %");
  display.println("Temperature: "); display.print(lc.getCellTemperature()); display.println(" C");
  
  display.display();
  delay(2000);
}
