#ifndef COLOR_TRACKER_H
#define COLOR_TRACKER_H

#include "common_types.h"
#include "esp_err.h"
#include "camera.h"

// Your declarations here
#define MIN_S 100
#define MIN_V 50
#define HSV_H_MAX 180

#define IN_RANGE 255
#define OUT_OF_RANGE 0

typedef struct {
    uint8_t h;
    uint8_t s;
    uint8_t v;
} hsv_pixel_t;

typedef struct {
    uint8_t min;
    uint8_t max;
} h_range_t;

typedef struct {
    int x;
    int y;
} point_t;

typedef struct {
    // The "Centroid" (for steering)
    point_t centroid;

    // The "Bounding Box" (for visualizing or collision logic)
    point_t top_left;     // Upper Left point
    point_t bottom_right; // Lower Right point

    // The "Mass" (for distance estimation)
    uint32_t area;     // Total number of valid pixels
} color_blob_t;

// Define color ranges in HSV space
static const h_range_t COLOR_RED   = { .min = 170,   .max = 10  };

static const h_range_t COLOR_ORANGE    = { .min = 11,  .max = 25  };
static const h_range_t COLOR_YELLOW    = { .min = 26,  .max = 34  };
static const h_range_t COLOR_GREEN     = { .min = 35,  .max = 85  };
static const h_range_t COLOR_CYAN      = { .min = 86,  .max = 99  };
static const h_range_t COLOR_BLUE      = { .min = 100, .max = 130 };
static const h_range_t COLOR_PURPLE    = { .min = 131, .max = 169 };

esp_err_t compute_blob(camera_fb_t *fb, h_range_t *target_color, color_blob_t *blob) ;

#endif // COLOR_TRACKER_H
