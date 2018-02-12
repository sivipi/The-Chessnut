/**********************************************************/
/*                     THE CHESSNUT                       */
/* Authors: Sivan Schick, sivanschick@mail.tau.ac.il      */
/*			Zohar Meir,   zoharmeir1@mail.tau.ac.il       */
/*														  */
/* file:	 gui.c                                        */
/* contents: gui mode "main" function and frameworks	  */
/* 			 requires designated ./Graphics folder        */
/*			 game resolution: 800x600                     */
/**********************************************************/
#include "gui.h"

SDL_Surface *screen = NULL;
SDL_Surface *central = NULL;
SDL_Surface *back = NULL;
SDL_Surface *pieces = NULL;
SDL_Surface *pieces_frame = NULL;
SDL_Surface *buttons = NULL;
control_t *mainMenu, *loadMenu, *saveMenu, *selectionMenu, *AIMenu, *gameWindow, *pieceSelection, *difficultyMenu, *promotionsMenu;

/* main control function
 * return 0 on succesful execution, 1 for error*/
int guiMode() {
	int window=MAIN_MENU;
	startGame = 1;

	/*init all surfaces and apply initial backgroung image*/
	if(initSurfaces() == GUI_ERROR) {
		print_SDL_error("failed to load surfaces");
		quitGUI();
		return 1;
	}

	 /*init/build all menus*/
	 mainMenu = constructMainMenu();
	 loadMenu = constructSaveLoadMenu(LOAD);
	 saveMenu = constructSaveLoadMenu(SAVE);
	 selectionMenu =  constructSelectionMenu();
	 AIMenu = constructAIMenu();
	 gameWindow = constructGameWindow();
	 pieceSelection = constructPieceSelectionMenu();
	 difficultyMenu = constructDifficultyMenu();
	 promotionsMenu = constructPromotionsMenu();
	if (mainMenu==NULL || loadMenu==NULL || saveMenu==NULL || \
			selectionMenu==NULL || AIMenu==NULL || gameWindow==NULL || pieceSelection==NULL || \
				difficultyMenu==NULL || promotionsMenu==NULL) {
		quitGUI();
		return 1;
	}

	 /*window switch*/
	 while (window!=GUI_ERROR && window!=QUIT) {
		 switch(window) {
		 case MAIN_MENU:
			 /* open main menu */
			 window = runMainMenu(mainMenu);
			 break;
		 case LOAD_MENU:
			 /* if load - open loading menu */
			 window = runSaveLoadMenu(loadMenu, LOAD);
			 break;
		 case SAVE_MENU:
			 /* if save - open saving menu */
			 window = runSaveLoadMenu(saveMenu, SAVE);
			 break;
		 case SELECTION_MENU:
			 /* player selection window: take default values (or loaded ones) from globals in chessprog.c */
			 window = runSelectionMenu(selectionMenu);
			 break;
		 case PIECE_SELECTION:
			 /* select pieces to set on board */
			 window = runPieceSelectionMenu(pieceSelection);
			 break;
		 case AI_MENU:
			 /* if PVA - AI setting window: take default values (or loaded ones) from globals in chessprog.c */
			 window = runAIMenu(AIMenu);
			 break;
		 case GAME:
			 /* game window */
			 window = runGame(gameWindow);
			 break;
		 case QUIT:
		 case GUI_ERROR:
			 break;
		 }
	 }

	 quitGUI();
	 return 0;
}

/************* factories ****************/

control_t* initControl(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect) {
	control_t* control = (control_t*)malloc(sizeof(control_t));
	if (control!=NULL) {
		control->parent = parent;
		control->child = control->next = NULL;
		control->source = source;
		control->srcRect = srcRect;
		control->dstRect = dstRect;
	} else
		print_malloc_error;
	return control;
}

control_t* initWindow(SDL_Surface* source) {
	control_t* control=initControl(NULL, source, initRect(0,0,SCREEN_WIDTH,SCREEN_HIGHT), initRect(0,0,SCREEN_WIDTH,SCREEN_HIGHT));
	if (control!=NULL)
		control->type = WINDOW;
	return control;
}

control_t* initPanel(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect) {
	control_t* control=initControl(parent, source, srcRect, dstRect);
	if (control!=NULL)
		control->type = PANEL;
	return control;
}

control_t* initLabel(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect) {
	control_t* control=initControl(parent, source, srcRect, dstRect);
	if (control!=NULL)
		control->type = LABEL;
	return control;
}

control_t* initButton(control_t* parent, SDL_Surface* source, SDL_Rect srcRect, SDL_Rect dstRect) {
	control_t* control=initControl(parent, source, srcRect, dstRect);
	if (control!=NULL)
		control->type = BUTTON;
	return control;
}

/********** menu construction ***********/

control_t* constructMainMenu() {
	control_t *window, *menu, *label, *newButton, *loadButton, *quitButton;

	window = initWindow(screen);
	menu = initPanel(window, INIT_MENU_PANEL);
	label = initLabel(menu, INIT_MAIN_MENU_LABEL);
	newButton = initButton(menu, INIT_NEW_BUTTON);
	loadButton = initButton(menu, INIT_LOAD_GAME_BUTTON);
	quitButton = initButton(menu, INIT_MAIN_MENU_QUIT);

	if (window==NULL || menu==NULL || label==NULL || \
			newButton==NULL || loadButton==NULL || quitButton==NULL) {
		/* still not properloy connected controls, and so we free normally */
		free(window);
		free(menu);
		free(label);
		free(newButton);
		free(loadButton);
		free(quitButton);
		return NULL;
	}

	/*Heritage*/
	window->child = menu;
	menu->child = label;
	label->next = newButton;
	newButton->next = loadButton;
	loadButton->next = quitButton;

	return window;
}

control_t* constructSaveLoadMenu(char c) {
	control_t *window, *menu, *label, *cancelButton, *actionButton; /*action button: save/load*/
	control_t* slots[7];
	char isNull = 0;

	window = initWindow(screen);
	menu = initPanel(window, INIT_MENU_PANEL);
	label = c==LOAD ? initLabel(menu, INIT_LOAD_MENU_LABEL) : initLabel(menu, INIT_SAVE_MENU_LABEL);
	cancelButton = initButton(menu, INIT_LOAD_CANCEL_BUTTON);
	actionButton = c==LOAD ? initButton(menu, INIT_LOAD_BUTTON) : initButton(menu, INIT_SAVE_BUTTON);
	for (int i=0 ; i<7 ; i++)
		isNull = (slots[i]=initButton(menu, INIT_SLOTS_BUTTON(i)))==NULL? 1:isNull;

	if (window==NULL || menu==NULL || label==NULL || \
			cancelButton==NULL || actionButton==NULL || isNull) {
		/* still not properloy connected controls, and so we free normally */
		free(window);
		free(menu);
		free(label);
		free(cancelButton);
		free(actionButton);
		for (int i=0 ; i<7 ; i++)
			free(slots[i]);
		return NULL;
	}

	/*Heritage*/
	window->child = menu;
	menu->child = label;
	label->next = slots[0];
	for (int i=0 ; i<6 ; i++)
		slots[i]->next = slots[i+1];
	slots[6]->next = actionButton;
	actionButton->next = cancelButton;

	return window;
}

