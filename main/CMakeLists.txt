idf_component_register(SRCS "esp32-autonomous-delivery-robocar.c"
                    INCLUDE_DIRS "."
                    REQUIRES motor_driver ble_driver navigator sensor_hub tools common)
