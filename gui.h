/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 gui.h                                        */
/* contents: gui mode "main" function and frameworks	  */
/* 			 requires designated ./Graphics folder        */
/*			 game resolution: 800x600                     */
/**********************************************************/
#include "chessprog.h"
#include "console.h"
#include "files.h"
#include "minimax.h"

#ifndef GUI_H_
#define GUI_H_

#include <SDL/SDL.h>		//TODO: make sure proper format for submission
#include <SDL/SDL_video.h>
#include <time.h>

#define DEBUG_GUI 0

/* main window parameters */
#define SCREEN_WIDTH 800
#define SCREEN_HIGHT 600
#define SCREEN_BPP 24

/* sprite sheets' parameters */
#define TILE_WIDTH 74
#define TILE_HIGHT 74
#define NEXT_PIECE 80

#define STUPID_DELAY 1000
#define PATH_FORMAT "Files/chess%d.xml"

#define SLOTS 7
#define PIECES "Graphics/pieces.bmp"
#define PIECES_FRAME "Graphics/pieces_frame.bmp"
#define BACK "Graphics/back.bmp"
#define BUTTONS "Graphics/buttons.bmp"
#define CENTRAL "Graphics/central.bmp"

/* control switch parameters */
#define MAIN_MENU 'm'
#define LOAD_MENU 'l'
#define SAVE_MENU 'v'
#define SELECTION_MENU 's'
#define PIECE_SELECTION 'p'
#define AI_MENU 'a'
#define GAME 'g'
#define QUIT 'q'
#define GUI_ERROR -1

typedef struct control_t {
	struct control_t* parent;
	struct control_t* child; /*point to leftmost child */
	struct control_t* next; /*all children point right, rightmost points to NULL*/
	int type;
	SDL_Surface* source;
	SDL_Rect srcRect;
	SDL_Rect dstRect; /* position relative to parent */
} control_t;

#define WINDOW 'w'
#define BUTTON 'b'
#define PANEL 'p'
#define LABEL 'l'

int guiMode();
void quitGUI();

/* factories */
control_t* initControl(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect);
control_t* initWindow(SDL_Surface* source);
control_t* initPanel(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect);
control_t* initLabel(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect);
control_t* initButton(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect);

/* menu construction */
control_t* constructMainMenu();
#define SAVE 's'
#define LOAD 'l'
control_t* constructSaveLoadMenu(char c); /*c determines if save or load - 's'/'l'*/
control_t* constructSelectionMenu();
control_t* constructAIMenu();
control_t* constructGameWindow();
control_t* constructPieceSelectionMenu();
control_t* constructPromotionsMenu();
control_t* constructDifficultyMenu();

/* menu functions */
int runMainMenu(control_t* root);
#define SLOTS_ERROR "ERROR: you must first select a game slot.\n"
int runSaveLoadMenu(control_t* root, char c);
#define PIECE_OVERFLOW 'o'
int runSelectionMenu(control_t* root);
int runPieceSelectionMenu(control_t* root);
int runAIMenu(control_t* root);
int runGame(control_t* root);
int playerTurn(control_t* root, int boardEvent, int panelEvent);
int computerPlay_GUI(control_t* root, int state);
char runPromotionsMenu(control_t* root);
int getHintDifficulty();

/* utilities */
SDL_Surface *loadImage(char* filename);
int applySurface(SDL_Rect* offset, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip);
int initSurfaces();
SDL_Rect initRect(int x, int y, int w, int h);
int blitControl(control_t* root, SDL_Rect offset);
int display();
void freeControl(control_t* root);

int setPiece_GUI(pos_t pos, char piece, char frame);
int rm_GUI(pos_t pos,char frame);
int playMove_GUI(move_t* move);
int initBoard_GUI();
int markPossible_GUI(pos_t pos);

int attemptedCastling(pos_t from, pos_t to);

