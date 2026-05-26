#ifndef DOOR_H
#define DOOR_H

#include <ESP32Servo.h>

const int ServoPin = 47;

extern Servo S;

void servo_init();

#endif