#include <Arduino.h>
#include <MACROS.h>
#include <SPI.h>
#include <SD.h>
#include "KS0108_GLCD.h"    // include KS0108 GLCD library


// KS0108 GLCD library initialization according to the following connection:
// KS0108_GLCD(DI, RW, E, DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7, CS1, CS2, RES);
KS0108_GLCD display = KS0108_GLCD(LCDDI, LCDRW, LCDE, LCDDB0, LCDDB1, LCDDB2, LCDDB3, LCDDB4, LCDDB5, LCDDB6, LCDDB7, LCDCS1, LCDCS2, LCDRST);


void setup(void) {

  pinMode(LCDDI,OUTPUT);
  pinMode(LCDRW,OUTPUT);
  pinMode(LCDE,OUTPUT);
  pinMode(MOSISD,OUTPUT);
  pinMode(MISOSD,OUTPUT);
  pinMode(SCKSD,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);

  digitalWrite(LED_BUILTIN,HIGH);
  Serial.begin(115200);
  while (!Serial);
  delay(500);
  Serial.println("init");

  if ( display.begin(KS0108_CS_ACTIVE_HIGH) == false ) {
    Serial.println( F("display initialization failed!") );    // lack of RAM space
  }

  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(50, 30, KS0108_ON);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);

  // SPI.setTX(MOSISD);
  // SPI.setRX(MISOSD);
  // SPI.setSCK(SCKSD);
  // SPI.begin();
  // SPI.transfer(0xab);
  // SPI.end();

  // Serial.println("SPI init");

  // if (!SD.begin(CS_SD,SPI))
  // {
  //   Serial.println("SD init fail");
  // }

  // File dataFile = SD.open("/datalog.txt", FILE_WRITE);
  // if (!dataFile)
  // {
  //   Serial.println("SD file init fail");
  // }
  // dataFile.print("testing");
  // dataFile.close();
  

  digitalWrite(LED_BUILTIN,LOW);
  delay(500);
  digitalWrite(LED_BUILTIN,HIGH);

  Serial.println("out of setup");
}

void loop() {
  // u8g2.firstPage();
  // do {
  //   u8g2.setFont(u8g2_font_ncenB14_tr);
  //   u8g2.drawStr(0,24,"Hello World!");
  // } while ( u8g2.nextPage() );
}