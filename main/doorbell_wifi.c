#include "doorbell_wifi.h"
#include "esp_wifi.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define TAG "doorbell_wifi"                    // 日志标签
#define WIFI_CONNECTED_BIT BIT0                // WiFi连接状态事件组标志位

static esp_event_handler_instance_t wifi_instance = NULL;      // WiFi事件处理实例
static esp_event_handler_instance_t ip_instance = NULL;        // IP事件处理实例
static esp_event_handler_instance_t wifi_prov_instance = NULL; // WiFi配网事件处理实例
static EventGroupHandle_t wifi_event_group = NULL;             // WiFi事件组句柄，用于同步WiFi连接状态

/**
 * @brief WiFi事件处理函数
 * 
 * 处理各种WiFi相关事件:
 * - WIFI_EVENT_STA_START: STA模式启动时连接WiFi
 * - WIFI_EVENT_STA_DISCONNECTED: WiFi断开时重新连接
 * - IP_EVENT_STA_GOT_IP: 获取到IP地址时设置事件标志
 * - WIFI_PROV_EVENT_END: 配网结束时清理资源
 * 
 * @param arg 用户参数
 * @param event_base 事件基础类型
 * @param event_id 事件ID
 * @param event_data 事件相关数据
 */
static void doorbell_wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // STA模式启动时开始连接WiFi
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // WiFi断开连接时重新连接
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // 获取到IP地址，设置连接成功标志位
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_END)
    {
        // 配网完成后反初始化配网管理器并注销事件处理函数
        wifi_prov_mgr_deinit();
        esp_event_handler_instance_unregister(WIFI_PROV_EVENT, WIFI_PROV_END, wifi_prov_instance);
    }
}

static void doorbell_wifi_nvs_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        /* NVS partition was truncated
         * and needs to be erased */
        ESP_ERROR_CHECK(nvs_flash_erase());

        /* Retry nvs_flash_init */
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(ret);
}

/**
 * @brief 初始化WiFi模块
 * 
 * 该函数完成以下初始化工作:
 * 1. 创建事件组用于同步
 * 2. 初始化TCP/IP协议栈
 * 3. 创建默认事件循环并注册事件处理函数
 * 4. 初始化WiFi
 * 5. 根据是否已配网决定启动配网流程还是直接连接WiFi
 */
void doorbell_wifi_init(void)
{
    // 创建event_group用于WiFi连接状态同步
    wifi_event_group = xEventGroupCreate();
    doorbell_wifi_nvs_init();
    // 1. 初始化TCP/IP协议栈
    ESP_ERROR_CHECK(esp_netif_init());
    // 2. 创建默认事件循环，并注册回调函数
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //      WIFI事件回调
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &doorbell_wifi_event_handler, NULL, &wifi_instance));
    //      TCP/IP事件回调
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &doorbell_wifi_event_handler, NULL, &ip_instance));
    //      配网Manager事件回调
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_PROV_EVENT, WIFI_PROV_END, &doorbell_wifi_event_handler, NULL, &wifi_prov_instance));
    // 3. 初始化WIFI
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // 4. 创建配网Manager
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM};
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
    // 5. 检查是否配网
    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    ESP_LOGI(TAG, "当前配网状态: %s", provisioned ? "已配网" : "未配网"); // 新增日志
    if (!provisioned)
    {
        // 如果未配网，执行配网操作

        // 指定安全协议为WIFI_PROV_SECURITY_1
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        // 设置配网安全参数
        wifi_prov_security1_params_t *security_params = "doorbell_250305";

        // 根据mac地址获取Service Name，格式为DOORBELL_XXYYZZ
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        char service_name[16];
        snprintf(service_name, 16, "DOORBELL_%02X%02X%02X", mac[3], mac[4], mac[5]);

        // 启动配网流程
        wifi_prov_mgr_start_provisioning(security, security_params, service_name, NULL);

        // 显示二维码用于配网
        char *qrcode_str = NULL;
        asprintf(&qrcode_str, "{\"ver\":\"v1\",\"name\":\"%s\""
                              ",\"pop\":\"%s\",\"transport\":\"ble\"}",
                 service_name, security_params);
        ESP_LOGI(TAG, "打开下面的网址获取二维码：\nhttps://espressif.github.io/esp-jumpstart/qrcode.html?data=%s", qrcode_str);
        free(qrcode_str);
    }
    else
    {
        // 如果已配网，清理配网Manager，连接WIFI
        wifi_prov_mgr_deinit();
        esp_event_handler_instance_unregister(WIFI_PROV_EVENT, WIFI_PROV_END, wifi_prov_instance);
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

/**
 * @brief 重置WiFi配网信息
 * 
 * 调用此函数将清除已保存的WiFi配网信息，
 * 设备将需要重新进行配网才能连接到WiFi网络。
 * 通常在需要重新配置网络或忘记当前网络时使用。
 */
void doorbell_wifi_reset_provisioning(void)
{
    ESP_ERROR_CHECK(wifi_prov_mgr_reset_provisioning());
}

/**
 * @brief 反初始化WiFi模块
 * 
 * 断开WiFi连接并释放WiFi相关资源
 */
void doorbell_wifi_deinit(void)
{
    // 断开WiFi连接
    // ESP_ERROR_CHECK(esp_wifi_disconnect());
    esp_wifi_disconnect();
    // 反初始化WiFi模块
    // ESP_ERROR_CHECK(esp_wifi_deinit());
    esp_wifi_deinit();
}