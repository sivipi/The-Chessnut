/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 chessprog.c                                  */
/* contents: main function, game logics and utilities     */
/**********************************************************/
#include "chessprog.h"
#include "console.h"
#include "gui.h"

/* GLOBALS */
char gameBoard[BOARD_SIZE][BOARD_SIZE]; /*[cols][rows]*/
unsigned startGame = 0;
unsigned minimaxDepth = 1;
char userColor = WHITE;
char gameMode = PVP;
char nextPlayer = WHITE;
char wk=0, wlr=0, wrr=0, bk=0, blr=0, brr=0; /* flags for castling state */

int main(int argc, char* argv[]) {
	int ret=1;
	setvbuf(stdout, NULL, _IONBF, 0); /*Eclipse console bug workaround*/
	srand(1); /* for pseudo-random move selection in minimax */

	assert(argc<=2);
	if (argc==1 || strcmp(argv[1], "console")==0) {
		ret=consoleMode();
	} else if (strcmp(argv[1], "gui")==0) {
		ret=guiMode();
	} else {
		printf("Bad command line argument, exiting.\n");
	}
	quit(ret);
	return ret;
}

/******************* game logics ************************/

unsigned start(char board[BOARD_SIZE][BOARD_SIZE]) {
	unsigned answer[12];
	countPieces(board, answer);
	if (answer[5]==1 && answer[11]==1 && !isCheck(board, invColor(nextPlayer))) {
		/*each player has 1 king, and nextPlayer can't kill opponent's king*/
		/*update castling flags and start*/
		wk = board[4][0]!=W_KING? 1:wk;
		wlr = board[0][0]!=W_ROOK? 1:wlr;
		wrr = board[7][0]!=W_ROOK? 1:wrr;
		bk = board[4][7]!=B_KING? 1:bk;
		blr = board[0][7]!=B_ROOK? 1:blr;
		brr = board[7][7]!=B_ROOK? 1:brr;
		startGame = 1;
	}
	else
		startGame = 0;
	return startGame;
}

/* play move and update board
 * @pre: move is legal for playMove
 * @post: if move ends in last row and piece is pawn - promote*/
void playMove(char board[BOARD_SIZE][BOARD_SIZE], move_t* move) {
	pos_t from=move->curr, to=move->next->curr;
	char piece = board[from.col][from.row];

	setPiece(board, from, EMPTY); /*take piece from*/
	setPiece(board, to, piece); /*set piece to*/
	updateCastlingFlags(piece, from);

	if (move->special != NORM) { /*handle pawn promotion and castling*/
		if (move->special == CASTLE) { /*rook already in proper place, need to move king*/
			from.col = 4; /*king's col, row already in place*/
			to.col += to.col>from.col? 1:-1; /*offset king's destination by that of rook's*/
			piece = board[from.col][from.row]; /*remember which king*/
			setPiece(board, from, EMPTY); /*take piece from*/
			setPiece(board, to, piece); /*set piece to*/
			updateCastlingFlags(piece, from);
		} else { /*pawn promotion, move was previously checked and is legal*/
			setPiece(board, to, move->special);
		}
	}
}

/* used after playMove to update global castling state flags */
void updateCastlingFlags(char piece, pos_t pos) {
	switch (piece) {
	case W_KING:
		wk = 1;
		break;
	case B_KING:
		bk = 1;
		break;
	case W_ROOK:
		if (pos.col==0)
			wlr = 1;
		else if (pos.col==7)
			wrr = 1;
		break;
	case B_ROOK:
		if (pos.col==0)
			blr = 1;
		else if (pos.col==7)
			brr = 1;
		break;
	}
}

/* evaluate board state and return a defined value for check/mate/tie/continute
 * return -1 as allocation error code
 */
int evalBoard(char board[BOARD_SIZE][BOARD_SIZE], char nextPlayer) {
	int hasMoves, check = isCheck(board, nextPlayer);
	if (check == -1) /*allocation error in isCheck*/
		return -1;
	hasMoves = canMove(board, nextPlayer);
	switch (hasMoves) {
	case -1: /*allocation error*/
		return -1;
	case 0: /*can't move*/
		return check? invColor(nextPlayer):TIE; /*color indicates mate (win)*/
	default: /*can move*/
		return check? CHECK:CONTINUE;
	}
}

