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
#include "math_util.h"

#define INPUT_DISPLAY_X 290
#define INPUT_DISPLAY_Y 212
#define MINI_BOX_WIDTH 8
#define MINI_BOX_HEIGHT 8
#define INPUT_ROWS 2
#define INPUT_COLS 2

#define STICK_INPUT_X INPUT_DISPLAY_X - ((INPUT_COLS + 1) * (MINI_BOX_WIDTH))
#define STICK_INPUT_Y INPUT_DISPLAY_Y
#define STICK_BOX_LENGTH 2 * (MINI_BOX_HEIGHT + 1) + 1

#define KART_METRICS_X 64
#define KART_METRICS_Y 188
#define KART_METRICS_BOX_WIDTH 188
#define KART_METRICS_COL2 95

#define METRICS_NONE 0
#define METRICS_KART_ALIGNED 1  
#define METRICS_MAP_ALIGNED 2

struct buttonRender gButtonInfo[] = {
    {A_BUTTON, { 0, 192, 255 }, 1, 1,},
    {B_BUTTON, { 0, 255, 0 }, 0, 0,},
    {Z_TRIG, { 211, 211, 211 }, 0, 1,},
    {R_TRIG, { 211, 211, 211 }, 1, 0,},
};

static bool sKartMetricsMode = METRICS_KART_ALIGNED;
static bool sKartMetricsToggled = false;
static bool sDisplayInputs = true;
static bool sInputsToggled = false;

static void practice_format_f32(char* dst, f32 value, u8 whole_digits) {

//this formats a floating point value to a string
//it does is by breaking it into whole and fractional parts, 
//and then printing each part divided by a "." separator.
//whole digits specifies the minimum number of characters used to represent the whole number  
//which will result in padding with spaces if necessary (useful for aligning numbers)

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
    whole_digits--;
    while (whole_digits >= 1){
        if (whole < menu_pow(10,whole_digits)){
            dst[i] = ' ';
            i++;
        }
        whole_digits--;
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

static void practice_print_f32(s32 x, s32 y, char* label, f32 value, s8 whole_digits) {
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
    practice_format_f32(&line[i], value, whole_digits);
    //print the string
    debug_print_str2(x, y, line);
}

static void draw_input_display(void) {
    struct Controller* controller;
    u16 held;
    s32 i;
    s32 x;
    s32 y;
    s32 alpha;
    s32 stickX;
    s32 stickY;
    struct buttonRender* buttonInfo;

    if (!sDisplayInputs){
        return;
    }

    // handle time trial replay
    if ((gModeSelection == TIME_TRIALS) && (gActiveScreenMode == SCREEN_MODE_1P) && (D_8015F890 == 1)){
        controller = gControllerEight;
    } else {
        controller = gControllerOne;
    }

    held = controller->button;

    //draw the buttons
    for (i = 0; i < 4; i++){
        buttonInfo = &gButtonInfo[i];
        if (held & buttonInfo->button){
            alpha = 255;
        } else {
            alpha = 64;
        }
        x = (INPUT_DISPLAY_X + 1) + (buttonInfo->xPos * (MINI_BOX_WIDTH + 1));
        y = (INPUT_DISPLAY_Y + 1) + (buttonInfo->yPos * (MINI_BOX_HEIGHT + 1));
        gDisplayListHead = draw_box(gDisplayListHead, x, y, x + MINI_BOX_WIDTH, y + MINI_BOX_HEIGHT, buttonInfo->color[0],
                                    buttonInfo->color[1], buttonInfo->color[2], alpha);
    }

    //draw the analog stick
    gDisplayListHead = draw_box(gDisplayListHead, STICK_INPUT_X, STICK_INPUT_Y, STICK_INPUT_X + STICK_BOX_LENGTH,
                            STICK_INPUT_Y + STICK_BOX_LENGTH, 0, 0, 0, 64);
    stickX = STICK_INPUT_X + 7 + ((s32) ((f32) controller->rawStickX / 63.0f * 6.0f));
    stickY = STICK_INPUT_Y + 7 + ((s32) ((f32) controller->rawStickY / -63.0f * 6.0f));
    gDisplayListHead =
        draw_box(gDisplayListHead, stickX, stickY, stickX + 5, stickY + 5, 255,
                 255, 255, 255);
}

static void show_kart_metrics(void) {
    Player* player = gPlayerOne;
    f32 angle;
    s16 angle_n64;
    s32 x;
    s32 y;

    if (sKartMetricsMode == METRICS_NONE){
        return;
    }

    if (gPlayerCountSelection1 != 1) {
        return;
    }

    x = KART_METRICS_X;
    y = KART_METRICS_Y;

    gDisplayListHead = draw_box(gDisplayListHead, x, y, x + KART_METRICS_BOX_WIDTH,
                               y + 40, 0, 0, 0, 64);
    load_debug_font();

    //plot the player's position and angle
    // debug_print_str2 adds +20 to x/y internally.
    x -= 12;
    y -= 18;
    
    practice_print_f32(x, y, "X:", player->pos[0], 4);
    practice_print_f32(x, y + 10, "Y:", player->pos[1], 4);
    practice_print_f32(x, y + 20, "Z:", player->pos[2], 4);

    angle_n64 = player->rotation[1];
    angle = ((f32) (u16) angle_n64 / 65536.0f) * 360.0f;
    practice_print_f32(x, y + 30, "A:", angle, 4);

    //plot the player's velocity
    if (sKartMetricsMode == METRICS_KART_ALIGNED){
        practice_print_f32(x + KART_METRICS_COL2, y, " fwd:", player->velocity[2] * coss(angle_n64) - player->velocity[0] * sins(angle_n64), 1);
        practice_print_f32(x + KART_METRICS_COL2, y + 10, "rght:", -(player->velocity[2] * sins(angle_n64) + player->velocity[0] * coss(angle_n64)), 1);
        practice_print_f32(x + KART_METRICS_COL2, y + 20, "horz:", sqrtf((player->velocity[0] * player->velocity[0]) + (player->velocity[2] * player->velocity[2])), 1);
        practice_print_f32(x + KART_METRICS_COL2, y + 30, "vert:", player->velocity[1], 1);
    } else if (sKartMetricsMode == METRICS_MAP_ALIGNED){
        practice_print_f32(x + KART_METRICS_COL2, y, "  SX:", player->velocity[0], 1);
        practice_print_f32(x + KART_METRICS_COL2, y + 10, "  SZ:", player->velocity[2], 1);
        practice_print_f32(x + KART_METRICS_COL2, y + 20, "  SH:", player->speed, 1);
        practice_print_f32(x + KART_METRICS_COL2, y + 30, "  SY:", player->velocity[1], 1);
    }
    func_80057778();
}

void practice_render(void) {
    if (gPracticeMode) {
        if (!gIsGamePaused){
            if ((gControllerOne->buttonPressed & U_JPAD) == U_JPAD) {
                if (!sKartMetricsToggled){
                    sKartMetricsMode = (sKartMetricsMode + 1) % 3;
                    sKartMetricsToggled = true;
                }
            } else {
                sKartMetricsToggled = false;
            }
            if ((gControllerOne->buttonPressed & D_JPAD) == D_JPAD) {
                if (!sInputsToggled){
                    sDisplayInputs = !sDisplayInputs;
                    sInputsToggled = true;
                }
            } else {
                sInputsToggled = false;
            }
        }
        draw_input_display();
        show_kart_metrics();
    }
}