control_t* constructPieceSelectionMenu(){
	control_t *window, *panel_pieceSelection;
	control_t *label;
	control_t *button_cancel, *button_pieces[12];
	int i;
	char isNull=0;

	/*main window*/
	window = initWindow(screen);

	/*panels*/
	panel_pieceSelection = initPanel(window, INIT_PIECE_SELECTION_PANEL);

	/*lables*/
	label = initLabel(panel_pieceSelection, INIT_PIECE_SELECTION_LABEL);

	/*buttons*/
	button_cancel = initButton(panel_pieceSelection, INIT_PIECE_SELECTION_CANCEL_BUTTON);

	for (i=0; i<6; i++){
		isNull = (button_pieces[i]=initButton(panel_pieceSelection, INIT_PIECES_SELECTION_WHITES(i)))==NULL?1:isNull;;
		isNull = (button_pieces[i+6] = initButton(panel_pieceSelection, INIT_PIECES_SELECTION_BLACKS(i)))==NULL?1:isNull;
	}

	if (window==NULL || panel_pieceSelection==NULL || label==NULL || \
			button_cancel==NULL || isNull) {
		/* still not properly connected controls, and so we free normally */
		free(window);
		free(panel_pieceSelection);
		free(label);
		free(button_cancel);
		for (i=0 ; i<12 ; i++)
			free(button_pieces[i]);
		return NULL;
	}

	/*Heritage*/
	window->child = panel_pieceSelection;
	panel_pieceSelection->child = label;
	label->next = button_pieces[0];
	for (i=0 ; i<12 ; i++)
		button_pieces[i]->next = button_pieces[i+1];
	button_pieces[11]->next = button_cancel;

	return window;
}

control_t* constructSelectionMenu(){
	control_t *window, *selection;
	control_t *label_gameMode, *label_nextPlayer, *label_gameSettings, *label_updateBoard;
	control_t *button_pvp, *button_pva, *button_white, *button_black, *button_set, *button_rm;
	control_t *button_mainMenu, *button_start;

	/*main window*/
	window = initWindow(screen);

	/*selection panel*/
	selection = initPanel(window, INIT_SELECTION_PANEL);

	/*lables*/
	label_gameSettings = initLabel(selection, INIT_SELECTION_GAME_SETTINGS_LABEL);
	label_gameMode = initLabel(selection, INIT_SELECTION_GAME_MODE_LABEL);
	label_nextPlayer = initLabel(selection, INIT_SELECTION_NEXT_PLAYER_LABEL);
	label_updateBoard = initLabel(selection, INIT_SELECTION_UPDATE_BOARD_LABEL);

	/*buttons*/
	button_pvp = initLabel(selection, INIT_PVP_BUTTON);
	button_pva = initLabel(selection, INIT_PVA_BUTTON);
	button_white = initLabel(selection, INIT_WHITE_BUTTON);
	button_black = initLabel(selection, INIT_BLACK_BUTTON);
	button_set = initLabel(selection, INIT_SET_BUTTON);
	button_rm = initLabel(selection, INIT_RM_BUTTON);
	button_mainMenu = initLabel(selection, INIT_MAIN_MENU_BUTTON);
	button_start = initLabel(selection, INIT_START_BUTTON);

	if (window==NULL || selection==NULL || label_gameMode==NULL || label_nextPlayer==NULL ||	\
			button_pvp==NULL || button_pva==NULL || button_white==NULL || button_black==NULL || \
			button_set==NULL || button_rm==NULL || button_mainMenu==NULL || button_start==NULL  \
			|| label_gameSettings==NULL || label_updateBoard==NULL) {
		/* still not properly connected controls, and so we free normally */
		free(window);
		free(selection);
		free(label_gameMode);
		free(label_nextPlayer);
		free(label_gameSettings);
		free(label_updateBoard);
		free(button_pvp);
		free(button_pva);
		free(button_white);
		free(button_black);
		free(button_set);
		free(button_rm);
		free(button_mainMenu);
		free(button_start);
		return NULL;
	}

	/*Heritage*/
	window->child = selection;
	selection->child = label_gameSettings;
	label_gameSettings->next = label_gameMode;
	label_gameMode->next = button_pvp;
	button_pvp->next = button_pva;
	button_pva->next = label_nextPlayer;
	label_nextPlayer->next = button_white;
	button_white->next = button_black;
	button_black->next = label_updateBoard;
	label_updateBoard->next = button_set;
	button_set->next = button_rm;
	button_rm->next = button_mainMenu;
	button_mainMenu->next = button_start;

	return window;
}

control_t* constructGameWindow(){
	control_t *window, *panel_side, *panel_central;
	control_t *label_title, *label_getMoves, *label_check, *label_end, *label_go;
	control_t *button_save, *button_on, *button_off, *button_mainMenu, *button_hint;

	/*main window*/
	window = initWindow(screen);

	/*selection panel*/
	panel_side = initPanel(window, INIT_GAME_SIDE_PANEL);

	/*lables*/
	label_title = initLabel(panel_side, INIT_GAME_TITLE_LABEL);
	label_getMoves = initLabel(panel_side, INIT_GAME_GET_MOVES_LABEL);
	label_check = initLabel(panel_side, INIT_CHECK_LABEL);
	label_go = initLabel(panel_side, INIT_GO);

	/*buttons*/
	button_save = initLabel(panel_side, INIT_GAME_SAVE_BUTTON);
	button_hint = initButton(panel_side, INIT_GAME_HINT_BUTTON);
	button_on = initLabel(panel_side, INIT_GAME_ON_BUTTON);
	button_off = initLabel(panel_side, INIT_GAME_OFF_BUTTON);
	button_mainMenu = initLabel(panel_side, INIT_GAME_MAIN_MENU_BUTTON);

	/*end of game panel and label*/
	panel_central = initPanel(window, INIT_EMPTY_MENU_PANEL);
	label_end = initLabel(panel_central, INIT_GAME_END_LABEL);

	if (window==NULL || panel_side==NULL || label_getMoves==NULL || label_title==NULL || label_check==NULL || \
			button_save==NULL || button_hint==NULL || button_on==NULL || button_off==NULL || button_mainMenu==NULL || \
				panel_central==NULL || label_end==NULL || label_go==NULL) {
		/* still not properly connected controls, and so we free normally */
		free(window);
		free(panel_side);
		free(label_getMoves);
		free(label_title);
		free(label_check);
		free(button_save);
		free(button_hint);
		free(button_on);
		free(button_off);
		free(button_mainMenu);
		free(panel_central);
		free(label_end);
		free(label_go);
		return NULL;
	}

	/*Heritage*/
	window->child = panel_side;
		panel_side->child = label_title;
		label_title->next = button_save;
		button_save->next = label_check;
		label_check->next =	label_getMoves;
		label_getMoves->next = button_hint;
		button_hint->next = button_on;
		button_on->next = button_off;
		button_off->next = button_mainMenu;
		button_mainMenu->next = label_go;
	panel_side->next = panel_central;
		panel_central->child = label_end;


	return window;
}

