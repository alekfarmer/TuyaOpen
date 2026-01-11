/**
 * @file app_display.c
 * @author Tuya Inc.
 * @brief Handle display initialization and message processing
 *
 * This source file provides the implementation for initializing the display system,
 * creating a message queue, and handling display messages in a separate task.
 * It includes functions to initialize the display, send messages to the display,
 * and manage the display task.
 *
 * @copyright Copyright (c) 2021-2025 Tuya Inc. All Rights Reserved.
 *
 */

#include "tuya_cloud_types.h"

#include "board_display.h"
#include "tuya_lvgl.h"

#include "font_awesome_symbols.h"
#include "ui_display.h"

#include "tal_log.h"
#include "tal_queue.h"
#include "tal_thread.h"

#include "tkl_memory.h"

#include "lvgl.h"

/***********************************************************
************************macro define************************
***********************************************************/

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

OPERATE_RET board_display_init(void);
lv_obj_t* build_board_graphic(lv_obj_t* parent, int board_cols, int board_rows, DropDirection drop_dir, grid_metrics_t *metrics);

/***********************************************************
***********************variable define**********************
***********************************************************/

/***********************************************************
***********************function define**********************
***********************************************************/

/**
 * @brief Initialize the display system
 *
 * @param None
 * @return OPERATE_RET Initialization result, OPRT_OK indicates success
 */

OPERATE_RET board_display_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    // memset(&sg_display, 0, sizeof(TUYA_DISPLAY_T));

    // lvgl initialization
    TUYA_CALL_ERR_RETURN(tuya_lvgl_init());
    PR_DEBUG("lvgl init success");

    tuya_lvgl_mutex_lock();


    lv_obj_t *scr_main = lv_obj_create(NULL); // Create a new screen object
    lv_obj_set_size(scr_main, LV_HOR_RES, LV_VER_RES); // Use full screen size
    lv_obj_align(scr_main, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(scr_main, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
    lv_obj_set_style_border_width(scr_main, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr_main, 0, LV_PART_MAIN);

    /* Disable scrolling on the main container */
    lv_obj_set_scroll_dir(scr_main, LV_DIR_NONE);
    lv_obj_clear_flag(scr_main, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scr_main, LV_SCROLLBAR_MODE_OFF);

    lv_scr_load(scr_main); // Make this screen active

    {
        lv_obj_t * dbg_rect = lv_obj_create(scr_main);
        lv_obj_set_size(dbg_rect, 10, 10);
        lv_obj_set_pos(dbg_rect, 0, 0);
        lv_obj_set_style_bg_color(dbg_rect, lv_color_hex(0xFF0000), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(dbg_rect, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(dbg_rect, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(dbg_rect, 0, LV_PART_MAIN);
        lv_obj_clear_flag(dbg_rect, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_move_foreground(dbg_rect);
    }

    grid_metrics_t metrics;
    lv_obj_t* board = build_board_graphic(scr_main, 6, 7, DROP_Y_PLUS, &metrics);
    lv_obj_move_foreground(board);

    tuya_lvgl_mutex_unlock();

    PR_DEBUG("lvgl write success");

    // TUYA_CALL_ERR_RETURN(tal_queue_create_init(&sg_display.queue_hdl, sizeof(DISPLAY_MSG_T), 8));
    // THREAD_CFG_T cfg = {
    //     .thrdname = "chat_ui",
    //     .priority = THREAD_PRIO_2,
    //     .stackDepth = 1024 * 4,
    // };
    // TUYA_CALL_ERR_RETURN(tal_thread_create_and_start(&sg_display.thrd_hdl, NULL, NULL, __chat_bot_ui_task, NULL, &cfg));
    // PR_DEBUG("chat bot ui task create success");

    return rt;
}

lv_obj_t* build_board_graphic(lv_obj_t* parent, int board_cols, int board_rows, DropDirection drop_dir, grid_metrics_t *metrics)
{
    if (parent == NULL) parent = lv_scr_act();

    PR_DEBUG("Generating board of size %d x %d: cell size %d x %d", board_cols, board_rows);

    int w = lv_obj_get_width(parent);
    int h = lv_obj_get_height(parent);

    /* Connect Four board: 7 columns x 6 rows */
    const int grid_cols = board_cols;
    const int grid_rows = board_rows;
    int padding = 0; /* padding from parent edges */

    /* Compute cell sizes to fill padded area */
    int max_w = w - 2 * padding;
    int max_h = h - 2 * padding;
    if (max_w < 0) max_w = 0;
    if (max_h < 0) max_h = 0;

    int cell_w = (board_cols > 0) ? (max_w / board_cols) : 0;
    int cell_h = (board_rows > 0) ? (max_h / board_rows) : 0;
    if (cell_w <= 0) cell_w = 1;
    if (cell_h <= 0) cell_h = 1;

    /* Store cell metrics for later positioning */
    metrics->cell_w = cell_w;
    metrics->cell_h = cell_h;
    metrics->cell_size = LV_MIN(cell_w, cell_h);
    metrics->drop_dir = drop_dir;

    /* Board fills padded area exactly */
    int board_w = max_w;
    int board_h = max_h;
    int board_x = padding;
    int board_y = padding;

    /* Create board background */
    lv_obj_t * board = lv_obj_create(parent);
    lv_obj_set_size(board, board_w, board_h);
    lv_obj_set_pos(board, board_x, board_y);
    lv_obj_set_style_bg_color(board, lv_color_hex(0x0000FF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(board, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(board, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(board, 0, LV_PART_MAIN); /* force no corner radius */

    /* Disable board scrolling (ensure it won't move) */
    lv_obj_set_scroll_dir(board, LV_DIR_NONE);
    lv_obj_clear_flag(board, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(board, LV_SCROLLBAR_MODE_OFF);

    /* Create circular holes inside the board (background color) */
    for (int r = 0; r < grid_rows; r++) {
        for (int c = 0; c < grid_cols; c++) {
            int hole_d = (LV_MIN(metrics->cell_w, metrics->cell_h) * 80) / 100; /* 80% of smaller cell dimension */
            lv_obj_t * hole = lv_obj_create(board);
            lv_obj_set_size(hole, hole_d, hole_d);
            lv_obj_set_style_radius(hole, LV_RADIUS_CIRCLE, LV_PART_MAIN);
            lv_obj_set_style_bg_color(hole, lv_color_hex(0xf8f5f0), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(hole, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_border_width(hole, 0, LV_PART_MAIN);
            lv_obj_clear_flag(hole, LV_OBJ_FLAG_CLICKABLE);

            /* Compute hole position spread evenly across full board width/height so padding is symmetric */
            int hx = (c * board_w) / grid_cols + ((board_w / grid_cols) - hole_d) / 2;
            int hy = (r * board_h) / grid_rows + ((board_h / grid_rows) - hole_d) / 2;
            lv_obj_set_pos(hole, hx, hy);
        }
    }

    return board;
}