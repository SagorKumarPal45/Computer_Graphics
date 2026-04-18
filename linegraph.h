/*
 * linegraph.h – Modern 4-quadrant line graph
 * Fixed: LEFT/RIGHT/TOP/BOTTOM wrapped in #ifndef guards
 * Modern: deep-blue grid, white axis labels, animated Bresenham lines,
 *         red dot markers with glow ring, Office-blue accent lines.
 */
#ifndef LINEGRAPH_H
#define LINEGRAPH_H

#ifndef LG_LEFT
#define LG_LEFT   100
#endif
#ifndef LG_RIGHT
#define LG_RIGHT  900
#endif
#ifndef LG_TOP
#define LG_TOP     80
#endif
#ifndef LG_BOTTOM
#define LG_BOTTOM 520
#endif

#define LINE_TICKS        6
#define LINE_ANIM_SPEED   20      /* higher = fewer delay() calls */
#define CIRCLE_ANIM_DELAY  1
#define VERTEX_PAUSE_MS   10

#include <graphics.h>
#include <cstdio>
#include <cmath>
#include "algorithms.h"

/* Modern palette for the line graph */
#define LG_COL_BG        WHITE
#define LG_COL_GRID      LIGHTGRAY
#define LG_COL_AXIS      DARKGRAY
#define LG_COL_LINE      BLUE
#define LG_COL_DOT       RED
#define LG_COL_LABEL     BLACK
#define LG_COL_QLABEL    DARKGRAY
#define LG_COL_TITLE     BLACK

inline void linegraphWorldToScreen(int wx, int wy, int spanX, int spanY,
    int ox, int oy, float half_w, float half_h, int& sx, int& sy)
{
    sx = ox + (int)floor((wx / (float)spanX) * half_w + 0.5f);
    sy = oy - (int)floor((wy / (float)spanY) * half_h + 0.5f);
}

inline void drawLineGraph(int x[], int y[], int n)
{
    setbkcolor(LG_COL_BG);
    cleardevice();
    const int xOff = ((getmaxx() + 1) - 1000) / 2;
    const int yOff = ((getmaxy() + 1) - 600) / 2;
    const int gLeft = LG_LEFT + xOff;
    const int gRight = LG_RIGHT + xOff;
    const int gTop = LG_TOP + yOff;
    const int gBottom = LG_BOTTOM + yOff;

    const int ox     = (gLeft + gRight)  / 2;
    const int oy     = (gTop  + gBottom) / 2;
    const float hw   = (gRight  - gLeft)   / 2.0f;
    const float hh   = (gBottom - gTop)    / 2.0f;

    /* ── Title bar (Office-style mini header) ─────────────────────── */
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, getmaxx() + 1, 68);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 8, 68);
    setbkcolor(BLUE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    setcolor(WHITE);
    int tw = textwidth((char*)"LINE GRAPH (4 Quadrants)");
    outtextxy(((getmaxx() + 1) - tw) / 2, 16, (char*)"LINE GRAPH (4 Quadrants)");
    setbkcolor(LG_COL_BG);
    setcolor(LIGHTBLUE);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(0, 70, getmaxx() + 1, 70);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    /* Compute spans */
    int spanX = 1, spanY = 1;
    for (int i = 0; i < n; i++) {
        int ax = x[i] >= 0 ? x[i] : -x[i];
        int ay = y[i] >= 0 ? y[i] : -y[i];
        if (ax > spanX) spanX = ax;
        if (ay > spanY) spanY = ay;
    }

    /* ── Grid lines ──────────────────────────────────────────────── */
    setcolor(LG_COL_GRID);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
    for (int k = -LINE_TICKS; k <= LINE_TICKS; k++) {
        int gx, gy_d;
        linegraphWorldToScreen((spanX * k) / LINE_TICKS, 0,
            spanX, spanY, ox, oy, hw, hh, gx, gy_d);
        line(gx, gTop, gx, gBottom);

        int gx_d, gy;
        linegraphWorldToScreen(0, (spanY * k) / LINE_TICKS,
            spanX, spanY, ox, oy, hw, hh, gx_d, gy);
        line(gLeft, gy, gRight, gy);
    }

    /* ── Axes ────────────────────────────────────────────────────── */
    setcolor(LG_COL_AXIS);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(gLeft - 12, oy, gRight + 12, oy);
    line(ox, gTop - 12, ox, gBottom + 12);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    char buf[50];

    /* ── Tick labels ─────────────────────────────────────────────── */
    setcolor(LG_COL_LABEL);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    for (int k = -LINE_TICKS; k <= LINE_TICKS; k++) {
        int px, py0;
        linegraphWorldToScreen((spanX * k) / LINE_TICKS, 0,
            spanX, spanY, ox, oy, hw, hh, px, py0);
        line(px, oy - 4, px, oy + 4);
        if (k != 0) {
            sprintf(buf, "%d", (spanX * k) / LINE_TICKS);
            outtextxy(px - textwidth((char*)buf) / 2, oy + 8, buf);
        }

        int px0, py;
        linegraphWorldToScreen(0, (spanY * k) / LINE_TICKS,
            spanX, spanY, ox, oy, hw, hh, px0, py);
        line(px0 - 4, py, px0 + 4, py);
        if (k != 0) {
            sprintf(buf, "%d", (spanY * k) / LINE_TICKS);
            outtextxy(px0 - 8 - textwidth((char*)buf), py - 5, buf);
        }
    }

    /* Axis direction labels */
    setcolor(BLUE);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    outtextxy(gRight - 30, oy - 16, (char*)"+X");
    outtextxy(ox + 6,        gTop + 4, (char*)"+Y");

    /* Quadrant labels */
    setcolor(LG_COL_QLABEL);
    outtextxy(ox + (int)(hw * 0.55f), oy - (int)(hh * 0.55f), (char*)"I");
    outtextxy(ox - (int)(hw * 0.63f), oy - (int)(hh * 0.55f), (char*)"II");
    outtextxy(ox - (int)(hw * 0.70f), oy + (int)(hh * 0.48f), (char*)"III");
    outtextxy(ox + (int)(hw * 0.48f), oy + (int)(hh * 0.48f), (char*)"IV");

    /* ── Animated data line + markers ───────────────────────────── */
    setcolor(LG_COL_LINE);
    for (int i = 0; i < n; i++) {
        int px, py;
        linegraphWorldToScreen(x[i], y[i], spanX, spanY, ox, oy, hw, hh, px, py);

        if (i > 0) {
            int px2, py2;
            linegraphWorldToScreen(x[i-1], y[i-1], spanX, spanY, ox, oy, hw, hh, px2, py2);
            /* Draw 3-pixel thick line manually */
            bresenhamLineAnimated(px2, py2, px, py, BLUE, LINE_ANIM_SPEED);
            bresenhamLineAnimated(px2+1, py2, px+1, py, BLUE, LINE_ANIM_SPEED);
        }

        /* Outer glow ring */
        bresenhamCircleAnimated(px, py, 8, LIGHTBLUE, 0);
        /* Inner filled dot */
        bresenhamCircleAnimated(px, py, 5, LG_COL_DOT, CIRCLE_ANIM_DELAY);

        /* Label */
        setcolor(LG_COL_LABEL);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        sprintf(buf, "(%d,%d)", x[i], y[i]);
        outtextxy(px + 10, py - 16, buf);
        setcolor(LG_COL_LINE);

        delay(VERTEX_PAUSE_MS);
    }
}

#endif
