#include <stdio.h>
#include "motor_driver.h"

motor_config_t motor_left = {
    .side = LEFT,
    .duty = 0,
    .dir = FORWARD,
    .pwm_A = PWM_LA,
    .pwm_B = PWM_LB
};

motor_config_t motor_right = {
    .side = RIGHT,
    .duty = 0,
    .dir = FORWARD,
    .pwm_A = PWM_RA,
    .pwm_B = PWM_RB
};

static esp_err_t motor_set_up(motor_config_t *config);
static esp_err_t motor_update(motor_config_t *config);

esp_err_t motor_driver_init(){
    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ESP_ERROR_CHECK(motor_set_up(&motor_left));
    ESP_ERROR_CHECK(motor_set_up(&motor_right));

    return ESP_OK;
}

esp_err_t motor_set_duty(motor_config_t *config, uint32_t duty) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if(duty >= (1 << LEDC_DUTY_RES)){
        duty = (1 << LEDC_DUTY_RES) - 1;
    }
    config->duty = duty;

    // Set the duty cycle for the specified motor
    ESP_ERROR_CHECK(motor_update(config));
    return ESP_OK;
}

esp_err_t motor_set_dir(motor_config_t *config, motor_dir_t dir) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    config->dir = dir;
    ESP_ERROR_CHECK(motor_update(config));
    return ESP_OK;
}

esp_err_t motor_stop(motor_config_t *config) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    config->duty = 0;
    ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, config->pwm_A, 0));
    ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, config->pwm_B, 0));
    return ESP_OK;
}

esp_err_t car_stop() {
    ESP_ERROR_CHECK(motor_stop(&motor_left));
    ESP_ERROR_CHECK(motor_stop(&motor_right));
    return ESP_OK;
}  

uint32_t percent_to_duty(float percent) {
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;
    return (uint32_t)((percent / 100.0f) * ((1 << LEDC_DUTY_RES) - 1));
}

// esp_err_t motor_ramp_duty(motor_config_t *config, uint32_t start_duty, uint32_t target_duty, uint32_t time_ms) {
//     if (config == NULL) {
//         return ESP_ERR_INVALID_ARG;
//     }

//     ESP_ERROR_CHECK(motor_set_duty(config, start_duty));

//     ESP_ERROR_CHECK(ledc_set_fade_with_time(LEDC_MODE, 
//                                             (config->dir == FORWARD) ? config->pwm_A : config->pwm_B, 
//                                             target_duty, 
//                                             time_ms));
//     ESP_ERROR_CHECK(ledc_fade_start(LEDC_MODE, 
//                                     (config->dir == FORWARD) ? config->pwm_A : config->pwm_B, 
//                                     LEDC_FADE_NO_WAIT));
//     config->duty = target_duty;

//     return ESP_OK;
// }
/**
 * Private functions
 */
static esp_err_t motor_set_up(motor_config_t *config) {
    // Prepare and then apply the LEDC PWM channel configuration
    if(config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_num_t OUT_A, OUT_B;

    if(config->side == LEFT){
        OUT_A = GPIO_LA;
        OUT_B = GPIO_LB;
    } else if(config->side == RIGHT){
        OUT_A = GPIO_RA;
        OUT_B = GPIO_RB;
    } else {
        return ESP_ERR_INVALID_ARG;
    }

    ledc_channel_config_t ledc_A = {
        .speed_mode     = LEDC_MODE,
        .channel        = config->pwm_A,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = OUT_A,
        .duty           = config->duty,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_A));

    ledc_channel_config_t ledc_B = {
        .speed_mode     = LEDC_MODE,
        .channel        = config->pwm_B,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = OUT_B,
        .duty           = config->duty,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_B));
    return ESP_OK;
}

static esp_err_t motor_update(motor_config_t *config) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if(config->dir == FORWARD) {
        // Set GPIOs for FORWARD direction
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, config->pwm_A, config->duty));
        ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, config->pwm_B, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, config->pwm_A));
    } else{
        // Set GPIOs for BACKWARD direction
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, config->pwm_B, config->duty));
        ESP_ERROR_CHECK(ledc_stop(LEDC_MODE, config->pwm_A, 0));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, config->pwm_B));
    }

    return ESP_OK;
}