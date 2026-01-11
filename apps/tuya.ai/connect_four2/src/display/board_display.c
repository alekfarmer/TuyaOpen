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
#include "connect4.h"
#include "app_chat_bot.h"

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

//#define DEBUG_GRAPHICS

/***********************************************************
***********************typedef define***********************
***********************************************************/

/***********************************************************
********************function declaration********************
***********************************************************/

// Board blink: set color, then restore after hold time using a timer
static lv_obj_t* s_board_bg = NULL;
static lv_timer_t* s_blink_restore_timer = NULL;

static void board_blink_restore_cb(lv_timer_t* timer) {
    if (s_board_bg) {
        lv_obj_set_style_bg_color(s_board_bg, BOARD_COLOR, LV_PART_MAIN);
    }
    if (s_blink_restore_timer) {
        lv_timer_del(s_blink_restore_timer);
        s_blink_restore_timer = NULL;
    }
}

void blink_board_for_turn(void) {
    if (!s_board_bg) return;
    // Set to the current turn color instantly
    lv_color_t base = (game.state == USER_TURN) ? USER_PIECE_COLOR : CHATBOT_PIECE_COLOR;

    lv_obj_set_style_bg_color(s_board_bg, base, LV_PART_MAIN);
    // Set a timer to restore the blue background after hold time
    if (s_blink_restore_timer) {
        lv_timer_del(s_blink_restore_timer);
    }
    s_blink_restore_timer = lv_timer_create(board_blink_restore_cb, BOARD_BLINK_HOLD_TIME, NULL);
}

OPERATE_RET board_display_init(void);
lv_obj_t* build_board_graphic(lv_obj_t* parent);
void set_piece_color(lv_obj_t* piece, lv_color_t color);
void clear_board();

/***********************************************************
***********************variable define**********************
***********************************************************/

lv_obj_t* g_board_cells[BOARD_ROWS][BOARD_COLS];

/***********************************************************
***********************function define**********************
***********************************************************/

// Event handler to update cell's player on press
static void press_piece(lv_obj_t * cell) {

    // Get the index of the cell
    CellPos* cell_pos = (CellPos*)lv_obj_get_user_data(cell);
    int col = cell_pos->col;

    if (game.state == USER_TURN) {
        dropPiece(&game, col); 
    } else if ((game.state == CHATBOT_TURN || game.state == WAIT_FOR_START) && ALLOW_USER_TAKE_CHATBOT_TURN) {
        dropPiece(&game, col); 
    } else {
        PR_ERR("It's not your turn. Current game state: %d", game.state);
        return;
    }

    // // Update the game state
    // Cell currentState = game.board[row][col];
    // if (currentState == EMPTY) {
    //    setPiece(&game, cell_pos->row, cell_pos->col, USER); 
    // } else if (currentState == USER) {
    //     setPiece(&game, cell_pos->row, cell_pos->col, CHATBOT);
    // } else {
    //     setPiece(&game, cell_pos->row, cell_pos->col, EMPTY);
    // }
    
}

// Event handler to update cell's player on press
static void cell_press_handler(lv_event_t * e) {
    lv_obj_t * cell = lv_event_get_target(e);
    press_piece(cell);
}

// // Timer callback to turn next cell red
// static void next_cell_red_timer_cb(lv_timer_t * timer) {
//     static int cell_idx = 0;
//     int total_cells = BOARD_ROWS * BOARD_COLS;
//     int row = cell_idx / BOARD_COLS;
//     int col = cell_idx % BOARD_COLS;
//     press_piece(g_board_cells[row][col]);
//     cell_idx++;
//     if (cell_idx >= total_cells) {
//         cell_idx = 0;
//     }
// }

