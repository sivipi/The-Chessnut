/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 minimax.h                                    */
/* contents: minimax and scoring function				  */
/**********************************************************/
#include "chessprog.h"
#include <limits.h>
#ifndef MINIMAX_H_
#define MINIMAX_H_

#define DEBUG_MM 0
#define DEBUG_MM_SCORE 0

#define MM_ERROR (20*WIN_A) /*set impossible score as minimax error code*/

#define PLAYER_A 'A'
#define PLAYER_B 'B'
#define WIN_A 1000
#define WIN_B -1000
#define TIE_A -999
#define TIE_B 999
#define MM_LIMIT 1000000 /*boards limit for minimax best*/
#define DEPTH_FACTOR 6

#define MIN_INF INT_MIN
#define MAX_INF INT_MAX

#define LIST_ALL 1   /*used to return all moves and their score*/
#define LIST_BEST 0  /*used to return just moved with best score*/

movesList_t* keepBestMoves(movesList_t* answer, int bestScore);
int scoringFunction(char board[BOARD_SIZE][BOARD_SIZE], char playerA, char currentPlayer, int depth, int realDepth);
movesList_t* miniMax_lst(char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, char playerA, char currentPlayer, char returnList);
int miniMax_move(move_t* move, char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, char playerA, char currentPlayer);
move_t* miniMax_env(char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, char playerA, char currentPlayer);
int miniMax_rec(char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, char playerA, char currentPlayer, \
		int alpha, int beta, int maxBoards, int realDepth);

#endif /* MINIMAX_H_ */
