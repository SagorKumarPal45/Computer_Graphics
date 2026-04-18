/*
 * histogram3d.h – Modern 3D histogram
 * Fixed: removed <dos.h>, renamed LEFT/RIGHT/TOP/BOTTOM macros to HG_ prefix,
 *        removed int left/right/top variable names that shadowed macros.
 * Modern: rich color front/top/side faces, animated grow-up bars, Office palette.
 */
#ifndef HISTOGRAM3D_H
#define HISTOGRAM3D_H

#define HG_LEFT   100
#define HG_RIGHT  900
#define HG_TOP     80
#define HG_BOTTOM 520
#define HIST_TICKS  5

#include <graphics.h>
#include <cstdio>
/* <dos.h> removed – delay() is already provided by <graphics.h> / WinBGIm */

/* Rich modern colours for the 3D faces */
#define HG_COL_FRONT    BLUE
#define HG_COL_TOP      LIGHTBLUE
#define HG_COL_SIDE     CYAN
#define HG_COL_OUTLINE  BLACK
#define HG_COL_GRID     LIGHTGRAY
#define HG_COL_AXIS     DARKGRAY
#define HG_COL_LABEL    BLACK

inline void drawSmooth3DBar(int bLeft, int bTop, int bRight, int bBottom, int depth)
{
    /* Front face */
    setfillstyle(SOLID_FILL, HG_COL_FRONT);
    bar(bLeft, bTop, bRight, bBottom);

    /* Top face (isometric parallelogram) */
    int topPoly[8] = {
        bLeft,          bTop,
        bLeft  + depth, bTop   - depth,
        bRight + depth, bTop   - depth,
        bRight,         bTop
    };
    setfillstyle(SOLID_FILL, HG_COL_TOP);
    fillpoly(4, topPoly);

    /* Right side face */
    int sidePoly[8] = {
        bRight,         bTop,
        bRight + depth, bTop    - depth,
        bRight + depth, bBottom - depth,
        bRight,         bBottom
    };
    setfillstyle(SOLID_FILL, HG_COL_SIDE);
    fillpoly(4, sidePoly);

    /* Outline */
    setcolor(HG_COL_OUTLINE);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
    line(bLeft,          bTop,           bLeft  + depth, bTop   - depth);
    line(bRight,         bTop,           bRight + depth, bTop   - depth);
    line(bRight,         bBottom,        bRight + depth, bBottom - depth);
    line(bLeft  + depth, bTop   - depth, bRight + depth, bTop   - depth);
    line(bRight + depth, bTop   - depth, bRight + depth, bBottom - depth);
    line(bRight,         bTop,           bRight,         bBottom);
}

inline void drawHistogram3D(int data[], int n)
{
    setbkcolor(WHITE);
    cleardevice();
    const int xOff = ((getmaxx() + 1) - 1000) / 2;
    const int yOff = ((getmaxy() + 1) - 600) / 2;
    const int left = HG_LEFT + xOff;
    const int right = HG_RIGHT + xOff;
    const int top = HG_TOP + yOff;
    const int bottom = HG_BOTTOM + yOff;

    /* ── Office-style title header ──────────────────────────────── */
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, getmaxx() + 1, 68);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 8, 68);
    setbkcolor(BLUE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    setcolor(WHITE);
    int tw = textwidth((char*)"3D HISTOGRAM");
    outtextxy(((getmaxx() + 1) - tw) / 2, 16, (char*)"3D HISTOGRAM");
    setbkcolor(WHITE);
    setcolor(LIGHTBLUE);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(0, 70, getmaxx() + 1, 70);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    /* ── Grid ──────────────────────────────────────────────────── */
    setcolor(HG_COL_GRID);
    for (int gx = left; gx <= right; gx += 80)
        line(gx, top, gx, bottom);
    for (int gy = top; gy <= bottom; gy += 80)
        line(left, gy, right, gy);

    /* ── Axes ──────────────────────────────────────────────────── */
    setcolor(HG_COL_AXIS);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(left - 10, bottom + 10, right + 10, bottom + 10);
    line(left - 10, bottom + 10, left  - 10, top    - 10);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    /* Safety: at least one data point */
    if (n <= 0) return;

    for (int i = 0; i < n; i++) {
        if (data[i] < 0) {
            setcolor(RED);
            settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
            outtextxy(xOff + 190, yOff + 300, (char*)"Histogram cannot use negative values.");
            setcolor(DARKGRAY);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
            outtextxy(xOff + 250, yOff + 330, (char*)"Please enter only positive values.");
            return;
        }
    }

    int maxVal = data[0];
    for (int i = 1; i < n; i++)
        if (data[i] > maxVal) maxVal = data[i];
    if (maxVal <= 0) maxVal = 1;

    float barWidth = (right - left) / (float)n;
    float scale    = (bottom - top) / (float)maxVal;

    /* ── Y-axis tick labels ─────────────────────────────────────── */
    char buf[32];
    setcolor(HG_COL_LABEL);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    for (int i = 0; i <= HIST_TICKS; i++) {
        int val = (maxVal * i) / HIST_TICKS;
        int py  = bottom - (int)(val * scale);
        line(left - 15, py, left - 10, py);
        sprintf(buf, "%d", val);
        outtextxy(left - 50, py - 5, buf);
    }

    /* ── Animated bar growth ───────────────────────────────────── */
    const int steps    = 24;
    const int barDepth = 8;

    for (int step = 0; step <= steps; step++)
    {
        for (int i = 0; i < n; i++) {
            int bleft  = left + (int)(i * barWidth) + 10;
            int bright = left + (int)((i + 1) * barWidth) - 10;
            int h      = (int)(data[i] * scale * step / steps);
            int btop   = bottom - h;

            drawSmooth3DBar(bleft, btop, bright, bottom, barDepth);

            if (step == steps) {
                /* Value label on top */
                setcolor(HG_COL_LABEL);
                settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
                sprintf(buf, "%d", data[i]);
                outtextxy((bleft + bright) / 2 - 8, btop - 22, buf);

                /* X-axis label */
                sprintf(buf, "V%d", i + 1);
                outtextxy((bleft + bright) / 2 - 5, bottom + 14, buf);
            }
        }
        if (step < steps) delay(80);
    }
}

#endif