#pragma once

#include <stdint.h>
#include <stddef.h>

// 自己的MQTT服务器
#define MQTT_BROKER_URI "mqtt://192.168.97.102:1883"
#define TOPIC_PRIFIX "doorbell/"

// 公共服务器
// #define MQTT_BROKER_URI "ws://broker.emqx.io:8083/mqtt"
// #define TOPIC_PRIFIX "doorbell/"

typedef struct
{
    char cmd[16];
    void (*callback)(void *arg);
    void *arg;
} mqtt_cmd_t;

void doorbell_mqtt_init(void);

void doorbell_mqtt_deinit(void);

/**
 * @brief 发布消息到服务器
 *
 * @param message 消息指针
 * @param len 消息长度，如果传0，使用strlen(message)
 */
void doorbell_mqtt_publish(const char *message, size_t len);

/**
 * @brief 注册命令
 *
 * @param cmd 命令指针
 */
void doorbell_mqtt_register_cmd(const mqtt_cmd_t *cmd);