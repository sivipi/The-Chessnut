/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 minimax.c                                    */
/* contents: minimax and scoring function				  */
/**********************************************************/
#include "minimax.h"
#include "console.h"

/* same as regular minimax, but returns list of best scoring moves, not just one
 * @pre: depth>0
 * @post: returns NULL for allocation error or illegal depth*/
movesList_t* miniMax_lst(char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, char playerA, char currentPlayer, char returnList) {
	/* playerA = computerColor (who we run the algorithm for) -> white/black (maximizing player)
	 * currentPlayer = A/B (are we in min or max level? A=max, B=min)
	 */
	movesList_t* moves = NULL;
	movesList_t* nextMove=NULL;
	int bestScore = currentPlayer==PLAYER_A? MIN_INF:MAX_INF; /*init to worst possible score for current player*/
	int tmp, maxBoards=MM_LIMIT;
	int alpha=MIN_INF, beta=MAX_INF;
	int fromRow, fromCol, toRow, toCol;
	char special, undoTo, undoFrom;
	char castling[8];
	int i;

	moves = getAllLegalMoves(board, currentPlayer==PLAYER_A? playerA:invColor(playerA));
	if (moves==NULL)
		return NULL;
	for (nextMove=moves ; !isEmpty(nextMove) ; nextMove=nextMove->next)
		nextMove->curr->score = bestScore; /*init all scores to worse possible*/
	nextMove=NULL;

	if (depth==BEST) /*makes sure we only use this limit for best option*/
		maxBoards /= isEmpty(moves)? 1:moves->size;
	if (depth==0) {
		return NULL; /*error code*/
	} else if (isEmpty(moves)) {
		bestScore = scoringFunction(board, playerA, currentPlayer, depth, 0);
		if (bestScore == MM_ERROR) { /*allocation error code*/
			freeList(moves);
			return NULL;
		}
	} else {
		for (nextMove=moves ; nextMove != NULL ; nextMove=nextMove->next) {
			saveCastlingFlags();
			saveLastMove();
			playMove(board, nextMove->curr);
			tmp = miniMax_rec(board, depth==BEST? depth:depth-1, \
					playerA, currentPlayer==PLAYER_A? PLAYER_B:PLAYER_A, alpha, beta, maxBoards, 1);
			restoreCastlingFlags();
			restoreLastMove();
			if (tmp == MM_ERROR) {
				freeList(moves); /*free all resources before returning error code*/
				return NULL;
			}
			nextMove->curr->score = tmp; /*update score for current move*/

			if (DEBUG_MM) {
				printf("depth 0: playing ");
				printMove(nextMove->curr);
			}
			
			if (currentPlayer==PLAYER_A && tmp>bestScore) { /*maximize score*/
				bestScore = tmp;
			} else if (currentPlayer==PLAYER_B && tmp<bestScore) { /*minimize score*/
				bestScore = tmp;
			}
			/*we can't use pruning in first level if we want all moves to get real scores*/
		}
	}

	if (DEBUG_MM_SCORE) {
		printf("bestScore is: %d\n", bestScore);
		printf("moves before keepBestMoves:\n");
		printMovesList(moves);
	}
	if (returnList == LIST_BEST) {
		moves = keepBestMoves(moves, bestScore); /*keep moves with best score*/
		if (DEBUG_MM_SCORE) {
			printf("moves after keepBestMoves:\n");
			printMovesList(moves);
		}
	} else if (returnList==LIST_ALL && DEBUG_MM_SCORE)
		printf("keeping all moves!\n");

	return moves;
}

movesList_t* keepBestMoves(movesList_t* answer, int bestScore){

	movesList_t *bestMovesList = answer, *keepNext;

	/*safety check, nothing to change*/
	if (answer == NULL)
		return NULL;
	if (answer->size == 0 || answer->size == 1)
		return answer;

	bestMovesList = answer;
	while (bestMovesList != NULL){ /* Deleting Moves whith sub optimal score */
		keepNext = bestMovesList->next;
		if (bestMovesList->curr == NULL || bestMovesList->curr->score != bestScore)
		{
			if (bestMovesList==answer && keepNext!=NULL) /*about to delete first move*/
				keepNext = bestMovesList;  /*head of list will contain second element, so that's our next move*/
			deleteMoveFromList(bestMovesList->curr,answer);
		}
		bestMovesList = keepNext;
	}

	return answer;
}