/* return 1 if playerColor's king is threatened, else 0 */
int isCheck(char board[BOARD_SIZE][BOARD_SIZE], char playerColor) {
	int i, a, b, c, d, row, col;
	int flagColor;
	pos_t kingPos, pos;
	char color = playerColor;
	flagColor = (color==WHITE) ? 1 : -1;

	kingPos = getKingPos(board, color);

	for (int row=kingPos.row-1 ; row<=kingPos.row+1 ; row+=2) {
		for (int col=kingPos.col-1 ; col<=kingPos.col+1 ; col+=2) {
			pos.col=col; pos.row=row;

			/*advance upon diagonal to find rival piece*/
			/*Condition: [!stop] && [inBoard] && [empty || opponent] */
			for (; inBoard(pos.col, pos.row) && ( board[pos.col][pos.row]==EMPTY || \
					color!=getColor(board[pos.col][pos.row]) );) {
					switch(board[pos.col][pos.row]){
						case EMPTY:
							break;
						case W_BISHOP:
						case B_BISHOP:
						case W_QUEEN:
						case B_QUEEN:
							return 1;
						case W_PAWN:
						case B_PAWN:
							if(pos.row==kingPos.row+flagColor)
								return 1;
							break;
						case W_KING:
						case B_KING:
							if((pos.row==kingPos.row+1)||(pos.row==kingPos.row-1))
								return 1;
							break;
					}

				if (board[pos.col][pos.row]!=EMPTY)
					break;
				/*advance upon diagonal*/
				pos.row += pos.row>kingPos.row? 1:-1;
				pos.col += pos.col>kingPos.col? 1:-1;
			}
		}
	}

	/*Straight Lines - Queen, Rook, King(1 step)*/

	for (int row=kingPos.row-1 ; row<=kingPos.row+1 ; row+=1) {
		for (int col=kingPos.col-1 ; col<=kingPos.col+1 ; col+=1) {
			pos.col=col; pos.row=row;
			/*Only for same row/colums but not both simultaneously*/
			if (!(col==kingPos.col&&row==kingPos.row)&&((col==kingPos.col)||(row==kingPos.row))) {
				/*advance upon row/colums to find rival piece*/
				/*Condition: [!stop] && [inBoard] && [empty || opponent] */
				for (; inBoard(pos.col, pos.row) && ( board[pos.col][pos.row]==EMPTY || \
						color!=getColor(board[pos.col][pos.row]) );) {

					switch(board[pos.col][pos.row]){
						case EMPTY:
							break;
						case W_ROOK:
						case B_ROOK:
						case W_QUEEN:
						case B_QUEEN:
							return 1;
						case W_KING:
						case B_KING:
							if((pos.row==kingPos.row+1)||(pos.row==kingPos.row-1)|| \
									(pos.col==kingPos.col+1)||(pos.col==kingPos.col-1))
								return 1;
							break;
					}

					if (board[pos.col][pos.row]!=EMPTY)
						break;

					/*advance on row/column*/
					if (pos.row != kingPos.row)
						pos.row += (pos.row>kingPos.row)? 1:-1;  /* up:down */
					else /*if (pos.col != startPos.col)*/
						pos.col += (pos.col>kingPos.col)? 1:-1;  /* right:left */
				}
			}
		}
	}

	/*Knight - 8 Positions*/
	a=1;
	b=2;
	c=2;
	d=4;
	for(i=0;i<2;i++){ /*2 Iterations - switching rows with columns*/
		for (row=kingPos.row-b ; row<=kingPos.row+b ; row+=d) {  /*2 Iterations*/
			for (col=kingPos.col-a ; col<=kingPos.col+a ; col+=c) {  /*2 Iterations*/
				/*Condition: inBoard*/
				if(inBoard(col, row) && color!=getColor(board[col][row])){
					switch(board[col][row]){
						case W_KNIGHT:
						case B_KNIGHT:
							return 1;
					}
				}
			}
		}
	a=2;
	b=1;
	c=4;
	d=2;
	}

	return 0;
}

/*return true iff pos is contained in any of the moves in moveslist*/
int canReachPos(pos_t pos, movesList_t* moves) {
	for (movesList_t* nextMove=moves ; nextMove!=NULL ; nextMove=nextMove->next)
		for (move_t* move=nextMove->curr ; move!=NULL ; move=move->next)
			if (move->curr.row==pos.row && move->curr.col==pos.col)
				return 1;
	return 0;
}

/* return 1 if player can move, 0 if can't, -1 for error code */
int canMove(char board[BOARD_SIZE][BOARD_SIZE], char player) {
	int retVal;
	movesList_t* moves = getAllLegalMoves(board, player);
	if (moves == NULL)
		return -1; /*allocation error*/
	retVal = isEmpty(moves)? 0:1; /*list is empty iff can't move*/
	freeList(moves);
	return retVal;
}

/* @post: on allocation error return: NULL
 * @post: on empty list (0 moves) return: list with size 0
 */
movesList_t* getAllLegalMoves(char board[BOARD_SIZE][BOARD_SIZE], char color){
	char c;
	unsigned i, j;
	pos_t pos;
	movesList_t *movesList, *tmp_moves;

	movesList = initEmptyList();
	if (movesList == NULL)
		return NULL; /*allocation error code*/

	for (i = 0; i < BOARD_SIZE; i++){ /* rows - left to right */
		for (j = 0; j < BOARD_SIZE; j++){ /* columns - left to right */
			c = board[i][j]; /* w B */
			if (c != EMPTY && color == getColor(c)){ /* only if piece is in given color */
				pos.col = i;
				pos.row = j;
				tmp_moves = getMovesForPiece(board, pos);
				if (tmp_moves == NULL){
					freeList(movesList);
					freeList(tmp_moves);
					return NULL;
				}
				if (c == W_KING || c == B_KING) /*eliminates double castling move*/
					deleteCastlingMoves(tmp_moves);

				movesList = extendMovesList(movesList, tmp_moves);
			}
		}
	}

	return movesList;
}

/* delete castling moves from list if there are any */
void deleteCastlingMoves(movesList_t *tmp_moves){
	movesList_t *head;
	movesList_t* keepNext;

	if (isEmpty(tmp_moves))
		return;

	head = tmp_moves;
	while (head != NULL){ /* Deleting Moves with special == CASTLE */
		keepNext = head->next;
		if (head->curr->special == CASTLE)
		{
			if (head==tmp_moves && keepNext!=NULL) /*about to delete first move*/
				keepNext = head;  /*head of list will contain second element, so that's our next move*/
			deleteMoveFromList(head->curr,tmp_moves);
		}
		head = keepNext;
	}

}