/*returns 1 if a legit piece was clicked, 0 if not, GUI_ERROR for error*/
int markSelected_GUI(pos_t pos); /*mark only for nextPlayer*/
int showMoves_GUI(movesList_t* moves, pos_t startPos);
int showMoveCastle (move_t* move, pos_t startPos);
int showMove(move_t* move); /*Should print to console the errors himself*/
int analizeState_GUI(int boardState, control_t* root);

SDL_Rect pos2rec(pos_t pos);
int inControl(control_t* control, SDL_Event* e);
int inRect(SDL_Rect* rect, SDL_Event* e);
void clickButton(control_t* button);
pos_t click2pos(SDL_Event e);
char pieceFromSelectionPanel(control_t *root, SDL_Event *e);
SDL_Rect dstSlots(int i);

#define waitEvent(e) 	if (SDL_PollEvent(&e) == 0) { \
							SDL_Delay(100); \
							continue; \
						}
#define showBoard()	if (applySurface(&offset,back,screen,&boardClip)==GUI_ERROR ||	\
					initBoard_GUI()==GUI_ERROR || display() == GUI_ERROR){return GUI_ERROR;}

#define print_SDL_error(msg) (fprintf(stderr, "ERROR: %s.\n", msg))
#define displayError(srcRect, dstRect)	SDL_Rect src=srcRect, dst=dstRect; \
								applySurface(&dst, buttons, screen, &src); \
								display(); \
								SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE); /*ignore new events in queue*/ \
								SDL_Delay(2000); \
								SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE)


/********* rectangle macros and parameters *********/

/*All Menus Init*/
#define INIT_MENU_PANEL central, SRC_MENU_PANEL, DST_MENU_PANEL
#define INIT_EMPTY_MENU_PANEL central, SRC_EMPTY, DST_MENU_PANEL
#define SRC_MENU_PANEL initRect(0,0,500,400)
#define DST_MENU_PANEL initRect(95,100,500,400)

#define DIF_OFFSET 66
#define SLOTS_OFFSET 60
#define SELECTED_OFFSET 80

/*Error messages*/
#define SRC_ERROR_TOO_MANY_OF_THAT_PIECE 		initRect(970,600,320,24)
#define SRC_ERROR_IMPLICIT_PROMOTION			initRect(970,640,290,27)
#define DST_ERROR_IMPLICIT_PROMOTION			initRect(155,269,290,27)
#define SRC_ERROR_PIECE_OVERFLOW				initRect(970,680,228,24)
#define DST_ERROR_PIECE_OVERFLOW				initRect(138+95,323+100,264,24)
#define SRC_ERROR_BOARD_INIT					initRect(970,730,385,24)
#define DST_ERROR_BOARD_INIT					initRect(108,272,385,24)

/*main menu*/
#define INIT_MAIN_MENU_LABEL	buttons, SRC_MAIN_MENU_LABEL, DST_MAIN_MENU_LABEL
#define INIT_NEW_BUTTON			buttons, SRC_NEW_BUTTON, DST_NEW_BUTTON
#define INIT_LOAD_GAME_BUTTON	buttons, SRC_LOAD_GAME_BUTTON, DST_LOAD_GAME_BUTTON
#define INIT_MAIN_MENU_QUIT		buttons, SRC_MAIN_MENU_QUIT, DST_MAIN_MENU_QUIT

#define SRC_MAIN_MENU_LABEL		initRect(0,400,280,45)
#define DST_MAIN_MENU_LABEL		initRect(123,67,261,42)
#define SRC_NEW_BUTTON			initRect(0,0,206,66)
#define DST_NEW_BUTTON			initRect(150,127,206,66)
#define SRC_LOAD_GAME_BUTTON	initRect(210,0,206,66)
#define DST_LOAD_GAME_BUTTON 	initRect(150,127+TILE_HIGHT,206,66)
#define SRC_MAIN_MENU_QUIT 		initRect(420,0,206,66)
#define DST_MAIN_MENU_QUIT 		initRect(150,127+2*TILE_HIGHT,206,66)

