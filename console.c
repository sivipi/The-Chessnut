/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 console.c                                    */
/* contents: console mode "main" function and pasrsing    */
/**********************************************************/
#include "console.h"
#include "minimax.h"
#include "files.h"

char* getInput(){
	int len=0;
	for (int c=getchar() ;c != '\r' && c != '\n' ; c=getchar()) {
		str_in[len++] = c;
		if(len == MAX_INPUT)
			break;
	}
	str_in[len] = '\0';
	return str_in;
}

int consoleMode() {
	char* in;
	int earlyWin/*, userTurn=1*/;

	/*Settings state*/
	init_board(gameBoard);
	print_board(gameBoard);
	while (!startGame) {
		print_message(ENTER_SETTINGS);
		in = getInput();
		parseSettings(in);
	}
	if (DEBUG)
		print_board(gameBoard);

	/*check for early win*/
	earlyWin = evalBoard(gameBoard, nextPlayer);
	analizeState(earlyWin); /*analize board state and determine proper prints and value for startGame*/

	/*Game state*/
	while (startGame) {
		if (gameMode==PVA) {
			if (userColor == nextPlayer) {
				printf("%s player - enter your move:\n", userColor==WHITE? "White":"Black");
				in = getInput();
				nextPlayer = parseGame(in)? invColor(nextPlayer):nextPlayer; /*only switch turns if user played*/
			} else {
				analizeState(computerPlay()); /*analize board state and determine proper prints and value for startGame*/
				nextPlayer = invColor(nextPlayer);
			}
		} else { /*gameMode==PVP*/
			printf("%s player - enter your move:\n", nextPlayer==WHITE? "White":"Black");
			in = getInput();
			userColor = nextPlayer; /*parseGame functions use userColor as current playing user color*/
			nextPlayer = parseGame(in)? invColor(nextPlayer):nextPlayer; /*only switch turns if user played*/
		}
	}
	return 0;
}

/*settings state*/
void parseSettings(char* s) {
	int tmp;
	char color, piece;
	pos_t p;

	if (strncmp(s, "game_mode ", 10)==0) {
		s = skipSpaces(s+10);
		if (strlen(s)==1 && (*s=='1' || *s=='2')) {
			gameMode = *s=='1'? PVP:PVA;
			printf("Running game in %s mode\n", gameMode==PVP? "2 players":"player vs. AI");
		} else /*invalid input*/
			printf("Wrong game mode\n");
	}
	else if (strncmp(s, "difficulty ", 11)==0 && gameMode==PVA) {
		s = skipSpaces(s+11);
		if (strncmp(s, "depth ", 6)==0) {
			s = skipSpaces(s+6);
			if (strlen(s) && '1'<=*s && *s<='4') /*evaluates by ascii in range*/
				minimaxDepth = atoi(s);
			else
				print_message(WRONG_MINIMAX_DEPTH);
		} else if (strcmp(s, "best")==0) {
			minimaxDepth = BEST;
		} else
			print_message(ILLEGAL_COMMAND);
		if (DEBUG)
			printf("depth: %u\n", minimaxDepth);
	}
	else if (strncmp(s, "user_color ", 11)==0 && gameMode==PVA) {
		s = skipSpaces(s+11);
		if (strcmp(s, "white")==0)
			userColor = WHITE;
		else if (strcmp(s, "black")==0)
			userColor = BLACK;
		else
			print_message(WRONG_COLOR);
		if (DEBUG)
			printf("user color: %c\n", userColor);
	}
	else if (strncmp(s, "load ", 5)==0) {
		s = skipSpaces(s+5);
		if ((tmp=loadGame(s))==0)
			print_board(gameBoard);
		else /*file does not exist*/
			printf("Wrong file name\n");
	}
	else if (strcmp(s, "clear")==0) {
		clearBoard(gameBoard);
		if (DEBUG)
			print_board(gameBoard);
	}
	else if (strncmp(s, "next_player ", 12)==0) {
		s = skipSpaces(s+12);
		if (strcmp(s, "white")==0)
			nextPlayer = WHITE;
		else if (strcmp(s, "black")==0)
			nextPlayer = BLACK;
		else
			print_message(WRONG_COLOR);
	}
	else if (strncmp(s, "rm ", 3)==0) {
		s = skipSpaces(s+3);
		p = parsePos(s);
		if (isValidPos(p))
			rm(gameBoard, p);
		else
			print_message(WRONG_POSITION);
		if (DEBUG)
			print_board(gameBoard);
	}
	else if (strncmp(s, "set ", 4)==0) {
		s = skipSpaces(s+4); /*get to pos argument*/
		p = parsePos(s);
		if (!isValidPos(p)) {
			print_message(WRONG_POSITION);
		} else {
			s = strchr(s, '>')+1; /*get to color argument, guaranteed to find '>' since pos is valid*/
			s = skipSpaces(s);
			if (strncmp(s, "white", 5)==0)
				color = WHITE;
			else if (strncmp(s, "black", 5)==0)
				color = BLACK;
			else {
				print_message(WRONG_COLOR);
				return;
			}

			s = skipSpaces(s+5); /*get to piece argument*/
			piece = parsePiece(s, color);
			if (piece == '\0') { /*invalid/illegal piece error code*/
				print_message(NO_PIECE);
			} else if ((piece==W_PAWN && p.row==BOARD_SIZE-1) || (piece==B_PAWN && p.row==0)) { /*handler for implicit promotion*/
				print_message(WRONG_POSITION);
			} else {
				setPiece(gameBoard, p, piece);
			}
			if (DEBUG)
				print_board(gameBoard);
		}
	}
	else if (strcmp(s, "print")==0) {
		print_board(gameBoard);
	}
	else if (strcmp(s, "quit")==0) {
		if (DEBUG)
			print_message("quitting game");
		quit(0);
	}
	else if (strcmp(s, "start")==0) {
		startGame = start(gameBoard);  /*start if can start*/
		if (DEBUG)
			printf("startGame: %u\n", startGame);
		if (!startGame)
			print_message(WROND_BOARD_INITIALIZATION);
	}
	else { /* no match to any known command */
		print_message(ILLEGAL_COMMAND);
	}
}