/* @pre: There is a piece in the given position
 * @post: if recieves a move of length 1, treats same as empty movesList
 */
movesList_t* getMovesForPiece(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos){

	movesList_t *allMoves;
	char piece, color;
	piece = board[startPos.col][startPos.row];
	color = getColor(piece);

	switch (piece) {
	case W_PAWN:
	case B_PAWN:
		allMoves = getPawnMove(board, startPos, color);
		break;
	case W_KNIGHT:
	case B_KNIGHT:
		allMoves = getKnightMove(board, startPos, color);
		break;
	case W_BISHOP:
	case B_BISHOP:
		allMoves = getBishopMove(board, startPos, color);
		break;
	case W_ROOK:
	case B_ROOK:
		allMoves = getRookMove(board, startPos, color);
		break;
	case W_QUEEN:
	case B_QUEEN:
		allMoves = getQueenMove(board, startPos, color);
		break;
	case W_KING:
	case B_KING:
		allMoves = getKingMove(board, startPos, color);
		break;
	}

	if (allMoves == NULL)
		return NULL; /*allocation error*/

	keepAllLegalMoves(board, allMoves, color);

	if (isEmpty(allMoves) || allMoves->curr->size<2) { /*handles illegal move return*/
		freeList(allMoves);
		allMoves = initEmptyList();
	}

	return allMoves;
}

movesList_t* getPawnMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color){

	int col, row;
	move_t *captureR = NULL, *captureL = NULL, *movement = NULL;
	movesList_t* answer, *promotions;
	pos_t pos;

	answer = initEmptyList();
	captureR = addPosToMove(captureR, startPos);
	captureL = addPosToMove(captureL, startPos);
	movement = addPosToMove(movement, startPos);
	if (captureR == NULL || captureL == NULL || movement == NULL || answer == NULL) {
		freeMove(captureR);
		freeMove(captureL);
		freeMove(movement);
		freeList(answer);
		return NULL; /*allocation error code*/
	}

	if (color == WHITE){
		if (startPos.row<BOARD_SIZE - 2){ /* Not at the top 2 rows of the board */
			row = startPos.row + 1; /* up */
			col = startPos.col;

			/*straight movement*/
			if (board[col][row] == EMPTY) {
				pos.col = col;
				pos.row = row;
				if (addPosToMove(movement, pos) == NULL){
					badAllocMoveList(captureL, captureR, answer);
				}
				answer = addMoveToMoves(answer, movement);
			}

			/*eliminations*/
			col = startPos.col + 1; /* right */
			if (inBoard(col, row) && board[col][row] != EMPTY && getColor(board[col][row]) != color) { /* not at right end of board && occupied */
				pos.col = col;
				pos.row = row;
				if (addPosToMove(captureR, pos) == NULL){
					badAllocMoveList(captureL, movement, answer);
				}
				answer = addMoveToMoves(answer, captureR);
			}
			col = startPos.col - 1; /* left */
			if (inBoard(col, row) && board[col][row] != EMPTY && getColor(board[col][row]) != color) { /* not at left end of board && occupied */
				pos.col = col;
				pos.row = row;
				if (addPosToMove(captureL, pos) == NULL){
					badAllocMoveList(captureR, movement, answer);
				}
				answer = addMoveToMoves(answer, captureL);
			}

		}
		else if (startPos.row == BOARD_SIZE - 2){ /* In second last row - before promotion */
			if ((promotions = getPawnPromotions(board, startPos, color)) == NULL) {
				freeMove(captureR);
				freeMove(captureL);
				freeMove(movement);
				return NULL;
			}
			answer = extendMovesList(answer, promotions);
		}
	}
	else { /* color == BLACK */
		if (startPos.row>1){ /* Not at the bottom 2 rows of the board */
			row = startPos.row - 1; /* down */
			col = startPos.col;

			/*straight movement*/
			if (board[col][row] == EMPTY) {
				pos.col = col;
				pos.row = row;
				if (addPosToMove(movement, pos) == NULL){
					badAllocMoveList(captureL, captureR, answer);
				}
				answer = addMoveToMoves(answer, movement);
			}

			/*eliminations*/
			col = startPos.col + 1; /* right */
			if (inBoard(col, row) && board[col][row] != EMPTY && getColor(board[col][row]) != color) { /* not at right end of board && occupied */
				pos.col = col;
				pos.row = row;
				if (addPosToMove(captureR, pos) == NULL){
					badAllocMoveList(captureL, movement, answer);
				}
				answer = addMoveToMoves(answer, captureR);
			}
			col = startPos.col - 1; /* left */
			if (inBoard(col, row) && board[col][row] != EMPTY && getColor(board[col][row]) != color) { /* not at left end of board && occupied */
				pos.row = row;
				pos.col = col;
				if (addPosToMove(captureL, pos) == NULL){
					badAllocMoveList(captureR, movement, answer);
				}
				answer = addMoveToMoves(answer, captureL);
			}

		}
		else if (startPos.row == 1) { /* In second last row - before promotion */
			if ((promotions = getPawnPromotions(board, startPos, color)) == NULL) {
				freeMove(captureR);
				freeMove(captureL);
				freeMove(movement);
				return NULL;
			}
			answer = extendMovesList(answer, promotions);
		}
	}

	if (captureL->size == 1)
		freeMove(captureL);
	if (captureR->size == 1)
		freeMove(captureR);
	if (movement->size == 1)
		freeMove(movement);

	return answer;
}

