#include "doorbell_button.h"
#include "button_adc.h"

// 前门铃按钮句柄，用于管理前门铃按钮设备
static button_handle_t front_button = NULL;

// 后门铃按钮句柄，用于管理后门铃按钮设备
static button_handle_t back_button = NULL;

/**
 * @brief 初始化门铃按钮设备
 *
 * 配置并初始化门铃按钮的硬件接口和相关参数
 */
void doorbell_button_init(void)
{
    // button_config_t参数为零的话，就是使用默认值，默认值在SDK配置编辑器中的Iot Button
    // 初始化按钮配置结构体，使用默认配置
    button_config_t btn_cfg = {0};
    // 配置ADC按钮参数
    button_adc_config_t btn_adc_cfg = {
        .unit_id = ADC_UNIT_1, // 使用ADC单元1
        .adc_channel = 7,      // ADC通道7
        .button_index = 0,     // 按钮索引为0（前门铃按钮）
        .min = 0,              // 按钮电压范围最小值
        .max = 50,             // 按钮电压范围最大值
    };

    // 创建前门铃ADC按钮设备
    ESP_ERROR_CHECK(iot_button_new_adc_device(&btn_cfg, &btn_adc_cfg, &front_button));

    // 修改配置参数以创建后门铃按钮
    btn_adc_cfg.button_index = 1; // 按钮索引为1（后门铃按钮）
    btn_adc_cfg.min = 1485;       // 后门铃按钮电压范围最小值
    btn_adc_cfg.max = 1815;       // 后门铃按钮电压范围最大值
    // 创建后门铃ADC按钮设备
    ESP_ERROR_CHECK(iot_button_new_adc_device(&btn_cfg, &btn_adc_cfg, &back_button));
}

/**
 * @brief 注册门铃按钮前置回调函数
 * 
 * @param event 按钮事件类型，如按下、释放、长按等
 * @param callback 事件触发时调用的回调函数指针
 * @param arg 传递给回调函数的用户参数
 */
void doorbell_button_register_front_callback(button_event_t event, button_cb_t callback, void *arg)
{
    iot_button_register_cb(front_button, event, NULL, callback, arg);
}

/**
 * @brief 注册门铃按钮后置回调函数
 * 
 * @param event 按钮事件类型，如按下、释放、长按等
 * @param callback 事件触发时调用的回调函数指针
 * @param arg 传递给回调函数的用户参数
 */
void doorbell_button_register_back_callback(button_event_t event, button_cb_t callback, void *arg)
{
    iot_button_register_cb(back_button, event, NULL, callback, arg);
}

/**
 * @brief 释放门铃按钮设备资源
 *
 * 释放按钮设备占用的资源并恢复到初始状态
 */
void doorbell_button_deinit(void)
{
    iot_button_delete(front_button);
    iot_button_delete(back_button);
}