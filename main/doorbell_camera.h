#pragma once

#include "esp_camera.h"

/**
 * @brief 初始化门铃摄像头
 * 
 * 配置并初始化摄像头模块，包括设置引脚定义、像素格式、帧大小等参数
 * 必须在使用摄像头其他功能之前调用
 */
void doorbell_camera_init(void);

/**
 * @brief 反初始化门铃摄像头
 * 
 * 释放摄像头资源，关闭摄像头模块
 * 在程序结束或不再需要使用摄像头时调用
 */
void doorbell_camera_deinit(void);

/**
 * @brief 拍摄一张照片
 * 
 * 从摄像头获取一帧图像数据
 * 
 * @return camera_fb_t* 指向图像帧缓冲区的指针，包含图像数据
 *         返回NULL表示拍摄失败
 */
camera_fb_t* doorbell_camera_capture(void);

/**
 * @brief 释放图像帧缓冲区
 * 
 * 释放由doorbell_camera_capture函数获取的图像帧缓冲区内存
 * 
 * @param fb 指向需要释放的图像帧缓冲区的指针
 */
void doorbell_camera_release(camera_fb_t *fb);