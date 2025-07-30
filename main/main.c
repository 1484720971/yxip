#include "doorbell_led.h"
#include "doorbell_button.h"
#include "esp_log.h"
#include "doorbell_codec.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "doorbell_camera.h"
#include "doorbell_wifi.h"
#include "esp_system.h"
#include "doorbell_mqtt.h"

#define TAG "Main"

static bool led_on = false;
static void led_switch(void *arg)
{
    if (led_on)
    {
        doorbell_led_off();
        led_on = false;
    }
    else
    {
        doorbell_led_on();
        led_on = true;
    }
}

void app_main(void)
{
    doorbell_led_init();
    doorbell_wifi_init();
    doorbell_mqtt_init();

    mqtt_cmd_t cmd = {
        .arg = NULL,
        .cmd = "led_switch",
        .callback = led_switch
    };
    doorbell_mqtt_register_cmd(&cmd);
}