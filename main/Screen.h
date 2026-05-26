#ifndef SCREEN_H
#define SCREEN_H

#include<LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C lcd;

struct screen_message{
    char line_one[17];
    char line_two[17];
};

const screen_message password_correct = {"Clave correcta", "Bienvenido!!!"};
const screen_message password_incorrect = {"Clave incorrecta", "Acesso denegado!"};

void screen_init();

// lcd.setCursor(0, 0); El segundo valor es para usar linea 0 o 1
// lcd.print("Hola Mundo");
// lcd.display(); | lcd.noDisplay();
// lcd.cursor(); | lcd.noCursor();

#endif