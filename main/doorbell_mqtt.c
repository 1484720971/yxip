#include "doorbell_mqtt.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_random.h"
#include "cJSON.h"

#define TAG "doorbell_mqtt"

static esp_mqtt_client_handle_t mqtt_client = NULL;

static char *mqtt_data_topic = NULL;
static char *mqtt_cmd_topic = NULL;
static char *mqtt_client_id = NULL;

static mqtt_cmd_t mqtt_cmds[10];

static void doorbell_mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_t *event = event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        // 订阅
        esp_mqtt_client_subscribe(mqtt_client, mqtt_cmd_topic, 0);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        // 订阅成功就发一条消息
        doorbell_mqtt_publish("{\"status\":\"ready\"}", 0);
        break;
    case MQTT_EVENT_DATA:
        // 来的数据是类似{"cmd": "led_switch"}的JSON串
        // 解析并执行指令
        ESP_LOGI(TAG, "Data received %.*s", event->data_len, event->data);
        cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
        if (!root)
        {
            ESP_LOGW(TAG, "JSON parse error");
            break;
        }
        cJSON *cmd_json = cJSON_GetObjectItem(root, "cmd");
        if (!cJSON_IsString(cmd_json))
        {
            ESP_LOGW(TAG, "JSON cmd error");
            cJSON_Delete(root);
            break;
        }
        ESP_LOGI(TAG, "cmd: %s", cmd_json->valuestring);
        for (size_t i = 0; i < 10; i++)
        {
            ESP_LOGI(TAG, "cmds: i: %u, cmd: %s", i, mqtt_cmds[i].cmd);
            if (mqtt_cmds[i].cmd[0] == 0)
            {
                // 指令列表遍历完成，return
                break;
            }

            if (strcmp(cmd_json->valuestring, mqtt_cmds[i].cmd) == 0)
            {
                // 找到指令，执行回调
                ESP_LOGI(TAG, "MQTT cmd matched: %s", cmd_json->valuestring);
                mqtt_cmds[i].callback(mqtt_cmds[i].arg);
                break;
            }
        }

        cJSON_Delete(root);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR, %d", event->error_handle->connect_return_code);
        break;
    default:
        ESP_LOGI(TAG, "Unhandled MQTT event %ld", event_id);
        break;
    }
}

static char *doorbell_mqtt_generate_uuid(void)
{
    // 原始UUID数据 (16字节/128位)
    unsigned char uuid[16];

    // 最终UUID字符串 (36字符 + 空终止符)
    char *uuid_str = malloc(37 * sizeof(char));
    if (!uuid_str)
    {
        return NULL;
    }

    // 随机生成uuid
    esp_fill_random(uuid, 16);

    // 设置版本4标识 (第6字节的高4位为0100)
    uuid[6] = (uuid[6] & 0x0F) | 0x40;

    // 设置变体标识 (第8字节的高2位为10)
    uuid[8] = (uuid[8] & 0x3F) | 0x80;

    // 转换为标准UUID字符串格式
    snprintf(uuid_str, 37,
             "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             uuid[0], uuid[1], uuid[2], uuid[3],
             uuid[4], uuid[5], uuid[6], uuid[7],
             uuid[8], uuid[9], uuid[10], uuid[11],
             uuid[12], uuid[13], uuid[14], uuid[15]);

    return uuid_str;
}

void doorbell_mqtt_init(void)
{
    // 构建mqtt topic
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac); // 获取mac地址

    asprintf(&mqtt_data_topic, TOPIC_PRIFIX "%02x-%02x-%02x-%02x-%02x-%02x/data", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    asprintf(&mqtt_cmd_topic, TOPIC_PRIFIX "%02x-%02x-%02x-%02x-%02x-%02x/cmd", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    ESP_LOGI(TAG, "mqtt_data_topic: %s", mqtt_data_topic);
    ESP_LOGI(TAG, "mqtt_cmd_topic: %s", mqtt_cmd_topic);

    // 生成uuid
    mqtt_client_id = doorbell_mqtt_generate_uuid();
    ESP_LOGI(TAG, "mqtt_client_id: %s", mqtt_client_id);

    // mqtt 初始化
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.client_id = mqtt_client_id,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_config);
    assert(mqtt_client);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, doorbell_mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    // 初始化指令区
    memset(mqtt_cmds, 0, sizeof(mqtt_cmds));
}

void doorbell_mqtt_deinit(void)
{
    esp_mqtt_client_stop(mqtt_client);
    esp_mqtt_client_unregister_event(mqtt_client, ESP_EVENT_ANY_ID, doorbell_mqtt_event_handler);
    esp_mqtt_client_destroy(mqtt_client);

    free(mqtt_cmd_topic);
    free(mqtt_data_topic);
    free(mqtt_client_id);
}

/**
 * @brief 发布消息到服务器
 *
 * @param message 消息指针
 * @param len 消息长度，如果传0，使用strlen(message)
 */
void doorbell_mqtt_publish(const char *message, size_t len)
{
    esp_mqtt_client_publish(mqtt_client, mqtt_data_topic, message, len, 0, 0);
}

/**
 * @brief 注册命令
 *
 * @param cmd 命令指针
 */
void doorbell_mqtt_register_cmd(const mqtt_cmd_t *cmd)
{
    // 保存指令
    // 找到保存指令的索引
    size_t i = 0;
    for (i = 0; i < 10; i++)
    {
        if (mqtt_cmds[i].cmd[0] == 0)
        {
            break;
        }
        if (strcmp(mqtt_cmds[i].cmd, cmd->cmd) == 0)
        {
            ESP_LOGW(TAG, "mqtt_cmd %s is already registered, overwriting", cmd->cmd);
            break;
        }
        
    }
    
    if (i >= 10)
    {
        ESP_LOGW(TAG, "mqtt_cmds is full");
        return;
    }
    
    memcpy(&mqtt_cmds[i], cmd, sizeof(mqtt_cmd_t));
}