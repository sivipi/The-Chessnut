/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 chessprog.h                                  */
/* contents: main function, game logics and utilities     */
/**********************************************************/
#ifndef CHESSPROG_H_
#define CHESSPROG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#define DEBUG 0

#define WHITE 'w'
#define BLACK 's'
#define invColor(C) ((C)==WHITE? BLACK:WHITE)
#define getColor(P) (islower(P)? WHITE:BLACK)

#define W_PAWN 'm'
#define B_PAWN 'M'
#define W_BISHOP 'b'
#define B_BISHOP 'B'
#define W_ROOK 'r'
#define B_ROOK 'R'
#define W_KNIGHT 'n'
#define B_KNIGHT 'N'
#define W_QUEEN 'q'
#define B_QUEEN 'Q'
#define W_KING 'k'
#define B_KING 'K'
#define EMPTY ' '
#define CASTLE 'z' /*special mark for castling move*/
#define NORM '\0' /*mark for non-special moves*/

#define CONTINUE 'c'
#define TIE 't'
#define CHECK 'C'

#define BOARD_SIZE 8
#define inBoard(C,R) (0<=(C) && 0<=(R) && (C)<BOARD_SIZE && (R)<BOARD_SIZE)
/*Condition: inBoard && pos != startPos && [empty || opponent]*/
#define LEGAL 	inBoard(pos.col, pos.row) && (board[pos.col][pos.row]==EMPTY || \
				color!=getColor(board[pos.col][pos.row]))

typedef struct { /*index representation for location on board, range [0,BOARD_SIZE-1]*/
	int col;
	int row;
} pos_t;

typedef struct move_t { /*first pos_t in move_t representds starting pos*/
	pos_t curr;
	struct move_t* next;
	unsigned size; /*represent amount of elements in linked list*/
	int score; /*to store minimax score*/
	char special;  /*to store markers for special moves (castling and promotions)*/
} move_t;

typedef struct movesList_t {
	move_t* curr;
	struct movesList_t* next;
	unsigned size;
} movesList_t;

/* GLOBALS */
extern char gameBoard[BOARD_SIZE][BOARD_SIZE]; /*[cols][rows]*/
extern unsigned startGame;
extern unsigned minimaxDepth;
extern char userColor;
extern char gameMode;
extern char nextPlayer;
extern char wk, wlr, wrr, bk, blr, brr;

#define BEST 5 /*not actual depth, just higher than max (4) to act as code*/
#define PVP 1
#define PVA 2
#define computerColor (invColor(userColor))

void quit(int ret); /*set startGame to 0, both settings end game*/

/*settings state*/
void clearBoard(char board[BOARD_SIZE][BOARD_SIZE]);
void rm(char board[BOARD_SIZE][BOARD_SIZE], pos_t position);
void setPiece(char board[BOARD_SIZE][BOARD_SIZE], pos_t pos, char piece);
unsigned start(char board[BOARD_SIZE][BOARD_SIZE]); /*if can start return 1, else 0*/
void countPieces(char board[BOARD_SIZE][BOARD_SIZE], unsigned answer[12]);

/*game state*/
unsigned isValidMove(move_t* move);
unsigned isValidPos(pos_t pos); /*deals with case 1 in command move*/
unsigned isUserPos(char board[BOARD_SIZE][BOARD_SIZE], pos_t pos, char color); //zohar /*deals with case 2 in command move*/
unsigned isLegalMove(char board[BOARD_SIZE][BOARD_SIZE], move_t* userMove, char color); //zohar /*deals with case 3 in command move*/
unsigned isSameMove(move_t* m1, move_t* m2);
void playMove(char board[BOARD_SIZE][BOARD_SIZE], move_t *move); /*update board*/
int evalBoard(char board[BOARD_SIZE][BOARD_SIZE], char nextPlayer); /*decide if win/tie/cont*/
int isCheck(char board[BOARD_SIZE][BOARD_SIZE], char nextPlayer);
pos_t getKingPos(char board[BOARD_SIZE][BOARD_SIZE], char player);
int canReachPos(pos_t pos, movesList_t* moves);
void updateCastlingFlags(char piece, pos_t pos);