control_t* constructAIMenu(){

	control_t *window, *panel_AI;
	control_t *label_playerColor, *label_Difficulty;
	control_t *button_white, *button_black, *button_dif[4], *button_best, *button_cancel, *button_start;
	int i, isNull = 0;

	/*main window*/
	window = initWindow(screen);

	/*panels*/
	panel_AI = initPanel(window, INIT_AI_PANEL);

	/*lables*/
	label_playerColor = initLabel(panel_AI, INIT_AI_PLAYER_COLOR_LABEL);
	label_Difficulty = initLabel(panel_AI, INIT_AI_DIFFICULTY_LABEL);

	/*buttons*/
	button_white = initButton(panel_AI, INIT_AI_WHITE_BUTTON);
	button_black = initButton(panel_AI, INIT_AI_BLACK_BUTTON);
	button_start = initButton(panel_AI, INIT_AI_START_BUTTON);
	button_cancel = initButton(panel_AI, INIT_AI_CANCEL_BUTTON);
	button_best = initButton(panel_AI, INIT_AI_BEST_BUTTON);

	for (i=0 ; i<4 ; i++)
		isNull = (button_dif[i]=initButton(panel_AI, INIT_AI_DIF_BUTTON(i)))==NULL? 1:isNull;

	if (window==NULL || panel_AI==NULL || label_playerColor==NULL || label_Difficulty==NULL || \
			button_best==NULL||	button_white==NULL || button_black==NULL || button_start==NULL \
			|| button_cancel==NULL || isNull) {
		/* still not properly connected controls, and so we free normally */
		free(window);
		free(panel_AI);
		free(label_playerColor);
		free(label_Difficulty);
		free(button_white);
		free(button_black);
		free(button_start);
		free(button_cancel);
		free(button_best);

		for (int i=0 ; i<4 ; i++)
			free(button_dif[i]);

		return NULL;
	}

	/*Heritage*/
	window->child = panel_AI;
	panel_AI->child = label_playerColor;
	label_playerColor->next = button_white;
	button_white->next = button_black;
	button_black->next = label_Difficulty;
	label_Difficulty->next = button_dif[0];

	for (i=0 ; i<3 ; i++)
		button_dif[i]->next = button_dif[i+1];

	button_dif[3]->next = button_best;
	button_best->next = button_start;
	button_start->next = button_cancel;

	return window;
}

control_t* constructPromotionsMenu(){
    control_t *window, *panel_menu, *label_promotions;
    control_t *pieces[4];
    int isNull=0;

    /*init controls*/
    window = initWindow(screen);
    panel_menu = initPanel(window, INIT_MENU_PANEL);
    label_promotions = initLabel(panel_menu, INIT_PIECE_SELECTION_LABEL);

    for (int i=0; i<4; i++){
        isNull = (pieces[i]=initButton(panel_menu, INIT_PROMOTIONS_BUTTON(i)))==NULL? 1:isNull;
    }

    if (window==NULL || panel_menu==NULL || label_promotions==NULL || isNull) {
        /* still not properloy connected controls, and so we free normally */
        free(window);
        free(panel_menu);
        free(label_promotions);
        for (int i=0; i<4; i++){
            free(pieces[i]);
        }
        return NULL;
    }

    /*Heritage*/
    window->child = panel_menu;
    panel_menu->child = label_promotions;
    label_promotions->next = pieces[0];
    for (int i=0; i<3; i++)
        pieces[i]->next =pieces[i+1];

    return window;
}

control_t* constructDifficultyMenu(){
    control_t *window, *panel_menu, *label_dif;
    control_t *difs[5];
    int isNull=0;

    /*init controls*/
    window = initWindow(screen);
    panel_menu = initPanel(window, INIT_MENU_PANEL);
    label_dif = initLabel(panel_menu, INIT_DIFFICULTY_LABEL);

    for (int i=0; i<5; i++){
        isNull = (difs[i]=initButton(panel_menu, INIT_DIF_MENU_BUTTON(i)))==NULL? 1:isNull;
    }

    if (window==NULL || panel_menu==NULL || label_dif==NULL || isNull) {
        /* still not properloy connected controls, and so we free normally */
        free(window);
        free(panel_menu);
        free(label_dif);
        for (int i=0; i<4; i++){
            free(difs[i]);
        }
        return NULL;
    }

    /*Heritage*/
    window->child = panel_menu;
    panel_menu->child = label_dif;
    label_dif->next = difs[0];
    for (int i=0; i<4; i++)
        difs[i]->next =difs[i+1];

    return window;
}

/*********** menu functions ************/

int runMainMenu(control_t* root) {
	control_t* newButton = root->child->child->next;
	control_t* loadButton = newButton->next;
	control_t* quitButton = loadButton->next;
	SDL_Event e;
	SDL_Rect boardClip = initRect(0,0,600,600);

	if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display() == GUI_ERROR)
		return GUI_ERROR;
	while(1) {
		waitEvent(e);
		if (e.type == SDL_QUIT)
			return QUIT;
		/* handle mouse clicks */
		if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
			if (inControl(newButton, &e)) {
				resetGlobals();
				init_board(gameBoard);
				SDL_Rect offset = {0,0,0,0};
				showBoard();
				return SELECTION_MENU;
			}
			if (inControl(loadButton, &e))
				return LOAD_MENU;
			if (inControl(quitButton, &e))
				return QUIT;
		}
	}

	return QUIT;
}

int runSaveLoadMenu(control_t* root, char c) {
	control_t *cancelButton, *actionButton;
	control_t* slots[7];
	SDL_Event e;
	SDL_Rect r = {0,0,0,0}; /*used for initial blitControl function call*/
	int selected = -1; /*init to index out of range*/
	char path[MAX_INPUT];

	slots[0]=root->child->child->next;
	for (int i=0 ; i<6 ; i++) {
		slots[i]->srcRect = SRC_SLOTS_BUTTON(i,0); /*unselect the button*/
		slots[i+1] = slots[i]->next;
	}
	slots[6]->srcRect = SRC_SLOTS_BUTTON(6,0); /*unselect the button*/
	actionButton = slots[6]->next;
	cancelButton = actionButton->next;

	while(1) {
		if (blitControl(root, r)==GUI_ERROR || display()==GUI_ERROR)
			return GUI_ERROR;
		waitEvent(e);
		if (e.type == SDL_QUIT)
			return QUIT;
		/* handle mouse clicks */
		if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
			for (int i=0 ; i<7 ; i++) { /*check all slots*/
				if (i!=selected && inControl(slots[i], &e)) {
					slots[i]->srcRect = SRC_SLOTS_BUTTON(i,1); /*select the button*/
					if (selected != -1) /*not first selection*/
						slots[selected]->srcRect = SRC_SLOTS_BUTTON(selected,0); /*unselect the button*/
					selected = i;
					break;
				}
			}
			if (inControl(cancelButton, &e))
				return c==LOAD? MAIN_MENU:GAME;
			if (inControl(actionButton, &e)) {
				sprintf(path, PATH_FORMAT, selected+1); /*files are 1...7 and indexes are 0...6*/
				if (selected == -1) {
					print_message(SLOTS_ERROR); /*also print to console*/
				} else if (c==LOAD && loadGame(path)==0) {  /*loaded successfully*/
					return SELECTION_MENU;
				} else if (c==SAVE && saveGame(path)==0) { /*save successfully*/
					return GAME;
				} else { /*must be error opening file*/
					print_message(WRONG_FILE_NAME); /*print to console*/
					displayError(SRC_ERROR_FILE, DST_ERROR_FILE); /*blit, display, delay*/
				}
			}
		}
	}

	return QUIT;
}

