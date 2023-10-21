/*
 * MIT License
 *
 * Copyright (c) 2023 <#author_name#>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "yolo_dice_face.h"


// Emulator only: need time() to seed the random number generator.
#if __EMSCRIPTEN__
#include <time.h>
#include <emscripten.h>
#endif

static void game_reset(yolo_dice_state_t *state) {
    int i;
    state->game_active = false;
    state->roll_count = 0;
    state->cursor_position = 0;
    state->dice_held = 0;
    state->bonus_score = 0;
    for (i = 0; i < 13; i++) {
        state->score_categories[i] = false;
        state->dice_scores[i] = 0;
    }
    state->bonus_rounds = 	0;
    state->rounds_remaining = 13;
    state->score = 0;
    state->c_upper = true;
    state->c_runs = true;
    state-> c_sets = true;
    for (i = 0; i < 5; i++) state->dice[i] = 0;
    for (i = 0; i < 6; i++) state->dice_count[i] = 0;
    state->sum = 0;
    state->d_set_three = false;
    state->d_set_four = false;
    state->d_yolo = false;
    state->d_short_run = false;
    state->d_long_run = false;
    state->d_full_house = false;
    state->d_max_upper = 0;
    state->d_max_bonus = 0;
    
    state->yolo_count = 0;
    state->upper_bonus = false;
    state->upper_score = 0;
	state->can_haz_yolo_bonus = false;
    
    watch_clear_indicator(WATCH_INDICATOR_24H);
    watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
    #if __EMSCRIPTEN__
    
    EM_ASM( {
        console.log( "LET'S PLAY YOLO!" )
    } );
    #endif
}
static bool is_set(yolo_dice_state_t *state, int n) {
    int i;
    for (i = 0;i < 6;i++) {
        if (state->dice_count[i] >= n) return true;
    }
    return false;
}




static bool is_full_house(yolo_dice_state_t *state) {
    bool two = false, three = false;
    int i;
    for (i = 0;i < 6;i++) {
        if (state->dice_count[i] == 2) two = true;
        if (state->dice_count[i] == 3) three = true;
    }
    if (two && three) {
        return true;
    } 
        
	return false;
    
}

static bool is_long_run(yolo_dice_state_t *state) {
    int  c = 0, max = 0, i;
    for (i = 0;i < 6;i++) {
        if (state->dice_count[i] > 0) {
            c = c+1;
            if (c > max) max = max+1;
        } else {
            c = 0;
        }
    }
    // check for 5 consecutive ones in dice_count
    if (max == 5) {
        return true;
    }
        
	return false;
    
}

static bool is_short_run(yolo_dice_state_t *state) {
    int  c = 0, max = 0, i;
    for (i = 0; i < 6; i++) {
        if (state->dice_count[i] > 0) {
            c++;
            if (c > max) max++;
        } else {
            c = 0;
        }
    }
    // check for 4 consecutive ones (or >1) in dice_count
    if (max >= 4) return true;
    
	return false;
}

static uint8_t upper(yolo_dice_state_t *state, int n) {
    return state->dice_count[n-1] * n;
}

static uint8_t set(yolo_dice_state_t *state, int n) {
    if (is_set(state, n)) return state->sum;
    return 0;
}

static uint8_t full_house(yolo_dice_state_t *state) {
    if (is_full_house(state)) return 25;
    return 0;
}

static uint8_t yolo(yolo_dice_state_t *state) {

    if (state->d_yolo) return 50;
    return 0;
    
}

static uint8_t short_run(yolo_dice_state_t *state) {

    if (state->d_short_run) return 30;
    return 0;
    
}

static uint8_t long_run(yolo_dice_state_t *state) {

    if (state->d_long_run) return 40;
    return 0;
    
}

static int sum(yolo_dice_state_t *state) {
    int i, s=0;
	
    for ( i= 0;i < 6 ;i++) {
        s+=state->dice_count[i] * (i+1);
    }
    return s;
}
// find the largest set with the biggest score
static uint8_t max_upper(yolo_dice_state_t *state) {
    
        int  i, max_pos =-1, max = 1;
        
        for (i = ONE;i<SIX+1;i++) {
            // check if category is open to be claimed
            if (!state->score_categories[i]) {
            
                
                if (state->dice_count[i]>=max) {
                    
                    max = state->dice_count[i];
                    max_pos = i;
                 
               if(state->dice_count[i] == max) max_pos = i;
                }
            }
        


        }

    if (max_pos == -1) { 
        return 0;
    } else {
        return max_pos+1;
    }
    
}

static uint8_t max_bonus(yolo_dice_state_t *state) {

    int max=0, i;
    
    //search all categories except chance for largest score
    for (i=ONE; i<CHANCE; i++){
        if (state->dice_scores[i] > max) {
            max = state->dice_scores[i];
        }
    }
    return max;
}

static void hold(yolo_dice_state_t *state, int n) {
	// move dice at position n to beginning and shift others right
 
    int x,i;
    x = state->dice[n];
    for (i = 0; i < n; i++) state->dice[n-i] = state->dice[n-i-1];
    state->dice[0] = x;
    
	// number of dice at the beginning that are 'held' and should not be 'rolled'
    state->dice_held++;
    
}

static void unhold(yolo_dice_state_t *state, int n) {
	// swap the dice at position n with the right-most held dice and decrease num of dice held.
 
    int x,h;
    h = state->dice_held-1;
    x = state->dice[h];
    state->dice[h] = state->dice[n];
    state->dice[n] = x;
    
    state->dice_held--;
    
    
}

static void cursor_next(yolo_dice_state_t *state) {

    switch(state->game_mode) { 
                
		case BONUS_ROLL:
        case ROLL:
            if (state->roll_count < 3) {
   
                
                state->cursor_position=(state->cursor_position + 1) % 6;
                if (state->cursor_position == state->dice_held) {
                    state->cursor_position= (state->cursor_position + 1) % 6 ;
                
                }
            }
			
			// for update_lcd to blank the position on next tick
			state->ticks = 4;
			
            break;
		case BONUS_SCORE:
        case SCORE:
        {
            // create a list of bools - is category available to be selected?
            bool cats[6] = {state->c_upper, state->c_sets, state->c_runs, \
                            !state->score_categories[FULL_HOUSE], !state->score_categories[CHANCE], !state->score_categories[YOLO]};
			           
			// find next open category in list
            int i;
            for (i=0;i<7;i++) {
                if (cats[(state->cursor_position+i+1) % 6]) {
                    state->cursor_position = (state->cursor_position + i + 1) %6;
                    break;
                }
            }
			
			// update_lcd to blank the position on next tick
			state->ticks = 4;
        } 
            break;
        case SET:
			// both options open
            if (!state->score_categories[SET_THREE] && !state->score_categories[SET_FOUR]) {
                 state->cursor_position=(state->cursor_position + 1) % 2;
			// only set 3
            } else if (!state->score_categories[SET_THREE]) {
                state->cursor_position=0;
			// only set 4
            } else state->cursor_position=1;
           
            break;
        case RUN:
			// both options open
            if (!state->score_categories[SHORT_RUN] && !state->score_categories[LONG_RUN]) {
                 state->cursor_position=(state->cursor_position + 1) % 2;
			// only short run
            } else if (!state->score_categories[SHORT_RUN]) {
                state->cursor_position=0;
			// only long run
            } else state->cursor_position=1;
           
            break;
        case UPPER:
        {
			// find next open upper category
            int i;
            for (i=ONE;i<SIX+1;i++) {
                if (!state->score_categories[ (state->cursor_position+i+1) % 6] ) {
                    state->cursor_position = (state->cursor_position + i + 1) %6;
                    break;
                }
            }
			
			// update_lcd to blank the position on next tick
			state->ticks = 4;

            break;
        }
        case MENU:
            // state->game_mode = ROLL;
            break;
        case GAME_OVER:
            break;
    }   

        
}
static void cursor_reset(yolo_dice_state_t *state) {

    switch(state->game_mode) { 
        
		case BONUS_ROLL:
        case ROLL:
			// start at first open dice in dice_list
            state->cursor_position = state->dice_held+1;
            break;

		case BONUS_SCORE:
        case SCORE:
			//jump to yolo category if it is open and user has yolo, else wrap to next open category
			state->cursor_position = 5;
			
			// no yolo
			if(!state->d_yolo) {
				cursor_next(state);
				
			// yolo yes, open no
			} else if (state->score_categories[YOLO]) {
				cursor_next(state);
			}
			
			
            break;

        case SET:
			// both open and set 4 can score points
            if (!state->score_categories[SET_FOUR] && state->d_set_four) state->cursor_position = 1;
			// only set 4 available
            else if (!state->score_categories[SET_FOUR] && state->score_categories[SET_THREE]) state->cursor_position = 1;
            // set 3
			else state->cursor_position = 0;
            break;
        case RUN:
            
            // cursor_pos 0 : short run; 1 : long run
            
			// both open and long run can score points
            if (!state->score_categories[LONG_RUN] && state->d_long_run) {
                state->cursor_position = 1;
			// only long run
			} else if (!state->score_categories[LONG_RUN] && state->score_categories[SHORT_RUN]) {
				state->cursor_position = 1;
			// short run
			} else {
                state->cursor_position = 0;
            }
            break;
        case UPPER:
			// jump to longest set with most points 
            if (state->d_max_upper > 0) {
                state->cursor_position = state->d_max_upper-1;
            } else {
                state->cursor_position = 5;
                cursor_next(state);
            }
            break;
        case MENU:
				state->cursor_position = 0;
            break;
        case GAME_OVER:
            break;
    }   

        
}

static void update_dice_count(yolo_dice_state_t *state) {
	// count of each dice value in dice_list by index (0 = ONES, 1 = TWOS...)

    int i;
    for (i = 0; i < 6; i++) {
    state->dice_count[i] = 0;
    }
	
	for (i=0; i<5; i++){
		state->dice_count[ state->dice[i] - 1 ]++;
	}

    
    
}


static void _yolo_dice_face_update_lcd(yolo_dice_state_t *state) {
    switch(state->game_mode) { 
        
        case INTRO:
        {
            char buf[11];
			
			// tick = 1hz
			
			
            // new game
            if (state->rounds_remaining == 13){
                watch_display_string("     YOLO ", 0);
				
			
            // game in progress, show score every 4 secs    
            } else if (state->ticks == 0 && state->rounds_remaining < 13) {
                
                sprintf(buf, "YO%2d %3d  ", state->rounds_remaining, state->score);
                watch_display_string(buf, 0);
            } else {
                sprintf(buf, "  %2d YOLO ", state->rounds_remaining);
                watch_display_string(buf, 0);
            }
            
            break;
        }
		
		case ROLL_ANIMATION:
		
		{
			char c[1];
			int length;
			int write_delay;
			int start_position;
			
			
			length = 5-state->dice_held;
			start_position = 5+state->dice_held;
			write_delay = 5;
			
			// update rolls remaining
			if (state->ani_ticks==0) {

				
				sprintf(c, "%d", state->roll_count);
				
				watch_display_string(c, 3);
				
			}
			
			
			
			
			if ( state->ani_ticks < length + write_delay ) {
				
				// clear old dice
				if ( state->ani_ticks < length ) {
				
					watch_display_string("=", start_position + state->ani_ticks );
					
				}
				
				// write new dice
				if (state->ani_ticks >= write_delay ) {
					
					sprintf(c, "%d", state->dice[state->dice_held+ (state->ani_ticks-write_delay) ]);
					watch_display_string(c, start_position + state->ani_ticks-write_delay  );
				}
				
				
				state->ani_ticks++;

			} else {
				// continue
				state->game_mode = ROLL;
				cursor_reset(state);
				
				// reset ticks so cursor is solid for .5 sec before blinking
				state->ticks = 0;
				_yolo_dice_face_update_lcd(state);
				
			}
			break;
			
		}
		case YOLO_ANIMATION:
		{
			char c[1];
			if (state->ani_ticks <8) {
				
				// clear dice on screen
				if(state->ani_ticks<=5){
					watch_display_string(" ", state->ani_ticks+4);
				}

				// write new dice
				if (state->ani_ticks>2) {
					
					
					sprintf(c, "%d", state->dice[state->ani_ticks-3]);
					watch_display_string(c, state->ani_ticks+2);
					
				}
			state->ani_ticks++;
			
			
			} else if (state->ani_ticks < 10) {
				state->ani_ticks++;
			
			} else if (state->ani_ticks == 10) {
				state->ani_ticks++;
				watch_display_string(" YOLO ", 4);
			} else if (state->ani_ticks < 16) {
				
				state->ani_ticks++;
		
			} else {
			
				if (state->rounds_remaining > 0) {
					state->game_mode = SCORE;
					
				} else {
					state->game_mode = BONUS_SCORE;
				}
				cursor_reset(state);
			// break;
			}
		break;
		}

		case BONUS_ROLL:
		//  combined with ROLL for now
        case ROLL:
        {
            
            char dstr[6];
            
            sprintf(dstr, "%d%d%d%d%d", state->dice[0], state->dice[1], state->dice[2], state->dice[3], state->dice[4]);
            
            int h;
            h= state->dice_held;
            
            char buf[5];
            sprintf(buf, "%cO %d", state->rounds_remaining == 0 ? 'B' : 'Y', state->rounds_remaining == 0 ? state->bonus_rounds+1 : state->roll_count);

            watch_display_string(buf, 0);
            int i;
            
            // display held dice
            for (i=0;i<h;i++) {
                watch_display_string((char[2]) { (char) dstr[i], '\0' },4+i);
            }
            
            // display separator char
            watch_display_string("=", 4+h);
            
            // display open dice
            for (i=0;i<5-h;i++) {
            
                watch_display_string((char[2]) { (char) dstr[i+h], '\0' }, 4+h+1+i);
            

            }
            
            // cursor blink, overwrites with blank character at cursor position
            if (state->ticks > 3 && state->roll_count<3 && state->roll_count > 0) watch_display_string(" ", state->cursor_position+4);
            
            // bonus round blink
            if (state->rounds_remaining == 0 && state->roll_count == 0) {
                if (state->ticks > 3) {
                    watch_clear_indicator(WATCH_INDICATOR_SIGNAL);
                    watch_display_string("    ", 0);
                    
                } else {
                    watch_set_indicator(WATCH_INDICATOR_SIGNAL);
                    
                }
            }
                
        }
            break;
        case SCORE:
        {
            int display_score;
            char display_info;
            
            // get state info based on which category user has highlighted
            switch(state->cursor_position) {
                
                // Upper
                case 0:
                    display_score = state->dice_count[state->d_max_upper-1] * state->d_max_upper;
                    display_info = state->d_max_upper + '0';
                    
                    break;
                    
                // Set
                case 1:
                    if (state->d_set_four && !state->score_categories[SET_FOUR] ) {
                        display_score = state->sum;
                        display_info = '4';
                    }else if (state->d_set_three && !state->score_categories[SET_THREE] ) {
                        display_score = state->sum;
                        display_info = '3';
                    }else {
                        display_score = 0;
                        display_info = ' ';
                    }

                    break;

                // Run
                case 2:
                    if (state->d_long_run && !state->score_categories[LONG_RUN] ) {
                        display_score = 39;
                        display_info = 'L';
                    }else if (state->d_short_run && !state->score_categories[SHORT_RUN] ) {
                        display_score = 30;
                        display_info = 'S';
                    }else {
                        display_score=0;
                        display_info = ' ';
                    }

                    break;
                
                // Full house
                case 3:
                    display_score = state->dice_scores[FULL_HOUSE];
                    display_info = ' ';

                    break;
                
                // Choice
                case 4:
                    display_score = state->sum;
                    display_info = ' ';

                    break;
                    
                // Yolo
                case 5:
                    display_score = state->dice_scores[YOLO];
                    display_info = ' ';

                    break;
                // default:
                //     display_score=88;
                    
            
            //blink


            
            }

                
            char buf[11];
            
 
            sprintf(buf, "%c %2d%C%C%C%C%C%C",display_info, display_score, \
                state->c_upper ? 'U' : ' ', \
                state->c_sets ? 'S' : ' ', \
                state->c_runs ? 'r' : ' ', \
                !state->score_categories[FULL_HOUSE] ? 'F' : ' ', \
                !state->score_categories[CHANCE] ? 'C' : ' ', \
                !state->score_categories[YOLO] ? 'Y' : ' ' \
            );
            
            watch_display_string(buf, 0);
            
			// cursor blink
            if (state->ticks > 3 ) {
                watch_display_string(" ", state->cursor_position+4);
                
            }
            
            
            // }
            break;
        }
            
                
        
        case SET:
        {
            char buf[11];
			
			if (state->cursor_position ==0 ) {
				sprintf(buf, "  %2d Set 3", state->dice_scores[SET_THREE]);
			} else {
				sprintf(buf, "  %2d Set 4", state->dice_scores[SET_FOUR]);
			}

            watch_display_string(buf, 0);
			
			// cursor blink
            if (state->ticks > 3 ) watch_display_string(" ", 9);
            
                
            break;
        }
        
        case RUN:
        {
            char buf[11];
            
			if (state->cursor_position==0) {
				sprintf(buf, "  %2dShOrt ", state->dice_scores[SHORT_RUN]);
				if (state->ticks > 3 ) watch_display_string("  ", 2);
			} else {
				sprintf(buf, "    lOnG%2d", state->dice_scores[LONG_RUN]);
				if (state->ticks > 3 ) watch_display_string("  ", 8);
			}
			watch_display_string(buf, 0);
			
			// cursor blink
			if (state->cursor_position==0) {
				if (state->ticks > 3 ) watch_display_string("     ", 4);
			} else {
				if (state->ticks > 3 ) watch_display_string("    ", 4);
			}
            
            break;
        }
        case BONUS_SCORE:
		{
			char buf[11];
			if (state->rounds_remaining == 0) {
			sprintf(buf, "BO    %2d  ", state->d_max_bonus);
			watch_display_string(buf, 0);
			}
            break;
				
		}
        case UPPER:
        {
            int i;
            char buf[11];
            sprintf(buf, "  %2d123456", upper(state, state->cursor_position+1));
            watch_display_string(buf, 0);
            
            for (i=ONE;i<SIX+1;i++) {
                if (state->score_categories[i]) watch_display_string(" ", i+4);
            }
            
            //blink
            if (state->ticks > 3 ) watch_display_string(" ", state->cursor_position+4);

            break;
        }
        case MENU:
            watch_display_string("    rESET=", 0);
			if (state->ticks > 3) watch_display_string(" ", 9);
			
            break;
        case GAME_OVER:
        {
            char buf[11];
            sprintf(buf, "YO   %3d  ", state->score);
            watch_display_string(buf, 0);

			//blink for scores 300 and greater
			if (state->score >= 300 && state->ticks > 3 ) {
				watch_display_string("      ", 4);
			}
            break;
            
        }
    }
    
}

static void update_dice_scores(yolo_dice_state_t *state) {
    int i;
    for (i = ONE; i<SIX+1; i++) state->dice_scores[i] = upper(state, i+1);
    state->dice_scores[FULL_HOUSE] = full_house(state);
    state->dice_scores[SET_THREE] = set(state, 3);
    state->dice_scores[SET_FOUR] = set(state, 4);
    state->dice_scores[SHORT_RUN] = short_run(state);
    state->dice_scores[LONG_RUN] = long_run(state);
    state->dice_scores[CHANCE] = state->sum;
    state->dice_scores[YOLO] = yolo(state);
}
//roll
static void roll(yolo_dice_state_t *state) {

	int i, r;
	for(i = 0;i < 5 - state->dice_held ;i++) {

			
		#if __EMSCRIPTEN__
		r = rand() %  6 + 1;

		#else
		r = arc4random_uniform(6) + 1;
		// r = arc4random() %  6 + 1;
		
		#endif
		state->dice[i+state->dice_held]= r;
	}

	state->roll_count++;
    update_dice_count(state);
	state->sum = sum(state);
	
    // what aspects about the current 5 dice true?
    state->d_set_three = is_set(state,3);
    state->d_set_four = is_set(state,4);
    state->d_yolo = is_set(state,5);
    state->d_short_run = is_short_run(state);
    state->d_long_run = is_long_run(state);
    state->d_full_house = is_full_house(state);
    state->d_max_upper = max_upper(state);
	


    // what are the possible scores for each category?
    update_dice_scores(state);
    state->d_max_bonus = max_bonus(state);    


	state->ani_ticks = 0;
	if (state->d_yolo) {
		// jump to score mode if yolo is rolled
		state->game_mode = YOLO_ANIMATION;
	} else{
		state->game_mode = ROLL_ANIMATION;
    }

}

static void next_turn(yolo_dice_state_t *state) {
	
	
	// YOLO BONUS is updated in next_turn() so it can be applied in bonus rounds as well
	if (state->can_haz_yolo_bonus && state->d_yolo) {
		
		if ( state->yolo_count > 0 ) {
			state->bonus_score += 50;
		}
		state->bonus_rounds = state->bonus_rounds + 1;
		state->yolo_count = state->yolo_count + 1;
	}
	
		if (!state->upper_bonus && state->upper_score >= 63) {
			state->upper_bonus = true;
			state->bonus_score += 24;
			state->bonus_rounds = state->bonus_rounds + 1;
			watch_set_indicator(WATCH_INDICATOR_24H);
			watch_set_indicator(WATCH_INDICATOR_SIGNAL);
		}
	

    
	
	//UPDATE ROUNDS REMAINING and GOTO NEXT ROUND
    if (state->rounds_remaining > 0) {
		state->rounds_remaining = state->rounds_remaining - 1;
	}
	// Game Over
    if (state->rounds_remaining == 0 && state->bonus_rounds == 0) {
        state->score += state->bonus_score;
        state->game_mode = GAME_OVER;
        state->game_active = false;
	// Bonus Roll
    } else if (state->rounds_remaining==0 && state->bonus_rounds > 0) {
        state->bonus_rounds--;
        state->game_mode = BONUS_ROLL;
	// Standard Roll
    } else {
        state->game_mode = ROLL;
    }
    
	    int i,c=0;
		
		
	// CLEAR DICE DATA
    state->dice_held = 0;
    state->roll_count = 0;
    // clear_dice_count(state);
    state->d_max_bonus=0;
	for (i=0;i<5;i++) state->dice[i]=0;
    
	
	
	// UPDATE WHETHER CATEGORY GROUPS ARE SELECTABLE

	// count selected upper categories
    for (i=ONE;i<SIX+1;i++) c+=state->score_categories[i];
    if (c==6) state->c_upper =false;

    if (state->score_categories[SET_THREE] && state->score_categories[SET_FOUR]) state->c_sets = false;
    
    if (state->score_categories[SHORT_RUN] && state->score_categories[LONG_RUN]) state->c_runs = false;
    
    
 
    #if __EMSCRIPTEN__
    // EM_ASM( {
        // console.log("ONE "+$0+" TWO "+$1+" THREE "+$2+" FOUR "+$3+"  FIVE "+$4+"  SIX "+$5+"  3S "+$6+"  4S "+$7+"  FULL "+$8+"  SHORT "+$9+"  LONG "+$10+"  CH "+$11+"  YO "+$12 +"  BONUS: "+$13+" "+$14);
        
    // }, state->dice_scores[ONE], state->dice_scores[TWO], state->dice_scores[THREE], state->dice_scores[FOUR], state->dice_scores[FIVE], state->dice_scores[SIX], state->dice_scores[SET_THREE], state->dice_scores[SET_FOUR], state->dice_scores[FULL_HOUSE], state->dice_scores[SHORT_RUN], state->dice_scores[LONG_RUN], state->dice_scores[CHANCE], state->dice_scores[YOLO], state->bonus_score, state->bonus_rounds);
    
    EM_ASM( {
        console.log( "SCORE: " + $0 + " ROUNDS LEFT: " + $1 )
    }, state->score, state->rounds_remaining);
    #endif
    
}
static void write_score(yolo_dice_state_t *state, int category) {
	
	// check category has not already been claimed. prevents dbl clicks
    if (!state->score_categories[category]) {	
	
	
		//UPDATE BONUS SCORES


		

		
		// handle bonus for successful yolo category, enable yolo bonus on subsequent yolo -- in next_turn()
		if (category==YOLO && state->d_yolo) {
			state->can_haz_yolo_bonus = true;
			watch_set_indicator(WATCH_INDICATOR_SIGNAL);
		}
			
        //keep track of upper subscore
        if (category <= SIX) {
            state->upper_score += state->dice_scores[category];

        }
		
		// track whether a category has already been selected
        state->score_categories[category] = true;
		
		//adds dice score for the given category to the score sum
        state->score += state->dice_scores[category];
		
		// go to next round
        next_turn(state);
		
		#if __EMSCRIPTEN__
		EM_ASM( {
			console.log( "UPPER: " + $0 + " BONUS: " + $1 + " BONUS ROLLS: " + $2 )
		}, state->upper_score, state->bonus_score, state->bonus_rounds);
		#endif
		

    }
    
        
}

void yolo_dice_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(yolo_dice_state_t));
        memset(*context_ptr, 0, sizeof(yolo_dice_state_t));
        // Do any one-time tasks in here;the inside of this conditional happens only at boot.
    }
    

    // Emulator only: Seed random number generator
    #if __EMSCRIPTEN__
    srand(time(NULL));
    #endif
    // Do any pin or peripheral setup here;this will be called whenever the watch wakes from deep sleep.
}

void yolo_dice_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    yolo_dice_state_t *state = (yolo_dice_state_t *)context;
	state->ticks=0;

    // Handle any tasks related to your watch face coming on screen.
}

bool yolo_dice_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    yolo_dice_state_t *state = (yolo_dice_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            // #if __EMSCRIPTEN__
            // EM_ASM( {
                // console.log( "ACTIVE 1" )
            // });
            // #endif

            if ( !state->game_active ) {
                game_reset(state);
                state->game_active = true;
                // movement_illuminate_led();
                // roll(state);
            }
            
            state->game_mode = INTRO;
            
            _yolo_dice_face_update_lcd(state);
            // movement_request_tick_frequency(8);
            
            
            break;
        case EVENT_TICK:
            state->ticks = (state->ticks + 1) % 8;
			if (state->game_mode == ROLL_ANIMATION || state->game_mode == YOLO_ANIMATION) _yolo_dice_face_update_lcd(state);
            else if (state->ticks % 4 == 0) _yolo_dice_face_update_lcd(state);
			break;
        case EVENT_LIGHT_BUTTON_DOWN:
            //suppress light
            break;
        
        case EVENT_MODE_BUTTON_UP:
            
            switch(state->game_mode) { 
                
                case INTRO:
                    movement_move_to_next_face();
					
					if (!state->game_active) {
						game_reset(state);
					}
                    
                    break;
				case BONUS_ROLL:
                case ROLL:
                    if (state->roll_count < 3) roll(state);
                    break;
				case BONUS_SCORE:
                case SCORE:
                    state->game_mode=ROLL;
                    break;
                case SET:
                case RUN:
                case UPPER:
                    state->game_mode=SCORE;
                    break;
                case MENU:
                    // movement_move_to_next_face();
                    state->game_mode = ROLL;
                    break;
                case GAME_OVER:
                    game_reset(state);
                    state->game_mode = INTRO;
                    break;
            }
            
            cursor_reset(state);
            _yolo_dice_face_update_lcd(state);
            
            break;
        case EVENT_MODE_LONG_PRESS:
            switch(state->game_mode) { 
                
                case INTRO:
					if (!state->game_active) {
						game_reset(state);
					}
                    movement_move_to_face(0);
                    break;
                case ROLL:
                case SCORE:
                case SET:
                case RUN:

                case UPPER:
                    state->game_mode = MENU;
                    break;
                case MENU:
                    state->game_mode = ROLL;
                    break;
                case GAME_OVER:
				    game_reset(state);
                    state->game_mode = INTRO;
                    movement_move_to_face(0);
                    break;
        }
            
            _yolo_dice_face_update_lcd(state);
            break;
            
        
        case EVENT_LIGHT_BUTTON_UP:
            
            switch(state->game_mode) { 
                
                case INTRO:
                    movement_request_tick_frequency(8);
					if (!state->game_active) {
						game_reset(state);
					}
                    state->game_mode = ROLL;
                    cursor_reset(state);
                    break;
				case BONUS_ROLL:
					cursor_next(state);
					if (state->roll_count == 3) {
                        state->game_mode = BONUS_SCORE;
                        cursor_reset(state);
                    }
                    break;
                case ROLL:
					cursor_next(state);
                    if (state->roll_count == 3) {
						if (state->rounds_remaining > 0) {
							state->game_mode = SCORE;
						} else { 
						state->game_mode = BONUS_SCORE;
						}
                        cursor_reset(state);
                    }
                    break;
                case BONUS_SCORE:
                case SCORE:
                case SET:
                case RUN:
                case UPPER:
                    cursor_next(state);
                    break;
                case MENU:
                    movement_move_to_face(0);
                    break;
                case GAME_OVER:
                    game_reset(state);
                    state->game_mode = INTRO;
                    break;
        }
            _yolo_dice_face_update_lcd(state);
            
            break;
        case EVENT_ALARM_LONG_PRESS:
            switch(state->game_mode) { 
                
                case INTRO:
					if (!state->game_active) {
						game_reset(state);
					}
                    movement_request_tick_frequency(8);
                    state->game_mode = ROLL;
                    cursor_reset(state);
                    break;
				case BONUS_ROLL:
                    if (state->roll_count==0){
                        roll(state);
                        cursor_reset(state);
                    
                    } else {
                        state->game_mode = BONUS_SCORE;
                        cursor_reset(state);
                    }
                    break;
                case ROLL:
                    if (state->roll_count==0){
                        roll(state);
                        cursor_reset(state);
                    
                    } else {
						state->game_mode = SCORE;
						if (state->rounds_remaining == 0) { 
							state->game_mode = BONUS_SCORE;
						}
                        cursor_reset(state);
                    }
                    break;
				case BONUS_SCORE:
                case SCORE:
                case SET:
                case RUN:
                case UPPER:
                    break;
                case MENU:
                    // game_reset(state);
                    state->game_mode = GAME_OVER;
                    break;
                case GAME_OVER:
                    game_reset(state);
                    state->game_mode = INTRO;
                    break;
            }
            _yolo_dice_face_update_lcd(state);
            
            break;
        case EVENT_ALARM_BUTTON_UP:
            switch(state->game_mode) { 
                case INTRO:
					if (!state->game_active) {
						game_reset(state);
					}
                    movement_request_tick_frequency(8);
                    state->game_mode = ROLL;
                    cursor_reset(state);
                    break;
				case BONUS_ROLL:
					switch (state->roll_count) {
                        
                        case 0:
                            break;
                        case 1:
                        case 2:
                            if (state->cursor_position < state->dice_held) {
                                unhold(state, state->cursor_position);
                                if (state->cursor_position == state->dice_held) cursor_next(state);
                                
                            } else {
                                hold(state, state->cursor_position-1);
                                cursor_next(state);
                            }
                            break;
                        case 3:
                            state->game_mode = BONUS_SCORE;
                            cursor_reset(state);
                            break;
                        
                        
                    }
                    


                    break;
                case ROLL:
                    
                    switch (state->roll_count) {
                        
                        case 0:
                            break;
                        case 1:
                        case 2:
                            if (state->cursor_position < state->dice_held) {
                                unhold(state, state->cursor_position);
                                if (state->cursor_position == state->dice_held) cursor_next(state);
                                
                            } else {
                                hold(state, state->cursor_position-1);
                                cursor_next(state);
                            }
                            break;
                        case 3:
							state->game_mode = SCORE;
							if (state->rounds_remaining == 0) { 
								state->game_mode = BONUS_SCORE;
							}
							
                            cursor_reset(state);
                            break;
                        
                        
                    }
                    


                    break;
					
					
                case BONUS_SCORE:
                case SCORE:
                    if (state->rounds_remaining == 0) {
                        state->bonus_score+=state->d_max_bonus;
                        next_turn(state);
                        
                    } else {
                        switch(state->cursor_position) {
                            // ones
                            case 0:
                                state->game_mode = UPPER;
                                cursor_reset(state);
                                break;
                            //twos
                            case 1:
                                
                                state->game_mode = SET;
                                cursor_reset(state);
                                break;
                                
                            case 2:
                                state->game_mode = RUN;
                                cursor_reset(state);
                                break;
                                
                            case 3:
                                write_score(state, FULL_HOUSE);
                                break;
                                
                            case 4:
                                write_score(state, CHANCE);
                                break;
                                
                            case 5:
                                write_score(state, YOLO);
                                break;
                    }
                    }
                    break;
                    
                case SET:
                    if (state->cursor_position == 0) write_score(state, SET_THREE);
                    else write_score(state, SET_FOUR);
                    break;
                
                case RUN:
                    if (state->cursor_position == 0) write_score(state, SHORT_RUN);
                    else write_score(state, LONG_RUN);
                    break;
                

                    
                case UPPER:
                    write_score(state, state->cursor_position);
                    break;
                    
                case MENU:

                    break;
                case GAME_OVER:
                    game_reset(state);
                    state->game_mode = INTRO;
                    break;
            
            }
            _yolo_dice_face_update_lcd(state);
            break;
        case EVENT_TIMEOUT:
            // Your watch face will receive this event after a period of inactivity. If it makes sense to resign,
            // you may uncomment this line to move back to the first watch face in the list:
            state->game_mode = INTRO;
            movement_request_tick_frequency(1);
            
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            // If you did not resign in EVENT_TIMEOUT, you can use this event to update the display once a minute.
            // Avoid displaying fast-updating values like seconds, since the display won't update again for 60 seconds.
            //watch_set_indicator(WATCH_INDICATOR_PM):
            // You should also consider starting the tick animation, to show the wearer that this is sleep mode:
            //watch_start_tick_animation(500);
            break;
        default:
            // Movement's default loop handler will step in for any cases you don't handle above:
            // * EVENT_LIGHT_BUTTON_DOWN lights the LED
            // * EVENT_MODE_BUTTON_UP moves to the next watch face in the list
            // * EVENT_MODE_LONG_PRESS returns to the first watch face (or skips to the secondary watch face, if configured)
            // You can override any of these behaviors by adding a case for these events to this switch statement.
            return movement_default_loop_handler(event, settings);
    }

    // return true if the watch can enter standby mode. Generally speaking, you should always return true.
    // Exceptions:
    //  * If you are displaying a color using the low-level watch_set_led_color function, you should return false.
    //  * If you are sounding the buzzer using the low-level watch_set_buzzer_on function, you should return false.
    // Note that if you are driving the LED or buzzer using Movement functions like movement_illuminate_led or
    // movement_play_alarm, you can still return true. This guidance only applies to the low-level watch_ functions.
    return true;
}

void yolo_dice_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