/* Deals with promotion+elimination or just promotion.
* @pre: (startPos.row==1 && color==BLACK) || (startPos.row==BOARD_SIZE-1 && color==WHITE)
* @post: Will return a movesList_t* of size 4-12
*/
movesList_t* getPawnPromotions(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color){

	int i;
	move_t *promQ, *promR, *promB, *promN;
	movesList_t* answer = initEmptyList();
	pos_t pos;
	int colFlag =  (color == WHITE) ? 1 : -1;
	char p;

	if (answer==NULL)
		return NULL; /*allocation error code*/

	for (i = -1; i<2; i++){
		/* ((board[pos.col][pos.row]) != EMPTY) || (i == 0) */
		pos.row = startPos.row + colFlag; /* WHITE - up, BLACK - down */
		pos.col = startPos.col + i;
		p = board[pos.col][pos.row];
		if ((i == 0 && p == EMPTY) || (i != 0 && inBoard(pos.col, pos.row) && p != EMPTY && getColor(p) != color)) {
			/*Creating 4 new moves*/
			promQ = addPosToMove(NULL, startPos);
			promR = addPosToMove(NULL, startPos);
			promB = addPosToMove(NULL, startPos);
			promN = addPosToMove(NULL, startPos);
			if (promQ == NULL || promR == NULL || promB == NULL || promN == NULL) {
				freeMove(promQ);
				freeMove(promR);
				freeMove(promB);
				freeMove(promN);
				freeList(answer);
				return NULL; /*allocation error code*/
			}
			/*Building 4 promotion moves*/
			addPosToMove(promQ, pos);
			addPosToMove(promR, pos);
			addPosToMove(promB, pos);
			addPosToMove(promN, pos);
			/*Updating special fields in promotion moves - for both nodes*/
			promQ->special = promQ->next->special = color == WHITE ? W_QUEEN : B_QUEEN;
			promR->special = promR->next->special = color == WHITE ? W_ROOK : B_ROOK;
			promB->special = promB->next->special = color == WHITE ? W_BISHOP : B_BISHOP;
			promN->special = promN->next->special = color == WHITE ? W_KNIGHT : B_KNIGHT;
			/*Adding promotion moves to answer*/
			answer = addMoveToMoves(answer, promQ);
			answer = addMoveToMoves(answer, promR);
			answer = addMoveToMoves(answer, promB);
			answer = addMoveToMoves(answer, promN);
		}
	}

	return answer;
}

movesList_t* getBishopMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color) {
	move_t* move = NULL;
	movesList_t* answer = NULL, *tmpList = NULL;
	pos_t pos;

	answer = initEmptyList();
	if (answer == NULL)
		return NULL;

	for (int row = startPos.row - 1; row <= startPos.row + 1; row += 2) {
		for (int col = startPos.col - 1; col <= startPos.col + 1; col += 2) {
			pos.col = col; pos.row = row;

			/*advance upon diagonal to find all empty spots*/
			/*Condition: [inBoard] && [empty || opponent] */
			for (move = NULL, tmpList = NULL; LEGAL; move = NULL, tmpList = NULL) {

				if ((move = addPosToMove(move, pos)) == NULL) {
					freeList(answer);
					return NULL; /*allocation error*/
				}
				if ((tmpList = addMoveToMoves(answer, move)) == NULL) {
					freeList(answer);
					freeMove(move);
					return NULL; /*allocation error*/
				}

				answer = tmpList;

				if (board[pos.col][pos.row] != EMPTY)
					break;

				/*advance upon diagonal*/
				pos.row += pos.row>startPos.row ? 1 : -1;
				pos.col += pos.col>startPos.col ? 1 : -1;

			}
		}
	}

	/*Push startPos to the beggining of all the moves in answer*/
	if (!isEmpty(answer)) {
		if ((tmpList = addPosToStart(answer, startPos)) == NULL) {
			freeList(answer);
			return NULL; /*allocation error*/
		}
	}

	return answer;
}

movesList_t* getRookMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color) {
	move_t* move = NULL;
	movesList_t* answer = NULL, *tmpList = NULL, *castling;
	pos_t pos;

	answer = initEmptyList();
	if (answer == NULL)
		return NULL;

	for (int row = startPos.row - 1; row <= startPos.row + 1; row += 1) {
		for (int col = startPos.col - 1; col <= startPos.col + 1; col += 1) {
			pos.col = col; pos.row = row;
			/*Only for same row/colums but not both simultaneously*/
			if (!(col == startPos.col&&row == startPos.row) && ((col == startPos.col) || (row == startPos.row))) {
				/*advance upon row/colums to find all empty spots*/
				/*Condition: [inBoard] && [empty || opponent)] */
				for (move = NULL, tmpList = NULL; LEGAL; move = NULL, tmpList = NULL) {

					if ((move = addPosToMove(move, pos)) == NULL) {
						freeList(answer);
						return NULL; /*allocation error*/
					}
					if ((tmpList = addMoveToMoves(answer, move)) == NULL) {
						freeList(answer);
						freeMove(move);
						return NULL; /*allocation error*/
					}

					answer = tmpList;

					if (board[pos.col][pos.row] != EMPTY)
						break;

					/*advance on row/column*/
					if (pos.row != startPos.row)
						pos.row += (pos.row>startPos.row) ? 1 : -1;  /* up:down */
					else /*if (pos.col != startPos.col)*/
						pos.col += (pos.col>startPos.col) ? 1 : -1;  /* right:left */
				}
			}
		}
	}

	/*Push startPos to the beggining of all the moves in answer*/
	if (!isEmpty(answer)) {
		if ((tmpList = addPosToStart(answer, startPos)) == NULL) {
			freeList(answer);
			return NULL; /*allocation error*/
		}
	}

	/*Adding Castling moves*/
	if ((castling = getCastlingMoves(board, color)) == NULL){
		freeList(answer);
		freeMove(move);
		return NULL;
	}

	if (DEBUG){
		printf("\n");
		printMovesList(castling);
		printf("\n");
	}
	/*keep only castling move for THIS rook*/
	if (castling->size==0) {} /*do nothing*/
	else if (castling->size==1) { /*Make sure it's the right one*/
		if(castling->curr->curr.col!=startPos.col){ /*if not - initEmptyList*/
			freeList(castling);
			castling = initEmptyList();
		}
	} else { /*delete the surplus move*/
		if (castling->curr->curr.col!=startPos.col) /*first move is surplus*/
			deleteMoveFromList(castling->curr,castling);
		else /*second move is surplus*/
			deleteMoveFromList(castling->next->curr,castling);
	}

	if ((tmpList = extendMovesList(answer, castling)) == NULL) {
		freeList(answer);
		freeMove(move);
		freeList(castling);
		return NULL; /*allocation error*/
	}

	return answer;
}

