#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <stdint.h>
#include "common_types.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define GPIO_LA     GPIO_NUM_18
#define GPIO_LB     GPIO_NUM_19
#define GPIO_RA     GPIO_NUM_32
#define GPIO_RB     GPIO_NUM_33

#define PWM_LA           LEDC_CHANNEL_0
#define PWM_LB           LEDC_CHANNEL_1
#define PWM_RA           LEDC_CHANNEL_2
#define PWM_RB           LEDC_CHANNEL_3

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

typedef enum {
    LEFT,
    RIGHT,
    SIDE_MAX
} motor_side_t;

typedef enum {
    FORWARD = 1,
    BACKWARD = -1
} motor_dir_t;

typedef struct {
    motor_side_t side;
    uint32_t duty;
    motor_dir_t dir;
    ledc_channel_t pwm_A;
    ledc_channel_t pwm_B;
} motor_config_t;


extern motor_config_t motor_left;
extern motor_config_t motor_right;

esp_err_t motor_driver_init();
esp_err_t motor_set_duty(motor_config_t *config, uint32_t duty);
esp_err_t motor_set_dir(motor_config_t *config, motor_dir_t dir);
esp_err_t motor_stop(motor_config_t *config);
esp_err_t car_stop();
uint32_t percent_to_duty(float percent);

#endif // MOTOR_DRIVER_H
