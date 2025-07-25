#include <stdio.h>
#include "doorbell_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void)
{
    doorbell_led_init();

    doorbell_led_on();
    vTaskDelay(pdMS_TO_TICKS(5000));

    doorbell_led_off();
    doorbell_led_deinit();
}