/*****************************************************************/ /**
* @file tts_demo.h
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
* <tr><td>2024-09-02 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/
#ifndef __TTS_DEMO_H__
#define __TTS_DEMO_H__

#include "qosa_def.h"
#include "qosa_sys.h"

/*===========================================================================
 *  Macro Definition
 ===========================================================================*/

#define CONFIG_UNIRTOS_TTS_DEMO_TASK_STACK_SIZE 4096
#define UNIR_TTS_DEMO_TASK_PRIO                 QOSA_PRIORITY_NORMAL
#define QCM_ES8311_I2C_ADDR                      0x18

/*===========================================================================
  *  Enum
  ===========================================================================*/

void unir_demo_audio_init();

#endif /* __TTS_DEMO_H__ */