/* queen_moves = rook_moves + bishop_moves - castling_moves */
movesList_t* getQueenMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color) {
	movesList_t* answer = NULL, *rook = NULL, *bishop = NULL;

	if ((bishop = getBishopMove(board, startPos, color)) == NULL) {
		return NULL; /*allocation error*/
	}

	if ((rook = getRookMove(board, startPos, color)) == NULL) {
		freeList(bishop);
		return NULL; /*allocation error*/
	}

	if ((answer = extendMovesList(rook, bishop)) == NULL) {
		freeList(bishop);
		freeList(rook);
		return NULL; /*allocation error*/
	}

	deleteCastlingMoves(answer);

	return answer;
}

movesList_t* getKingMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color) {
	move_t* move = NULL;
	movesList_t* answer = NULL, *tmpList = NULL, *castling;
	pos_t pos;

	answer = initEmptyList();
	if (answer == NULL)
		return NULL; /*allocation error*/

	for (int row = startPos.row - 1; row <= startPos.row + 1; row += 1) {
		for (int col = startPos.col - 1; col <= startPos.col + 1; col += 1) {
			pos.col = col; pos.row = row;
			move = NULL; tmpList = NULL;
			/*Condition: inBoard && pos != startPos && [empty || opponent]*/
			if (LEGAL && !(col == startPos.col&&row == startPos.row)){
				if ((move = addPosToMove(move, pos)) == NULL) {
					freeList(answer);
					return NULL; /*allocation error*/
				}
				if ((tmpList = addMoveToMoves(answer, move)) == NULL) {
					freeList(answer);
					freeMove(move);
					return NULL; /*allocation error*/
				}
				answer = tmpList;
			}
		}
	}

	/*Push startPos to the beggining of all the moves in answer*/
	if (!isEmpty(answer)) {
		if ((tmpList = addPosToStart(answer, startPos)) == NULL) {
			freeList(answer);
			return NULL; /*allocation error*/
		}
	}

	/*Adding Castling moves*/
	if ((castling = getCastlingMoves(board, color)) == NULL){
		freeList(answer);
		freeMove(move);
		return NULL; /*allocation error*/
	}
	if ((tmpList = extendMovesList(castling, answer)) == NULL) {
		freeList(answer);
		freeMove(move);
		freeList(castling);
		return NULL; /*allocation error*/
	}

	answer = tmpList;

	return answer;
}

/*Will return 0, 1 or 2 possible castling moves in respect to color*/
movesList_t* getCastlingMoves(char board[BOARD_SIZE][BOARD_SIZE], char color){
	move_t* move = NULL;
	movesList_t* answer = NULL, *tmpList = NULL;
	pos_t pos;
	char lastMove[BOARD_SIZE], k, r, l, kingPiece;
	int i, row;

	answer = initEmptyList();
	/*Keeping last move */
	row = color == WHITE ? 0 : BOARD_SIZE - 1;
	for (i = 0; i<BOARD_SIZE; i++)
		lastMove[i] = board[i][row];

	/*flags*/
	k = color == WHITE ? wk : bk; /*king*/
	r = color == WHITE ? wrr : brr; /*right rook*/
	l = color == WHITE ? wlr : blr; /*left rook*/

	kingPiece = color == WHITE ? W_KING : B_KING;

	if (!k){
		/*Right*/
		if (!r && board[5][row] == EMPTY && board[6][row] == EMPTY && !isCheck(board, color)){ /*Empty & not Check*/
			board[4][row] = EMPTY;
			board[5][row] = kingPiece;
			if (!isCheck(board, color)){
				board[5][row] = EMPTY;
				board[6][row] = kingPiece;
				if (!isCheck(board, color)){ /*a legal casteling move will be added*/
					pos.col = 7;
					pos.row = row;
					if ((move = addPosToMove(move, pos)) == NULL) {
						freeList(answer);
						return NULL; /*allocation error*/
					}
					pos.col = 5;
					pos.row = row;
					if ((addPosToMove(move, pos)) == NULL) {
						freeList(answer);
						freeMove(move);
						return NULL; /*allocation error*/
					}
					move->special = move->next->special = CASTLE;
					if ((tmpList = addMoveToMoves(answer, move)) == NULL) {
						freeList(answer);
						freeMove(move);
						return NULL; /*allocation error*/
					}

					answer = tmpList;
				}
			}
		}

		/*Restoring the board for next move*/
		for (i = 0; i<BOARD_SIZE; i++)
			board[i][row] = lastMove[i];
		move = NULL;

		/*Left*/
		if (!l && board[2][row] == EMPTY && board[3][row] == EMPTY && !isCheck(board, color)){ /*Empty & not Check*/
			board[4][row] = EMPTY;
			board[3][row] = kingPiece;
			if (!isCheck(board, color)){
				board[3][row] = EMPTY;
				board[2][row] = kingPiece;
				if (!isCheck(board, color)){
					pos.col = 0;
					pos.row = row;
					if ((move = addPosToMove(move, pos)) == NULL) { /*a legal casteling move will be added*/
						freeList(answer);
						return NULL; /*allocation error*/
					}
					pos.col = 3;
					pos.row = row;
					if ((addPosToMove(move, pos)) == NULL) {
						freeList(answer);
						freeMove(move);
						return NULL; /*allocation error*/
					}
					move->special = move->next->special = CASTLE;
					if ((tmpList = addMoveToMoves(answer, move)) == NULL) {
						freeList(answer);
						freeMove(move);
						return NULL; /*allocation error*/
					}

					answer = tmpList;
				}
			}
		}
	}

	/*Restoring the board for next move*/
	for (i = 0; i<BOARD_SIZE; i++) /*For Castling - restoring the entire row*/
		board[i][row] = lastMove[i];

	return answer;
}

