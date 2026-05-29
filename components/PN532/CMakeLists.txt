idf_component_register(
    SRCS "pn532.c" "pn532_driver.c" "pn532_driver_hsu.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_driver_gpio esp_driver_uart
)