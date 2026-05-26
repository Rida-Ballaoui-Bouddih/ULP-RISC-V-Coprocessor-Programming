#include"led.h"

#include <stdio.h>
#include "esp_log.h"
#include "driver/gpio.h"

#define GPIO_LED GPIO_NUM_19

void led_init(void){
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = 1ULL << GPIO_LED;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
    gpio_set_drive_capability(GPIO_LED, GPIO_DRIVE_CAP_3);
}

void led_state(bool state){
    gpio_set_level(GPIO_LED, state);
}
