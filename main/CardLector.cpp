#include "CardLector.h"

Adafruit_PN532 nfc(PN532_SDA, PN532_SCL);

void card_lector_init(){

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("No se detecta PN532"); //Remplazar
    while (1);
  }

  Serial.println("PN532 detectado!"); //Remplazar
  nfc.SAMConfig();

}