movesList_t* getKnightMove(char board[BOARD_SIZE][BOARD_SIZE], pos_t startPos, char color) {

	move_t* move = NULL;
	movesList_t* answer = NULL, *tmpList = NULL;
	pos_t pos;
	int i, a, b, c, d;
	int row, col;

	answer = initEmptyList();
	if (answer == NULL)
		return NULL;

	a = 1;
	b = 2;
	c = 2;
	d = 4;
	for (i = 0; i<2; i++){
		for (row = startPos.row - b; row <= startPos.row + b; row += d) {
			for (col = startPos.col - a; col <= startPos.col + a; col += c) {
				pos.col = col; pos.row = row;
				/*Condition: inBoard && (empty || opponent)*/
				if (LEGAL){
					if ((move = addPosToMove(move, pos)) == NULL) {
						freeList(answer);
						return NULL; /*allocation error*/
					}
					if ((tmpList = addMoveToMoves(answer, move)) == NULL) {
						freeList(answer);
						freeMove(move);
						return NULL; /*allocation error*/
					}
					answer = tmpList;
					move = NULL;
				}
			}
		}
		a = 2;
		b = 1;
		c = 4;
		d = 2;
	}

	/*Push startPos to the beggining of all the moves in answer*/
	if (!isEmpty(answer)) {
		if ((tmpList = addPosToStart(answer, startPos)) == NULL) {
			freeList(answer);
			return NULL; /*allocation error*/
		}
	}

	return answer;
}

/*Deletes all illegal moves from the list and returns answer->size*/
int keepAllLegalMoves(char board[BOARD_SIZE][BOARD_SIZE], movesList_t* answer, char color){

	unsigned i, fromCol, fromRow, toCol, toRow, special;
	movesList_t* nextMove, *keepNext;
	char castling[BOARD_SIZE], undoFrom, undoTo;

	/*safety check, nothing to change*/
	if (answer == NULL || answer->size == 0)
		return 0;

	for (nextMove = answer; nextMove != NULL && nextMove->curr != NULL ; nextMove = keepNext){ /*for each move*/

		keepNext = nextMove->next;

		/*Keeping last move */
		saveLastMove();
		saveCastlingFlags();

		playMove(board, nextMove->curr);
		if (isCheck(board, color)) {
			if (nextMove==answer && keepNext!=NULL) /*about to delete first move*/
				keepNext = nextMove;
			deleteMoveFromList(nextMove->curr, answer);
		}

		/*Restoring the board for next move*/
		restoreCastlingFlags();
		restoreLastMove();
		/*End of move loop - Now moves to nextMove->next*/
	}

	return answer->size;

}

/******************* utilities ************************/

/*called to terminate program*/
void quit(int ret){
	startGame = 0;
	exit(ret);
}

void rm(char board[BOARD_SIZE][BOARD_SIZE], pos_t pos){

	board[pos.col][pos.row] = EMPTY;

}

void clearBoard(char board[BOARD_SIZE][BOARD_SIZE]){
	unsigned i,j;
	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			board[i][j] = EMPTY;
		}
	}
}

void setPiece(char board[BOARD_SIZE][BOARD_SIZE], pos_t pos, char piece) {

	board[pos.col][pos.row] = piece;

}

/*returns pointer to first non-space char from s*/
char* skipSpaces(char* s) {
	for (; isspace(*s) && *s!='\0' ; s++);
	return s;
}

unsigned isValidMove(move_t* move) {
	while (move != NULL) {
		if (!isValidPos(move->curr))
			return 0;
		move = move->next;
	}
	return 1;
}

/*deals with case 1 in command move*/
unsigned isValidPos(pos_t pos) {
	if (pos.col<0 || pos.row<0 || pos.col>=BOARD_SIZE || pos.row>=BOARD_SIZE) /*outside board*/
		return 0;
	return 1;
}

/* deals with case 2 in command move
 * @pre: pos is valid on board
 */