/* @pre: move is valid and legal
 * execute and calculate minimax score for move as it would be in actual minimax */
int miniMax_move(move_t* move, char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, char playerA, char currentPlayer) {
	int bestScore = currentPlayer==PLAYER_A? MIN_INF:MAX_INF; /*init to worst possible score for current player*/
	movesList_t* moves;
	int alpha=MIN_INF, beta=MAX_INF, maxBoards=MM_LIMIT;
	char newBoard[BOARD_SIZE][BOARD_SIZE];
	saveCastlingFlags();

	/* duplicate depth for actual minimax best */
	if ((moves=getAllLegalMoves(board, currentPlayer==PLAYER_A? playerA:invColor(playerA))) == NULL)
		return MM_ERROR;
	maxBoards /= moves->size; /*move is in moves and so !isEmpty(moves)*/
	freeList(moves);

	copyBoard(newBoard, board);
	if (DEBUG_MM_SCORE) {
		for (unsigned i=depth ; i<4 ; i++)
			putchar('\t');
		printf("depth %u: playing ", depth);
		printMove(move);
	}
	playMove(newBoard, move);
	bestScore = miniMax_rec(newBoard, depth==BEST? depth:depth-1, \
		playerA, currentPlayer==PLAYER_A? PLAYER_B:PLAYER_A, alpha, beta, maxBoards, 1);

	restoreCastlingFlags();
	return bestScore;
}

/* @post: return MM_ERROR in case of bad alloc
 */
int miniMax_rec(char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, \
		char playerA, char currentPlayer, int alpha, int beta, int maxBoards, int realDepth) {
	/* playerA = computerColor (who we run the algorithm for) -> white/black (maximizing player)
	 * currentPlayer = A/B (are we in min or max level? A=max, B=min)
	 */
	movesList_t* moves = NULL;
	movesList_t* nextMove=NULL;
	int best_factor = depth==BEST? 10:1;
	int bestScore = currentPlayer==PLAYER_A? MIN_INF:MAX_INF; /*init to worst possible score for current player*/
	int tmp;
	int fromRow, fromCol, toRow, toCol;
	char special, undoTo, undoFrom;
	char castling[8];
	int i;

	if (depth!=0 && (maxBoards>1 || realDepth<4)) { /*not terminal node - prevent wasted moves generation*/
		moves = getAllLegalMoves(board, currentPlayer==PLAYER_A? playerA:invColor(playerA));
		if (moves==NULL) {
			return MM_ERROR; /*allocation error code*/
		}
		for (nextMove=moves ; !isEmpty(nextMove) ; nextMove=nextMove->next)
			nextMove->curr->score = bestScore; /*initall scores to worse possible*/
		nextMove=NULL;
	}

	if (depth==BEST && !isEmpty(moves) && maxBoards>=moves->size) /*makes sure we only use this limit for best option*/
		maxBoards /= moves->size;
	if (depth==0 || isEmpty(moves) || (maxBoards<=1 && realDepth>=4)) {
		bestScore = scoringFunction(board, playerA, currentPlayer, depth, realDepth);
		if (bestScore == MM_ERROR) { /*allocation error code*/
			freeList(moves);
			return MM_ERROR;
		}
	} else { /*for maxBoards<moves->size use cntr to itterate on exactly maxBoards number of moves*/
		for (nextMove=moves ; nextMove!=NULL ; nextMove=nextMove->next) {
			saveCastlingFlags();
			saveLastMove();
			playMove(board, nextMove->curr);
			tmp = miniMax_rec(board, depth==BEST? depth:depth-1, playerA, \
					currentPlayer==PLAYER_A? PLAYER_B:PLAYER_A, alpha, beta, \
							maxBoards<moves->size? 0:maxBoards, realDepth+1); /*force next level to evalute the board if maxed out*/
			restoreCastlingFlags();
			restoreLastMove();
			if (tmp == MM_ERROR) {
				bestScore = tmp; /*causes error code to go to the main caller*/
				break;
			}
			nextMove->curr->score = tmp; /*update score for current move*/

			if (DEBUG_MM) {
				for (int i=realDepth ; i>0 ; i--)
					printf("\t");
				printf("depth %d: playing ", realDepth);
				printMove(nextMove->curr);
			}

			if (currentPlayer==PLAYER_A && tmp>bestScore) { /*maximize score*/
				bestScore = tmp;
				alpha = alpha>bestScore? alpha:bestScore; /*maximize alpha*/
			} else if (currentPlayer==PLAYER_B && tmp<bestScore) { /*minimize score*/
				bestScore = tmp;
				beta = beta<bestScore? beta:bestScore; /*minimize beta*/
			}
			if (beta<=alpha) {
				if (DEBUG_MM) {
					for (int i=realDepth ; i>0 ; i--)
						putchar('\t');
					printf("depth %d: pruned!\n", realDepth);
				}
				break;
			}
		}
	}

	/*adjust score for tie - needs to be second worse option for the opposite of currentPlayer*/
	if ( (bestScore==TIE_A*best_factor && currentPlayer==PLAYER_A) || (bestScore==TIE_B*best_factor && currentPlayer==PLAYER_B) )
		bestScore *= -1;

	freeList(moves); /*free all moves in list*/
	return bestScore;
}