int runSelectionMenu(control_t* root) {

	control_t* button_pvp = root->child->child->next->next;
	control_t* button_pva = button_pvp->next;
	control_t* button_white = button_pva->next->next;
	control_t* button_black = button_white->next;
	control_t* button_set = button_black->next->next;
	control_t* button_rm = button_set->next;
	control_t* button_mainMenu = button_rm->next;
	control_t* button_start = button_mainMenu->next;
	SDL_Event e;
	pos_t pos;
	int newEvent=1, rm_flag=0;

	/* Sync globals
	 * Checking non-defaults in case no globals initialized*/

	if(gameMode==PVA){
		button_pvp->srcRect=SRC_PVP_BUTTON(0);
		button_pva->srcRect=SRC_PVA_BUTTON(1);
	} else {
		button_pvp->srcRect=SRC_PVP_BUTTON(1);
		button_pva->srcRect=SRC_PVA_BUTTON(0);
	}

	if(nextPlayer==BLACK){
		button_white->srcRect=SRC_WHITE_BUTTON(0);
		button_black->srcRect=SRC_BLACK_BUTTON(1);
	} else {
		button_white->srcRect=SRC_WHITE_BUTTON(1);
		button_black->srcRect=SRC_BLACK_BUTTON(0);
	}

	button_set->srcRect = SRC_SET_BUTTON;
	button_rm->srcRect = SRC_RM_BUTTON;

	while(1) {
		if (newEvent){
			newEvent=0;
			if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || initBoard_GUI()==GUI_ERROR \
					|| display()==GUI_ERROR)
				return GUI_ERROR;
		}
		waitEvent(e);
		if (e.type == SDL_QUIT)
			return QUIT;
		/* handle mouse clicks */
		if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
			if (inControl(button_pvp, &e) && (gameMode==PVA)){
				gameMode = PVP;
				clickButton(button_pvp);
				clickButton(button_pva);
				newEvent=1;
			} else if (inControl(button_pva, &e) && (gameMode==PVP)) {
				gameMode = PVA;
				clickButton(button_pvp);
				clickButton(button_pva);
				newEvent=1;
			} else if (inControl(button_white, &e) && (nextPlayer==BLACK)) {
				nextPlayer = WHITE;
				clickButton(button_white);
				clickButton(button_black);
				newEvent=1;
			} else if (inControl(button_black, &e) && (nextPlayer==WHITE)) {
				nextPlayer = BLACK;
				clickButton(button_white);
				clickButton(button_black);
				newEvent=1;
			} else if (inControl(button_set, &e)) {
				clickButton(button_set);
				if (rm_flag)
					clickButton(button_rm);
				if ((blitControl(root, initRect(0,0,0,0))==GUI_ERROR)||((display()==GUI_ERROR)))
					return GUI_ERROR;
				return PIECE_SELECTION;
			} else if (inControl(button_rm, &e)) {
				clickButton(button_rm);
				rm_flag = 1-rm_flag;
				newEvent=1;
			} else if (inControl(button_mainMenu, &e)) {
				return MAIN_MENU;
			} else if (inControl(button_start, &e)) {
				if(start(gameBoard))
					return gameMode==PVA? AI_MENU:GAME;
				else {
					displayError(SRC_ERROR_BOARD_INIT, DST_ERROR_BOARD_INIT);
					newEvent = 1;
				}
			} else if (rm_flag) { /*only if not in any of the other buttons*/
				pos = click2pos(e);
				if (inBoard(pos.col,pos.row)) {/* click inside the board */
					rm(gameBoard, pos);
				} else { /* click outside the board */
					clickButton(button_rm);
					rm_flag = 0; /*will exit 'remove piece' mode*/
				}
				newEvent = 1;
			}
		}
	}
	return QUIT;
}

int runPieceSelectionMenu(control_t* root) {

	control_t* button_cancel = root->child->child->next->next->next->next->next->next->next->next->next->next->next->next->next;
	SDL_Rect offset = initRect(0,0,0,0), boardClip = initRect(0,0,600,600);
	SDL_Event e;
	pos_t pos;
	char piece;

	while(1) {
		if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display()==GUI_ERROR)
			return GUI_ERROR;
		waitEvent(e);
		if (e.type == SDL_QUIT)
			return QUIT;
		/* handle mouse clicks */
		if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
			if (inControl(button_cancel, &e)){
				showBoard();
				return SELECTION_MENU;
			}
			piece = pieceFromSelectionPanel(root,&e);
			if (piece == PIECE_OVERFLOW){
				displayError(SRC_ERROR_PIECE_OVERFLOW, DST_ERROR_PIECE_OVERFLOW);
				if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display()==GUI_ERROR)
					return GUI_ERROR;
			} else if (piece != EMPTY) {
				/*to make the board visible*/
				showBoard();
				while (1) {
					waitEvent(e);
					if (e.type == SDL_QUIT)
						return QUIT;
					if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
						pos = click2pos(e);
						/* click is not out of the board */
						if (inBoard(pos.col,pos.row)) {
							 /*handler for implicit promotion*/
							if ((piece==W_PAWN && pos.row==BOARD_SIZE-1) || (piece==B_PAWN && pos.row==0)) {
								print_message(WRONG_POSITION);
								displayError(SRC_ERROR_IMPLICIT_PROMOTION,DST_ERROR_IMPLICIT_PROMOTION);
								return PIECE_SELECTION;
							} else
								setPiece(gameBoard, pos, piece);
							if (initBoard_GUI() < 0 || display() < 0)
								return GUI_ERROR;
							return SELECTION_MENU;
						} else { /*click was not in board after a piece was selected*/
							SDL_PushEvent(&e); /*push back event to queue*/
							return SELECTION_MENU; /*transfer control to selection menu*/
						}
					}
				}
			}
		}
	}
	return SELECTION_MENU;
}

int runAIMenu(control_t* root){
	control_t* button_white = root->child->child->next;
	control_t* button_black = button_white->next;
	control_t* button_dif[5];
	button_dif[0] = button_black->next->next;
	for (int i=0 ; i<4 ; i++)
		button_dif[i+1] = button_dif[i]->next; /*button 5 is best*/
	control_t* button_start = button_dif[4]->next;
	control_t* button_cancel = button_start->next;

	SDL_Rect offset = initRect(0,0,0,0), boardClip = initRect(0,0,600,600);
	SDL_Event e;
	int newEvent=1, selected = minimaxDepth-1;

	/*Sync globals*/

	if(userColor==BLACK){
		button_white->srcRect=SRC_WHITE_BUTTON(0);
		button_black->srcRect=SRC_BLACK_BUTTON(1);
	} else {
		button_white->srcRect=SRC_WHITE_BUTTON(1);
		button_black->srcRect=SRC_BLACK_BUTTON(0);
	}
	for (int i=0 ; i<5 ; i++) { /*check all difficulty buttons*/
		if (i==selected)
			button_dif[i]->srcRect = SRC_DIF_BUTTON(i,1); /*select the button*/
		else
			button_dif[i]->srcRect = SRC_DIF_BUTTON(i,0); /*unselect the button*/
	}

	while(1) {
		if (newEvent){
			newEvent=0;
			if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display()==GUI_ERROR)
				return GUI_ERROR;
		}
		waitEvent(e);
		if (e.type == SDL_QUIT)
			return QUIT;
		/* handle mouse clicks */
		if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
			if (inControl(button_white, &e) && (userColor==BLACK)) {
				userColor = WHITE;
				clickButton(button_white);
				clickButton(button_black);
				newEvent=1;
			} else if (inControl(button_black, &e) && (userColor==WHITE)) {
				userColor = BLACK;
				clickButton(button_white);
				clickButton(button_black);
				newEvent=1;
			} else if (inControl(button_start, &e)){
				showBoard();
				return GAME;
			}else if (inControl(button_cancel, &e)){
				showBoard();
				return SELECTION_MENU;
			}
			/*check all difficulty buttons*/
			for (int i=0 ; i<5 ; i++) {
				if (i!=selected && inControl(button_dif[i], &e)) {
					button_dif[i]->srcRect = SRC_DIF_BUTTON(i,1); /*select the button*/
					button_dif[selected]->srcRect = SRC_DIF_BUTTON(selected,0); /*unselect the button*/
					selected = i;
					newEvent=1;
					minimaxDepth = i+1;
					break;
				}
			}
		}
	}
	return MAIN_MENU; /*Only to calm compiler*/
}