/*load/save menu*/
#define INIT_LOAD_MENU_LABEL	buttons, SRC_LOAD_MENU_LABEL, DST_LOAD_MENU_LABEL
#define INIT_SAVE_MENU_LABEL	buttons, SRC_SAVE_MENU_LABEL, DST_LOAD_MENU_LABEL
#define INIT_SLOTS_BUTTON(i)	buttons, SRC_SLOTS_BUTTON(i,0), DST_SLOTS_BUTTON(i)
#define INIT_LOAD_BUTTON		buttons, SRC_LOAD_BUTTON, DST_LOAD_MENU_BUTTON
#define INIT_SAVE_BUTTON		buttons, SRC_SAVE_BUTTON, DST_LOAD_MENU_BUTTON
#define INIT_LOAD_CANCEL_BUTTON	buttons, SRC_CANCEL_BUTTON, DST_LOAD_CANCEL_BUTTON
#define INIT_ERROR_FILE			buttons, SRC_ERROR_FILE, DST_ERROR_FILE

#define SRC_LOAD_MENU_LABEL 	initRect(300,400,280,45)
#define SRC_SAVE_MENU_LABEL 	initRect(600,400,270,45)
#define DST_LOAD_MENU_LABEL 	initRect(123,67,280,45) /*same for both*/
#define SRC_SLOTS_BUTTON(i,s) 	initRect(800+i*SLOTS_OFFSET,200+s*SELECTED_OFFSET,46,46)
#define DST_SLOTS_BUTTON(i) 	dstSlots(i)
#define SRC_SAVE_BUTTON 		initRect(1220,200,64,34)
#define SRC_LOAD_BUTTON 		initRect(1300,200,64,34)
#define DST_LOAD_MENU_BUTTON 	initRect(121,281,64,34)
#define SRC_CANCEL_BUTTON 		initRect(1380,200,64,34) /*shared*/
#define DST_LOAD_CANCEL_BUTTON 	initRect(321,281,64,34)
#define SRC_ERROR_FILE 			initRect(870,400,264,24)
#define DST_ERROR_FILE 			initRect(121+95,323+100,264,24)

/*Selection Menu*/
#define INIT_SELECTION_PANEL				back, SRC_SELECTION_PANEL, DST_SELECTION_PANEL
#define INIT_SELECTION_GAME_SETTINGS_LABEL	buttons, SRC_SELECTION_GAME_SETTINGS_LABEL, DST_SELECTION_GAME_SETTINGS_LABEL
#define INIT_SELECTION_GAME_MODE_LABEL		buttons, SRC_GAME_MODE_LABEL, DST_GAME_MODE_LABEL
#define INIT_SELECTION_NEXT_PLAYER_LABEL	buttons, SRC_NEXT_PLAYER_LABEL, DST_NEXT_PLAYER_LABEL
#define INIT_PVP_BUTTON						buttons, SRC_PVP_BUTTON(0), DST_PVP_BUTTON
#define INIT_PVA_BUTTON						buttons, SRC_PVA_BUTTON(0), DST_PVA_BUTTON
#define INIT_BLACK_BUTTON					buttons, SRC_BLACK_BUTTON(0), DST_BLACK_BUTTON
#define INIT_WHITE_BUTTON					buttons, SRC_WHITE_BUTTON(0), DST_WHITE_BUTTON
#define INIT_SELECTION_UPDATE_BOARD_LABEL	buttons, SRC_SELECTION_UPDATE_BOARD_LABEL, DST_SELECTION_UPDATE_BOARD_LABEL
#define INIT_RM_BUTTON						buttons, SRC_RM_BUTTON, DST_RM_BUTTON
#define INIT_SET_BUTTON						buttons, SRC_SET_BUTTON, DST_SET_BUTTON
#define INIT_MAIN_MENU_BUTTON				buttons, SRC_MAIN_MENU_BUTTON, DST_MAIN_MENU_BUTTON
#define INIT_START_BUTTON					buttons, SRC_START_BUTTON, DST_START_BUTTON

