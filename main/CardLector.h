#ifndef CARDLECTOR_H
#define CARDLECTOR_H

#include <Adafruit_PN532.h>

#define PN532_SDA 8
#define PN532_SCL 9

extern Adafruit_PN532 nfc;

void card_lector_init();

/*
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
    Serial.println("Found an card");
    Serial.print("  UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("  UID Value: "); Serial.print(uid[0], HEX); Serial.print(uid[1], HEX); Serial.print(uid[2], HEX); Serial.print(uid[3], HEX); Serial.print(uid[4], HEX); Serial.print(uid[5], HEX); Serial.println(uid[6], HEX);
    //nfc.PrintHex(uid, uidLength);
    

    
    delay(2000);
  }
*/

#endif