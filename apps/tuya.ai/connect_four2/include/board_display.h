#include <lvgl.h>
/**
 * @file app_display.h
 * @brief Header file for Tuya Display System
 *
 * This header file provides the declarations for initializing the display system
 * and sending messages to the display. It includes the necessary data types and
 * function prototypes for interacting with the display functionality.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __BOARD_DISPLAY_H__
#define __BOARD_DISPLAY_H__

#include "tuya_cloud_types.h"

#include "lang_config.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

typedef enum { DROP_X_PLUS, DROP_X_NEG, DROP_Y_PLUS, DROP_Y_NEG } DropDirection;

typedef struct grid_metrics_t {
    int cell_w;
    int cell_h;
    int cell_size;
    DropDirection drop_dir;
} grid_metrics_t;

/***********************************************************
********************function declaration********************
***********************************************************/
/**
 * @brief Initialize the display system
 *
 * @param None
 * @return OPERATE_RET Initialization result, OPRT_OK indicates success
 */
OPERATE_RET board_display_init(void);


#endif /* __BOARD_DISPLAY_H__ */
