/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 files.c                                      */
/* contents: save and load functions					  */
/* 			 requires ./Files folder                      */
/**********************************************************/
#include "files.h"

/* Saves the game to given saveFile
 * Will return -1 on failure, 0 on success */
int saveGame(char* saveFile){
	FILE* f;
	char c;
	unsigned i,j;

	if((f = fopen(saveFile,"w")) == NULL){
		return -1;
	}

	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n",f);
	fputs("<game>\n",f);

		if(fprintf(f, "<next_turn>%s</next_turn>\n", nextPlayer==WHITE? "White":"Black")<0) {
			fclose(f);
			return -1; }

		if(fprintf(f, "<game_mode>%c</game_mode>\n", '0'+gameMode)<0) {
			fclose(f);
			return -1; }

		if(fputs("<difficulty>",f)<0) {
			fclose(f);
			return -1; }
		if (gameMode==PVA) {
			if (minimaxDepth==BEST)
				fputs("best",f);
			else
				fprintf(f, "%u", minimaxDepth);
		}
		if(fputs("</difficulty>\n",f)<0) {
			fclose(f);
			return -1; }

		if(fputs("<user_color>",f)<0) {
			fclose(f);
			return -1; }
		if (gameMode==PVA && fputs(userColor==WHITE? "White":"Black",f)<0) { /*writes titled colors as per instructions and xml examples*/
			fclose(f);
			return -1; }
		if(fputs("</user_color>\n",f)<0) {
			fclose(f);
			return -1; }

		if(fputs("<board>\n",f)<0) {
			fclose(f);
			return -1; }
		for(i=BOARD_SIZE; i>0; i--){ /*rows*/
			if(fprintf(f,"<row_%d>",i)<0) {
				fclose(f);
				return -1; }
			for (j=0; j<BOARD_SIZE; j++){ /*columns*/
				c = (gameBoard[j][i-1]==EMPTY) ? '_':gameBoard[j][i-1];
				if(fputc(c,f)<0) {
					fclose(f);
					return -1; }
			}
			if(fprintf(f,"</row_%d>\n",i)<0) {
				fclose(f);
				return -1; }
		}
		if(fputs("</board>\n",f)<0) {
			fclose(f);
			return -1; }

		if(fputs("<general>\n",f)<0) {
			fclose(f);
			return -1; }
			if (fprintf(f, "<castling>%d%d%d%d%d%d</castling>\n", (int)wk, (int)wlr, (int)wrr, (int)bk, (int)blr, (int)brr)<0) {
				fclose(f);
				return -1; }
		if(fputs("</general>\n",f)<0) {
			fclose(f);
			return -1; }

	if(fputs("</game>\n",f)<0) {
		fclose(f);
		return -1; }

	fclose(f);

	return 0;
}

/* Loads the game from given loadFile
 * Will return -1 on failure, 0 on success */
int loadGame(char* loadFile){
	FILE* f;
	char *s, line[51] = ""; /*upper bound for line length in xml file format*/
	char game_o=0, game_c=0, next_turn=0, game_mode=0, difficulty=0, user_color=0, board_o=0, board_c=0;
	char general_o=0, general_c=0, castling=0;

	if((f = fopen(loadFile,"r")) == NULL){
		return -1;
	}

	if (freadline(f, line)==EOF || strcmp(line, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>")!=0) { /*not proper format*/
		fclose(f);
		return -1;
	}

	for (int e=freadline(f, line) ; e!=EOF ; e=freadline(f, line)) {
		s = skipSpaces(line);
		if (strcmp(s, "<game>")==0) {
			game_o = 1;
		} else if (strncmp(s, "<next_turn>", 11)==0) {
			s+=11;
			if (strncmp(s, "White", 5)==0 || strncmp(s, "white", 5)==0)
				nextPlayer = WHITE;
			else if (strncmp(s, "Black", 5)==0 || strncmp(s, "black", 5)==0)
				nextPlayer = BLACK;
			if (strstr(s, "</next_turn>")!=NULL)
				next_turn = 1;
		} else if (strncmp(s, "<game_mode>", 11)==0) {
			if (s[11]=='1' || s[11]=='2')
				gameMode = s[11]=='1'? PVP:PVA;
			if (strstr(s, "</game_mode>")!=NULL) {
				game_mode = 1;
				if (gameMode == PVP)
					user_color = difficulty = 1;
			}
		} else if (strncmp(s, "<difficulty>", 12)==0) {
			if (!difficulty) {
				if (strncmp(s+12, "best", 4)==0)
					minimaxDepth = BEST;
				else
					minimaxDepth = s[12]-'0';
			}
			if (strstr(s, "</difficulty>")!=NULL)
				difficulty = 1;
			else
				difficulty = 0;
		} else if (strncmp(s, "<user_color>", 12)==0) {
			s+=12;
			if (!user_color) {
				if (strncmp(s, "White", 5)==0 || strncmp(s, "white", 5)==0)
					nextPlayer = WHITE;
				else if (strncmp(s, "Black", 5)==0 || strncmp(s, "black", 5)==0)
					nextPlayer = BLACK;
			}
			if (strstr(s, "</user_color>")!=NULL)
				user_color = 1;
			else
				user_color = 0;
		} else if (strncmp(s, "<board>", 7)==0) {
			board_o = 1;
		} else if (strncmp(s, "<row_", 5)==0) {
			if (isdigit(s[5]) && s[5]<='8' && s[5]>='1' && s[21]==s[5] && \
					s[6]=='>' && strncmp(s+15, "</row_", 6)==0 && s[22]=='>') {
				s+=7;
				for (int col=0, row=s[14]-'1' ; col<BOARD_SIZE ; col++)
					gameBoard[col][row] = s[col]=='_'? EMPTY:s[col];
			}
		} else if (strncmp(s, "</board>", 8)==0) {
			board_c = 1;
		} else if (strncmp(s, "<general>", 9)==0) {
			general_o = 1;
		} else if (strncmp(s, "<castling>", 10)==0) {
			s+=10;
			if (strncmp(s+6, "</castling>", 11)==0) {
				wk = s[0]-'0';
				wlr = s[1]-'0';
				wrr = s[2]-'0';
				bk = s[3]-'0';
				blr = s[4]-'0';
				brr = s[5]-'0';
				castling = 1;
			}
		} else if (strncmp(s, "</general>", 9)==0) {
			general_c = 1;
		} else if (strncmp(s, "</game>", 7)==0) {
			game_c = 1;
		}
	}
	fclose(f);

	/*check status flags and update castling and return val*/
	if (!castling) { /*didn't load explicit castling information from file*/
		wk = gameBoard[4][0]!=W_KING;
		wlr = gameBoard[0][0]!=W_ROOK;
		wrr = gameBoard[7][0]!=W_ROOK;
		bk = gameBoard[4][7]!=B_KING;
		blr = gameBoard[0][7]!=B_ROOK;
		brr = gameBoard[7][7]!=B_ROOK;
		castling = 1;
	}

	return !(game_o&&game_c&&next_turn&&game_mode&&difficulty&&user_color&&board_o&&board_c&&castling) && general_o==general_c;
}

int freadline(FILE* f, char s[51]) {
	int len=0;
	for (int c=fgetc(f) ;c != '\r' && c != '\n' ; c=fgetc(f)) {
		if (len==0 && c==EOF)
			return EOF;
		s[len++] = c;
		if(len == 50)
			break;
	}
	s[len] = '\0';
	return len;
}