int runGame(control_t* root){
	control_t* label_go = root->child->child->next->next->next->next->next->next->next->next;
	int retVal = CONTINUE;
	int state = evalBoard(gameBoard, nextPlayer); /*check for early win*/
	if (analizeState_GUI(state, root) == GUI_ERROR)
		return GUI_ERROR;

	if (initBoard_GUI()==GUI_ERROR || blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display()==GUI_ERROR)
		/* blit control is second in case we start at mate */
		return GUI_ERROR;
	while (startGame) {
		label_go->srcRect = nextPlayer==WHITE? SRC_GO_WHITE:SRC_GO_BLACK;
		if (gameMode==PVA) {
			if (userColor == nextPlayer) { /*user's turn*/
				if ((retVal = playerTurn(root, 0, 1)) != CONTINUE)
					return retVal;
			} else {
				if (computerPlay_GUI(root, state) == GUI_ERROR) /*receive state of the board to determine if can play*/
					return GUI_ERROR; /*analize board state and determine proper prints and value for startGame*/
			}
		} else { /*gameMode==PVP*/
			userColor = nextPlayer; /*game functions use userColor as current playing user color*/
			if ((retVal = playerTurn(root, 0, 1)) != CONTINUE)
				return retVal;
		}
		nextPlayer = invColor(nextPlayer);
		state = evalBoard(gameBoard, nextPlayer);
		if (analizeState_GUI(state, root) == GUI_ERROR)
			return GUI_ERROR;
	}

	return playerTurn(root, 0, 1); /*only enter here with startGame==0 to disable playing*/
}

int analizeState_GUI(int boardState, control_t* root) {
	control_t *label_check = root->child->child->next->next;
	control_t *panel_central = root->child->next;
	control_t *label_end = panel_central->child;;

	switch (boardState) {
	case CONTINUE:
		label_check->srcRect = SRC_EMPTY;
		label_end->srcRect = SRC_EMPTY;
		panel_central->srcRect = SRC_EMPTY;
		break;
	case CHECK:
		if (DEBUG_GUI)
			printf("Check!\n");
		label_check->srcRect = SRC_CHECK;
		label_end->srcRect = SRC_EMPTY;
		panel_central->srcRect = SRC_EMPTY;
		break;
	case TIE:
		startGame = 0; /*game over, will exit game loop*/
		if (DEBUG_GUI)
			printf(TIE_MSG);
		label_check->srcRect = SRC_EMPTY;
		label_end->srcRect = SRC_TIE;
		panel_central->srcRect = SRC_MENU_PANEL;
		break;
	case WHITE:
	case BLACK:
		startGame = 0; /*game over, will exit game loop*/
		if (DEBUG_GUI)
			printf("Mate! %s player wins the game\n", boardState==WHITE? "White":"Black");
		SDL_Delay(2000); //TODO: 0110 consider adding delay here to see what move killed you...
		label_check->srcRect = SRC_EMPTY;
		label_end->srcRect = boardState==WHITE? SRC_MATE_WHITE:SRC_MATE_BLACK;
		panel_central->srcRect = SRC_MENU_PANEL;
		break;
	case -1:
		startGame = 0; /*game over, will exit game loop*/
		print_malloc_error;
		break;
	}
	return boardState;
}

int computerPlay_GUI(control_t* root, int state) {
	time_t end, start = time(NULL); /*timing for "stupid delay"*/
	int delay;
	move_t* move;

	if (state==TIE || state==WHITE || state==BLACK) { /*game end*/
		startGame = 0; /*will exit game loop*/
		return CONTINUE;
	}
	/* display GO label and CHECK message for benifit of human player */
	if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display()==GUI_ERROR)
		return GUI_ERROR;

	move = miniMax_env(gameBoard, minimaxDepth, computerColor, PLAYER_A); /*extract best move for computer*/
	if (move == NULL) { /*allocation error*/
		print_malloc_error;
		return GUI_ERROR;
	}

	/*"stupid delay" - make computer wait at least 1 second before playing*/
	end = time(NULL);
	delay = (int)(end-start)*1000;
	delay = delay>STUPID_DELAY? 0 : STUPID_DELAY-delay;
	SDL_Delay(delay);

	if (initBoard_GUI()==GUI_ERROR || playMove_GUI(move)==GUI_ERROR || display()==GUI_ERROR) {
		freeMove(move);
		return GUI_ERROR;
	}
	playMove(gameBoard, move);
	if (DEBUG_GUI) {
		printf("Computer: move ");
		printMove(move);
	}
	freeMove(move);
	return CONTINUE;
}

