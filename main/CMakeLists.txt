idf_component_register(SRCS "app_main.c" "relay_ctl.c"
                    INCLUDE_DIRS "."
                    REQUIRES protocol_examples_common tuya-iot-link-sdk qrcode)
