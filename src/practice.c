#include <ultra64.h>
#include <macros.h>
#include <common_structs.h>
#include <defines.h>
#include <PR/os_cont.h>

#include "main.h"
#include "code_800029B0.h"
#include "menu_items.h"
#include "render_objects.h"
#include "practice.h"

#define PRACTICE_INPUT_X 247
#define PRACTICE_INPUT_Y 212
#define PRACTICE_DETAIL_X 62
#define PRACTICE_DETAIL_Y 170
#define PRACTICE_GLYPH_W 8
#define PRACTICE_DETAIL_BOX_LEFT 2
#define PRACTICE_DETAIL_BOX_RIGHT 180
#define PRACTICE_DETAIL_COL2 95


//input display
//colors of the buttons
static const s16 sInputColors[][3] = {
    { 0, 192, 255 },   { 0, 255, 0 },     { 255, 255, 255 }, { 255, 0, 0 },     { 192, 192, 192 }, { 192, 192, 192 },
    { 192, 192, 192 }, { 192, 192, 192 }, { 0, 0, 0 },       { 0, 0, 0 },       { 255, 255, 255 }, { 255, 255, 255 },
    { 255, 255, 0 },   { 255, 255, 0 },   { 255, 255, 0 },   { 255, 255, 0 },
};

//positions of the buttons
static const s16 sInputCoords[][2] = {
    { 3, 1 }, { 3, 2 }, { 5, 1 }, { 3, 0 }, { 1, 0 }, { 1, 2 }, { 0, 1 }, { 2, 1 },
    { 0, 0 }, { 0, 0 }, { 4, 0 }, { 6, 0 }, { 5, 0 }, { 5, 2 }, { 4, 1 }, { 6, 1 },
};

//this is a list of which padbuttons are used in the input display
static const u8 sInputUsed[] = { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1 };


//color of the analog stick
static const s16 sStickColors[][3] = {
    { 192, 192, 192 }, { 192, 192, 192 }, { 192, 192, 192 }, { 192, 192, 192 }, { 255, 255, 255 },
};

//position of analog stick 
static const s16 sStickCoords[][2] = {
    { 1, 0 }, { 1, 2 }, { 0, 1 }, { 2, 1 }, { 1, 1 },
};

static const u16 sStickBits[] = { U_JPAD, D_JPAD, L_JPAD, R_JPAD };

static bool sPracticeDetailsVisible = true;
static bool sInputDisplayWasEnabled = false;




static void practice_format_f32(char* dst, f32 value) {

//this formats a floating point value to a string
//it does is by breaking it into whole and fractional parts, 
//and then printing each part divided by a "." separator.

    s32 whole;
    s32 frac;
    s32 i;
    s32 len;
    s32 n;
    char digits[12];

    i = 0;
    whole = (s32) value;
    frac = (s32) ((value - (f32) whole) * 1000.0f);
    if (frac < 0) {
        frac = -frac;
    }

    if (whole == 0) {
        dst[i] = '0';
        i++;
    } else {
        n = whole;
        len = 0;
        while (n > 0) {
            digits[len] = (char) ('0' + (n % 10));
            len++;
            n /= 10;
        }
        while (len > 0) {
            len--;
            dst[i] = digits[len];
            i++;
        }
    }
    // I never said this would be pretty.
    dst[i] = '.';
    i++;
    dst[i] = (char) ('0' + ((frac / 100) % 10));
    i++;
    dst[i] = (char) ('0' + ((frac / 10) % 10));
    i++;
    dst[i] = (char) ('0' + (frac % 10));
    i++;
    dst[i] = '\0';
}

static void practice_print_f32(s32 x, s32 y, char* label, f32 value) {
    char line[20];
    s32 i;

    //add the x: / y: / z: / a: / etc.
    i = 0;
    while (label[i] != 0) {
        line[i] = label[i];
        i++;
    }
    //add the negative sign or space
    if (value < 0.0f) {
        line[i] = '-';
        value = -value;
    } else {
        line[i] = ' ';
    }
    i++;
    //prep the floating point value add to string
    practice_format_f32(&line[i], value);
    //print the string
    debug_print_str2(x, y, line);
}

