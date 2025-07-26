#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "doorbell_led.h"
#include "doorbell_button.h"
#include "doorbell_codec.h"

#define TAG "Main"
/**
 * @brief 前门铃按钮回调函数
 *        当检测到前门铃按钮单击事件时被调用
 * @param button_handle 按钮句柄指针
 * @param usr_data 用户自定义数据指针
 */
static void front_buton_cb(void *button_handle, void *usr_data)
{
    // 打印前门铃按钮按下的日志信息
    ESP_LOGI(TAG, "Front button pressed");
    // 打开门铃LED灯
    doorbell_led_on();
}

/**
 * @brief 后门铃按钮回调函数
 *        当检测到后门铃按钮单击事件时被调用
 * @param button_handle 按钮句柄指针
 * @param usr_data 用户自定义数据指针
 */
static void back_buton_cb(void *button_handle, void *usr_data)
{
    // 打印后门铃按钮按下的日志信息
    ESP_LOGI(TAG, "Back button pressed");
    // 关闭门铃LED灯
    doorbell_led_off();
}

void app_main(void)
{
    /* ------------- 测试：门铃LED灯和按钮 ------------- */
    // doorbell_led_init();
    // doorbell_button_init();
    // // 注册前门铃按钮单击事件回调函数
    // // 参数: BUTTON_SINGLE_CLICK表示单击事件，front_buton_cb是回调函数，NULL是传递给回调函数的参数
    // doorbell_button_register_front_callback(BUTTON_SINGLE_CLICK, front_buton_cb, NULL);
    // // 注册后门铃按钮单击事件回调函数
    // // 参数: BUTTON_SINGLE_CLICK表示单击事件，back_buton_cb是回调函数，NULL是传递给回调函数的参数
    // doorbell_button_register_back_callback(BUTTON_SINGLE_CLICK, back_buton_cb, NULL);

    /* ------------- 测试：音频编解码器 ------------- */
    // 初始化音频编解码器，配置相关硬件和接口
    doorbell_codec_init();
    // 设置音频输出音量为60（音量等级根据编解码器规格而定）
    doorbell_codec_set_volume(60);
    // 设置麦克风输入增益为10（控制录音灵敏度）
    doorbell_codec_set_mic_gain(10);
    // 打开音频编解码器，使其进入可操作状态
    doorbell_codec_open();
    // 分配2048字节的内存缓冲区用于音频数据读写
    void *buf = malloc(2048);
    // 进入无限循环，持续进行音频数据的读取和回放
    // 实现音频回环功能：从麦克风读取音频数据并立即通过扬声器播放
    while (1)
    {
        // 从编解码器读取音频数据到缓冲区（录音）
        doorbell_codec_read(buf, 2048);
        // 将缓冲区中的音频数据写入编解码器进行播放（播放）
        doorbell_codec_write(buf, 2048);
    }
}