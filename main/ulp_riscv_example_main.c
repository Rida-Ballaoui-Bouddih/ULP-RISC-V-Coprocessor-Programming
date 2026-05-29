#include <stdio.h>
#include <stdint.h>
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include "soc/rtc_periph.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "ulp_riscv.h"
#include "ulp_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi.h"
#include "mqtt_service.h"
#include "sdkconfig.h"
#include "pn532_driver_hsu.h"
#include "pn532.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "rom/uart.h"
#include "smbus.h"
#include "i2c-lcd1602.h"


// MQTT
#define MQTT_SERVER_IP            "broker.emqx.io"
#define MQTT_SERVER_PORT          "1883"
#define MQTT_SERVER_URI           "mqtt://" MQTT_SERVER_IP ":" MQTT_SERVER_PORT

#define MQTTX_TOPIC_TX               "Puente_de_las_bolas_TX"
#define MQTTX_TOPIC_RX               "Puente_de_las_bolas_RX"


#define RESET_PIN      (-1)
#define IRQ_PIN        (-1)
#define HSU_HOST_RX    (4)
#define HSU_HOST_TX    (5)
#define HSU_UART_PORT  UART_NUM_1
#define HSU_BAUD_RATE  (921600)

#define LCD_NUM_ROWS               2
#define LCD_NUM_COLUMNS            32
#define LCD_NUM_VISIBLE_COLUMNS    16

#define I2C_MASTER_NUM           I2C_NUM_0
#define I2C_MASTER_TX_BUF_LEN    0                     // disabled
#define I2C_MASTER_RX_BUF_LEN    0                     // disabled
#define I2C_MASTER_FREQ_HZ       100000
#define I2C_MASTER_SDA_IO        CONFIG_I2C_MASTER_SDA
#define I2C_MASTER_SCL_IO        CONFIG_I2C_MASTER_SCL

static void i2c_master_init(void){
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_LEN, I2C_MASTER_TX_BUF_LEN, 0);
}

static uint8_t _wait_for_user(void){

    uint8_t c = 0;

    #ifdef USE_STDIN
        while (!c){
            STATUS s = uart_rx_one_char(&c);
            if (s == OK){
                printf("%c", c);
            }
            vTaskDelay(1);
        }
    #else
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    #endif
        return c;

}

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

static void init_ulp_program(void);

