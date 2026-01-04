idf_component_register(SRCS "web_streamer.c"
                    INCLUDE_DIRS "include"
                    REQUIRES common esp_timer esp32-camera esp_http_server esp_wifi nvs_flash tools)