unsigned isUserPos(char board[BOARD_SIZE][BOARD_SIZE], pos_t pos, char color) {
	if (board[pos.col][pos.row]==EMPTY)
		return 0;
	return color == getColor(board[pos.col][pos.row]);
}

/*deals with case 3 in command move*/
unsigned isLegalMove(char board[BOARD_SIZE][BOARD_SIZE], move_t* userMove, char color) {
	movesList_t* moves;
	unsigned result=0;
	moves = getAllLegalMoves(board, color);
	for (movesList_t* move = moves ; move != NULL ; move = move->next) {
		if (isSameMove(userMove, move->curr)) {
			result = 1;
			break;
		}
	}
	freeList(moves);
	return result;
}

unsigned isSameMove(move_t* m1, move_t* m2) {
	for (; m1!=NULL && m2!=NULL ; m1=m1->next, m2=m2->next) {
		if (m1->curr.row != m2->curr.row || m1->curr.col != m2->curr.col ||
				m1->size != m2->size || m1->special != m2->special)
			return 0;
	}
	if (m1!=NULL || m2!=NULL) /*sanity check*/
		return 0;
	return 1;
}

/*return pos for king matching player color*/
pos_t getKingPos(char board[BOARD_SIZE][BOARD_SIZE], char player) {
	pos_t p;
	char myKing;
	myKing = player==WHITE ? W_KING : B_KING;
	for (p.col=0 ; p.col<BOARD_SIZE ; p.col++)
		for (p.row=0 ; p.row<BOARD_SIZE ; p.row++)
			if (board[p.col][p.row]==myKing)
				return p;
	return p; /*just to calm down the compiler*/
}

/*Will fill unsigned answer[12] - [m,n,b,r,q,k,M,N,B,R,Q,K]*/
void countPieces(char board[BOARD_SIZE][BOARD_SIZE], unsigned answer[12]){
	unsigned i, j, m = 0, n = 0, b = 0, r = 0, q = 0, k = 0, M = 0, N = 0, B = 0, R = 0, Q = 0, K = 0;
	char c = ' ';

	for (i = 0; i < BOARD_SIZE; i++){
		for (j = 0; j < BOARD_SIZE; j++){
			c = board[i][j];
			switch (c) {
			case W_PAWN:
				m++;
				break;
			case W_KNIGHT:
				n++;
				break;
			case W_BISHOP:
				b++;
				break;
			case W_ROOK:
				r++;
				break;
			case W_QUEEN:
				q++;
				break;
			case W_KING:
				k++;
				break;
			case B_PAWN:
				M++;
				break;
			case B_KNIGHT:
				N++;
				break;
			case B_BISHOP:
				B++;
				break;
			case B_ROOK:
				R++;
				break;
			case B_QUEEN:
				Q++;
				break;
			case B_KING:
				K++;
				break;

			}
		}
	}
	answer[0] = m;
	answer[1] = n;
	answer[2] = b;
	answer[3] = r;
	answer[4] = q;
	answer[5] = k;
	answer[6] = M;
	answer[7] = N;
	answer[8] = B;
	answer[9] = R;
	answer[10] = Q;
	answer[11] = K;
}

void copyBoard(char newBoard[BOARD_SIZE][BOARD_SIZE], char sourceBoard[BOARD_SIZE][BOARD_SIZE]){
	for (unsigned i = 0; i<BOARD_SIZE; i++)
	for (unsigned j = 0; j<BOARD_SIZE; j++)
		newBoard[i][j] = sourceBoard[i][j];
}

void init_board(char board[BOARD_SIZE][BOARD_SIZE]){
	int i, j;
	for (j = 1; j < BOARD_SIZE - 1; j++){ /*rows*/
		for (i = 0; i < BOARD_SIZE; i++){ /*columns*/
			if (j == 1){
				board[i][j] = W_PAWN;
			}
			else if (j == BOARD_SIZE - 2){
				board[i][j] = B_PAWN;
			}
			else{
				board[i][j] = EMPTY;
			}
		}
	}
	/*White*/
	board[0][0] = W_ROOK;
	board[1][0] = W_KNIGHT;
	board[2][0] = W_BISHOP;
	board[3][0] = W_QUEEN;
	board[4][0] = W_KING;
	board[5][0] = W_BISHOP;
	board[6][0] = W_KNIGHT;
	board[7][0] = W_ROOK;

	/*Black*/
	board[0][BOARD_SIZE - 1] = B_ROOK;
	board[1][BOARD_SIZE - 1] = B_KNIGHT;
	board[2][BOARD_SIZE - 1] = B_BISHOP;
	board[3][BOARD_SIZE - 1] = B_QUEEN;
	board[4][BOARD_SIZE - 1] = B_KING;
	board[5][BOARD_SIZE - 1] = B_BISHOP;
	board[6][BOARD_SIZE - 1] = B_KNIGHT;
	board[7][BOARD_SIZE - 1] = B_ROOK;

}

void resetGlobals() {
	startGame = 0;
	minimaxDepth = 1;
	userColor = WHITE;
	gameMode = PVP;
	nextPlayer = WHITE;
	wk=0; wlr=0; wrr=0; bk=0; blr=0; brr=0;
}

/******************* list utilities ************************/

/*@pre: receive only dynamically allocated pointers*/
void freeMove(move_t* move) {
	if (move == NULL)
		return;
	freeMove(move->next);
	free(move);
}

