#pragma once

#include "iot_button.h"

/**
 * @brief 初始化门铃按钮设备
 * 
 * 配置并初始化门铃按钮的硬件接口和相关参数
 */
void doorbell_button_init(void);


/**
 * @brief 注册门铃按钮前置回调函数
 * 
 * @param event 按钮事件类型，如按下、释放、长按等
 * @param callback 事件触发时调用的回调函数指针
 * @param arg 传递给回调函数的用户参数
 */
void doorbell_button_register_front_callback(button_event_t event, button_cb_t callback, void *arg);

/**
 * @brief 注册门铃按钮后置回调函数
 * 
 * @param event 按钮事件类型，如按下、释放、长按等
 * @param callback 事件触发时调用的回调函数指针
 * @param arg 传递给回调函数的用户参数
 */
void doorbell_button_register_back_callback(button_event_t event, button_cb_t callback, void *arg);

/**
 * @brief 释放门铃按钮设备资源
 * 
 * 释放按钮设备占用的资源并恢复到初始状态
 */
void doorbell_button_deinit(void);