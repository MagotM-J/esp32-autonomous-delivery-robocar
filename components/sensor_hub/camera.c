#include "camera.h"
#include "camera_pins.h"

static const char *TAG = "camera";

void register_camera(int clk_freq, const pixformat_t pixel_fromat, const framesize_t frame_size, int quality, const uint8_t fb_count){
    camera_config_t camera_config = {
        .pin_pwdn       = PIN_PWDN,
        .pin_reset      = PIN_RESET,
        .pin_xclk       = PIN_XCLK,
        .pin_sccb_sda   = PIN_SIOD,
        .pin_sccb_scl   = PIN_SIOC,
        .pin_d7         = PIN_D7,
        .pin_d6         = PIN_D6,
        .pin_d5         = PIN_D5,
        .pin_d4         = PIN_D4,
        .pin_d3         = PIN_D3,
        .pin_d2         = PIN_D2,
        .pin_d1         = PIN_D1,
        .pin_d0         = PIN_D0,
        .pin_vsync      = PIN_VSYNC,
        .pin_href       = PIN_HREF,
        .pin_pclk       = PIN_PCLK,

        .xclk_freq_hz   = clk_freq,
        .ledc_timer     = CAMERA_TIMER,
        .ledc_channel   = CAMERA_CHANNEL,
        .pixel_format   = pixel_fromat,
        .frame_size     = frame_size,
        .jpeg_quality   = quality,
        .fb_count       = fb_count,
        .fb_location    = FB_LOCATION,
        .grab_mode      = GRAB_MODE
    };

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }
    ESP_LOGI(TAG, "Camera initialized");

    //OV2640 settings( vertically flipped)
    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
    // Disable Auto White Balance (AWB)
    s->set_whitebal(s, 0);       
    s->set_awb_gain(s, 0);       
    
    //Boost saturation at the hardware level
    s->set_saturation(s, 2);
}

camera_fb_t* camera_capture(){
    //capture a frame
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Frame buffer could not be acquired");
        return NULL;
    }

    return fb;
}