#include "connect4.h"
#include "board_display.h"
#include <stdio.h>
#include <string.h>
#include "app_chat_bot.h"
#include "ai_audio_agent.h"

void initGame(Connect4Game *game)
{
    game->currentPlayer = USER;
    game->movesMade     = 0;
    game->state         = WAIT_FOR_START;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            game->board[i][j] = EMPTY;
        }
    }
    clear_board();
}

int dropPiece(Connect4Game *game, int col)
{
    static char board_str[256] = {0};
    if (col < 0 || col >= COLS) {
        return 0; // Invalid column
    }

    // Check from bottom up for the first empty spot
    for (int i = ROWS - 1; i >= 0; i--) {
        if (game->board[i][col] == EMPTY) {
            setPiece(game, i, col, game->currentPlayer);
            game->movesMade++;

            exportBoardToString(game, board_str);
            serial_print(board_str);

            CellPos p0, p3;
            if (checkWin(game, &p0, &p3)) {
                highlight_winning_pieces(&p0, &p3);
                game->state = (game->currentPlayer == USER) ? USER_WIN : CHATBOT_WIN;
                if (game->currentPlayer == USER) {
                    serial_print("I win!");
                    ai_text_agent_upload((uint8_t *)"I win", sizeof("I win"));
                } else {
                    ai_text_agent_upload((uint8_t *)"Chatbot wins", sizeof("Chatbot wins"));
                    serial_print("Chatbot wins!");
                }
                game->state = WAIT_FOR_START;

                initGame(game);
                ai_text_agent_upload((uint8_t *)prompt_data, sizeof(prompt_data));
            } else if (isBoardFull(game)) {
                game->state = DRAW;
                serial_print("It's a draw!");
                game->state = WAIT_FOR_START;
                initGame(game);
                ai_text_agent_upload((uint8_t *)prompt_data, sizeof(prompt_data));
            } else {
                game->currentPlayer = (game->currentPlayer == USER) ? CHATBOT : USER;
                game->state         = (game->currentPlayer == USER) ? USER_TURN : CHATBOT_TURN;
                if (game->currentPlayer == USER) {
                    serial_print("Your turn.");
                } else {
                    ai_text_agent_upload((uint8_t *)board_str, strlen(board_str));
                    serial_print("Chatbot's turn.");
                }
            }
            return 1; // Success
        }
    }
    serial_print("Column is full.");
    return 0; // Column full
}

int setPiece(Connect4Game *game, int row, int col, Cell player)
{
    // PR_DEBUG("setPiece: Setting piece at (%d, %d) to player %d", row, col, player);
    // Sets the new player information and updates the graphics
    game->board[row][col] = player;

    // graphics update
    lv_obj_t *piece = get_piece_lv_obj(row, col);
    gfx_update_piece_to_player(piece, player);

    // char boardStr[ROWS * COLS * 3 + ROWS + 1] = {0}; // Enough space for board representation
    // exportBoardToString(game, boardStr);
    // PR_DEBUG("%s", boardStr);
    return 0;
}

int checkWin(Connect4Game *game, CellPos *p0, CellPos *p3)
{
    Cell p = game->currentPlayer;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (game->board[r][c] != p)
                continue;

            // Check Horizontal
            if (c + 3 < COLS && game->board[r][c + 1] == p && game->board[r][c + 2] == p &&
                game->board[r][c + 3] == p) {
                if (p0)
                    *p0 = (CellPos){r, c};
                if (p3)
                    *p3 = (CellPos){r, c + 3};
                return 1;
            }

            // Check Vertical
            if (r + 3 < ROWS && game->board[r + 1][c] == p && game->board[r + 2][c] == p &&
                game->board[r + 3][c] == p) {
                if (p0)
                    *p0 = (CellPos){r, c};
                if (p3)
                    *p3 = (CellPos){r + 3, c};
                return 1;
            }

            // Check Diagonal (Down-Right)
            if (r + 3 < ROWS && c + 3 < COLS && game->board[r + 1][c + 1] == p && game->board[r + 2][c + 2] == p &&
                game->board[r + 3][c + 3] == p) {
                if (p0)
                    *p0 = (CellPos){r, c};
                if (p3)
                    *p3 = (CellPos){r + 3, c + 3};
                return 1;
            }

            // Check Diagonal (Up-Right)
            if (r - 3 >= 0 && c + 3 < COLS && game->board[r - 1][c + 1] == p && game->board[r - 2][c + 2] == p &&
                game->board[r - 3][c + 3] == p) {
                if (p0)
                    *p0 = (CellPos){r, c};
                if (p3)
                    *p3 = (CellPos){r - 3, c + 3};
                return 1;
            }
        }
    }
    return 0;
}

int isBoardFull(Connect4Game *game)
{
    return game->movesMade >= ROWS * COLS;
}

void exportBoardToString(Connect4Game *game, char *buffer)
{
    buffer[0] = '\0'; // Initialize string as empty
    char temp[8];     // Temporary holder for cell strings

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            char symbol = '.';
            if (game->board[i][j] == USER)
                symbol = 'X';
            if (game->board[i][j] == CHATBOT)
                symbol = 'O';

            sprintf(temp, "%c ", symbol);
            strcat(buffer, temp);
        }
        strcat(buffer, "\n"); // Add newline at the end of each row
    }
}