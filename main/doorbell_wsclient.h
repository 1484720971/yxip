#pragma once

#include "esp_websocket_client.h"

// WebSocket图像数据传输的URL后缀路径
// 用于拼接完整的图像数据传输URL
#define WS_IMAGE_SUFFIX "/ws/image"
// WebSocket音频数据传输的URL后缀路径
// 用于拼接完整的音频数据传输URL
#define WS_AUDIO_SUFFIX "/ws/from_esp"
// WebSocket服务器端口号
#define WS_PORT "8000"

/**
 * @brief 初始化WebSocket客户端
 *        配置网络接口和WebSocket连接参数
 */
void doorbell_wsclient_init(void);

/**
 * @brief 反初始化WebSocket客户端
 *        释放网络资源和清理连接状态
 */
void doorbell_wsclient_deinit(void);

/**
 * @brief 建立WebSocket连接
 *        连接到配置的WebSocket服务器
 */
void doorbell_wsclient_connect(void);

/**
 * @brief 断开WebSocket连接
 *        主动关闭与服务器的连接
 */
void doorbell_wsclient_disconnect(void);

/**
 * @brief 发送图像数据到WebSocket服务器
 * @param buf 图像数据缓冲区指针
 * @param len 图像数据长度(字节数)
 */
void doorbell_wsclient_send_image(void *buf, int len);

/**
 * @brief 发送音频数据到WebSocket服务器
 * @param buf 音频数据缓冲区指针
 * @param len 音频数据长度(字节数)
 */
void doorbell_wsclient_send_audio(void *buf, int len);

/**
 * @brief 注册音频数据接收回调函数
 * @param callback 音频数据接收回调函数指针
 *        回调函数参数说明:
 *        - arg: 用户自定义参数
 *        - buf: 接收到的音频数据缓冲区
 *        - len: 接收到的音频数据长度
 * @param arg 传递给回调函数的用户自定义参数
 */
void doorbell_wsclient_register_audio_callback(void (*callback)(void *arg, void *buf, int len), void *arg);