/*parse first pos in string s*/
pos_t parsePos(char* s) {
	pos_t p = {BOARD_SIZE+1, BOARD_SIZE+1}; /*use BOARD_SIZE+1 as invalid pos code*/
	char *start, *mid, *end;
	start = strchr(s, '<');
	mid = strchr(s, ',');
	end = strchr(s, '>');

	if (start==NULL || mid==NULL || end==NULL) /*illegal pos format*/
		return p;
	if (mid-start != 2 && end-mid != 2 && *(end+1) != '\0') /* there should be precisely 1 char between each char*, end is really end*/
		return p;

	p.col = *(start+1)-'a'; /* will check validity after return */
	p.row = *(mid+1)-'1';  /* will check validity after return */

	return p;
}

/* parse piece from name string with respect to current gameBoard,
 * i.e. will return '\0' if adding the piece invalidates the board.
 */
char parsePiece(char* s, char color) {
	char piece = '\0';
	unsigned answer[12], index=0, offset=color==WHITE? 0:6; /*for countPieces*/
	countPieces(gameBoard, answer);	/*Will fill unsigned answer[12] - [m,n,b,r,q,k,M,N,B,R,Q,K]*/

	if (strcmp(s, "king")==0) {
		piece = color==WHITE? W_KING:B_KING;
		index = 5;
		if (answer[index+offset]==1)
			piece = '\0';
	} else if (strcmp(s, "queen")==0) {
		piece = color==WHITE? W_QUEEN:B_QUEEN;
		index = 4;
		if (answer[index+offset]==1)
			piece = '\0';
	} else if (strcmp(s, "rook")==0) {
		piece = color==WHITE? W_ROOK:B_ROOK;
		index = 3;
		if (answer[index+offset]==2)
			piece = '\0';
	} else if (strcmp(s, "knight")==0) {
		piece = color==WHITE? W_KNIGHT:B_KNIGHT;
		index = 1;
		if (answer[index+offset]==2)
			piece = '\0';
	} else if (strcmp(s, "bishop")==0) {
		piece = color==WHITE? W_BISHOP:B_BISHOP;
		index = 2;
		if (answer[index+offset]==2)
			piece = '\0';
	} else if (strcmp(s, "pawn")==0) {
		piece = color==WHITE? W_PAWN:B_PAWN;
		index = 0;
		if (answer[index+offset]==8)
			piece = '\0';
	}

	return piece;
}

