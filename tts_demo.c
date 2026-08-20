/*****************************************************************/ /**
* @file audio_demo.c
* @brief
* @author larson.li@quectel.com
* @date 2025-05-14
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description"
* <tr><td>2023-06-13 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/
#include "qosa_sys.h"
#include "qosa_gpio.h"
#include "qosa_def.h"
#include "qosa_log.h"
#include "tts_demo.h"
#include <stdlib.h>
#include <string.h>
#include "qcm_audio.h"
#include "qosa_iic.h"
#include "qosa_virtual_file.h"
#include "qosa_tts.h"
#include "qcm_spi_nor.h"
#include "qosa_pinctrl.h"
#include "unirtos_app_init_registry.h"

/*===========================================================================
 *  Macro Definition
 ===========================================================================*/
#define QOS_LOG_TAG LOG_TAG_DEMO
//#define QOSA_TTS_RESOURCE_SET  //TTS resource inventory is stored in external flash

/*===========================================================================
 *  Variate
 ===========================================================================*/
static qosa_task_t g_unir_tts_demo_task = QOSA_NULL;
static qosa_sem_t  g_audio_tts_play_finish_sem;

// codec Register configuration
#define ES8311_INIT_REG                                                                                                                                        \
    {                                                                                                                                                          \
        {0x01, 0x30, 0x00}, {0x02, 0x00, 0x00}, {0x03, 0x20, 0x00}, {0x16, 0x20, 0x00}, {0x04, 0x20, 0x00}, {0x05, 0x00, 0x00}, {0x0B, 0x00, 0x00},            \
            {0x0C, 0x00, 0x00}, {0x0F, 0x44, 0x00}, {0x10, 0x1F, 0x00}, {0x11, 0x7F, 0x00}, {0x00, 0x80, 0x00}, {0x00, 0x80, 0x00}, {0x01, 0x3F, 0x00},        \
            {0x01, 0xBF, 0x00}, {0x02, 0x18, 0x00}, {0x05, 0x00, 0x00}, {0x03, 0x10, 0x00}, {0x04, 0x10, 0x00}, {0x07, 0x00, 0x00}, {0x08, 0xFF, 0x00},        \
            {0x06, 0x03, 0x00}, {0x01, 0xBF, 0x00}, {0x06, 0x03, 0x00}, {0x13, 0x10, 0x00}, {0x1B, 0x0A, 0x00}, {0x1C, 0x6A, 0x00}, {0x09, 0x0D, 0x00},        \
            {0x0A, 0x0D, 0x00}, {0x09, 0x0D, 0x00}, {0x0A, 0x0D, 0x00}, {0x32, 0xBF, 0x00}, {0x09, 0x0D, 0x00}, {0x0A, 0x0D, 0x00}, {0x17, 0xBF, 0x00},        \
            {0x0E, 0x02, 0x00}, {0x12, 0x00, 0x00}, {0x14, 0x1A, 0x00}, {0x14, 0x1A, 0x00}, {0x0D, 0x01, 0x00}, {0x15, 0x20, 0x00}, {0x37, 0x48, 0x00},        \
            {0x45, 0x00, 0x00}, {0x0D, 0x01, 0x00}, {0x45, 0x00, 0x00}, {0x37, 0x08, 0x00}, {0x14, 0x1A, 0x00}, {0x12, 0x00, 0x00}, {0x0E, 0x00, 0x00},        \
            {0x32, 0xBF, 0x00},                                                                                                                                \
    }

typedef struct
{
    qosa_uint32_t regAddr; /*!< Register address */
    qosa_uint16_t val;     /*!< The configured value */
    qosa_uint16_t delay;   /*!< delay time */
} AUD_CODEC_REG_T;

/**
 * @brief codec initialization function
 *
 * This function performs the initialization configuration of the ES8311 audio codec,
 * including the setting of pin functions, the initialization of the I2C interface,
 * as well as the configuration of the internal registers of the codec.
 *
 * @param NONE
 *
 * @return Initialize the result. A value of 0 indicates success, while any non-zero value indicates failure.
 */
int unir_codec_init()
{
    qosa_int32_t    ret = 0;
    AUD_CODEC_REG_T reg_list[] = ES8311_INIT_REG;
    // pin set
    qosa_pin_set_func(67, 2);
    qosa_pin_set_func(66, 2);

    // initialize IIC
    ret = qosa_i2c_init(QOSA_I2C_1, QOSA_IIC_STANDARD_MODE);

    qosa_task_sleep_ms(1000);
    // Configure the codec register
    for (int i = 0; i < sizeof(reg_list) / sizeof(reg_list[0]); i++)
    {
        ret = qosa_i2c_write(QOSA_I2C_1, QCM_ES8311_I2C_ADDR, reg_list[i].regAddr, (qosa_uint8_t *)&(reg_list[i].val), 1);
        qosa_task_sleep_ms(1);
    }
    return ret;
}

