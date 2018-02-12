/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 console.h                                    */
/* contents: console mode "main" function and pasrsing    */
/**********************************************************/
#include "chessprog.h"
#ifndef CONSOLE_H_
#define CONSOLE_H_

#define MAX_INPUT 50
char str_in[MAX_INPUT+1];

#define ENTER_SETTINGS "Enter game settings:\n"
#define WRONG_GAME_MODE "Wrong game mode\n"
#define TWO_PLAYERS_GAME_MODE "Running game in 2 players mode\n"
#define PLAYER_VS_AI_GAME_MODE "Running game in player vs. AI mode\n"
#define WRONG_MINIMAX_DEPTH "Wrong value for minimax depth. The value should be between 1 to 4\n"
#define WRONG_FILE_NAME "Wrong file name\n"
#define WRONG_POSITION "Invalid position on the board\n"
#define NO_PIECE "Setting this piece creates an invalid board\n"
#define NO_DICS "The specified position does not contain your piece\n"
#define WROND_BOARD_INITIALIZATION "Wrong board initialization\n"
#define WRONG_COLOR "Wrong color value. Should be either white or black.\n"
#define NO_ROOK "Wrong position for a rook\n"

#define ILLEGAL_COMMAND "Illegal command, please try again\n"
#define ILLEGAL_MOVE "Illegal move\n"
#define ILLEGAL_CASTLING "Illegal castling move\n"

#define WRONG_ROOK_POSITION "Wrong position for a rook\n"
#define ILLEGAL_CALTLING_MOVE "Illegal castling move\n"

#define TIE_MSG "The game ends in a tie\n"

#define perror_message(func_name) (fprintf(stderr, "Error: standard function %s has failed\n", func_name))
#define print_message(message) (printf("%s", message))
#define print_malloc_error perror_message("malloc")

int consoleMode();
char* getInput();

void parseSettings(char* s);
pos_t parsePos(char* s);
char parsePiece(char* s, char color);

unsigned parseGame(char* s); /*user's turn to play*/
move_t* parseCastling(char *s);
move_t* parseMoveFull(char* s);
move_t* parseMove(char* s);
void updateMoveSize(move_t* move);
int isPromotionMove(char board[BOARD_SIZE][BOARD_SIZE], move_t* move);

int computerPlay();
int analizeState(int boardState);

int printAllLegalMoves(char board[BOARD_SIZE][BOARD_SIZE], char color);
void printMove(move_t* move);
void printMovesList(movesList_t* movesList);

void print_board(char board[BOARD_SIZE][BOARD_SIZE]);
void print_line();

#endif /* CONSOLE_H_ */