int playerTurn(control_t* root, int boardEvent, int panelEvent){
	control_t *panel = root->child;
	control_t *label_title = panel->child;
	control_t *button_save = label_title->next;
	control_t *label_check = button_save->next;
	control_t *label_getMoves = label_check->next;
	control_t *button_hint = label_getMoves->next;
	control_t *button_on = button_hint->next;
	control_t *button_off = button_on->next;
	control_t *button_mainMenu = button_off->next;

	SDL_Event e;
	pos_t from, to;
	move_t *move=NULL;
	movesList_t *moves=NULL;
	int depth=minimaxDepth, moveFlag=0, retVal;
	static int showMoves = 0;

	/*Sync globals*/
	button_on->srcRect=SRC_GAME_ON_BUTTON(showMoves);
	button_off->srcRect=SRC_GAME_OFF_BUTTON(!showMoves);

	while(1) {
		/* only blit desired part of screen for new events */
		if (boardEvent) {
			if (initBoard_GUI()==GUI_ERROR) {
				retVal = GUI_ERROR;
				break;
			}
			if (moveFlag) {	/*mark select from, show moves if needed */
				if (markSelected_GUI(from)==GUI_ERROR || \
						(showMoves && showMoves_GUI(moves, from)==GUI_ERROR) || display()==GUI_ERROR) {
					retVal = GUI_ERROR;
					break;
				}
			}
		} if (panelEvent && blitControl(root, initRect(0,0,0,0))==GUI_ERROR) {
			retVal = GUI_ERROR;
			break;
		} if (boardEvent || panelEvent) {
			boardEvent = panelEvent = 0;
			if (display()==GUI_ERROR) {
				retVal = GUI_ERROR;
				break;
			}
		}
		waitEvent(e);
		if (e.type == SDL_QUIT) {
			retVal = QUIT;
			break;
		}
		/* handle mouse clicks */
		if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
			if (inControl(button_save, &e)){
				retVal = SAVE_MENU;
				break;
			} else if (startGame && inControl(button_hint, &e)) {
				if (moveFlag) { /*some piece is already selected*/
					moveFlag = 0;
					freeList(moves);
					moves = NULL;
				}
				if (gameMode==PVP) {
					depth = getHintDifficulty(difficultyMenu);
					if (depth==GUI_ERROR || depth==QUIT) {
						retVal = depth;
						break;
					}
				}
				/*redraw the board over the menu or over previous selection*/
				if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || initBoard_GUI()==GUI_ERROR) {
					retVal = GUI_ERROR;
					break;
				}
				if ((move = miniMax_env(gameBoard, depth, nextPlayer, PLAYER_A)) == NULL) {
					print_malloc_error;
					retVal = GUI_ERROR;
					break;
				}
				if (showMove(move)==GUI_ERROR || display()==GUI_ERROR){
					freeMove(move);
					retVal = GUI_ERROR;
					break;
				}
				freeMove(move);
				move = NULL;
			} else if (startGame && inControl(button_on, &e)) {
				showMoves=1;
				button_on->srcRect=SRC_GAME_ON_BUTTON(showMoves);
				button_off->srcRect=SRC_GAME_OFF_BUTTON(!showMoves);
				panelEvent=1;
				boardEvent=1; /*we also want to clean board selections when switching modes*/
			} else if (startGame && inControl(button_off, &e)) {
				showMoves=0;
				button_on->srcRect=SRC_GAME_ON_BUTTON(showMoves);
				button_off->srcRect=SRC_GAME_OFF_BUTTON(!showMoves);
				panelEvent=1;
				boardEvent=1; /*we also want to clean board selections when switching modes*/
			} else if (inControl(button_mainMenu, &e)) {
				retVal = MAIN_MENU;
				return retVal;
			} else if (startGame && !inControl(panel, &e)){ /*In Board*/
				if (!moveFlag) { /*First position choice*/
					from = click2pos(e);
					if (from.row!=8 && from.col!=-1) {/* click is not out of the board */
						if (isUserPos(gameBoard, from, nextPlayer)) { /*this means we clicked on a piece of our color*/
							if ((moves=getMovesForPiece(gameBoard, from)) == NULL) {
								print_malloc_error;
								return GUI_ERROR;
							}
							moveFlag = 1;
							boardEvent = 1;
						}
					}
				} else { /*After a first position was chosen*/
					to = click2pos(e);
					if (to.row!=8 && to.col!=-1) {/* click is not out of the board */
						if (to.col==from.col && to.row==from.row) { /*if same pos was clicked - clean and continue*/
							boardEvent = 1;
							moveFlag = 0;
							freeList(moves);
							moves = NULL;
							continue;
						}
						/*extract move*/
						move = NULL;
						if (attemptedCastling(from,to)) {
							from = from.col==4? to:from; /*from should be rook's pos for move matching*/
							for (movesList_t* nextMove=moves ; !isEmpty(nextMove) ; nextMove=nextMove->next) {
								if (nextMove->curr->special == CASTLE && nextMove->curr->curr.col == from.col) {
									move = nextMove->curr;
									break;
								}
							}
						} else { /*regular move or pawn promotion - can inspect only to pos*/
							for (movesList_t* nextMove=moves ; !isEmpty(nextMove) ; nextMove=nextMove->next) {
								if (nextMove->curr->next->curr.col == to.col && \
										nextMove->curr->next->curr.row == to.row) {
									move = nextMove->curr;
									break;
								}
							}
						}
						/*play move*/
						if (move != NULL) { /* a legal move was selected */
							if (move->special != NORM && move->special != CASTLE) { /*pawn promotion*/
								if ((move->special=runPromotionsMenu(promotionsMenu)) == NORM) { /*NORM means QUIT in this context*/
									retVal = QUIT;
									break;
								}
							}
							/*redraw the board to remove promotions menu or optional frames*/
							if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || initBoard_GUI()==GUI_ERROR) {
								retVal = GUI_ERROR;
								break;
							}
							if (playMove_GUI(move)==GUI_ERROR || display()==GUI_ERROR) {
								retVal = GUI_ERROR;
								break;
							}
							playMove(gameBoard, move);
							retVal = CONTINUE;
							break;
						}
						/*if no legal move was selected - wait for legal move or selection cancel*/
					}
				}
			}
		}
	}

	freeList(moves);
	return retVal;

}

char runPromotionsMenu(control_t* root){
    char promoteTo = NORM;
    control_t* pieces[4];
    pieces[0] = root->child->child->next;
    pieces[0]->srcRect = SRC_PROMOTIONS_BUTTON(0);
    for (int i=0 ; i<3 ; i++) {
        pieces[i+1] = pieces[i]->next;
        pieces[i+1]->srcRect = SRC_PROMOTIONS_BUTTON(i+1);
    }

    SDL_Event e;
	if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display() == GUI_ERROR)
		return NORM; /*will quit*/
    while(1 && promoteTo==NORM) {
        waitEvent(e);
        if (e.type == SDL_QUIT)
            return NORM; /*return NORM instead of QUIT to avoid W_QUEEN conflict*/
        /* handle mouse clicks */
        if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
            /*check all pieces buttons*/ //nbqr
            if (inControl(pieces[0], &e)) {
                promoteTo = (nextPlayer==WHITE) ? W_KNIGHT:B_KNIGHT;
            } else if (inControl(pieces[1], &e)) {
                promoteTo = (nextPlayer==WHITE) ? W_BISHOP:B_BISHOP;
            } else if (inControl(pieces[2], &e)) {
                promoteTo = (nextPlayer==WHITE) ? W_QUEEN:B_QUEEN;
            } else if (inControl(pieces[3], &e)) {
                promoteTo = (nextPlayer==WHITE) ? W_ROOK:B_ROOK;
            }
        }
    }
    return promoteTo;
}

int getHintDifficulty(control_t* root){
    control_t* panel_menu = root->child;
    control_t* label_dif = panel_menu->child;
    control_t* button_dif[5];
    button_dif[0] = label_dif->next;
    for (int i=0 ; i<4 ; i++)
        button_dif[i+1] = button_dif[i]->next; /*button 5 is best*/
    SDL_Event e;

	if (blitControl(root, initRect(0,0,0,0))==GUI_ERROR || display() == GUI_ERROR)
		return GUI_ERROR; /*will quit*/
    while(1) {
        waitEvent(e);
        if (e.type == SDL_QUIT)
            return QUIT;
        /* handle mouse clicks */
        if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) {
            /*check all difficulty buttons*/
            for (int i=0 ; i<5 ; i++) {
                if (inControl(button_dif[i], &e)) {
                    return i+1;
                }
            }
        }
    }
    return minimaxDepth; /*Only to calm compiler*/
}

/*************** UTILITIES *************/