/**
 * @brief TTS callback function to handle TTS events
 *
 * This function serves as a callback handler for TTS events, releasing a semaphore when TTS playback is completed.
 *
 * @param event Type of TTS event that indicates the specific TTS event occurred
 * @param data Pointer to the event-related data
 * @param size Size of the event data
 * @return No return value
 */
void tts_callback(qosa_tts_event_t event, qosa_uint8_t *data, qosa_uint32_t size)
{
    if (event == QOSA_TTS_EVENT_PLAY_FINISH)
    {
        qosa_sem_release(g_audio_tts_play_finish_sem);
    }
}


/**
 * @brief When a client needs to use a separate TTS solution, this callback is used to read the TTS repository. 
 * The location of the repository can be customized by the client, but file system interfaces cannot be directly manipulated within this interface.
 *
 * @param pBuffer: This address is used internally by the engine library. Data with an offset of ipos and a length of nsize is read into this address, 
                        and the internal engine will process it automatically.
 * @param iPos: The engine library needs to read the offset of the data relative to the parameter pParameter each time.
 * @param nSize: The length to be read does not need to be processed by the customer.
 *
 * @return No return value
 */
void user_read_res_cb(void* pBuffer,uint32_t iPos,uint32_t nSize)
{
    /*For example, if the TTS resources are placed on an external flash memory, SPI can be used for reading.
        N_addr is the offset address of the resource storage on the external flash memory. For example, 
        if it is stored at the beginning address of the flash memory, then N should be 0; otherwise, the corresponding address should be entered.*/
    //qcm_spi_nor_read(UNIR_TEST_SPI_PORT, pBuffer, N_addr+iPos, nSize);
    //QLOGV("tts p:%x,s%x", iPos, nSize);
}

/**
 * @brief Audio TTS test function
 *
 * This function tests the audio TTS (Text-to-Speech) functionality by configuring TTS parameters,
 * playing the specified text, waiting for playback completion using a semaphore, and finally
 * closing the TTS service.
 *
 * @return int Return value, 0 indicates success, -1 indicates failure
 */
int unir_audio_tts_test()
{
    qosa_tts_cfg_t config = {0};
    // Configure TTS parameter structure, set language to Chinese, callback function to tts_callback
    config.langusge = QOSA_TTS_LANGUAGE_CHN;
    config.callback = tts_callback;
    // If the customer needs to place the TTS resource library themselves, then read_cb needs to be registered.
    //config.read_cb = user_read_res_cb;
    
    qosa_tts_open(&config);
    qosa_tts_play(QOSA_TTS_ENCODING_UTF8, "123,欢迎使用移远模块", qosa_strlen("123,欢迎使用移远模块"));

    // Create semaphore to wait for TTS playback completion
    qosa_sem_create(&g_audio_tts_play_finish_sem, 0);
    if (QOSA_OK != qosa_sem_wait(g_audio_tts_play_finish_sem, QOSA_WAIT_FOREVER))
    {
        qosa_sem_delete(g_audio_tts_play_finish_sem);
        return -1;
    }

    qosa_sem_delete(g_audio_tts_play_finish_sem);

    qosa_tts_close(QOSA_FALSE);
    return 0;
}

/**
 * @brief tts demo processing function
 * @param arg Task parameter pointer
 * @return No return value
 *
 * This function demonstrates audio functions, including external codec initialization,
 * tts play
 */
void unir_tts_demo_process(void *arg)
{
    qosa_int32_t ret = 0;
    qosa_task_sleep_sec(5);
    QLOGV("enter unir_audio_demo_process !!!");
#if QOSA_TTS_RESOURCE_SET // If using the TTS separation solution, you need to mount the external file system first, and then set the resource path
    qcm_ext_flash_lfs_register(0);
    qosa_tts_resource_init((qosa_int8_t *)"/extnor/quectel_tts_resource_chinese_16k.bin",QOSA_NULL);
#endif
    qosa_aud_i2s_cfg_t i2s_aud_cfg = {0, 0, 1, 1};
    qosa_aud_output_ctrl(QOSA_AUDIO_OUTPUT_EXTERNAL, &i2s_aud_cfg);
    // External codec initialization
    ret = unir_codec_init();
    if (ret != 0)
    {
        QLOGD("codec init failed");
        return;
    }

    // tts play
    unir_audio_tts_test();
    qosa_task_sleep_sec(1);
    unir_audio_tts_test();
}
void unir_tts_demo_init(void)
{
    QLOGV("enter UniRTOS TTS DEMO !!!");
    if (g_unir_tts_demo_task == QOSA_NULL)
    {
        qosa_task_create(
            &g_unir_tts_demo_task,
            CONFIG_UNIRTOS_TTS_DEMO_TASK_STACK_SIZE,
            UNIR_TTS_DEMO_TASK_PRIO,
            "tts_demo",
            unir_tts_demo_process,
            QOSA_NULL,
            1
        );
    }
}
UNIRTOS_APP_EXPORT(335, "tts_demo", unir_tts_demo_init);
