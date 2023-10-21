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

#ifndef YOLO_DICE_FACE_H_
#define YOLO_DICE_FACE_H_

#include "movement.h"

/*
 * A DESCRIPTION OF YOUR WATCH FACE
 *
 * and a description of how use it
 *
 */

typedef enum {
    ONE = 0,             
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SET_THREE,
    SET_FOUR,
    FULL_HOUSE,
    SHORT_RUN,
    LONG_RUN,
    YOLO,
    CHANCE
} yolo_dice_names_t;

typedef enum {
    INTRO = 0,
	ROLL_ANIMATION,
	YOLO_ANIMATION,
    ROLL,
    SCORE,
    SET,
    RUN,
    UPPER,
    BONUS_SCORE,
	BONUS_ROLL,
    MENU,
    GAME_OVER
} yolo_dice_mode_t;

typedef struct {
    
    bool game_active;
    
    uint8_t roll_count;
    uint8_t cursor_position;
    uint8_t dice_held;
    
    // bool score_mode;
    int yolo_count;
    bool upper_bonus;
    int upper_score;
    int bonus_score;
	bool can_haz_yolo_bonus;
    bool score_categories [13];
    uint8_t dice_scores [13];
    uint8_t bonus_rounds;
    uint8_t rounds_remaining;
    uint8_t game_mode;
    int score;
    bool c_upper;
    bool c_runs;
    bool c_sets;
    uint8_t dice[5];
    uint8_t dice_count[6];
    int sum;
    bool d_set_three;
    bool d_set_four;
    bool d_yolo;
    bool d_short_run;
    bool d_long_run;
    bool d_full_house;

    uint8_t s_full_house;
    uint8_t s_set_three;
    uint8_t s_set_four;
    uint8_t s_short_run;
    uint8_t s_long_run;
    uint8_t s_yolo;
    uint8_t s_bonus;
    uint8_t d_max_upper;
    uint8_t d_max_bonus;
    int ticks;
	int ani_ticks;

    
    
} yolo_dice_state_t;

void yolo_dice_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr);
void yolo_dice_face_activate(movement_settings_t *settings, void *context);
bool yolo_dice_face_loop(movement_event_t event, movement_settings_t *settings, void *context);
void yolo_dice_face_resign(movement_settings_t *settings, void *context);
const char* string_int(int n);

#define yolo_dice_face ((const watch_face_t){ \
    yolo_dice_face_setup, \
    yolo_dice_face_activate, \
    yolo_dice_face_loop, \
    yolo_dice_face_resign, \
    NULL, \
})

#endif // YOLO_DICE_FACE_H_

