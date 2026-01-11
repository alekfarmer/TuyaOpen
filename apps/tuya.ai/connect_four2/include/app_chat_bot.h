/**
 * @file app_chat_bot.h
 * @brief app_chat_bot module is used to
 * @version 0.1
 * @date 2025-03-25
 */

#ifndef __APP_CHAT_BOT_H__
#define __APP_CHAT_BOT_H__

#include "tuya_cloud_types.h"
#include "connect4.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

typedef enum {
    AI_AUDIO_ALERT_NORMAL = 0,
    AI_AUDIO_ALERT_POWER_ON,
    AI_AUDIO_ALERT_NOT_ACTIVE,
    AI_AUDIO_ALERT_NETWORK_CFG,
    AI_AUDIO_ALERT_NETWORK_CONNECTED,
    AI_AUDIO_ALERT_NETWORK_FAIL,
    AI_AUDIO_ALERT_NETWORK_DISCONNECT,
    AI_AUDIO_ALERT_BATTERY_LOW,
    AI_AUDIO_ALERT_PLEASE_AGAIN,
    AI_AUDIO_ALERT_WAKEUP,
    AI_AUDIO_ALERT_LONG_KEY_TALK,
    AI_AUDIO_ALERT_KEY_TALK,
    AI_AUDIO_ALERT_WAKEUP_TALK,
    AI_AUDIO_ALERT_FREE_TALK,
} AI_AUDIO_ALERT_TYPE_E;

/***********************************************************
*******************varible definition***********************
***********************************************************/

extern Connect4Game game;

/***********************************************************
********************function declaration********************
***********************************************************/
OPERATE_RET app_chat_bot_init(void);

uint8_t app_chat_bot_get_enable(void);

/**
 * @brief Plays an alert sound based on the specified alert type.
 *
 * @param type - The type of alert to play, defined by the APP_ALERT_TYPE_E enum.
 * @return OPERATE_RET - Returns OPRT_OK if the alert sound is successfully played, otherwise returns an error code.
 */
OPERATE_RET ai_audio_player_play_alert(AI_AUDIO_ALERT_TYPE_E type);
int         check_for_match(uint8_t *buffer, size_t buflen, const char *pattern);
void        serial_print(const char *msg);

#if defined(ENABLE_KEYBOARD_INPUT) && (ENABLE_KEYBOARD_INPUT == 1)
// Include keyboard input header to get KEYBOARD_EVENT_E type
#include "keyboard_input.h"

/**
 * @brief Handles keyboard events from board layer
 *
 * This function is called from the board layer when keyboard
 * events are detected. All business logic is handled here.
 *
 * @param event - The keyboard event type
 */
void app_chat_bot_keyboard_event_handler(KEYBOARD_EVENT_E event);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APP_CHAT_BOT_H__ */