static void practice_draw_input_display(void) {
    struct Controller* controller;
    u16 held;
    u16 bit;
    s32 i;
    s32 x;
    s32 y;
    s32 alpha;
    s32 stickX;
    s32 stickY;

    // handle time trial replay
    if ((gModeSelection == TIME_TRIALS) && (gActiveScreenMode == SCREEN_MODE_1P) && (D_8015F890 == 1)){
        controller = gControllerEight;
    } else {
        controller = gControllerOne;
    }

    held = controller->button;
    bit = 0x8000;

    gDisplayListHead = draw_box(gDisplayListHead, PRACTICE_INPUT_X, PRACTICE_INPUT_Y, PRACTICE_INPUT_X + 43,
                               PRACTICE_INPUT_Y + 16, 0, 0, 0, 64);

    //draw the buttons
    for (i = 0; i < 16; i++) {
        if (sInputUsed[i]) {
            if (held & bit) {
                alpha = 255;
            } else {
                alpha = 64;
            }
            x = PRACTICE_INPUT_X + (sInputCoords[i][0] * 6);
            y = PRACTICE_INPUT_Y + (sInputCoords[i][1] * 5);
            gDisplayListHead = draw_box(gDisplayListHead, x + 1, y + 1, x + 6, y + 5, sInputColors[i][0],
                                       sInputColors[i][1], sInputColors[i][2], alpha);
        }
        bit >>= 1;
    }

    //draw the analog stick
    //plot the 4 directions of the analog stick
    for (i = 0; i < 4; i++) {
        if (controller->stickDirection & sStickBits[i]) {
            alpha = 255;
        } else {
            alpha = 32;
        }
        x = PRACTICE_INPUT_X + (sStickCoords[i][0] * 6);
        y = PRACTICE_INPUT_Y + (sStickCoords[i][1] * 5);
        gDisplayListHead = draw_box(gDisplayListHead, x + 1, y + 1, x + 6, y + 5, sStickColors[i][0],
                                   sStickColors[i][1], sStickColors[i][2], alpha);
    }

    stickX = PRACTICE_INPUT_X + ((sStickCoords[4][0] * 6) + (s32) ((f32) controller->rawStickX / 127.0f * 6.0f));
    stickY = PRACTICE_INPUT_Y + ((sStickCoords[4][1] * 5) + (s32) ((f32) controller->rawStickY / -127.0f * 5.0f));
    gDisplayListHead =
        draw_box(gDisplayListHead, stickX + 1, stickY + 1, stickX + 6, stickY + 5, sStickColors[4][0],
                 sStickColors[4][1], sStickColors[4][2], 255);
}

static void practice_draw_details(void) {
    Player* player = gPlayerOne;
    f32 angle;
    s32 x;
    s32 y;

    if (gPlayerCountSelection1 != 1) {
        return;
    }

    x = PRACTICE_DETAIL_X;
    y = PRACTICE_DETAIL_Y;

    gDisplayListHead = draw_box(gDisplayListHead, x + PRACTICE_DETAIL_BOX_LEFT, y + 18, x + PRACTICE_DETAIL_BOX_RIGHT,
                               y + 58, 0, 0, 0, 64);
    load_debug_font();

    //plot the player's position and angle
    // debug_print_str2 adds +20 to x/y internally.
    x -= 10;
    
    practice_print_f32(x, y, "X:", player->pos[0]);
    practice_print_f32(x, y + 10, "Y:", player->pos[1]);
    practice_print_f32(x, y + 20, "Z:", player->pos[2]);
    angle = ((f32) (u16) player->rotation[1] / 65536.0f) * 360.0f;
    practice_print_f32(x, y + 30, "A:", angle);

    //plot the player's velocity
    practice_print_f32(x + PRACTICE_DETAIL_COL2, y, "SX:", player->velocity[0]);
    practice_print_f32(x + PRACTICE_DETAIL_COL2, y + 10, "SY:", player->velocity[1]);
    practice_print_f32(x + PRACTICE_DETAIL_COL2, y + 20, "SZ:", player->velocity[2]);
    practice_print_f32(x + PRACTICE_DETAIL_COL2, y + 30, "SA:", player->speed);

    func_80057778();
}

void practice_update(void) {
    //defaults to ensure first boot turns on the details display.
    //will not be kept on after first race - maintain setting.
    if (gInputDisplay) {
        if (!sInputDisplayWasEnabled) {
            sPracticeDetailsVisible = true;
        }
        sInputDisplayWasEnabled = true;
    } else {
        return;
    }

    if ((gDemoMode != 0) || (gIsGamePaused != 0)) {
        return;
    }


    //turn the display on/off with the left trigger
    if (gControllerOne->buttonPressed & L_TRIG) {
        sPracticeDetailsVisible = !sPracticeDetailsVisible;
    }
}

void practice_render(void) {
    if (gInputDisplay) {
        practice_draw_input_display();
        if (sPracticeDetailsVisible) {
            practice_draw_details();
        }
    }
}
