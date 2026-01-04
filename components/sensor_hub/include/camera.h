#ifndef CAMERA_H
#define CAMERA_H

#include "common_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_camera.h"

#define XCLK_FREQ_HZ 20000000
#define CAMERA_TIMER LEDC_TIMER_1
#define CAMERA_CHANNEL LEDC_CHANNEL_5
#define PIXFORMAT PIXFORMAT_RGB565
#define FRAMESIZE FRAMESIZE_QVGA
#define QUALITY 12
#define COUNT 1
#define FB_LOCATION CAMERA_FB_IN_PSRAM
#define GRAB_MODE CAMERA_GRAB_WHEN_EMPTY

// Your declarations here
#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Initialize camera
     * 
     * @param clk_freq     Camera XCLK frequency in Hz. Usually 10MHz or 20MHz
     * 
     * @param pixformat    One of
     *                     - PIXFORMAT_RGB565
     *                     - PIXFORMAT_YUV422
     *                     - PIXFORMAT_GRAYSC
     *                     - PIXFORMAT_JPEG
     *                     - PIXFORMAT_RGB888
     *                     - PIXFORMAT_RAW
     *                     - PIXFORMAT_RGB444
     *                     - PIXFORMAT_RGB555
     * @param frame_size   One of
     *                     - FRAMESIZE_96X96,    // 96x96
     *                     - FRAMESIZE_QQVGA,    // 160x120
     *                     - FRAMESIZE_QCIF,     // 176x144
     *                     - FRAMESIZE_HQVGA,    // 240x176
     *                     - FRAMESIZE_240X240,  // 240x240
     *                     - FRAMESIZE_QVGA,     // 320x240
     *                     - FRAMESIZE_CIF,      // 400x296
     *                     - FRAMESIZE_HVGA,     // 480x320
     *                     - FRAMESIZE_VGA,      // 640x480
     *                     - FRAMESIZE_SVGA,     // 800x600
     *                     - FRAMESIZE_XGA,      // 1024x768
     *                     - FRAMESIZE_HD,       // 1280x720
     *                     - FRAMESIZE_SXGA,     // 1280x1024
     *                     - FRAMESIZE_UXGA,     // 1600x1200
     *                     - FRAMESIZE_FHD,      // 1920x1080
     *                     - FRAMESIZE_P_HD,     //  720x1280
     *                     - FRAMESIZE_P_3MP,    //  864x1536
     *                     - FRAMESIZE_QXGA,     // 2048x1536
     *                     - FRAMESIZE_QHD,      // 2560x1440
     *                     - FRAMESIZE_WQXGA,    // 2560x1600
     *                     - FRAMESIZE_P_FHD,    // 1080x1920
     *                     - FRAMESIZE_QSXGA,    // 2560x1920
     * 
     * @param quality      JPEG quality from 0-63. Lower number means higher quality
     * 
     * @param fb_count     Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed)
     */
    void register_camera(int clk_freq, const pixformat_t pixel_fromat, const framesize_t frame_size, int quality, const uint8_t fb_count);
    camera_fb_t* camera_capture();

#ifdef __cplusplus
}
#endif

#endif // CAMERA_H