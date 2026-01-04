idf_component_register(SRCS "color_tracker.c" "pid_controller.c"
                    INCLUDE_DIRS "include"
                    REQUIRES common sensor_hub esp32-camera)