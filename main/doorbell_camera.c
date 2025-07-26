#include "doorbell_camera.h"
#include "doorbell_config.h"

/**
 * @brief 初始化门铃摄像头
 * 
 * 配置并初始化摄像头模块，包括设置引脚定义、像素格式、帧大小等参数
 * 必须在使用摄像头其他功能之前调用
 */
void doorbell_camera_init(void)
{
    // 生成配置
    camera_config_t camera_config = {
        .pin_pwdn = CAM_PWDN_PIN,        // Power down引脚，用于控制摄像头电源关闭
        .pin_reset = CAM_RESET_PIN,      // 复位引脚，用于重置摄像头模块
        .pin_xclk = CAM_MCLK_PIN,        // 主时钟引脚，提供摄像头工作时钟
        .pin_sccb_sda = I2C_SDA_PIN,     // I2C数据线，用于配置摄像头寄存器
        .pin_sccb_scl = I2C_SCL_PIN,     // I2C时钟线，用于配置摄像头寄存器
        .pin_d7 = CAM_D7_PIN,            // 数据位7
        .pin_d6 = CAM_D6_PIN,            // 数据位6
        .pin_d5 = CAM_D5_PIN,            // 数据位5
        .pin_d4 = CAM_D4_PIN,            // 数据位4
        .pin_d3 = CAM_D3_PIN,            // 数据位3
        .pin_d2 = CAM_D2_PIN,            // 数据位2
        .pin_d1 = CAM_D1_PIN,            // 数据位1
        .pin_d0 = CAM_D0_PIN,            // 数据位0
        .pin_vsync = CAM_VSYNC_PIN,      // 垂直同步引脚，标识一帧的开始
        .pin_href = CAM_HREF_PIN,        // 行同步引脚，标识一行数据的有效性
        .pin_pclk = CAM_PCLK_PIN,        // 像素时钟引脚，控制数据采样时序

        .xclk_freq_hz = 20000000,        // XCLK频率设置为20MHz
        .ledc_timer = LEDC_TIMER_0,      // 使用LEDC定时器0
        .ledc_channel = LEDC_CHANNEL_0,  // 使用LEDC通道0
        .pixel_format = PIXFORMAT_JPEG,  // 像素格式设置为JPEG压缩格式
        .frame_size = FRAMESIZE_QVGA,    // 帧大小设置为QVGA (320x240)
        .jpeg_quality = 10,              // JPEG压缩质量，数值越小质量越高
        .fb_count = 2,                   // 帧缓冲区数量为2个
        .fb_location = CAMERA_FB_IN_PSRAM, // 帧缓冲区位于PSRAM中
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY, // 当缓冲区为空时抓取新帧
    };

    // 初始化摄像头
    ESP_ERROR_CHECK(esp_camera_init(&camera_config));
}

/**
 * @brief 反初始化门铃摄像头
 * 
 * 释放摄像头资源，关闭摄像头模块
 * 在程序结束或不再需要使用摄像头时调用
 */
void doorbell_camera_deinit(void)
{
    ESP_ERROR_CHECK(esp_camera_deinit());
}

/**
 * @brief 拍摄一张照片
 * 
 * 从摄像头获取一帧图像数据
 * 
 * @return camera_fb_t* 指向图像帧缓冲区的指针，包含图像数据
 *         返回NULL表示拍摄失败
 */
camera_fb_t* doorbell_camera_capture(void)
{
    return esp_camera_fb_get();
}

/**
 * @brief 释放图像帧缓冲区
 * 
 * 释放由doorbell_camera_capture函数获取的图像帧缓冲区内存
 * 
 * @param fb 指向需要释放的图像帧缓冲区的指针
 */
void doorbell_camera_release(camera_fb_t *fb)
{
    esp_camera_fb_return(fb);
}