int initSurfaces() {
	 /* init SDL - loading surfaces */
	 if( SDL_Init( SDL_INIT_VIDEO ) == -1 )
		 return GUI_ERROR;

	 if ((screen=SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HIGHT, SCREEN_BPP, SDL_SWSURFACE ))==NULL)
		 return GUI_ERROR;
	 SDL_WM_SetCaption( "The Chess Nut", NULL );

	back = loadImage(BACK);
	pieces = loadImage(PIECES);
	pieces_frame = loadImage(PIECES_FRAME);
	buttons = loadImage(BUTTONS);
	central = loadImage(CENTRAL);
	if (back==NULL || pieces==NULL || pieces_frame==NULL || buttons==NULL || central==NULL)
		return GUI_ERROR;

	 SDL_Rect offset = {0,0,0,0};

	 return applySurface(&offset,back,screen,NULL);
}

/*initial call for root should be with rect {0,0}*/
int blitControl(control_t* root, SDL_Rect offset) {
	if (root == NULL)
		return 0;

	offset.x += root->dstRect.x;
	offset.y += root->dstRect.y;
	if (applySurface(&offset, root->source, screen, &root->srcRect) == GUI_ERROR) {
		print_SDL_error("failed to blit surface");
		return GUI_ERROR;
	}

	for (control_t* curr=root->child; curr!=NULL ; curr=curr->next) {
		if (blitControl(curr, offset)==GUI_ERROR)
			return GUI_ERROR;
	}

	return 0;
}

int display() {
	if (SDL_Flip(screen) == -1) {
		print_SDL_error("failed to flip screen");
		return GUI_ERROR;
	}
	return 0;
}

void freeControl(control_t* root) {
    if (root == NULL)
        return;

    for (control_t* curr=root, *toFree; curr!=NULL ;) {
        freeControl(curr->child);
        toFree = curr;
        curr = curr->next;
        free(toFree);
    }
}

/* check if event is inside the control (convert relative to absolute coordinates) */
int inControl(control_t* control, SDL_Event* e) {
	SDL_Rect rect = {0,0,control->dstRect.w,control->dstRect.h};
	for (; control!=NULL ; control=control->parent) {
		rect.x += control->dstRect.x;
		rect.y += control->dstRect.y;
	}
	return inRect(&rect, e);
}

/*Return 1 iff mouse click event is in rect*/
int inRect(SDL_Rect* rect, SDL_Event* e){

	int inRect = 0 ,x ,y ,h ,w ,mouse_x, mouse_y;

	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	mouse_x = e->button.x;
	mouse_y = e->button.y;

	if (mouse_x>=x && mouse_x <=(x+w) && mouse_y>=y && mouse_y<=(y+h))
		inRect = 1;

	return inRect;
}

/*Frame takes 'y'/'n'*/
int setPiece_GUI(pos_t pos, char piece, char frame){

	SDL_Rect clip, offset;
	SDL_Surface *source;

	source = (frame == 'y') ? pieces_frame : pieces;
	offset = pos2rec(pos);
	clip.h = TILE_WIDTH;
	clip.w = TILE_HIGHT;
	clip.y = (getColor(piece)==BLACK) ? NEXT_PIECE : 0; /*Clip 1st row for WHITE or 2nd for BLACK*/

	switch (piece) {
	case EMPTY:
		source = back;
		clip = offset;
		break;
	case W_PAWN:
	case B_PAWN:
		clip.x = NEXT_PIECE*0;
		break;
	case W_KNIGHT:
	case B_KNIGHT:
		clip.x = NEXT_PIECE*1;
		break;
	case W_BISHOP:
	case B_BISHOP:
		clip.x = NEXT_PIECE*2;
		break;
	case W_QUEEN:
	case B_QUEEN:
		clip.x = NEXT_PIECE*3;
		break;
	case W_ROOK:
	case B_ROOK:
		clip.x = NEXT_PIECE*4;
		break;
	case W_KING:
	case B_KING:
		clip.x = NEXT_PIECE*5;
		break;
	}

	if (rm_GUI(pos,'n')<0 || applySurface(&offset,source,screen,&clip)<0)
		return GUI_ERROR;

	return 0;
}

int rm_GUI(pos_t pos,char frame){

	SDL_Rect clip, clipFrame;

	clip = pos2rec(pos);

	clipFrame.x = NEXT_PIECE;
	clipFrame.y = NEXT_PIECE*2;
	clipFrame.h = TILE_WIDTH;
	clipFrame.w = TILE_HIGHT;

	if (SDL_BlitSurface( back, &clip, screen, &clip )<0)
		return GUI_ERROR;
	if(frame=='y') /*leave a frame*/
		if (SDL_BlitSurface( pieces_frame, &clipFrame, screen, &clip )<0)
				return GUI_ERROR;

	return 0;
}

/*Will return a SDL_Rect representing a tile compatible with given pos*/
SDL_Rect pos2rec(pos_t pos){
	SDL_Rect clip;

	clip.x = 4 + pos.col*TILE_WIDTH; /*3 was 4*/
	clip.y = 4 + (7-pos.row)*TILE_HIGHT; /*3 was 4*/
	clip.h = TILE_WIDTH;
	clip.w = TILE_HIGHT;

	return clip;

}

/*Should be executed before playMove()
 *after the call to this function ends - frames will be on the from & to positions*/
int playMove_GUI(move_t* move){
	pos_t from=move->curr, to=move->next->curr;
	char piece = gameBoard[from.col][from.row];

	rm_GUI(from, 'y'); /*take piece from*/
	if (setPiece_GUI(to,piece,'y') < 0)/*set piece to*/
		return GUI_ERROR;

	if (move->special != NORM) { /*handle pawn promotion and castling*/
		if (move->special == CASTLE) { /*rook already in proper place, need to move king*/
			from.col = 4; /*king's col, row already in place*/
			to.col += to.col>from.col? 1:-1; /*offset king's destination by that of rook's*/
			piece = gameBoard[from.col][from.row]; /*remember which king*/
			rm_GUI(from, 'y'); /*take piece from*/
			if (setPiece_GUI(to, piece, 'y') < 0) /*set piece to*/
				return GUI_ERROR;
		} else { /*pawn promotion, move was previously checked and is legal*/
			if (setPiece_GUI(to, move->special, 'y') < 0)
				return GUI_ERROR;
		}
	}

	return 0;
}

/*@pre: click location is already in Board*/
pos_t locatePosFromClick(SDL_Event click){
	int x, y, row=0, col=0;
	pos_t pos;

	x = click.button.x;
	y = click.button.y;

	for ( ; x>3 ; x-=TILE_WIDTH){
		col++;
	}
	for ( ; y>3 ; y-=TILE_WIDTH){
		row++;
	}

	pos.row = 8-row;
	pos.col = 8-col;

	return pos;
}

int applySurface(SDL_Rect* offset, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip) {

	if (SDL_BlitSurface( source, clip, destination, offset )<0) {
		print_SDL_error("failed to blit surface");
		return GUI_ERROR;
	}

	return 0;
}

/*Will return surface with transparent background*/
SDL_Surface *loadImage(char* filename){
	SDL_Surface* loadedImage = NULL;
	if ((loadedImage = SDL_LoadBMP(filename))==NULL)
		return NULL;
	Uint32 colorkey = SDL_MapRGB( loadedImage->format, 0xFF, 0xFF, 0);
	SDL_SetColorKey( loadedImage, SDL_SRCCOLORKEY, colorkey );

	return loadedImage;
}