// Timer callback to trigger board blink animation every period
static void board_blink_timer_cb(lv_timer_t * timer) {
    if (game.state == USER_TURN || game.state == CHATBOT_TURN) {
        blink_board_for_turn();
    }
}

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

    lv_obj_t* board = build_board_graphic(scr_main);
    lv_obj_move_foreground(board);

    #ifdef DEBUG_GRAPHICS
    // for (int i = 0; i < BOARD_ROWS; i++) {
    //     set_cell_color(i % BOARD_ROWS, i % BOARD_COLS, lv_color_hex(0xFF0000)); // Diagonal red
    //     set_cell_color((i+1) % BOARD_ROWS, i % BOARD_COLS, lv_color_hex(0xFFFF00)); // Diagonal yellow
    // }

    // Draw a small red rectangle in the top left corner for debug
    lv_obj_t* debug_rect = lv_obj_create(board);
    lv_obj_set_size(debug_rect, 20, 20); // 20x20 pixels
    lv_obj_align(debug_rect, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(debug_rect, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_border_width(debug_rect, 0, LV_PART_MAIN);
    lv_obj_move_foreground(debug_rect);
    lv_obj_set_style_radius(debug_rect, 0, LV_PART_MAIN);

    // Draw a small red rectangle in the bottom right corner for debug
    lv_obj_t* debug_rect2 = lv_obj_create(board);
    lv_obj_set_size(debug_rect2, 20, 20); // 20x20 pixels
    lv_obj_align(debug_rect2, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(debug_rect2, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_border_width(debug_rect2, 0, LV_PART_MAIN);
    lv_obj_move_foreground(debug_rect2);
    lv_obj_set_style_radius(debug_rect2, 0, LV_PART_MAIN);
    #endif

    // lv_obj_add_event_cb(scr_main, screen_press_handler, LV_EVENT_PRESSED, NULL);
    // Create timer to turn next cell red every 2 seconds
    // lv_timer_create(next_cell_red_timer_cb, 500, NULL);
    lv_timer_create(board_blink_timer_cb, BOARD_BLINK_PERIOD, NULL);

    tuya_lvgl_mutex_unlock();

    // Start periodic timer to trigger board blink animation
    blink_board_for_turn();

    return rt;
}

lv_obj_t* build_board_graphic(lv_obj_t* parent)
{
    if (parent == NULL) parent = lv_scr_act();

    int w = lv_obj_get_height(parent); // Reversed for landscape
    int h = lv_obj_get_width(parent); // Reversed for landscape

    /* Connect Four board: 7 columns x 6 rows */
    int padding = 0; /* padding from parent edges */

    /* Compute cell sizes to fill padded area */
    int board_w = w - 2 * padding;
    int board_h = h - 2 * padding;
    if (board_w < 0) board_w = 0;
    if (board_h < 0) board_h = 0;
    int cell_w = (BOARD_COLS > 0) ? (board_w / (BOARD_COLS)) : 0;
    int cell_h = (BOARD_ROWS > 0) ? (board_h / (BOARD_ROWS)) : 0;
    if (cell_w <= 0) cell_w = 1;
    if (cell_h <= 0) cell_h = 1;

    /* Create board background */
    lv_obj_t * board = lv_obj_create(parent);
    s_board_bg = board;
    lv_obj_set_size(board, board_h, board_w); // Reversed for landscape
    lv_obj_set_style_pad_all(board, 0, LV_PART_MAIN);
    lv_obj_align(board, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(board, BOARD_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(board, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(board, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(board, 0, LV_PART_MAIN); /* force no corner radius */

    /* Disable board scrolling (ensure it won't move) */
    lv_obj_set_scroll_dir(board, LV_DIR_NONE);
    lv_obj_clear_flag(board, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(board, LV_SCROLLBAR_MODE_OFF);

    /* Create circular holes inside the board (background color) */
    for (int c = 0; c < BOARD_COLS; c++) {
        for (int r = 0; r < BOARD_ROWS; r++) {
            int hole_d = LV_MIN(cell_w, cell_h) * 0.8f; /* 80% of smaller cell dimension */
            lv_obj_t * hole = lv_obj_create(board);
            lv_obj_set_size(hole, hole_d, hole_d);
            lv_obj_set_style_radius(hole, LV_RADIUS_CIRCLE, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(hole, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_bg_color(hole, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_border_width(hole, 0, LV_PART_MAIN);

            #ifdef DEBUG_GRAPHICS
            // Add label to show row and column index, rotated 90 degrees CW
            lv_obj_t * label = lv_label_create(hole);
            char idx_text[8];
            snprintf(idx_text, sizeof(idx_text), "%d,%d", r, c);
            lv_label_set_text(label, idx_text);
            lv_obj_set_style_transform_angle(label, 900, LV_PART_MAIN); // 900 = 90deg in 0.1deg units
            lv_obj_center(label);
            lv_obj_set_style_text_color(label, lv_color_hex(0x333333), LV_PART_MAIN);
            // Remove scrollbars and scrolling from label
            lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_scrollbar_mode(label, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_scroll_dir(label, LV_DIR_NONE);
            #endif

            /* Compute hole position spread evenly across full board width/height so padding is symmetric */
            int hx = cell_w*c + (cell_w - hole_d)/2;
            int hy = cell_h*(BOARD_ROWS - r - 1) + (cell_h - hole_d)/2;
            lv_obj_align(hole, LV_ALIGN_TOP_LEFT, hy, hx); // Reversed for landscape

            // Store cell object for later access
            g_board_cells[r][c] = hole;

            CellPos* pos = (CellPos*)malloc(sizeof(CellPos));
            pos->row = r;
            pos->col = c;
            lv_obj_set_user_data(hole, pos);

            // Add event handler for press
            lv_obj_add_event_cb(hole, cell_press_handler, LV_EVENT_PRESSED, NULL);
        }

        // Set them all to white
        clear_board();
    }

    return board;
}

void gfx_update_piece_to_player(lv_obj_t* piece, Cell player)
{
    if (player == USER) {
        set_piece_color(piece, USER_PIECE_COLOR);
    } else if (player == CHATBOT) {
        set_piece_color(piece, CHATBOT_PIECE_COLOR);
    } else {
        set_piece_color(piece, EMPTY_PIECE_COLOR);
    }
}

void set_piece_color(lv_obj_t* piece, lv_color_t color)
{
    if (piece == NULL) {
        PR_ERR("set_piece_color: Invalid piece (NULL)");
        return;
    }

    // PR_DEBUG("set_piece_color: Setting color of piece");
    lv_obj_set_style_bg_color(piece, color, LV_PART_MAIN);

    // Make a border with a slightly darker color
    lv_color_t border_color = lv_color_darken(color, 50);
    lv_obj_set_style_border_color(piece, border_color, LV_PART_MAIN);
    lv_obj_set_style_border_width(piece, 4, LV_PART_MAIN);
}

lv_obj_t* get_piece_lv_obj(int row, int col)
{
    if (row < 0 || row >= BOARD_ROWS || col < 0 || col >= BOARD_COLS) {
        PR_ERR("get_piece_lv_obj: Invalid row or column index (%d, %d)", row, col);
        return NULL;
    }

    return g_board_cells[row][col];
}

void clear_board()
{
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            lv_obj_t* piece = get_piece_lv_obj(r, c);
            gfx_update_piece_to_player(piece, EMPTY);
        }
    }
}

void highlight_winning_pieces(CellPos* p0, CellPos* p3)
{
    if (p0 == NULL || p3 == NULL) return;

    int r0 = p0->row;
    int c0 = p0->col;
    int r3 = p3->row;
    int c3 = p3->col;

    int dr = (r3 - r0) / 3;
    int dc = (c3 - c0) / 3;

    for (int i = 0; i < 4; i++) {
        int r = r0 + i * dr;
        int c = c0 + i * dc;
        lv_obj_t* piece = get_piece_lv_obj(r, c);
        if (piece) {
            // Only change the border color to highlight
            lv_obj_set_style_border_color(piece, WINNING_PIECE_BORDER_COLOR, LV_PART_MAIN);
            lv_obj_set_style_border_width(piece, 6, LV_PART_MAIN);
        }
    }
}

void highlight_tie()
{
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            lv_obj_t* piece = get_piece_lv_obj(r, c);
            if (piece) {
                lv_obj_set_style_border_color(piece, TIE_PIECE_BORDER_COLOR, LV_PART_MAIN);
                lv_obj_set_style_border_width(piece, 6, LV_PART_MAIN);
            }
        }
    }
}