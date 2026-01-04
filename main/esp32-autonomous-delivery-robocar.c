#include <stdio.h>
#include "common_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "motor_driver.h"
#include "camera.h"       
#include "color_tracker.h"
#include "esp_camera.h"

static void motor_task(void *arg);
static void camera_task(void *arg);

void app_main(void){
    xTaskCreate(motor_task, "motor_task", 4096, NULL, 5, NULL);

    xTaskCreate(camera_task, "camera_task", 8192, NULL, 5, NULL);
}

static void motor_task(void *arg){

    ESP_ERROR_CHECK(motor_driver_init());

    while(1){
        ESP_ERROR_CHECK(motor_set_duty(&motor_left, percent_to_duty(100.0f)));
        ESP_ERROR_CHECK(motor_set_duty(&motor_right, percent_to_duty(100.0f)));
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        ESP_ERROR_CHECK(motor_set_dir(&motor_left, BACKWARD));
        ESP_ERROR_CHECK(motor_set_dir(&motor_right, BACKWARD));
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_ERROR_CHECK(motor_set_dir(&motor_left, FORWARD));
        ESP_ERROR_CHECK(motor_set_dir(&motor_right, FORWARD));
        vTaskDelay(pdMS_TO_TICKS(2000));

        
        ESP_ERROR_CHECK(car_stop());
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(motor_set_duty(&motor_left, percent_to_duty(100.0f)));
        ESP_ERROR_CHECK(motor_set_duty(&motor_right, percent_to_duty(100.0f)));
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Repeat with different speed

        ESP_ERROR_CHECK(motor_set_duty(&motor_left, percent_to_duty(25.0f)));
        ESP_ERROR_CHECK(motor_set_duty(&motor_right, percent_to_duty(25.0f)));
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        ESP_ERROR_CHECK(motor_set_dir(&motor_left, BACKWARD));
        ESP_ERROR_CHECK(motor_set_dir(&motor_right, BACKWARD));
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_ERROR_CHECK(motor_set_dir(&motor_left, FORWARD));
        ESP_ERROR_CHECK(motor_set_dir(&motor_right, FORWARD));
        vTaskDelay(pdMS_TO_TICKS(2000));

        
        ESP_ERROR_CHECK(car_stop());
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_ERROR_CHECK(motor_set_duty(&motor_left, percent_to_duty(25.0f)));
        ESP_ERROR_CHECK(motor_set_duty(&motor_right, percent_to_duty(25.0f)));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void camera_task(void *arg){
    // Initialize camera with desired settings
    register_camera(XCLK_FREQ_HZ, PIXFORMAT, FRAMESIZE, QUALITY, COUNT);

    while(1){
        camera_fb_t* fb = camera_capture();
        if(fb != NULL){
            // Process the captured frame here
            color_blob_t blob;
            if(compute_blob(fb, &COLOR_RED, &blob) == ESP_OK) {
                print_blob_info(&blob);
            } else {
                printf("No blob detected\n");
            }
            // Return the frame buffer back to be reused
            
            esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Capture a frame every second
    }
}