idf_component_register(SRCS "camera.c" "speed_encoder.c" "ultrasonic.c"
                    INCLUDE_DIRS "include"
                    REQUIRES common driver esp32-camera)