#define saveCastlingFlags() char wks=wk, wlrs=wlr, wrrs=wrr, bks=bk, blrs=blr, brrs=brr
#define restoreCastlingFlags() wk=wks; wlr=wlrs; wrr=wrrs; bk=bks; blr=blrs; brr=brrs

#define saveLastMove() 		fromCol = nextMove->curr->curr.col;		\
							fromRow = nextMove->curr->curr.row;		\
							toCol = nextMove->curr->next->curr.col; \
							toRow = nextMove->curr->next->curr.row; \
							special = nextMove->curr->special;		\
							if (special != CASTLE){					\
								undoFrom = board[fromCol][fromRow]; \
								undoTo = board[toCol][toRow];		\
							} else {								\
								for (i = 0; i<BOARD_SIZE; i++) /*For Castling - keeping the entire row*/ \
									castling[i] = board[i][fromRow];\
							}

#define restoreLastMove()	if (special != CASTLE){					\
								board[fromCol][fromRow] = undoFrom;	\
								board[toCol][toRow] = undoTo;		\
							} else {								\
								for (i = 0; i<BOARD_SIZE; i++) /*For Castling - keeping the entire row*/ \
									board[i][fromRow] = castling[i];\
							}

movesList_t* getAllLegalMoves(char board[BOARD_SIZE][BOARD_SIZE], char color);
void deleteCastlingMoves(movesList_t *tmp_moves);
int keepAllLegalMoves(char board[BOARD_SIZE][BOARD_SIZE], movesList_t* answer, char color);
movesList_t* getMovesForPiece(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos);
movesList_t* getPawnMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color);
movesList_t* getPawnPromotions(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color);
movesList_t* getKingMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color);
movesList_t* getCastlingMoves(char board[BOARD_SIZE][BOARD_SIZE], char color);
movesList_t* getQueenMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color);
movesList_t* getBishopMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos,  char color);
movesList_t* getRookMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos,  char color);
movesList_t* getKnightMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos,  char color);

int canMove(char board[BOARD_SIZE][BOARD_SIZE], char player);
int canMovePiece(char board[BOARD_SIZE][BOARD_SIZE], pos_t pos);

/*utilities*/
void init_board(char board[BOARD_SIZE][BOARD_SIZE]);
void resetGlobals();
char* skipSpaces(char* s);
void copyBoard(char newBoard[BOARD_SIZE][BOARD_SIZE] ,char sourceBoard[BOARD_SIZE][BOARD_SIZE]);
int inKingsRow(char board[BOARD_SIZE][BOARD_SIZE], pos_t pos);

/*linked lists*/
void freeList(movesList_t* list);
void freeMove(move_t* move);
void freeListWithException(movesList_t* list, move_t* exception);
movesList_t* addPosToStart (movesList_t *moves, pos_t pos);
move_t* addPosToStartOfMove (move_t* move, pos_t pos);
move_t* addPosToMove(move_t *move, pos_t pos);
movesList_t* addMoveToMoves(movesList_t *movesList, move_t* move);
movesList_t* extendMovesList(movesList_t *answer, movesList_t *tmp_list);
move_t* extendMoves(move_t *move1, move_t *move2);
void deleteMoveFromList(move_t *move, movesList_t *list);
movesList_t* initEmptyList();
int isEmpty(movesList_t* list);

#define badAllocMoveList(M1,M2,L) 	 \
			freeMove(M1); \
			freeMove(M2); \
			freeList(L); \
			return NULL; \

#define badAlloc3MovesList(M1,M2,M3,L) \
		freeMove(M1); \
		freeMove(M2); \
		freeMove(M3); \
		freeList(L); \
		return NULL; \

#endif /* CHESSPROG_H_ */