void quitGUI() {
	SDL_FreeSurface(back);
	SDL_FreeSurface(pieces);
	SDL_FreeSurface(pieces_frame);
	SDL_FreeSurface(buttons);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(central);

	freeControl(mainMenu);
	freeControl(loadMenu);
	freeControl(saveMenu);
	freeControl(selectionMenu);
	freeControl(AIMenu);
	freeControl(gameWindow);
	freeControl(pieceSelection);
	freeControl(difficultyMenu);
	freeControl(promotionsMenu);

	SDL_Quit();
}

SDL_Rect initRect(int x, int y, int w, int h){

	SDL_Rect rect = {x,y,w,h};

	return rect;
}

int initBoard_GUI(){
	int i, j;
	pos_t pos;
	char piece;

	for (j = 0; j < BOARD_SIZE; j++){ /*rows*/
		for (i = 0; i < BOARD_SIZE; i++){ /*columns*/
			piece = gameBoard[i][j];
			pos.row = j;
			pos.col = i;
			if (setPiece_GUI(pos, piece, 'n') < 0)
				return GUI_ERROR;
		}
	}
	return 0;
}

/* checks if the user attempted a castling move */
int attemptedCastling(pos_t from, pos_t to) {
	return  (from.row==0 && to.row==0 && \
				((from.col==4 && gameBoard[from.col][from.row]==W_KING && \
				(to.col==0 || to.col==7) && gameBoard[to.col][to.row]==W_ROOK) || \
				(to.col==4 && gameBoard[to.col][to.row]==W_KING && \
				(from.col==0 || from.col==7) && gameBoard[from.col][from.row]==W_ROOK))) || \
			(from.row==7 && to.row==7 && \
				((from.col==4 && gameBoard[from.col][from.row]==B_KING && \
				(to.col==0 || to.col==7) && gameBoard[to.col][to.row]==B_ROOK) || \
				(to.col==4 && gameBoard[to.col][to.row]==B_KING && \
				(from.col==0 || from.col==7) && gameBoard[from.col][from.row]==B_ROOK)));
}

/*Given a button, will change its image to selected or not selected*/
void clickButton(control_t* button){
	int row = button->srcRect.y;

	/*Not yet selected*/
	if (row==0 || row==200)
		button->srcRect.y += SELECTED_OFFSET;

	/*Already selected*/
	else
		button->srcRect.y -= SELECTED_OFFSET;
}

/*If click is out of the board - pos.row==8 || pos.col==-1*/
pos_t click2pos(SDL_Event e){
	int x, y;
	pos_t pos;
	pos.row = 8;
	pos.col = -1;
	x = e.button.x;
	y = e.button.y;

	while (x>5){
		pos.col++;
		x -= TILE_WIDTH;
	}
	while (y>5){
		pos.row--;
		y -= TILE_HIGHT;
	}

	return pos;
}

/*returns piece that was clicked.
 *if a click wasn't on a piece - returns EMPTY
 *if there is a piece overflow - returns PIECE_OVERFLOW */
char pieceFromSelectionPanel(control_t *root, SDL_Event *e) {

	//{W_PAWN, W_KNIGHT, W_BISHOP, W_QUEEN, W_ROOK, W_KING,
	// B_PAWN, B_KNIGHT, B_BISHOP, B_QUEEN, B_ROOK, B_KING};
	unsigned pieces[12]; /*for countPieces*/

	control_t *wp = root->child->child->next;
	control_t *wn = wp->next;
	control_t *wb = wn->next;
	control_t *wq = wb->next;
	control_t *wr = wq->next;
	control_t *wk = wr->next;
	control_t *bp = wk->next;
	control_t *bn = bp->next;
	control_t *bb = bn->next;
	control_t *bq = bb->next;
	control_t *br = bq->next;
	control_t *bk = br->next;

	countPieces(gameBoard, pieces);	/*Will fill unsigned answer[12] - [m,n,b,r,q,k,M,N,B,R,Q,K]*/

	if (inControl(wp,e)) {
		if(pieces[0]<8)
			return W_PAWN;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(wn,e)) {
		if (pieces[1]<2)
			return W_KNIGHT;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(wb,e)) {
		if (pieces[2]<2)
			return W_BISHOP;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(wq,e)) {
		if(pieces[4]<1)
			return W_QUEEN;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(wr,e)) {
		if(pieces[3]<2)
			return W_ROOK;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(wk,e)) {
		if(pieces[5]<1)
			return W_KING;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(bp,e)) {
		if(pieces[6]<8)
			return B_PAWN;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(bn,e)) {
		if(pieces[7]<2)
			return B_KNIGHT;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(bb,e)) {
		if(pieces[8]<2)
			return B_BISHOP;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(bq,e)) {
		if(pieces[10]<1)
			return B_QUEEN;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(br,e)) {
		if(pieces[9]<2)
			return B_ROOK;
		else
			return PIECE_OVERFLOW;
	}
	if (inControl(bk,e)) {
		if(pieces[11]<1)
			return B_KING;
		else
			return PIECE_OVERFLOW;
	}

	return EMPTY;

}

/* auxiliary function for blitting slots' buttons */
SDL_Rect dstSlots(int i){
	if (i<4)
		return initRect(92+2*i*46,147,46,46);
	else
		return initRect(92+46+(i-4)*2*46,193,46,46);
}

/* @post: caller must display() */
int markPossible_GUI(pos_t pos) {
	SDL_Rect rect = pos2rec(pos);
	SDL_Rect rectPos = SRC_POSSIBLE_FRAME;

	if (applySurface(&rect,pieces_frame,screen,&rectPos)<0)
		return GUI_ERROR;

	return 0;
}

/* returns 1 iff selected pos occupied by piece of nextPlayer's color
 * @post: caller must display()*/
int markSelected_GUI(pos_t pos) {
	SDL_Rect rect = pos2rec(pos);
	SDL_Rect rectPos = SRC_SELECTED_FRAME;
	if (gameBoard[pos.col][pos.row]!=EMPTY && getColor(gameBoard[pos.col][pos.row])==nextPlayer){
		if (applySurface(&rect,pieces_frame,screen,&rectPos)<0)
			return GUI_ERROR;
		return 1;
	}
	return 0;
}

/* @post: caller must display() */
int showMove(move_t* move){
	if (markSelected_GUI(move->curr)==GUI_ERROR)
		return GUI_ERROR;
	if (markPossible_GUI(move->next->curr)==GUI_ERROR)
		return GUI_ERROR;
	return 0;
}

/* @post: caller must display() */
int showMoveCastle (move_t* move, pos_t startPos) {
	pos_t first, second;
	first = startPos;
	if (startPos.col == 4) { /*king is @ startPos*/
		second = move->curr;
	} else { /*rook is @ startPos*/
		second.row = startPos.row;
		second.col = 4;
	}
	if (markSelected_GUI(first)==GUI_ERROR)
		return GUI_ERROR;
	if (markPossible_GUI(second)==GUI_ERROR)
		return GUI_ERROR;

	return 0;
}

/* @post: caller must display() */
int showMoves_GUI(movesList_t* moves, pos_t startPos){
	for(movesList_t* nextMove=moves; !isEmpty(nextMove); nextMove=nextMove->next ){
		if (nextMove->curr->special == CASTLE){
			if(showMoveCastle(nextMove->curr, startPos)==GUI_ERROR)
				return GUI_ERROR;
		} else {
			if(showMove(nextMove->curr)==GUI_ERROR)
				return GUI_ERROR;
		}
	}
	return 0;
}