#define SRC_SELECTION_PANEL					initRect(600,0,200,800)
#define DST_SELECTION_PANEL 				initRect(600,0,200,800)
#define SRC_SELECTION_GAME_SETTINGS_LABEL	initRect(0,500,176,21)
#define DST_SELECTION_GAME_SETTINGS_LABEL	initRect(12,10,176,21)
#define SRC_GAME_MODE_LABEL 				initRect(200,500,101,16)
#define DST_GAME_MODE_LABEL 				initRect(50,65,101,16)
#define SRC_PVP_BUTTON(s) 					initRect(920,s*SELECTED_OFFSET,176,46)
#define DST_PVP_BUTTON 						initRect(12,91,176,46)
#define SRC_PVA_BUTTON(s) 					initRect(1120,s*SELECTED_OFFSET,176,46)
#define DST_PVA_BUTTON 						initRect(12,146,176,46)
#define SRC_NEXT_PLAYER_LABEL 				initRect(460,500,112,16)
#define DST_NEXT_PLAYER_LABEL 				initRect(44,238,112,16)
#define SRC_WHITE_BUTTON(s) 				SRC_AI_WHITE_BUTTON(s)
#define DST_WHITE_BUTTON 					initRect(12,265,81,46)
#define SRC_BLACK_BUTTON(s)					SRC_AI_BLACK_BUTTON(s)
#define DST_BLACK_BUTTON 					initRect(107,265,81,46)
#define SRC_SELECTION_UPDATE_BOARD_LABEL	initRect(320,500,129,16)
#define DST_SELECTION_UPDATE_BOARD_LABEL	initRect(36,356,129,16)
#define SRC_SET_BUTTON 						initRect(1320,0,81,46)
#define DST_SET_BUTTON 						initRect(12,382,81,46)
#define SRC_RM_BUTTON 						initRect(1420,0,81,46)
#define DST_RM_BUTTON 						initRect(107,382,81,46)
#define SRC_MAIN_MENU_BUTTON 				initRect(1520,0,176,46)
#define DST_MAIN_MENU_BUTTON 				initRect(12,480,176,46)
#define SRC_START_BUTTON 					initRect(1720,0,176,46)
#define DST_START_BUTTON 					initRect(12,540,176,46)

/*Piece Selection Menu*/
#define INIT_PIECE_SELECTION_PANEL			INIT_MENU_PANEL
#define INIT_PIECES_SELECTION_WHITES(i)		buttons, SRC_PIECES_SELECTION_WHITES(i), DST_PIECES_SELECTION_WHITES(i)
#define INIT_PIECES_SELECTION_BLACKS(i)		buttons, SRC_PIECES_SELECTION_BLACKS(i), DST_PIECES_SELECTION_BLACKS(i)
#define INIT_PIECE_SELECTION_LABEL			buttons, SRC_PIECE_SELECTION_LABEL, DST_PIECE_SELECTION_LABEL
#define INIT_PIECE_SELECTION_CANCEL_BUTTON	buttons, SRC_SELECTION_CANCEL_BUTTON, DST_SELECTION_CANCEL_BUTTON

#define SRC_PIECE_SELECTION_LABEL			initRect(600,500,350,42)
#define DST_PIECE_SELECTION_LABEL			initRect(78,61,350,42)
#define SRC_PIECES_SELECTION_WHITES(i)		initRect(0+i*70,540,74,70)
#define DST_PIECES_SELECTION_WHITES(i)		initRect(38+i*70,121,74,70)
#define SRC_PIECES_SELECTION_BLACKS(i)		initRect(0+i*70,540+TILE_HIGHT,74,70)
#define DST_PIECES_SELECTION_BLACKS(i)		initRect(38+i*70,121+TILE_HIGHT,74,70)
#define SRC_SELECTION_CANCEL_BUTTON			SRC_CANCEL_BUTTON
#define DST_SELECTION_CANCEL_BUTTON			initRect(221,281,64,34)

