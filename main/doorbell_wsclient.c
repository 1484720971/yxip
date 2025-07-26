#include "doorbell_wsclient.h"
#include "doorbell_config.h"
#include "esp_log.h"
#include "esp_websocket_client.h"

#define TAG "doorbell_wsclient"

// WebSocket图像客户端句柄，用于管理图像数据的WebSocket连接
static esp_websocket_client_handle_t image_client = NULL;
// WebSocket音频客户端句柄，用于管理音频数据的WebSocket连接
static esp_websocket_client_handle_t audio_client = NULL;

// 音频客户端回调函数指针，初始化为空
static void (*audio_callback)(void *arg, const void *buf, int len) = NULL;
// 音频客户端回调函数的参数指针，初始化为空
static void *audio_callback_arg = NULL;


/**
 * @brief WebSocket客户端事件处理函数
 * 
 * 处理门铃系统中WebSocket连接的各种事件，包括连接建立、数据接收、连接断开等状态变化
 * 根据事件类型进行相应的处理，如记录日志或处理音频数据
 * 
 * @param handler_args 指向事件处理参数的指针，用于区分image_client和audio_client
 * @param base 事件基础类型
 * @param event_id 事件ID，标识具体的事件类型
 * @param event_data 指向事件数据的指针，包含事件相关的具体数据
 */
static void doorbell_wsclient_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id)
    {
    case WEBSOCKET_EVENT_BEGIN:
        ESP_LOGI(TAG, "Websocket %s created", handler_args == image_client ? "image" : "audio");
        break;
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Websocket %s connected", handler_args == image_client ? "image" : "audio");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Websocket %s disconnected", handler_args == image_client ? "image" : "audio");
        break;
    case WEBSOCKET_EVENT_DATA:
        // 仅处理audio_client的二进制数据
        if (data->op_code != 0x02 || handler_args != audio_client)
        {
            break;
        }
        audio_callback(audio_callback_arg, (const void *)data->data_ptr, data->data_len);
        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "Websocket %s error", handler_args == image_client ? "image" : "audio");
        break;
    case WEBSOCKET_EVENT_FINISH:
        ESP_LOGI(TAG, "Websocket %s finish", handler_args == image_client ? "image" : "audio");
        break;
    }
}


/**
 * @brief 初始化WebSocket客户端
 *        配置网络接口和WebSocket连接参数
 */
void doorbell_wsclient_init(void)
{
    esp_websocket_client_config_t ws_config = {
        .uri = "ws://" SERVER_IP ":" WS_PORT WS_AUDIO_SUFFIX,

    };
    audio_client = esp_websocket_client_init(&ws_config);

    ws_config.uri = "ws://" SERVER_IP ":" WS_PORT WS_IMAGE_SUFFIX;
    image_client = esp_websocket_client_init(&ws_config);

    esp_websocket_register_events(audio_client, WEBSOCKET_EVENT_ANY, doorbell_wsclient_event_handler, (void *)audio_client);
    esp_websocket_register_events(image_client, WEBSOCKET_EVENT_ANY, doorbell_wsclient_event_handler, (void *)image_client);
}

/**
 * @brief 反初始化WebSocket客户端
 *        释放网络资源和清理连接状态
 */
void doorbell_wsclient_deinit(void)
{
    esp_websocket_unregister_events(audio_client, WEBSOCKET_EVENT_ANY, doorbell_wsclient_event_handler);
    esp_websocket_unregister_events(image_client, WEBSOCKET_EVENT_ANY, doorbell_wsclient_event_handler);
    esp_websocket_client_destroy(audio_client);
    esp_websocket_client_destroy(image_client);
}

/**
 * @brief 建立WebSocket连接
 *        连接到配置的WebSocket服务器
 */
void doorbell_wsclient_connect(void)
{
    esp_websocket_client_start(audio_client);
    esp_websocket_client_start(image_client);
}

/**
 * @brief 断开WebSocket连接
 *        主动关闭与服务器的连接
 */
void doorbell_wsclient_disconnect(void)
{
    esp_websocket_client_stop(audio_client);
    esp_websocket_client_stop(image_client);
}

/**
 * @brief 通过WebSocket客户端发送图像数据
 * 
 * 该函数检查WebSocket客户端连接状态，如果已连接则发送二进制图像数据
 * 
 * @param buf 指向图像数据缓冲区的指针
 * @param len 图像数据的长度
 * @return 无返回值
 */
void doorbell_wsclient_send_image(void *buf, int len)
{
    /* 检查图像客户端是否已连接到WebSocket服务器 */
    if (esp_websocket_client_is_connected(image_client))
    {
        /* 发送二进制图像数据，超时时间为100毫秒 */
        esp_websocket_client_send_bin(image_client, buf, len, pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 通过WebSocket客户端发送音频数据
 * 
 * @param buf 音频数据缓冲区指针
 * @param len 音频数据长度
 * 
 * @return 无返回值
 * 
 * 该函数检查WebSocket客户端连接状态，如果已连接则发送二进制音频数据
 */
void doorbell_wsclient_send_audio(void *buf, int len)
{
    /* 检查音频客户端是否已连接 */
    if(esp_websocket_client_is_connected(audio_client))
    {
        /* 发送二进制音频数据，超时时间为100毫秒 */
        esp_websocket_client_send_bin(audio_client, buf, len, pdMS_TO_TICKS(100)); 
    }
}

/**
 * @brief 注册音频数据接收回调函数
 * @param callback 音频数据接收回调函数指针
 *        回调函数参数说明:
 *        - arg: 用户自定义参数
 *        - buf: 接收到的音频数据缓冲区
 *        - len: 接收到的音频数据长度
 * @param arg 传递给回调函数的用户自定义参数
 */
void doorbell_wsclient_register_audio_callback(void (*callback)(void *arg, void *buf, int len), void *arg)
{
    audio_callback = callback;
    audio_callback_arg = arg;
}