/*game state*/

/* parsing function for user input
 * @return: 1 iff user played a move (determine if need to switch turns)
 * @post: upon error, print proper message and change startGame to 0 if unrecoverable
 */
unsigned parseGame(char* s) { /*user's turn to play*/
	move_t* move=NULL;
	movesList_t* moves=NULL;
	pos_t p;
	int tmp, boardState;
	char* tmp_str;

	if (strncmp(s, "move ", 5)==0) {
		if ((move=parseMoveFull(s)) != NULL) { /*if NULL: error was printed in function and startGame updated if needed*/
			/*play, print, check for win and terminate if needed*/
			playMove(gameBoard, move);
			freeMove(move);
			print_board(gameBoard);
			boardState = evalBoard(gameBoard, invColor(nextPlayer)); /*evaluate the opposing player's options*/
			analizeState(boardState); /*analize board state and determine proper prints and value for startGame*/
			return 1;
		}
	}
	else if (DEBUG && strcmp(s, "get_moves")==0) { /*not required but helpful*/
		printAllLegalMoves(gameBoard, userColor);
	}
	else if (strncmp(s, "get_moves ", 10)==0) {
		s = skipSpaces(s+10);
		p = parsePos(s);
		if (!isValidPos(p)) /*case 1*/
			print_message(WRONG_POSITION);
		else if (!isUserPos(gameBoard, p, userColor)) /*case 2*/
			print_message(NO_DICS);
		else { /*print moves from selected pos*/
			moves = getMovesForPiece(gameBoard, p);
			if (moves==NULL) {
				startGame = 0; /*will exit game loop*/
				print_malloc_error;
			} else {
				printMovesList(moves);
				freeList(moves);
			}
		}
	}
	else if (strncmp(s, "get_best_moves ", 15)==0) {
		s = skipSpaces(s+15);
		if (strcmp(s, "best")==0)
			tmp = BEST;
		else /*numeric argument in legal range*/
			tmp = atoi(s);
		moves = miniMax_lst(gameBoard, tmp, userColor, PLAYER_A, LIST_BEST);
		if (moves == NULL) {
			startGame = 0; /*will exit game loop*/
			print_malloc_error;
		} else {
			printMovesList(moves);
			freeList(moves);
		}
	}
	else if (strncmp(s, "get_score ", 10)==0) {
		s = skipSpaces(s+10);
		if (strncmp(s, "best", 4)==0)
			tmp = BEST;
		else /*numeric argument in legal range*/
			tmp = *s - '0'; /*adjust d char relative to ascii*/
		if ((tmp_str = strstr(s, "move ")) != NULL)
			move = parseMoveFull(tmp_str); /*if NULL: error was printed in function and startGame updated if needed*/
		else if ((tmp_str = strstr(s, "castle ")) != NULL)
			move = parseCastling(tmp_str); /*if NULL: error was printed in function and startGame updated if needed*/
		else
			print_message(ILLEGAL_COMMAND);

		if (move != NULL) { /*if NULL: error was printed earlier and startGame updated if needed*/
			tmp = miniMax_move(move, gameBoard, tmp, userColor, PLAYER_A);
			if (tmp == MM_ERROR) {
				startGame = 0;
				print_malloc_error;
			} else { /*all is good, finally..*/
				printf("%d\n", tmp);
			}
			freeMove(move);
		}
	}
	else if (strncmp(s, "castle ", 7)==0) {
		if ((move=parseCastling(s)) != NULL) { /*play, print, check for win and terminate if needed*/
			playMove(gameBoard, move);
			freeMove(move);
			print_board(gameBoard);
			boardState = evalBoard(gameBoard, invColor(nextPlayer)); /*evaluate the opposing player's options*/
			analizeState(boardState); /*analize board state and determine proper prints and value for startGame*/
		}
	}
	else if (strncmp(s, "save ", 5)==0) {
		s = skipSpaces(s+5);
		if (saveGame(s)==-1) /*error code*/
			printf("Wrong file name\n"); /*the file cannot be created or overridden*/
	}
	else if (strcmp(s, "quit")==0) {
		startGame = 0;
	}
	else { /*no match to any known command*/
		print_message(ILLEGAL_COMMAND);
	}
	return 0; /*user did not play*/
}

