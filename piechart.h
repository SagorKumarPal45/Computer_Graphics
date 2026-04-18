/*
 * piechart.h – Modern 2D animated pie chart
 * Office-style header, clean legend panel, animated slice reveal,
 * callout lines via Bresenham and percentage labels.
 */
#ifndef PIECHART_H
#define PIECHART_H

#include <graphics.h>
#include <cmath>
#include <cstdio>
#include "algorithms.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* 10-colour palette for slices */
static const int PIE_COLORS[10] = {
    BLUE, RED, GREEN, CYAN, MAGENTA,
    YELLOW, LIGHTBLUE, LIGHTRED, LIGHTGREEN, BROWN
};

inline void drawPieChart2D(int data[], int n)
{
    setbkcolor(WHITE);
    cleardevice();
    const int xOff = ((getmaxx() + 1) - 1000) / 2;
    const int yOff = ((getmaxy() + 1) - 600) / 2;

    /* ── Office-style title header ─────────────────────────────── */
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, getmaxx() + 1, 68);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 8, 68);
    setbkcolor(BLUE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    setcolor(WHITE);
    int tw = textwidth((char*)"2D PIE CHART");
    outtextxy(((getmaxx() + 1) - tw) / 2, 16, (char*)"2D PIE CHART");
    setbkcolor(WHITE);
    setcolor(LIGHTBLUE);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(0, 70, getmaxx() + 1, 70);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    if (n <= 0) {
        setcolor(BLACK);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
        outtextxy(xOff + 320, yOff + 300, (char*)"No data segments.");
        return;
    }

    /* Slightly left so callout labels on the right have room before legend panel */
    const int cx = xOff + 395;
    const int cy = yOff + 310;
    const int r  = 165;

    /* Validate values and compute sum */
    long long total = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] < 0) {
            setcolor(RED);
            settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
            outtextxy(xOff + 190, yOff + 300, (char*)"Pie chart cannot use negative values.");
            setcolor(DARKGRAY);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
            outtextxy(xOff + 225, yOff + 330, (char*)"Please enter only positive values.");
            return;
        }
        total += (long long)data[i];
    }
    if (total <= 0) {
        setcolor(BLACK);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
        outtextxy(xOff + 260, yOff + 290, (char*)"Sum of values must be > 0");
        return;
    }

    int  start = 0;
    char buf[64];

    for (int i = 0; i < n; i++) {
        int v = data[i];

        int angle;
        if (i == n - 1)
            angle = 360 - start;
        else
            angle = (int)((long long)v * 360LL / total);
        if (angle < 0) angle = 0;
        if (angle > 360 - start) angle = 360 - start;

        int endAng = start + angle;
        int col    = PIE_COLORS[i % 10];

        if (angle > 0) {
            /* 2D slice with a light reveal animation */
            for (int rr = 10; rr <= r; rr += 10) {
                setfillstyle(SOLID_FILL, col);
                pieslice(cx, cy, start, endAng, rr);
                delay(1);
            }
            setfillstyle(SOLID_FILL, col);
            pieslice(cx, cy, start, endAng, r);

            /* Callout line + label */
            int mid  = start + angle / 2;
            double rad = mid * M_PI / 180.0;

            /* BGI pieslice: angles CCW from 3 o'clock, 90° = up (screen y decreases).
               Use cy - sin(...) so callouts match the slice rim (not math y-up). */
            int ex = cx + (int)(cos(rad) * (double)r + 0.5);
            int ey = cy - (int)(sin(rad) * (double)r + 0.5);

            const int ext = 52;
            int lx = cx + (int)(cos(rad) * (double)(r + ext) + 0.5);
            int ly = cy - (int)(sin(rad) * (double)(r + ext) + 0.5);

            bresenhamLineAnimated(ex, ey, lx, ly, DARKGRAY, 2);

            int pct = (int)((long long)v * 100LL / total);
            if (pct < 0) pct = 0;
            if (pct > 100) pct = 100;

            sprintf(buf, "%d (%d%%)", data[i], pct);
            setcolor(BLACK);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
            int labW = textwidth((char*)buf);
            int lbx  = lx + (cos(rad) >= 0.0 ? 6 : -labW - 6);
            int lby  = ly - 6;
            outtextxy(lbx, lby, buf);

            delay(80);
        }

        start = endAng;
    }

    /* Clean outline ring */
    setcolor(DARKGRAY);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
    circle(cx, cy, r);

    /* Legend panel — placed well right of pie + callout labels (no overlap) */
    const int legX1 = xOff + 800;
    const int legX2 = xOff + 1080;
    const int legY1 = yOff + 140;
    const int legY2 = yOff + 480;
    setfillstyle(SOLID_FILL, LIGHTGRAY);
    bar(legX1, legY1, legX2, legY2);
    setcolor(DARKGRAY);
    rectangle(legX1, legY1, legX2, legY2);
    setfillstyle(SOLID_FILL, BLUE);
    bar(legX1, legY1, legX1 + 6, legY2);
    setbkcolor(LIGHTGRAY);
    setcolor(BLACK);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    outtextxy(legX1 + 20, legY1 + 12, (char*)"Legend");
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);

    int yLegend = legY1 + 46;
    for (int i = 0; i < n && i < 10; i++) {
        int v = data[i];
        int pct = (int)((long long)v * 100LL / total);
        char leg[64];
        setfillstyle(SOLID_FILL, PIE_COLORS[i % 10]);
        bar(legX1 + 20, yLegend, legX1 + 37, yLegend + 14);
        setcolor(DARKGRAY);
        rectangle(legX1 + 20, yLegend, legX1 + 37, yLegend + 14);
        sprintf(leg, "Value %d: %d (%d%%)", i + 1, v, pct);
        setcolor(BLACK);
        outtextxy(legX1 + 47, yLegend + 2, leg);
        yLegend += 24;
    }
    setbkcolor(WHITE);
}

/* Backward-compatible alias: intentionally renders the 2D pie chart. */
inline void drawPieChart3D(int data[], int n)
{
    drawPieChart2D(data, n);
}

#endif