/*AI Menu*/
#define INIT_AI_PANEL					INIT_MENU_PANEL
#define INIT_AI_PLAYER_COLOR_LABEL		buttons, SRC_AI_PLAYER_COLOR, DST_AI_PLAYER_COLOR
#define INIT_AI_DIFFICULTY_LABEL		buttons, SRC_AI_DIFFICULTY_LABEL, DST_AI_DIFFICULTY_LABEL
#define INIT_AI_DIF_BUTTON(i) 			buttons, SRC_DIF_BUTTON(i,0), DST_DIF_BUTTON(i)
#define INIT_AI_BEST_BUTTON				buttons, SRC_AI_BEST_BUTTON(0), DST_AI_BEST_BUTTON
#define INIT_AI_WHITE_BUTTON			buttons, SRC_AI_WHITE_BUTTON(0), DST_AI_WHITE_BUTTON
#define INIT_AI_BLACK_BUTTON			buttons, SRC_AI_BLACK_BUTTON(0), DST_AI_BLACK_BUTTON
#define INIT_AI_START_BUTTON			buttons, SRC_AI_START_BUTTON, DST_AI_START_BUTTON
#define INIT_AI_CANCEL_BUTTON			buttons, SRC_AI_CANCEL_BUTTON, DST_AI_CANCEL_BUTTON

#define SRC_AI_PLAYER_COLOR 			initRect(1150,400,185,24)
#define DST_AI_PLAYER_COLOR 			initRect(160,56,185,24)
#define SRC_AI_WHITE_BUTTON(s) 			initRect(640,s*SELECTED_OFFSET,81,46)
#define DST_AI_WHITE_BUTTON 			initRect(113,97,81,46)
#define SRC_AI_BLACK_BUTTON(s) 			initRect(730,s*SELECTED_OFFSET,81,46)
#define DST_AI_BLACK_BUTTON 			initRect(312,97,81,46)
#define SRC_AI_DIFFICULTY_LABEL 		initRect(1340,400,144,24)
#define DST_AI_DIFFICULTY_LABEL 		initRect(181,168,144,24)
#define SRC_DIF_BUTTON(i,s) 			(i==BEST-1 ? SRC_AI_BEST_BUTTON(s) : SRC_SLOTS_BUTTON(i,s))
#define DST_DIF_BUTTON(i)	 			initRect(80+i*DIF_OFFSET,202,46,46)
#define SRC_AI_BEST_BUTTON(s)			initRect(820,0+s*SELECTED_OFFSET,81,46)
#define DST_AI_BEST_BUTTON			 	initRect(80+4*DIF_OFFSET,202,81,46)
#define SRC_AI_START_BUTTON			 	initRect(1460,200,64,34)
#define DST_AI_START_BUTTON 			DST_LOAD_MENU_BUTTON
#define SRC_AI_CANCEL_BUTTON			SRC_CANCEL_BUTTON
#define DST_AI_CANCEL_BUTTON 			DST_LOAD_CANCEL_BUTTON