/* auxiliary for "castle " parser to prevent code duplication in "get_score "
* @pre: char* s points to start of "castle " command
* @post: all error prints and status changes are dealt with inside the function, returns NULL upon failure
*/
move_t* parseCastling(char *s) {
	pos_t p;
	move_t* move = NULL;
	movesList_t* moves = NULL;

	s = skipSpaces(s + 7);
	p = parsePos(s);
	if (!isValidPos(p)) /*case 1*/
		print_message(WRONG_POSITION);
	else if (!isUserPos(gameBoard, p, userColor)) /*case 2*/
		print_message(NO_DICS);
	else if (gameBoard[p.col][p.row] != W_ROOK && gameBoard[p.col][p.row] != B_ROOK) /*no rook*/
		print_message(NO_ROOK);
	else if ((moves = getCastlingMoves(gameBoard, userColor)) == NULL) { /*allocation error*/
		startGame = 0; /*will exit game loop*/
		print_malloc_error;
	}
	else { /*check if castling not possible for s*/
		for (movesList_t* nextMove = moves; nextMove != NULL; nextMove = nextMove->next) /*either 0,1 or 2 moves*/
			if (nextMove->curr->curr.col == p.col && nextMove->curr->curr.row == p.row) /*make sure it's not the other one*/
				move = nextMove->curr;
		freeListWithException(moves, move); /*keep good move if exists, otherwise delete all*/
		if (move == NULL)
			print_message(ILLEGAL_CASTLING);
	}

	return move; /*will be NULL for all illegal values*/
}

/* auxiliary for "move " parser to prevent code duplication in "get_score "
 * @pre: char* s points to start of "move " command
 * @post: all error prints and status changes are dealt with inside the function
 */
move_t* parseMoveFull(char* s) {
	move_t* move = parseMove(s+5); /*prase whole move - start pos and move combined*/
	if (move == NULL) {
		print_malloc_error;
		startGame = 0;
	} else if (!isValidMove(move)) { /*case 1*/
		print_message(WRONG_POSITION);
		freeMove(move);
		move = NULL;
	} else if (!isUserPos(gameBoard, move->curr, userColor)) { /*case 2*/
		print_message(NO_DICS);
		freeMove(move);
		move = NULL;
	} else { /*move is within board limits and starts from user pos*/
		if (isPromotionMove(gameBoard, move) && move->special==NORM)
			move->special = move->next->special = userColor==WHITE? W_QUEEN:B_QUEEN;
		if (!isLegalMove(gameBoard, move, userColor)) { /*case 3*/
			print_message(ILLEGAL_MOVE);
			freeMove(move);
			move = NULL;
		}
	}
	return move;
}

/* parse fixed size moves of length 2
 * special moves are only parsed here, but validated later!
 */
move_t* parseMove(char* s) {
	move_t *move, *last;

	move = (move_t*)malloc(sizeof(move_t));
	if (move==NULL)
		return NULL; /*allocation error code, handle by caller*/
	move->curr = parsePos(s);
	move->size = 2;
	move->next = NULL;
	move->special = NORM;

	last = (move_t*)malloc(sizeof(move_t));
	if (last==NULL) {
		freeMove(move);
		return NULL; /*allocation error code, handle by caller*/
	}
	/*move up s to start of first pos after "to", we assume command in legal format*/
	s = strstr(s, "to"); /*skip starting pos*/
	s = strchr(s, '<'); /*point to start of second pos*/
	last->curr = parsePos(s);
	last->size = 1;
	last->next = NULL;
	last->special = NORM;
	move->next = last; /*concat move*/

	s = strchr(s, '>')+1; /*point to first char after second pos*/
	s = skipSpaces(s); /*point to start of promotion piece, or '\0'*/
	if (*s != '\0') { /*parse special move*/
		if (strcmp(s, "queen")==0)
			move->special = last->special = userColor==WHITE? W_QUEEN:B_QUEEN;
		else if (strcmp(s, "rook")==0)
			move->special = last->special = userColor==WHITE? W_ROOK:B_ROOK;
		else if (strcmp(s, "knight")==0)
			move->special = last->special = userColor==WHITE? W_KNIGHT:B_KNIGHT;
		else if (strcmp(s, "bishop")==0)
			move->special = last->special = userColor==WHITE? W_BISHOP:B_BISHOP;
	}

	return move;
}

