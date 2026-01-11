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
#include "connect4.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
************************macro define************************
***********************************************************/

#define BOARD_COLS         COLS
#define BOARD_ROWS         ROWS

#define BOARD_COLOR       lv_color_hex(0x0000FF) // Blue background
#define USER_PIECE_COLOR  lv_color_hex(0xFF0000) // Red
#define CHATBOT_PIECE_COLOR lv_color_hex(0xFFFF00) // Yellow
#define EMPTY_PIECE_COLOR lv_color_hex(0xf8f5f0) // Floral White
#define WINNING_PIECE_BORDER_COLOR lv_color_hex(0x000000) // Black
#define TIE_PIECE_BORDER_COLOR lv_color_hex(0xFF00FF) // Magenta

#define BOARD_BLINK_HOLD_TIME 10   // ms: how long the turn color is shown before restoring
#define BOARD_BLINK_PERIOD   2000    // ms: how often to trigger the blink

#define ALLOW_USER_TAKE_CHATBOT_TURN 1  // Set to 1 to allow user to take chatbot's turn for testing

/***********************************************************
*********************variable define************************
***********************************************************/

extern lv_obj_t* g_board_cells[BOARD_ROWS][BOARD_COLS];

/***********************************************************
***********************typedef define***********************
***********************************************************/

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


/**
 * @brief Update the color of a specific cell on the board based on the player
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @param player The player (USER, CHATBOT, or EMPTY)
 */
void gfx_update_piece_to_player(lv_obj_t* piece, Cell player);

/**
 * @brief Set the color of a specific cell on the board
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @param color The color to set
 */
void set_piece_color(lv_obj_t* piece, lv_color_t color);

/**
 * @brief Get the lvgl object for a specific cell on the board
 *
 * @param row The row of the cell
 * @param col The column of the cell
 * @return lv_obj_t* Pointer to the lvgl object representing the cell
 */
lv_obj_t* get_piece_lv_obj(int row, int col);

/**
 * @brief Clear the board display, setting all cells to empty
 *
 * @param None
 * @return None
 */
void clear_board();

/**
 * @brief Highlight the winning pieces on the board
 *
 * @param p0 The starting position of the winning sequence
 * @param p3 The ending position of the winning sequence
 */
void highlight_winning_pieces(CellPos* p0, CellPos* p3);

/**
 * @brief Highlight all pieces to indicate a tie
 */
void highlight_tie();

#endif /* __BOARD_DISPLAY_H__ */
