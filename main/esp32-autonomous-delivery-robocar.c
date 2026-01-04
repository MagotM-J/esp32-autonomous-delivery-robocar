#include <stdio.h>
#include "common_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor_driver.h"
#include "camera.h"
#include "color_tracker.h"
#include "esp_log.h"
#include "secrets.h"

// --- NEW COMPONENT ---
#include "web_streamer.h"

// Settings
#define WIFI_SSID SECRET_SSID
#define WIFI_PASS SECRET_PASS
#define BASE_SPEED 28.0f 
#define KP 0.04f
#define CENTER_X 160

void app_main(void){
    ESP_ERROR_CHECK(motor_driver_init());
    
    // 1. Start Camera
    register_camera(XCLK_FREQ_HZ, PIXFORMAT, FRAMESIZE, QUALITY, COUNT);
    
    // 2. Start Web Streamer (Simple one-liner now!)
    web_streamer_init(WIFI_SSID, WIFI_PASS);

    printf("Waiting for system warmup...\n");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // RGB565 Color Definitions
    const uint16_t COLOR_GREEN = 0x07E0;
    const uint16_t COLOR_BLUE  = 0x001F;

    uint8_t db = 0;

    while(1){
        camera_fb_t* fb = camera_capture();
        if(!fb) { vTaskDelay(1); continue; }

        color_blob_t blob;
        esp_err_t res = compute_blob(fb, &COLOR_RED, &blob);

        if(res == ESP_OK) { 
            // Visualization: Draw GREEN box (0x07E0) around target
            int w = blob.bottom_right.x - blob.top_left.x;
            int h = blob.bottom_right.y - blob.top_left.y;

            web_streamer_draw_overlay(fb, 
                                      blob.top_left.x, blob.top_left.y, w, h, // Box coords
                                      blob.centroid.x, blob.centroid.y,       // Center coords
                                      COLOR_GREEN, COLOR_BLUE);

            // Motor Logic
            int error = blob.centroid.x - CENTER_X;
            float turn_effort = error * KP;
            
            float left = BASE_SPEED + turn_effort;
            float right = BASE_SPEED - turn_effort;
            
            if(left > 100) { left = 100; }
            if(left < 0) { left = 0; }
            
            if(right > 100) { right = 100; }
            if(right < 0) { right = 0; }

            motor_set_dir(&motor_left, FORWARD);
            motor_set_dir(&motor_right, FORWARD);
            motor_set_duty(&motor_left, percent_to_duty(left));
            motor_set_duty(&motor_right, percent_to_duty(right));
        } else {
            if(db++ > 10) {
                printf("Target lost! Stopping car.\n");
                car_stop();
                db = 0;
            }
            
        }

        // 3. Update Stream (Simple one-liner)
        web_streamer_update_frame(fb);

        esp_camera_fb_return(fb); 
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}