#ifndef CONNECT4_H
#define CONNECT4_H

#define ROWS 6
#define COLS 7

#include <stdint.h>

typedef enum { WAIT_FOR_START, USER_TURN, CHATBOT_TURN, USER_WIN, CHATBOT_WIN, DRAW } GameState;
typedef enum { EMPTY = 0, USER = 1, CHATBOT = 2 } Cell;

typedef struct {
    Cell      board[ROWS][COLS];
    GameState state;
    int       currentPlayer;
    int       movesMade;
} Connect4Game;

// Function prototypes
void initGame(Connect4Game *game);
int  dropPiece(Connect4Game *game, int col);
int  checkWin(Connect4Game *game);
int  isBoardFull(Connect4Game *game);

void exportBoardToString(Connect4Game *game, char *buffer);

static const uint8_t prompt_data[] =
    "I want to play a game of Connect 4 with you. You will be Player O and I will be Player X.\n\n"
    "The Rules:\n"
    "1. The board is a grid of 6 rows and 7 columns.\n"
    "2. We take turns dropping a piece into one of the 7 columns (0-6).\n"
    "3. The goal is to get four of your pieces in a row (horizontally, vertically, or diagonally).\n"
    "4. After every move I make, I will send you the current state of the board.\n\n"
    "Your Instructions:\n"
    "- Analyze the current state. Look for immediate threats or opportunities.\n"
    "- Think step-by-step: 'If I play in column [N], what happens?'\n"
    "- Output your move clearly as: col:[Column Number].\n"
    "- Briefly explain your reasoning.\n\n"
    "The Board Format:\n"
    "Rows are 0 (top) to 5 (bottom). Columns are 0 (left) to 6 (right).\n"
    "Empty cells: '.', Player 1: 'X', Player 2: 'O'.\n\n"
    "Here is the current board:\n"
    "Respond with <Ready to Play> when you understand the rules and are ready to start the game.";

#endif