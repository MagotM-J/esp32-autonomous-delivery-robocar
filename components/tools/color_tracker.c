#include <stdio.h>
#include "color_tracker.h"

/**
 * Private function declarations
 */
static hsv_pixel_t rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b);
static int is_color_in_range(hsv_pixel_t *pixel, const h_range_t *range);

/**
 * Public function definitions
 */
esp_err_t compute_blob(camera_fb_t *fb, const h_range_t *target_color, color_blob_t *blob) {

    int sum_x = 0;
    int sum_y = 0;
    int count = 0;

    point_t top_left = { .x = fb->width, .y = fb->height };
    point_t bottom_right = { .x = 0, .y = 0 };

    if (fb->format != PIXFORMAT_RGB565) {
        printf("Error: format must be RGB565\n");
        return ESP_FAIL;
    }

    for(int y = 0; y < fb->height; y++) {
        for(int x = 0; x < fb->width; x++) {
            int pixel_index = (y * fb->width + x) * 2;

            uint8_t byte1 = fb->buf[pixel_index];
            uint8_t byte2 = fb->buf[pixel_index + 1];
            uint16_t pixel = (byte1 << 8) | byte2;

            uint8_t r = (pixel & 0xF800) >> 8;
            uint8_t g = (pixel & 0x07E0) >> 3;
            uint8_t b = (pixel & 0x001F) << 3;

            hsv_pixel_t hsv = rgb_to_hsv(r, g, b);

            if(is_color_in_range(&hsv, target_color)) {
                
                sum_x += x;
                sum_y += y;
                count++;
                // Update bounding box
                if(x < top_left.x)      top_left.x = x;
                if(y < top_left.y)      top_left.y = y;
                if(x > bottom_right.x)  bottom_right.x = x;
                if(y > bottom_right.y)  bottom_right.y = y;
            } 
        }
    }
    if(count < MIN_AREA) {
        // No pixels found
        blob->area = 0;
        return ESP_ERR_NOT_FOUND;
    }
    // Compute centroid
    blob->centroid.x = sum_x / count;
    blob->centroid.y = sum_y / count;
    blob->top_left = top_left;
    blob->bottom_right = bottom_right;
    blob->area = count;
    return ESP_OK;
}

void print_blob_info(color_blob_t *blob) {
    printf("Blob Info:\n");
    printf(" Centroid: (%d, %d)\n", blob->centroid.x, blob->centroid.y);
    printf(" Bounding Box: Top-Left(%d, %d), Bottom-Right(%d, %d)\n",
           blob->top_left.x, blob->top_left.y,
           blob->bottom_right.x, blob->bottom_right.y);
    printf(" Area (in pixels): %lu\n", blob->area);
}

/**
 * Private functions
 */

static hsv_pixel_t rgb_to_hsv(uint8_t r, uint8_t g, uint8_t b) {
    hsv_pixel_t hsv;
    uint8_t min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    uint8_t max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    uint8_t delta = max - min;

    // Value calculation
    hsv.v = max;

    if (max == 0 || delta == 0) {
        hsv.s = 0;
        hsv.h = 0;
        return hsv;
    }

    // Saturation calculation
    hsv.s = (uint8_t)((delta * 255) / max);

    // Hue calculation
    int32_t h_temp; 

    if (max == r) {
        h_temp = (30 * (g - b)) / delta;
    } else if (max == g) {
        h_temp = 60 + (30 * (b - r)) / delta;
    } else {
        h_temp = 120 + (30 * (r - g)) / delta;
    }

    if (h_temp < 0)    h_temp += 180;
    if (h_temp >= 180) h_temp -= 180;

    hsv.h = (uint8_t)h_temp;

    return hsv;
}

static int is_color_in_range(hsv_pixel_t *pixel, const h_range_t *range) {
    if(pixel->s < MIN_S || pixel->v < MIN_V) return 0;

    if(range->min > range->max){
        if (pixel->h >= range->min || pixel->h <= range->max) return 1;
        else return 0;
    }

    if (pixel->h >= range->min && pixel->h <= range->max) {
        return 1;
    }

    return 0;
}