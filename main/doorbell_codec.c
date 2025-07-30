#include "doorbell_codec.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "driver/i2s_std.h"
#include "driver/i2c.h"
#include "doorbell_config.h"

/* 音频编解码器控制接口指针 */
static const audio_codec_ctrl_if_t *ctrl_if = NULL;
/* 音频编解码器GPIO接口指针 */
static const audio_codec_gpio_if_t *gpio_if = NULL;
/* 音频编解码器主接口指针 */
static const audio_codec_if_t *codec_if = NULL;
/* 音频编解码器数据接口指针 */
static const audio_codec_data_if_t *data_if = NULL;


// 音频设备句柄
static esp_codec_dev_handle_t codec_dev = NULL;

// 静态变量用于存储I2S接收通道的句柄，初始化为NULL
static i2s_chan_handle_t rx_handle = NULL;
// 静态变量用于存储I2S发送通道的句柄，初始化为NULL
static i2s_chan_handle_t tx_handle = NULL; 

// I2C初始化
// static void doorbell_codec_i2c_init(void)
// {
//     i2c_config_t i2c_cfg = {
//         .mode = I2C_MODE_MASTER,
//         .sda_pullup_en = GPIO_PULLUP_ENABLE,
//         .scl_pullup_en = GPIO_PULLUP_ENABLE,
//         .master.clk_speed = 100000,
//     };
//     i2c_cfg.sda_io_num = I2C_SDA_PIN;
//     i2c_cfg.scl_io_num = I2C_SCL_PIN;
//     ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_cfg));
//     i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
// }

// I2S初始化
static void doorbell_codec_i2s_init(i2s_chan_handle_t *tx_handle_p, i2s_chan_handle_t *rx_handle_p)
{
    // 创建通道句柄
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear_after_cb = true;
    // 创建通道
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, tx_handle_p, rx_handle_p));

    // 创建标准模式句柄
    i2s_std_config_t std_config = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_BITS_PER_SAMPLE, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .bclk = I2S_BCK_PIN,
            .din = I2S_DIN_PIN,
            .dout = I2S_DOUT_PIN,
            .mclk = I2S_MCK_PIN,
            .ws = I2S_WS_PIN,
        },
    };
    // 初始化通道标准模式
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(*tx_handle_p, &std_config));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(*rx_handle_p, &std_config));

    // 启动通道
    ESP_ERROR_CHECK(i2s_channel_enable(*tx_handle_p));
    ESP_ERROR_CHECK(i2s_channel_enable(*rx_handle_p));
}

/**
 * @brief 初始化音频编解码器
 *        配置编解码器的基本参数和寄存器设置
 */
void doorbell_codec_init(void)
{
    /* ---------------- i2c控制通道初始化 ---------------- */
    // i2c初始化
    // doorbell_codec_i2c_init();
    // gpio_if和ctrl_if初始化
    audio_codec_i2c_cfg_t i2c_config = {
        .addr = ES8311_CODEC_DEFAULT_ADDR,
        .port = I2C_NUM_0,
    };
    ctrl_if = audio_codec_new_i2c_ctrl(&i2c_config);
    gpio_if = audio_codec_new_gpio();
    // 控制通道初始化
    es8311_codec_cfg_t es8311_config = {
        .ctrl_if = ctrl_if,
        .gpio_if = gpio_if,
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
        .pa_pin = I2S_PA_PIN,
        .use_mclk = true,
        .no_dac_ref = true,
        .mclk_div = 256,
    };
    codec_if = es8311_codec_new(&es8311_config);
    assert(codec_if);

    /* ---------------- i2s数据通道初始化 ---------------- */
    doorbell_codec_i2s_init(&tx_handle, &rx_handle);

    // 创建数字通道
    audio_codec_i2s_cfg_t i2s_config = {
        .rx_handle = rx_handle,
        .tx_handle = tx_handle,
    };
    data_if = audio_codec_new_i2s_data(&i2s_config);
    assert(data_if);

    // 创建音频设备
    esp_codec_dev_cfg_t codec_config = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT,
        .codec_if = codec_if,
        .data_if = data_if,
    };
    codec_dev = esp_codec_dev_new(&codec_config);
    assert(codec_dev);
}

/**
 * @brief 打开音频编解码器
 *        启动编解码器工作，使其进入可操作状态
 */
void doorbell_codec_open(void)
{
    esp_codec_dev_sample_info_t sample_info = {
        .bits_per_sample = I2S_BITS_PER_SAMPLE,             // 每个采样点的位数(如16bit、24bit等)
        .channel = 1,                                       // 音频通道数，1表示单声道模式
        .mclk_multiple = 256,                               // MCLK与采样率的倍数关系，用于时钟配置
        .sample_rate = I2S_SAMPLE_RATE,                     // 音频采样率(如8kHz、16kHz、44.1kHz等)
        .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0), // 通道掩码，0表示使用通道0(单声道模式)
    };

    // 打开编解码器设备并配置采样参数，如果打开失败则触发断言错误
    assert(esp_codec_dev_open(codec_dev, &sample_info) == ESP_CODEC_DEV_OK);
}

/**
 * @brief 设置音频输出音量
 * @param volume 音量等级，具体范围取决于编解码器硬件规格
 */
void doorbell_codec_set_volume(int volume)
{
    assert(esp_codec_dev_set_out_vol(codec_dev, volume) == ESP_CODEC_DEV_OK);
}

/**
 * @brief 设置麦克风增益
 * @param again 麦克风模拟增益值，控制录音输入灵敏度
 */
void doorbell_codec_set_mic_gain(int gain)
{
    assert(esp_codec_dev_set_in_gain(codec_dev, gain) == ESP_CODEC_DEV_OK);
}

/**
 * @brief 从编解码器读取音频数据
 * @param buf  存储读取数据的缓冲区指针
 * @param len  要读取的数据长度(字节数)
 */
void doorbell_codec_read(void *buf, int len)
{
    assert(esp_codec_dev_read(codec_dev, buf, len) == ESP_CODEC_DEV_OK);
    
}

/**
 * @brief 向编解码器写入音频数据
 * @param buf  包含待写入数据的缓冲区指针
 * @param len  要写入的数据长度(字节数)
 */
void doorbell_codec_write(void *buf, int len)
{
    assert(esp_codec_dev_write(codec_dev, buf, len) == ESP_CODEC_DEV_OK);
}

/**
 * @brief 关闭音频编解码器
 * 
 */
void doorbell_codec_close(void)
{
    assert(esp_codec_dev_close(codec_dev) == ESP_CODEC_DEV_OK);
}

/**
 * @brief 反初始化音频编解码器
 *        释放资源并关闭编解码器硬件
 */
void doorbell_codec_deinit(void)
{
    // 删除编解码器设备实例，释放相关资源
    esp_codec_dev_delete(codec_dev);

    audio_codec_delete_codec_if(codec_if);
    audio_codec_delete_ctrl_if(ctrl_if);
    audio_codec_delete_gpio_if(gpio_if);
    audio_codec_delete_data_if(data_if);
    
    // 禁用I2S接收和发送通道
    i2s_channel_disable(rx_handle);
    i2s_channel_disable(tx_handle);
    
    // 删除I2S接收和发送通道，释放通道资源
    i2s_del_channel(rx_handle);
    i2s_del_channel(tx_handle);
    
    // 删除I2C驱动，释放I2C总线资源
    i2c_driver_delete(I2C_NUM_0);
}