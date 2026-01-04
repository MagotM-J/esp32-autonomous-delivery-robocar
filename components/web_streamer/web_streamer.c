#include "web_streamer.h"

static const char *TAG = "WEB_STREAMER";

// --- SHARED MEMORY & LOCKS ---
static uint8_t *shared_frame_buf = NULL;
static size_t shared_frame_len = 0;
static int shared_width = 0;
static int shared_height = 0;
static portMUX_TYPE frame_lock = portMUX_INITIALIZER_UNLOCKED; 

// --- HTML PAGE (Makes image larger) ---
static const char* INDEX_HTML = 
"<html><head>"
"<style>"
"body { background-color: #111; display: flex; flex-direction: column; align-items: center; justify-content: center; color: white; font-family: sans-serif; height: 100vh; margin: 0; }"
"h2 { margin-bottom: 10px; }"
// CSS to scale the image up to 640x480 and keep edges sharp
"img { width: 640px; height: 480px; image-rendering: pixelated; border: 3px solid #333; border-radius: 4px; }"
"</style>"
"</head><body>"
"<h2>RoboCar Vision</h2>"
"<img src='/stream'>" // Points to the raw stream handler below
"</body></html>";

// --- WIFI EVENT HANDLER ---
static void wifi_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying to connect to WiFi...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "--------------------------------------------------");
        // Point user to the root URI now, not /stream
        ESP_LOGI(TAG, "DASHBOARD READY: http://" IPSTR "/", IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "--------------------------------------------------");
    }
}

// --- NEW: INDEX HANDLER (Serves HTML) ---
static esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

// --- STREAM HANDLER (Raw MJPEG Data) ---
static esp_err_t stream_handler(httpd_req_t *req) {
    esp_err_t res = ESP_OK;
    char part_buf[128];

    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=123456789000000000000987654321");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while (true) {
        size_t jpg_buf_len = 0;
        uint8_t *jpg_buf = NULL;

        portENTER_CRITICAL(&frame_lock);
        if (shared_frame_buf == NULL) {
            portEXIT_CRITICAL(&frame_lock);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        
        fmt2jpg(shared_frame_buf, shared_frame_len, shared_width, shared_height, PIXFORMAT_RGB565, 80, &jpg_buf, &jpg_buf_len);
        portEXIT_CRITICAL(&frame_lock);

        if (jpg_buf == NULL) continue;

        size_t hlen = snprintf(part_buf, 128, "\r\n--123456789000000000000987654321\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", jpg_buf_len);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)jpg_buf, jpg_buf_len);
        
        free(jpg_buf);
        if (res != ESP_OK) break;

        vTaskDelay(pdMS_TO_TICKS(80));
    }
    return res;
}

// --- PUBLIC FUNCTIONS ---

void web_streamer_init(const char* ssid, const char* password) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = { .threshold.authmode = WIFI_AUTH_WPA2_PSK },
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_wifi_start());

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    httpd_handle_t stream_httpd = NULL;

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        // Register handler for root "/" URI to show HTML page
        httpd_uri_t index_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(stream_httpd, &index_uri);

        // Register handler for raw stream
        httpd_uri_t stream_uri = {
            .uri       = "/stream",
            .method    = HTTP_GET,
            .handler   = stream_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}

void web_streamer_update_frame(camera_fb_t *fb) {
    if(!fb) return;
    
    portENTER_CRITICAL(&frame_lock);
    if (shared_frame_buf == NULL || shared_frame_len != fb->len) {
        if(shared_frame_buf) free(shared_frame_buf);
        shared_frame_buf = malloc(fb->len);
        shared_frame_len = fb->len;
        shared_width = fb->width;
        shared_height = fb->height;
    }
    if (shared_frame_buf) {
        memcpy(shared_frame_buf, fb->buf, fb->len);
    }
    portEXIT_CRITICAL(&frame_lock);
}

// Helper to write a single pixel in RGB565
static void set_pixel(camera_fb_t *fb, int x, int y, uint8_t hi, uint8_t lo) {
    if(x < 0 || x >= fb->width || y < 0 || y >= fb->height) return;
    int idx = (y * fb->width + x) * 2;
    fb->buf[idx] = hi;
    fb->buf[idx+1] = lo;
}

// --- UPDATED DRAWING FUNCTION ---
void web_streamer_draw_overlay(camera_fb_t *fb, int x, int y, int w, int h, int cx, int cy, uint16_t box_color, uint16_t center_color) {
    if(!fb) return;
    
    // Split colors into high/low bytes
    uint8_t box_hi = (box_color >> 8) & 0xFF;
    uint8_t box_lo = box_color & 0xFF;
    uint8_t center_hi = (center_color >> 8) & 0xFF;
    uint8_t center_lo = center_color & 0xFF;

    // 1. Draw Bounding Box
    for (int i = x; i < x + w; i++) {
        set_pixel(fb, i, y, box_hi, box_lo);     // Top
        set_pixel(fb, i, y + h, box_hi, box_lo); // Bottom
    }
    for (int i = y; i < y + h; i++) {
        set_pixel(fb, x, i, box_hi, box_lo);     // Left
        set_pixel(fb, x + w, i, box_hi, box_lo); // Right
    }

    // 2. Draw Crosshair at Centroid (cx, cy)
    // Draw a 7px wide horizontal line
    for(int i = cx - 3; i <= cx + 3; i++) {
        set_pixel(fb, i, cy, center_hi, center_lo);
    }
    // Draw a 7px tall vertical line
    for(int i = cy - 3; i <= cy + 3; i++) {
        set_pixel(fb, cx, i, center_hi, center_lo);
    }
}