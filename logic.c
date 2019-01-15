#include "logic.h"
#include "images/img_header.h"

#include <stdio.h>
#include <stdlib.h>

void initializeAppState(AppState* appState) {
    // TA-TODO: Initialize everything that's part of this AppState struct here.
    // Suppose the struct contains random values, make sure everything gets
    // the value it should have when the app begins.
    appState->gameOver = 0;
    appState->drag.airpixels = 0;
    appState->drag.jumping = 0;
    appState->drag.changed = 0;
    appState->drag.col= DRAG_START_COL;
    appState->drag.row = GROUND;
    appState->drag.direction = RIGHT;
    appState->drag.onFire = 0;

    for(int i = 0; i < MAX_SPIDERS; i++){
        appState->spiders[i].row = SPIDER_GROUND;
        appState->spiders[i].show = 0;
    }

    appState->fireball.show = 1;
    appState->fireball.row = GROUND - (MAX_AIRPIXELS / 2) + 10;
    appState->fireball.col = DRAG_START_COL;
    appState->charges = 0;

    appState->score = 0;
    appState->numSpiders = 0;
}

// TA-TODO: Add any process functions for sub-elements of your app here.
// For example, for a snake game, you could have a processSnake function
// or a createRandomFood function or a processFoods function.
//
// e.g.:
// static Snake processSnake(Snake* currentSnake);
// static void generateRandomFoods(AppState* currentAppState, AppState* nextAppState);

