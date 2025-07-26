#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief 初始化音频编解码器
 *        配置编解码器的基本参数和寄存器设置
 */
void doorbell_codec_init(void);

/**
 * @brief 打开音频编解码器
 *        启动编解码器工作，使其进入可操作状态
 */
void doorbell_codec_open(void);

/**
 * @brief 设置音频输出音量
 * @param volume 音量等级，具体范围取决于编解码器硬件规格
 */
void doorbell_codec_set_volume(int volume);

/**
 * @brief 设置麦克风增益
 * @param again 麦克风模拟增益值，控制录音输入灵敏度
 */
void doorbell_codec_set_mic_gain(int again);

/**
 * @brief 从编解码器读取音频数据
 * @param buf  存储读取数据的缓冲区指针
 * @param len  要读取的数据长度(字节数)
 */
void doorbell_codec_read(void *buf, int len);

/**
 * @brief 向编解码器写入音频数据
 * @param buf  包含待写入数据的缓冲区指针
 * @param len  要写入的数据长度(字节数)
 */
void doorbell_codec_write(void *buf, int len);

/**
 * @brief 关闭音频编解码器
 * 
 */
void doorbell_codec_close(void);

/**
 * @brief 反初始化音频编解码器
 *        释放资源并关闭编解码器硬件
 */
void doorbell_codec_deinit(void);