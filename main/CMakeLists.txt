# Set usual component variables
set(
    COMPONENT_SRCS 
        "ulp_riscv_example_main.c" 
        "../components/wifi/wifi.c"
        "../components/mqttx/mqtt_service.c"
        "../components/PN532/pn532.c"
        "../components/PN532/pn532_driver.c"
        "../components/PN532/pn532_driver_hsu.c"
        "../components/esp32-smbus/smbus.c"
        "../components/LCD/i2c-lcd1602.c")

set(
    COMPONENT_ADD_INCLUDEDIRS 
        "." 
        "../components/wifi"
        "../components/mqttx"
        "../components/PN532"
        "../components/esp32-smbus"
        "../components/LCD")

set(
    COMPONENT_REQUIRES 
        driver 
        freertos 
        esp_common 
        ulp 
        esp_driver_gpio 
        esp_wifi 
        esp_event 
        soc 
        nvs_flash
        esp_netif
        mqtt
        esp_driver_uart)

register_component()

#
# ULP support additions to component CMakeLists.txt.
#
# 1. The ULP app name must be unique (if multiple components use ULP).
set(ulp_app_name ulp_${COMPONENT_NAME})
#
# 2. Specify all C and Assembly source files.
#    Files should be placed into a separate directory (in this case, ulp/),
#    which should not be added to COMPONENT_SRCS.
set(ulp_riscv_sources "ulp/main.c")

#
# 3. List all the component source files which include automatically
#    generated ULP export file, ${ulp_app_name}.h:
set(ulp_exp_dep_srcs "ulp_riscv_example_main.c")

#
# 4. Call function to build ULP binary and embed in project using the argument
#    values above.
ulp_embed_binary(${ulp_app_name} "${ulp_riscv_sources}" "${ulp_exp_dep_srcs}")