move_t* miniMax_env(char board[BOARD_SIZE][BOARD_SIZE], unsigned depth, char playerA, char currentPlayer) {
	move_t* move = NULL;
	movesList_t *nextMove, *moves;

	moves = miniMax_lst(board, depth, playerA, currentPlayer, LIST_BEST);
	if (moves==NULL || isEmpty(moves)) /*minimax error*/
		return NULL;

	/* randomize selection in case of multiple options */
	int r = (rand() % moves->size);
	for (nextMove=moves ; r-->0 ; nextMove=nextMove->next);
	move = nextMove->curr;

	freeListWithException(moves, move);
	return move;

}

int scoringFunction(char board[BOARD_SIZE][BOARD_SIZE], char playerA, char currentPlayer, int depth, int realDepth){
	unsigned pieces[12];
	int score = 0, white = 0, black = 0, maxWhite, maximize;
	int eval;
	char player;

	maxWhite = (WHITE == playerA) ? 1 : -1; /* For white player - score is correct. For black player - should negate */
	maximize = (currentPlayer == PLAYER_A) ? 1 : -1; /* if A is current - score is correct. If B is current - should negate */
	player = maxWhite==maximize? WHITE:BLACK; /*current player is white/black*/
	countPieces(board, pieces); /*Will fill unsigned answer[12] - [m,n,b,r,q,k,M,N,B,R,Q,K]*/

	if (depth != BEST) { /*For minimax depth 0-4*/
		/*(M)pawn = 1 , kNight = 3 , Bishop = 3 , Rook = 5, Queen = 9, King=400*/
		white = pieces[0] + pieces[1] * 3 + pieces[2] * 3 + pieces[3] * 5 + pieces[4] * 9 + pieces[5] * 400;
		black = pieces[6] + pieces[7] * 3 + pieces[8] * 3 + pieces[9] * 5 + pieces[10] * 9 + pieces[11] * 400;
	}
	else { /* BEST - uses better scoring for knight and rook, but with a x10 factor to avoid fp numbers */
		/* basic score */
		white = pieces[0]*10 + pieces[1]*33 + pieces[2]*34 + pieces[3]*50 + pieces[4]*90;
		black = pieces[6]*10 + pieces[7]*33 + pieces[8]*34 + pieces[9]*50 + pieces[10]*90;
	} /* we must later adjust WIN/TIE scores with 10x factor, just to make sure they are the highest */

	eval=evalBoard(board,player);

	switch(eval) {
		case WHITE:
			score = WIN_A*maxWhite;
			break;
		case BLACK:
			score = WIN_B*maxWhite;
			break;
		case CONTINUE:
		case CHECK:
			if (depth==BEST && white==0 && black==0) { /* best sees 2 kings as tie */
				score = TIE_B*maximize;
			} else
				score = (white-black)*maxWhite;
			break;
		case TIE:
			score = TIE_B*maximize;
			break;
		case -1:
			return MM_ERROR; /*allocation error code*/
	}

	/* adjust WIN/TIE scores with 10x factor for BEST */
	if (depth==BEST && (eval==WHITE || eval==BLACK || eval==TIE)) {
		score = score*10;
		/*add estimate to depth from maxBoards - the shorter it took to get the best score, the better*/
		if (eval!=TIE) {
			realDepth = DEPTH_FACTOR-realDepth;
			realDepth = realDepth>0? realDepth:0;
			if (score>0)
				score += realDepth;
			else
				score -= realDepth;
		}
	}

	return score;
}