void app_main(void)
{
    /* If user is using USB-serial-jtag then idf monitor needs some time to
    *  re-connect to the USB port. We wait 1 sec here to allow for it to make the reconnection
    *  before we print anything. Otherwise the chip will go back to sleep again before the user
    *  has time to monitor any output.
    */
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* Initialize selected GPIO as RTC IO, enable input, disable pullup and pulldown */
    rtc_gpio_init(GPIO_NUM_0);
    rtc_gpio_set_direction(GPIO_NUM_0, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_0);
    rtc_gpio_pullup_dis(GPIO_NUM_0);
    rtc_gpio_hold_en(GPIO_NUM_0);

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    /* not a wakeup from ULP, load the firmware */
    if (cause != ESP_SLEEP_WAKEUP_ULP) {
        printf("Not a ULP-RISC-V wakeup, initializing it! \n");
        init_ulp_program();
    }

    /* ULP Risc-V read and detected a change in GPIO_0, prints */
    if (cause == ESP_SLEEP_WAKEUP_ULP) {
        printf("ULP-RISC-V woke up the main CPU! \n");
        printf("ULP-RISC-V read changes in GPIO_0 current is: %s \n",
            (bool)(ulp_gpio_level_previous == 0) ? "Low" : "High" );


        wifi_init();
        printf("WiFi is being initialized.\n");
        vTaskDelay(1000/portTICK_PERIOD_MS);

        for(;;){

            vTaskDelay(500/portTICK_PERIOD_MS);
            
            if(wifi_is_connected()){
                printf("I'm CONNECTED!!!\n");
                break;
            }

            else{
                printf("I'm disconnected :-(\n");
            }
        }
    
        mqtt_service_init(MQTT_SERVER_URI);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        mqtt_service_subscribe(MQTTX_TOPIC_RX);

        pn532_io_t pn532_io;
        esp_err_t err;
        pn532_new_driver_hsu(HSU_HOST_RX, HSU_HOST_TX, RESET_PIN, IRQ_PIN, HSU_UART_PORT, HSU_BAUD_RATE, &pn532_io);

        do{
            err = pn532_init(&pn532_io);
            if (err != ESP_OK) {
                printf("failed to initialize PN532\n");
                pn532_release(&pn532_io);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }while(err != ESP_OK);

        printf("get firmware version\n");
        uint32_t version_data = 0;
        do{
            err = pn532_get_firmware_version(&pn532_io, &version_data);
            if (ESP_OK != err) {
                printf("Didn't find PN53x board\n");
                pn532_reset(&pn532_io);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }while (ESP_OK != err);

        printf("Found chip PN5%x\n", (unsigned int)(version_data >> 24) & 0xFF);
        printf("Firmware ver. %d.%d\n", (int)(version_data >> 16) & 0xFF, (int)(version_data >> 8) & 0xFF);

        printf("Waiting for a Card ...\n");
        uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
        uint8_t uid_length;

        while(1){

            err = pn532_read_passive_target_id(&pn532_io, PN532_BRTY_ISO14443A_106KBPS, uid, &uid_length, 0);
            
            if(ESP_OK == err){
                printf("Found an card\n");
                printf("UID Length: %d bytes\n", uid_length);
                printf("UID Value:\n");
                printf("%x %x %x %x %x %x %x\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
                break;
            }
        }

        char uid_str[3 * sizeof(uid)];
        int pos = 0;
        for (int i = 0; i < sizeof(uid); i++) {
            pos += sprintf(&uid_str[pos], "%02X", uid[i]);
        }

        mqtt_service_publish(MQTTX_TOPIC_TX, uid_str, 13);

        i2c_master_init();
        i2c_port_t i2c_num = I2C_MASTER_NUM;
        uint8_t address = CONFIG_LCD1602_I2C_ADDRESS;

        smbus_info_t * smbus_info = smbus_malloc();
        smbus_init(smbus_info, i2c_num, address);
        smbus_set_timeout(smbus_info, 1000 / portTICK_PERIOD_MS);

        i2c_lcd1602_info_t * lcd_info = i2c_lcd1602_malloc();
        i2c_lcd1602_init(lcd_info, smbus_info, true, LCD_NUM_ROWS, LCD_NUM_COLUMNS, LCD_NUM_VISIBLE_COLUMNS);

        i2c_lcd1602_reset(lcd_info);


        printf("Backlight on\n");
        _wait_for_user();
        i2c_lcd1602_set_backlight(lcd_info, true);

        while(1){
            if(door_access == 0){
                _wait_for_user();
                i2c_lcd1602_move_cursor(lcd_info, 0, 0);
                i2c_lcd1602_write_string(lcd_info, "Clave incorrecta");
                i2c_lcd1602_move_cursor(lcd_info, 0, 1);
                i2c_lcd1602_write_string(lcd_info, "Acesso denegado!");

                vTaskDelay(4000 / portTICK_PERIOD_MS);

                break;
            }
            else if(door_access ==  1){
                _wait_for_user();
                i2c_lcd1602_move_cursor(lcd_info, 0, 0);
                i2c_lcd1602_write_string(lcd_info, "Clave correcta");
                i2c_lcd1602_move_cursor(lcd_info, 0, 1);
                i2c_lcd1602_write_string(lcd_info, "Bienvenido!!!");

                vTaskDelay(4000 / portTICK_PERIOD_MS);


                break;
            }
        }

        _wait_for_user();
        i2c_lcd1602_move_cursor(lcd_info, 0, 0);
        i2c_lcd1602_write_string(lcd_info, "Acerca tu ID    ");
        i2c_lcd1602_move_cursor(lcd_info, 0, 1);
        i2c_lcd1602_write_string(lcd_info, "Identificate!!! ");
        
    }

    /* Go back to sleep, only the ULP Risc-V will run */
    printf("Entering in deep sleep\n\n");

    /* Small delay to ensure the messages are printed */
    vTaskDelay(100);

    ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup());
    esp_deep_sleep_start();
}

static void init_ulp_program(void)
{
    esp_err_t err = ulp_riscv_load_binary(ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start));
    ESP_ERROR_CHECK(err);

    /* The first argument is the period index, which is not used by the ULP-RISC-V timer
     * The second argument is the period in microseconds, which gives a wakeup time period of: 20ms
     */
    ulp_set_wakeup_period(0, 20000);

    /* Start the program */
    err = ulp_riscv_run();
    ESP_ERROR_CHECK(err);
}