// This function processes your current app state and returns the new (i.e. next)
// state of your application.
AppState processAppState(AppState *currentAppState, u32 keysPressedBefore, u32 keysPressedNow) {
    /* TA-TODO: Do all of your app processing here. This function gets called
     * every frame.
     *
     * To check for key presses, use the KEY_JUST_PRESSED macro for cases where
     * you want to detect each key press once, or the KEY_DOWN macro for checking
     * if a button is still down.
     *
     * To count time, suppose that the GameBoy runs at a fixed FPS (60fps) and
     * that VBlank is processed once per frame. Use the vBlankCounter variable
     * and the modulus % operator to do things once every (n) frames. Note that
     * you want to process button every frame regardless (otherwise you will
     * miss inputs.)
     *
     * Do not do any drawing here.
     *
     * TA-TODO: VERY IMPORTANT! READ THIS PART.
     * You need to perform all calculations on the currentAppState passed to you,
     * and perform all state updates on the nextAppState state which we define below
     * and return at the end of the function. YOU SHOULD NOT MODIFY THE CURRENTSTATE.
     * Modifying the currentAppState will mean the undraw function will not be able
     * to undraw it later.
     */

    AppState nextAppState = *currentAppState;

    // DRAG STUFF ----------------------------------------------
    if(nextAppState.drag.jumping){
        if(nextAppState.drag.airpixels < MAX_AIRPIXELS/2){
            nextAppState.drag.row = GROUND - nextAppState.drag.airpixels;
            nextAppState.drag.airpixels += JUMP_UP_INCREMENT;
        } else if(nextAppState.drag.airpixels == MAX_AIRPIXELS){
            nextAppState.drag.jumping = 0;
            nextAppState.drag.airpixels = 0;
            nextAppState.drag.row = GROUND;
        } else if(nextAppState.drag.airpixels >= MAX_AIRPIXELS/2){
            nextAppState.drag.row = GROUND - (MAX_AIRPIXELS - nextAppState.drag.airpixels);
            nextAppState.drag.airpixels += JUMP_DOWN_INCREMENT;
        }
    }

    if(!nextAppState.drag.jumping && KEY_DOWN(BUTTON_UP, keysPressedNow)){
        nextAppState.drag.jumping = 1;
    }

    if(KEY_DOWN(BUTTON_LEFT, keysPressedNow) && !KEY_DOWN(BUTTON_RIGHT, keysPressedNow) && nextAppState.drag.col > 0){
        // drag wants to go right
        nextAppState.drag.col -= LATERAL_SPEED;
        nextAppState.drag.direction = LEFT;
    } else if(!KEY_DOWN(BUTTON_LEFT, keysPressedNow) && KEY_DOWN(BUTTON_RIGHT, keysPressedNow)){
        if((nextAppState.drag.onFire && nextAppState.drag.col < GBA_WIDTH - DRAG_FIRE_LEFT_WIDTH) || (!nextAppState.drag.onFire && nextAppState.drag.col < GBA_WIDTH - DRAG_LEFT_WIDTH)){
        // drag wants to go right
            nextAppState.drag.col += LATERAL_SPEED;
            nextAppState.drag.direction = RIGHT;
        }
    }

    // onFire control
    if(KEY_JUST_PRESSED(BUTTON_A, keysPressedNow, keysPressedBefore) && nextAppState.charges > 0){
        nextAppState.drag.onFire = 1;
    }

    // down button puts the drag on the ground
    if(KEY_JUST_PRESSED(BUTTON_DOWN, keysPressedNow, keysPressedBefore)){
        nextAppState.drag.jumping = 0;
        nextAppState.drag.row = GROUND;
        nextAppState.drag.airpixels = 0;
    }

    // SPIDER STUFF -----------------------------------------------

    // creating spiders
    if(IS_NTH_SECOND(5) && nextAppState.numSpiders < MAX_SPIDERS){
        int i = 0;
        while(nextAppState.spiders[i].show == 1){
            i++;
        }

        nextAppState.spiders[i].show = 1;
        nextAppState.spiders[i].direction = randint(0,2);

        if(nextAppState.spiders[i].direction == LEFT){
            nextAppState.spiders[i].col = GBA_WIDTH - SPIDER_WIDTH - 1;
        } else{
            nextAppState.spiders[i].col = 1;
        }

        nextAppState.numSpiders++;
    }

    // updating spider position
    for(int i = 0; i < nextAppState.numSpiders; i++){
        if(nextAppState.spiders[i].col == 0){
            nextAppState.spiders[i].direction = RIGHT;
        } else if(nextAppState.spiders[i].col == GBA_WIDTH - SPIDER_WIDTH){
            nextAppState.spiders[i].direction = LEFT;
        }
        if(nextAppState.spiders[i].direction == RIGHT){
            nextAppState.spiders[i].col++;
        } else{
            nextAppState.spiders[i].col--;
        }
    }

    // updating fireball's position
    if(!nextAppState.fireball.show && IS_NTH_SECOND(FIREBALL_INTERVAL) && nextAppState.charges < MAX_CHARGES){
        nextAppState.fireball.col = randint(0, GBA_WIDTH - FIREBALL_WIDTH);
        nextAppState.fireball.show = 1;
    }

    // INTERACTIONS ---------------------------------------------

    // processing deaths
    for(int i = 0; i < MAX_SPIDERS; i++){
        if(nextAppState.spiders[i].show){
            // spider is alive
            if(nextAppState.drag.row + DRAG_RIGHT_HEIGHT >= SPIDER_GROUND){
                // drag is on the same level as spiders
                if(nextAppState.drag.onFire){
                // drag is on fire
                    if(nextAppState.spiders[i].col + SPIDER_WIDTH >= nextAppState.drag.col &&
                        nextAppState.spiders[i].col <= nextAppState.drag.col + DRAG_FIRE_RIGHT_WIDTH/2){
                    // spider is on the left part of drag
                        if(nextAppState.drag.direction == RIGHT){
                        // drag is facing right and game is over
                            nextAppState.gameOver = 1;
                        } else{
                        // spider is dead
                            nextAppState.spiders[i].show = 0;
                            nextAppState.score++;
                            nextAppState.numSpiders--;
                            nextAppState.charges--;
                            nextAppState.drag.onFire = 0;
                        }
                    } else if(nextAppState.spiders[i].col + SPIDER_WIDTH >= nextAppState.drag.col + DRAG_FIRE_RIGHT_WIDTH/2 && nextAppState.spiders[i].col <= nextAppState.drag.col + DRAG_FIRE_RIGHT_WIDTH){
                    // spider is on the right part of drag
                        if(nextAppState.drag.direction == LEFT){
                        // drag is facing left and game is over
                            nextAppState.gameOver = 1;
                        } else{
                        // spider is dead
                            nextAppState.spiders[i].show = 0;
                            nextAppState.score++;
                            nextAppState.numSpiders--;
                            nextAppState.charges--;
                            nextAppState.drag.onFire = 0;
                        }
                    }
                } else{
                // drag is not on fire
                    if(nextAppState.spiders[i].col + SPIDER_WIDTH >= nextAppState.drag.col && nextAppState.spiders[i].col <= nextAppState.drag.col + DRAG_RIGHT_WIDTH){
                    // spider is touching drag
                        nextAppState.gameOver = 1;
                    }
                }
            }
        }
    }

    // processing getting fireballs/charges
    if(nextAppState.fireball.show){
        // fireball exists
        if(nextAppState.fireball.row + FIREBALL_HEIGHT > nextAppState.drag.row){
            // drag is above the fireball
            if(nextAppState.fireball.col + FIREBALL_WIDTH > nextAppState.drag.col && nextAppState.fireball.col <= nextAppState.drag.col + DRAG_RIGHT_WIDTH){
                // drag is on the same col as fireball
                nextAppState.fireball.show = 0;
                nextAppState.charges++;
            }
        }
    }


    // UNUSED(keysPressedBefore);
    // UNUSED(keysPressedNow);

    return nextAppState;
}