void freeList(movesList_t* list){

	if (list == NULL)
		return;
	else
		freeList(list->next);
	freeMove(list->curr);
	free(list);
}

/*free all moves in list (and list itself) except for exception move*/
void freeListWithException(movesList_t* list, move_t* exception) {
	if (list == NULL)
		return;
	freeListWithException(list->next, exception);
	if (list->curr != exception) /*compare by address*/
		freeMove(list->curr);
	free(list);
}

movesList_t* initEmptyList(){

	movesList_t* list;
	list = (movesList_t *)malloc(sizeof(movesList_t));
	if (list == NULL){
		return NULL; /*allocation error code*/
	}
	list->size = 0;
	list->curr = NULL;
	list->next = NULL;

	return list;
}

int isEmpty(movesList_t* list) {
	if (list == NULL || list->size == 0)
		return 1;
	return 0;
}

/*if movesList==NULL: create new list and return pointer*/
/*else: add move to the start of current list*/
/*@pre: move is not NULL*/
movesList_t* addMoveToMoves(movesList_t *movesList, move_t* move) /* Adds move to the start of the movesList */
{
	movesList_t *temp;

	if (movesList != NULL && isEmpty(movesList)) { /*when adding to empty list*/
		movesList->curr = move;
		movesList->next = NULL;
		movesList->size = 1;
		return movesList;
	}

	temp = (movesList_t *)malloc(sizeof(movesList_t));
	if (temp == NULL){
		return NULL; /*allocation error code*/
	}

	if (movesList == NULL) { /*creating new movesList*/
		temp->curr = move;
		temp->next = NULL;
		temp->size = 1;
		return temp;
	}

	/*copy first element to be second in list*/
	temp->curr = movesList->curr;
	temp->next = movesList->next;
	temp->size = movesList->size;
	/*old first element is now the new move*/
	movesList->curr = move;
	movesList->next = temp;
	movesList->size++;

	return movesList;
}

movesList_t* extendMovesList(movesList_t *answer, movesList_t *tmp_list){

	movesList_t *head = answer;

	if (tmp_list == NULL || isEmpty(tmp_list) || answer == NULL) {
		freeList(tmp_list);
		return answer;
	}

	if (isEmpty(answer)) {
		answer->curr = tmp_list->curr;
		answer->next = tmp_list->next;
		answer->size = tmp_list->size;
		free(tmp_list);
		return answer;
	}

	while (head->next != NULL) {
		head->size += tmp_list->size;
		head = head->next;
	}
	head->size += tmp_list->size;
	head->next = tmp_list;

	return answer;
}

/*if move==NULL: create new move and return pointer*/
/*else: add pos to the end of current move*/
move_t* addPosToMove(move_t *move, pos_t pos)
{
	move_t *temp = NULL, *head = NULL;
	temp = (move_t *)malloc(sizeof(move_t));
	if (temp == NULL){
		return NULL; /*allocation error code*/
	}
	temp->curr = pos;
	temp->next = NULL;
	temp->size = 1;
	temp->special = NORM;
	temp->score = 0;

	if (move == NULL) /*creating new move*/
		return temp;

	head = move;
	while (head->next != NULL) {
		head->size++;
		head = head->next;
	}
	head->size++;
	head->next = temp;

	return move;
}

void deleteMoveFromList(move_t *move, movesList_t *list){

	movesList_t *head, *current, *previous;
	move_t *toDelete;
	head = list;

	if (list == NULL)
		return;

	if (move == list->curr){ /* If deleted move is first in the list */
		toDelete = list->curr;
		if ((head = list->next) != NULL) {
			list->curr = head->curr;
			list->next = head->next;
		}
		else { /*if it is the ONLY move in the list*/
			list->curr = NULL;
		}
		list->size--;
		free(head);
		freeMove(toDelete);
		return;
	}

	current = list->next;
	previous = list;

	while (current != NULL && current->curr != move) {
		previous->size--;
		previous = current;
		current = current->next;
	}
	previous->size--;
	if (current == NULL) { /*move not found in list*/
		for (; list != NULL; list = list->next) /*restore size*/
			list->size++;
		return;
	}
	previous->next = current->next; /*bypass list node to be deleted*/
	freeMove(move); /* equivalent to freeMove(current->curr)*/
	free(current);

}

/*adds pos to start of each move in movesList*/
movesList_t* addPosToStart(movesList_t *moves, pos_t pos){

	movesList_t* head;
	head = moves;

	if (isEmpty(head)) {
		head->curr = addPosToStartOfMove(head->curr, pos);
		head->size = 1;
		return head;
	}

	while (head != NULL){
		head->curr = addPosToStartOfMove(head->curr, pos);
		head = head->next;
	}

	return moves;
}

/*if move==NULL: create new move and return pointer
else: add pos to the end of current move*/
move_t* addPosToStartOfMove(move_t* move, pos_t pos){

	move_t *temp;
	temp = (move_t *)malloc(sizeof(move_t));
	if (temp == NULL){
		return NULL; /*allocation error code*/
	}

	if (move == NULL) { /*creating new move*/
		temp->curr = pos;
		temp->next = NULL;
		temp->size = 1;
		temp->special = NORM;
		temp->score = 0;
		return temp;
	}

	/*copy first element to be second in moves*/
	temp->curr = move->curr;
	temp->next = move->next;
	temp->size = move->size;
	temp->special = move->special;
	temp->score = move->score;
	/*old first element is now the new pos*/
	move->curr = pos;
	move->next = temp;
	move->size++;

	return move;
}