/*used to update size for each element in move*/
void updateMoveSize(move_t* move) {
	unsigned n = move->size;
	do {
		move->size = n--;
		move = move->next;
	} while (move != NULL);
}

/*check if move is for pawn promotion*/
int isPromotionMove(char board[BOARD_SIZE][BOARD_SIZE], move_t* move) {
	pos_t start=move->curr, end=move->next->curr;
	return  (start.row==6 && end.row==7 && board[start.row][start.col]==W_PAWN) ||
			(start.row==1 && end.row==0 && board[start.row][start.col]==B_PAWN);
}

/*return the state of the board after playing the computer's turn
 *return -1 as error code and print*/
int computerPlay() {
	move_t* move = miniMax_env(gameBoard, minimaxDepth, computerColor, PLAYER_A); /*extract best move for computer*/
	int boardState;
	if (move == NULL) /*allocation error*/
		return -1;
	playMove(gameBoard, move);
	printf("Computer: move ");
	printMove(move);
	freeMove(move);
	print_board(gameBoard);
	boardState = evalBoard(gameBoard, userColor);

	return boardState;
}

/* wrapper function for board state code with prints and startGame flag change if needed */
int analizeState(int boardState) {
	switch (boardState) {
	case CONTINUE:
		break;
	case CHECK:
		printf("Check!\n");
		break;
	case TIE:
		startGame = 0; /*game over, will exit game loop*/
		printf(TIE_MSG);
		break;
	case WHITE:
	case BLACK:
		startGame = 0; /*game over, will exit game loop*/
		printf("Mate! %s player wins the game\n", boardState==WHITE? "White":"Black");
		break;
	case -1:
		startGame = 0; /*game over, will exit game loop*/
		print_malloc_error;
		break;
	}
	return boardState;
}

/* return 1 on success. -1 on fail */
int printAllLegalMoves(char board[BOARD_SIZE][BOARD_SIZE], char color){
	movesList_t *movesList;

	movesList = getAllLegalMoves(board ,color);
	if (movesList == NULL)
		return -1;

	for (movesList_t* move=movesList ; move!=NULL ; move=move->next ) /* print each move in movesList seperately */
		printMove(move->curr);

	freeList(movesList);
	return 1;
}

/*Prints a single move - auxiliary for printAllMoves() */
void printMove(move_t* move) {

	if (move == NULL)
		return;
	if (move->special == CASTLE) { /*For Castling move - assumes Rooks move*/
		printf("castle <%c,%d>", 'a'+move->curr.col, move->curr.row + 1);
		if (DEBUG_MM_SCORE)
			printf("\t(score: %d)", move->score);
		printf("\n");
		return;
	}

	printf("<%c,%d> to ", 'a'+move->curr.col, move->curr.row + 1);
	printf("<%c,%d>", 'a'+move->next->curr.col, move->next->curr.row + 1);

	if (move->special != NORM) { /*For pawn promotion*/
		switch(move->special) {
			case W_KNIGHT:
			case B_KNIGHT:
				printf(" knight");
				break;
			case W_BISHOP:
			case B_BISHOP:
				printf(" bishop");
				break;
			case W_ROOK:
			case B_ROOK:
				printf(" rook");
				break;
			case W_QUEEN:
			case B_QUEEN:
				printf(" queen");
				break;
		}
	}
	if (DEBUG_MM_SCORE)
		printf("\t(score: %d)", move->score);
	printf("\n");
}

void printMovesList(movesList_t* movesList) {
	for ( ; movesList != NULL ; movesList = movesList->next ) /* print each move in movesList seperately */
		printMove(movesList->curr);
}


void print_line(){
	int i;
	printf(" |");
	for (i = 1; i < BOARD_SIZE*4; i++){
		printf("-");
	}
	printf("|\n");
}

void print_board(char board[BOARD_SIZE][BOARD_SIZE])
{
	int i,j;
	print_line();
	for (j = BOARD_SIZE-1; j >= 0 ; j--)
	{
		printf("%d", j+1);
		for (i = 0; i < BOARD_SIZE; i++){
			printf("| %c ", board[i][j]);
		}
		printf("|\n");
		print_line();
	}
	printf("  ");
	for (j = 0; j < BOARD_SIZE; j++){
		printf(" %c  ", (char)('a' + j));
	}
	printf("\n");
}
