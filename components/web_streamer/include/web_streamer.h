#ifndef WEB_STREAMER_H
#define WEB_STREAMER_H

#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "esp_camera.h" // For camera_fb_t definition
#include <stdint.h> // Need this for uint16_t

// Start Wi-Fi and the Web Server
void web_streamer_init(const char* ssid, const char* password);

// Call this in your loop to push a frame to the browser
void web_streamer_update_frame(camera_fb_t *fb);

// --- UPDATED FUNCTION ---
// Now accepts cx and cy to draw a crosshair at the center
void web_streamer_draw_overlay(camera_fb_t *fb, int x, int y, int w, int h, int cx, int cy, uint16_t box_color, uint16_t center_color);

#endif