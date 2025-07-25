#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "doorbell_led.h"
#include "doorbell_button.h"

static void front_buton_cb(void *button_handle, void *usr_data)
{
    // ESP_LOGI(TAG, "Front button pressed");
    doorbell_led_on();
}

static void back_buton_cb(void *button_handle, void *usr_data)
{
    // ESP_LOGI(TAG, "Back button pressed");
    doorbell_led_off();
}

void app_main(void)
{
    doorbell_led_init();
    doorbell_button_init();

    // 注册前门铃按钮单击事件回调函数
    // 参数: BUTTON_SINGLE_CLICK表示单击事件，front_buton_cb是回调函数，NULL是传递给回调函数的参数
    doorbell_button_register_front_callback(BUTTON_SINGLE_CLICK, front_buton_cb, NULL);
    // 注册后门铃按钮单击事件回调函数
    // 参数: BUTTON_SINGLE_CLICK表示单击事件，back_buton_cb是回调函数，NULL是传递给回调函数的参数
    doorbell_button_register_back_callback(BUTTON_SINGLE_CLICK, back_buton_cb, NULL);
}