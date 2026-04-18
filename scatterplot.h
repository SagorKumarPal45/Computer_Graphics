/*
 * scatterplot.h – Modern animated scatter plot
 * Fixed: renamed LEFT/RIGHT/TOP/BOTTOM → SP_ prefix (no redefinition warnings)
 * Modern: Office-style header, animated circle markers with glow, grid, labels
 */
#ifndef SCATTERPLOT_H
#define SCATTERPLOT_H

#define SP_LEFT    100
#define SP_RIGHT   900
#define SP_TOP      80
#define SP_BOTTOM  520
#define SCATTER_TICKS 6

#include <graphics.h>
#include <cstdio>
#include "algorithms.h"

inline void drawScatterPlot(int x[], int y[], int n)
{
    setbkcolor(WHITE);
    cleardevice();
    const int xOff = ((getmaxx() + 1) - 1000) / 2;
    const int yOff = ((getmaxy() + 1) - 600) / 2;
    const int left = SP_LEFT + xOff;
    const int right = SP_RIGHT + xOff;
    const int top = SP_TOP + yOff;
    const int bottom = SP_BOTTOM + yOff;

    /* ── Office-style title header ─────────────────────────────── */
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, getmaxx() + 1, 68);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 8, 68);
    setbkcolor(BLUE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    setcolor(WHITE);
    int tw = textwidth((char*)"SCATTER PLOT");
    outtextxy(((getmaxx() + 1) - tw) / 2, 16, (char*)"SCATTER PLOT");
    setbkcolor(WHITE);
    setcolor(LIGHTBLUE);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(0, 70, getmaxx() + 1, 70);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    /* ── Grid ──────────────────────────────────────────────────── */
    setcolor(LIGHTGRAY);
    for (int gx = left; gx <= right; gx += 80) line(gx, top, gx, bottom);
    for (int gy = top;  gy <= bottom; gy += 80) line(left, gy, right, gy);

    /* ── Axes ──────────────────────────────────────────────────── */
    setcolor(DARKGRAY);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(left - 10, bottom + 10, right + 10, bottom + 10);
    line(left - 10, bottom + 10, left  - 10, top    - 10);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    if (n <= 0) return;

    int maxX = 0, maxY = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] < 0 || y[i] < 0) {
            setcolor(RED);
            settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
            outtextxy(xOff + 200, yOff + 300, (char*)"Scatter plot cannot use negative values.");
            setcolor(DARKGRAY);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
            outtextxy(xOff + 245, yOff + 330, (char*)"Please enter only non-negative X and Y values.");
            return;
        }
        if (x[i] > maxX) maxX = x[i];
        if (y[i] > maxY) maxY = y[i];
    }
    if (maxX < 1) maxX = 1;
    if (maxY < 1) maxY = 1;

    float xs = (right - left)   / (float)maxX;
    float ys = (bottom - top) / (float)maxY;
    char buf[50];

    /* ── Axis tick labels ───────────────────────────────────────── */
    setcolor(BLACK);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    for (int i = 0; i <= SCATTER_TICKS; i++) {
        int valX = (maxX * i) / SCATTER_TICKS;
        int px   = left + (int)(valX * xs);
        line(px, bottom + 10, px, bottom + 15);
        sprintf(buf, "%d", valX);
        outtextxy(px - 8, bottom + 20, buf);

        int valY = (maxY * i) / SCATTER_TICKS;
        int py   = bottom - (int)(valY * ys);
        line(left - 15, py, left - 10, py);
        sprintf(buf, "%d", valY);
        outtextxy(left - 50, py - 5, buf);
    }

    /* ── Axis direction labels ──────────────────────────────────── */
    setcolor(BLUE);
    outtextxy(right - 25, bottom + 10, (char*)"X");
    outtextxy(left  - 10, top + 4,     (char*)"Y");

    /* ── Animated data points (glow + dot) ─────────────────────── */
    for (int i = 0; i < n; i++) {
        int px = left   + (int)(x[i] * xs);
        int py = bottom - (int)(y[i] * ys);

        /* Outer glow */
        bresenhamCircleAnimated(px, py, 10, LIGHTBLUE, 0);
        /* Main dot */
        bresenhamCircleAnimated(px, py,  6, BLUE, 6);

        /* Cross-hair tick marks */
        setcolor(DARKGRAY);
        line(px, bottom + 10, px, bottom + 15);
        line(left - 15, py, left - 10, py);

        /* Label */
        setcolor(BLACK);
        settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
        sprintf(buf, "(%d,%d)", x[i], y[i]);
        outtextxy(px + 10, py - 16, buf);

        delay(120);
    }
}

#endif