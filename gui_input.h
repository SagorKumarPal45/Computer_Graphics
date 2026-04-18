/*
 * gui_input.h – Modernized input boxes with full validation support
 * Added: positiveOnly mode (blocks '-' key for non-negative charts)
 *        Value clamping passed from caller via maxAbsVal
 */
#ifndef GUI_INPUT_H
#define GUI_INPUT_H

#include <graphics.h>
#include <conio.h>
#include <cstdio>
#include <string.h>
#include <stdlib.h>

/* Navigation return codes */
#define INPUT_NAV_CONFIRM 0
#define INPUT_NAV_PREV    1
#define INPUT_NAV_NEXT    2
#define INPUT_NAV_BACK    3
#define INPUT_NAV_MOUSE   4

/* Box geometry */
#define INPBOX_PAD_X  6
#define INPBOX_PAD_Y  5
#define INPBOX_H      26

/* Returns pixel width of the text-entry area */
inline int inputBoxWidthPx(void)
{
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    return textwidth((char*)"0000000000000");
}

/* True if (mx,my) is inside the input box drawn at (bx,by) */
inline bool pointInInputBox(int mx, int my, int bx, int by)
{
    int boxW = inputBoxWidthPx();
    return  mx >= bx - INPBOX_PAD_X &&
            mx <= bx + boxW + INPBOX_PAD_X &&
            my >= by - INPBOX_PAD_Y &&
            my <= by + INPBOX_H + INPBOX_PAD_Y;
}

/* Hit-test for Line / Scatter X-Y field pairs (matches main.cpp ix=262/555) */
inline int hitTestScatterXYFields(int mx, int my, int n, int rowY0, int rowH)
{
    for (int i = 0; i < n; i++) {
        int ry = rowY0 + i * rowH;
        if (pointInInputBox(mx, my, 262, ry)) return 2 * i;
        if (pointInInputBox(mx, my, 555, ry)) return 2 * i + 1;
    }
    return -1;
}

/* Hit-test for Histogram / Pie / Area value fields (matches main.cpp ix=310) */
inline int hitTestValueFields(int mx, int my, int n, int rowY0, int rowH)
{
    for (int i = 0; i < n; i++) {
        int ry = rowY0 + i * rowH;
        if (pointInInputBox(mx, my, 310, ry)) return i;
    }
    return -1;
}

/*
 * getStringInput – draws a styled input box and reads digits.
 *
 * positiveOnly : when true, the '-' key is ignored (for histogram/pie/scatter/area)
 * maxDigits    : maximum number of digits allowed (not counting sign)
 *
 * Returns one of the INPUT_NAV_* codes.
 */
inline int getStringInput(int x, int y, char* buffer, int max_len,
                          int* outClickX, int* outClickY,
                          bool positiveOnly = false,
                          int  maxDigits    = 4)
{
    int index = (int)strlen(buffer);
    if (index >= max_len) { index = max_len - 1; buffer[index] = '\0'; }

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    int boxW = textwidth((char*)"0000000000000");

    /* Outer accent border (blue), inner border (gray) */
    setcolor(BLUE);
    rectangle(x - INPBOX_PAD_X - 1, y - INPBOX_PAD_Y - 1,
              x + boxW + INPBOX_PAD_X + 1, y + INPBOX_H + INPBOX_PAD_Y + 1);
    setcolor(DARKGRAY);
    rectangle(x - INPBOX_PAD_X, y - INPBOX_PAD_Y,
              x + boxW + INPBOX_PAD_X, y + INPBOX_H + INPBOX_PAD_Y);

    for (;;)
    {
        /* Clear box interior */
        setfillstyle(SOLID_FILL, LIGHTCYAN);
        bar(x - INPBOX_PAD_X + 1, y - INPBOX_PAD_Y + 1,
            x + boxW + INPBOX_PAD_X - 1, y + INPBOX_H + INPBOX_PAD_Y - 1);

        /* Draw current text */
        setbkcolor(LIGHTCYAN);
        setcolor(BLACK);
        outtextxy(x, y + 2, buffer);

        /* Cursor underline */
        int curX = x + textwidth(buffer);
        setcolor(BLUE);
        line(curX, y + INPBOX_H - 2, curX + 8, y + INPBOX_H - 2);

        setbkcolor(WHITE);

        /* Wait for key or mouse */
        if (outClickX && outClickY)
        {
            while (!kbhit())
            {
                if (ismouseclick(WM_LBUTTONDOWN)) {
                    *outClickX = mousex();
                    *outClickY = mousey();
                    clearmouseclick(WM_LBUTTONDOWN);
                    return INPUT_NAV_MOUSE;
                }
                delay(5);
            }
        }

        char ch = (char)getch();

        /* Extended keys (arrow keys) */
        if (ch == (char)0 || ch == (char)224 || ch == (char)-32) {
            int c2 = getch();
            if (c2 == 72) return INPUT_NAV_PREV;   /* Up   */
            if (c2 == 80) return INPUT_NAV_NEXT;   /* Down */
            continue;
        }

        if (ch == 13)  return INPUT_NAV_CONFIRM;
        if (ch == 9)   return INPUT_NAV_NEXT;
        if (ch == 32)  return INPUT_NAV_BACK;

        /* Backspace / Delete */
        if (ch == 8 || ch == 127) {
            if (index > 0) { index--; buffer[index] = '\0'; }
            continue;
        }

        /* Leading minus – only if not positiveOnly */
        if (ch == '-' && index == 0 && !positiveOnly) {
            buffer[0] = '-'; buffer[1] = '\0'; index = 1; continue;
        }

        /* Digits – enforce maxDigits limit */
        if (ch >= '0' && ch <= '9') {
            /* Count digit characters already entered */
            int digitCount = 0;
            for (int k = 0; k < index; k++)
                if (buffer[k] >= '0' && buffer[k] <= '9') digitCount++;

            if (digitCount >= maxDigits) continue;  /* too many digits */

            /* Replace leading zero */
            if (index == 1 && buffer[0] == '0') {
                buffer[0] = ch; buffer[1] = '\0'; index = 1; continue;
            }
            if (index == 2 && buffer[0] == '-' && buffer[1] == '0') {
                buffer[1] = ch; buffer[2] = '\0'; index = 2; continue;
            }
            if (index < max_len - 1) { buffer[index++] = ch; buffer[index] = '\0'; }
            continue;
        }
    }
}

/*
 * getNumberInput – wraps getStringInput, returns a parsed and clamped integer.
 *
 * positiveOnly : pass true for Histogram, Pie, Scatter, Area
 * minVal / maxVal : clamped range (caller defines per chart)
 */
inline int getNumberInput(int x, int y, int* nav, const int* initialValue,
                          int* outClickX, int* outClickY,
                          bool positiveOnly = false,
                          int  minVal       = -9999,
                          int  maxVal_      = 9999,
                          int  maxDigits    = 4)
{
    char buffer[32];
    if (initialValue != NULL && *initialValue != 0)
        sprintf(buffer, "%d", *initialValue);
    else
        buffer[0] = '\0';

    *nav = getStringInput(x, y, buffer, (int)sizeof(buffer) - 1,
                          outClickX, outClickY, positiveOnly, maxDigits);

    int val = atoi(buffer);
    /* Clamp to caller-specified range */
    if (val < minVal) val = minVal;
    if (val > maxVal_) val = maxVal_;
    return val;
}

#endif
