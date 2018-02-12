/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 files.h                                      */
/* contents: save and load functions					  */
/* 			 requires ./Files folder                      */
/**********************************************************/
#include "chessprog.h"
#ifndef FILES_H_
#define FILES_H_

#define SAVED_FILES 7

int saveGame(char* saveFile);
int loadGame(char* loadFile);
int freadline(FILE* f, char s[51]);

#endif
