#include "doorbell_led.h"
#include "doorbell_config.h" 
#include "led_strip.h"

static led_strip_handle_t led_strip = NULL;
void doorbell_led_init(void)
{
    led_strip_config_t led_strip_config = {
        .strip_gpio_num = LED_PIN,
        .max_leds = 2,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags.with_dma = true,
    };
    // ESP_ERROR_CHERK检查led_strip_new_rmt_device错误，如果不是ok，程序到这里会中断，日志里会提示这里的错误
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_strip_config, &rmt_config, &led_strip));
    // assert(led_strip);  // 断言，与ESP_ERROR_CHERK二选一
    led_strip_clear(led_strip);
}

void doorbell_led_on(void)
{
    // 为特定像素设置RGB颜色
    led_strip_set_pixel(led_strip, 0, 255, 255, 255);
    led_strip_set_pixel(led_strip, 1, 255, 255, 255);
    // 将内存颜色刷新到 LED 灯上
    led_strip_refresh(led_strip);
}

void doorbell_led_off(void)
{
    led_strip_clear(led_strip);
}

void doorbell_led_deinit(void)
{
    led_strip_del(led_strip);
}