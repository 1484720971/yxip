#include "doorbell_led.h"
#include "doorbell_button.h"
#include "esp_log.h"
#include "doorbell_codec.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "doorbell_camera.h"
#include "doorbell_wifi.h"
#include "esp_system.h"

#define TAG "Main"

static void front_button_cb(void *button_handle, void *usr_data)
{
    doorbell_wifi_reset_provisioning();
    esp_restart();
}

void app_main(void)
{
    doorbell_button_init();
    doorbell_button_register_front_callback(BUTTON_SINGLE_CLICK, front_button_cb, NULL);
    doorbell_wifi_init();
}