/*Game Window*/
#define INIT_GAME_SIDE_PANEL			INIT_SELECTION_PANEL
#define INIT_GAME_TITLE_LABEL			buttons, SRC_GAME_TITLE_LABEL, DST_GAME_TITLE_LABEL
#define INIT_GAME_GET_MOVES_LABEL		buttons, SRC_GAME_GET_MOVES_LABEL, DST_GAME_GET_MOVES_LABEL
#define INIT_CHECK_LABEL				buttons, SRC_EMPTY, DST_CHECK
#define INIT_GAME_SAVE_BUTTON			buttons, SRC_GAME_SAVE_BUTTON, DST_GAME_SAVE_BUTTON
#define INIT_GAME_HINT_BUTTON			buttons, SRC_GAME_HINT_BUTTON, DST_GAME_HINT_BUTTON
#define INIT_GAME_ON_BUTTON				buttons, SRC_GAME_ON_BUTTON(0), DST_GAME_ON_BUTTON
#define INIT_GAME_OFF_BUTTON			buttons, SRC_GAME_OFF_BUTTON(0), DST_GAME_OFF_BUTTON
#define INIT_GAME_MAIN_MENU_BUTTON		INIT_MAIN_MENU_BUTTON
#define INIT_GAME_END_LABEL				buttons, SRC_EMPTY, DST_MATE
#define INIT_GO							buttons, SRC_EMPTY, DST_GO

#define SRC_GAME_TITLE_LABEL			initRect(1140,500,176,31)
#define DST_GAME_TITLE_LABEL			initRect(12,12,176,31)
#define SRC_GAME_SAVE_BUTTON 			initRect(0,200,176,46)
#define DST_GAME_SAVE_BUTTON 			initRect(12,80,176,46)
#define SRC_GAME_GET_MOVES_LABEL 		initRect(980,500,110,16)
#define DST_GAME_GET_MOVES_LABEL		initRect(45,288,110,16)
#define SRC_GAME_HINT_BUTTON			initRect(400,200,81,46)
#define DST_GAME_HINT_BUTTON			initRect(60,314,81,46)
#define SRC_GAME_ON_BUTTON(s) 			initRect(200,200+s*SELECTED_OFFSET,81,46)
#define DST_GAME_ON_BUTTON 				DST_SET_BUTTON
#define SRC_GAME_OFF_BUTTON(s) 			initRect(300,200+s*SELECTED_OFFSET,81,46)
#define DST_GAME_OFF_BUTTON 			DST_RM_BUTTON
#define SRC_GO_WHITE					initRect(560,200,155,43)
#define SRC_GO_BLACK					initRect(560,280,155,43)
#define DST_GO							initRect(22,545,155,43)

#define SRC_SELECTED_FRAME              initRect(80,160,74,74);
#define SRC_POSSIBLE_FRAME              initRect(0,160,74,74);
#define SRC_EMPTY 						initRect(0,0,0,0)
#define DST_CHECK 						initRect(20,145,160,50)
#define SRC_CHECK 						initRect(1360,500,160,50)
#define SRC_TIE 						initRect(800,800,400,301)
#define SRC_MATE_BLACK					initRect(0,800,400,300)
#define SRC_MATE_WHITE 					initRect(400,800,400,300)
#define DST_MATE 						initRect(53,50,400,300) /*relative to central*/

/*difficulty menu*/
#define INIT_DIFFICULTY_LABEL			buttons, SRC_AI_DIFFICULTY_LABEL, DST_DIFFICULTY_LABEL
#define DST_DIFFICULTY_LABEL			initRect(181,148,144,24)
#define INIT_DIF_MENU_BUTTON(i)            buttons, SRC_DIF_MENU_BUTTON(i), DST_DIF_MENU_BUTTON(i)
#define SRC_DIF_MENU_BUTTON(i)            (i==BEST-1 ? SRC_AI_BEST_BUTTON(0) : SRC_SLOTS_BUTTON(i,0))
#define DST_DIF_MENU_BUTTON(i)            initRect(3+(i+1)*70,210,74,70)

/*promotions menu*/
#define INIT_PROMOTIONS_BUTTON(i)        buttons, SRC_PROMOTIONS_BUTTON(i), DST_PROMOTIONS_BUTTON(i)
#define SRC_PROMOTIONS_BUTTON(i)        ((nextPlayer==WHITE) ? initRect((i+1)*70,540,74,70) : initRect((i+1)*70,540+74,74,70))
#define DST_PROMOTIONS_BUTTON(i)        initRect(15+(i+1)*80,150,74,70)



#endif /* GUI_H_ */
