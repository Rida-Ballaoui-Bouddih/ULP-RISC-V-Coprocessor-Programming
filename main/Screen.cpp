#include"Screen.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

void screen_init(){
  Wire.begin(21, 20);
  lcd.init();
  lcd.backlight();
}