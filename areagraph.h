/*
 * areagraph.h – Smooth modern 3D area graph
 * Office-style header, smooth spline curve with 3D depth effect.
 */
#ifndef AREAGRAPH_H
#define AREAGRAPH_H

#define AG_LEFT    100
#define AG_RIGHT   900
#define AG_TOP      80
#define AG_BOTTOM  520
#define AREA_TICKS   6

#include <graphics.h>
#include <cstdio>
#include <cmath>

inline float areaCatmull(float p0, float p1, float p2, float p3, float t)
{
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
                  (-p0 + p2) * t +
                  (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                  (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

inline void drawAreaGraph(int data[], int n)
{
    setbkcolor(WHITE);
    cleardevice();
    const int xOff = ((getmaxx() + 1) - 1000) / 2;
    const int yOff = ((getmaxy() + 1) - 600) / 2;
    const int left = AG_LEFT + xOff;
    const int right = AG_RIGHT + xOff;
    const int top = AG_TOP + yOff;
    const int bottom = AG_BOTTOM + yOff;

    /* ── Office-style title header ─────────────────────────────── */
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, getmaxx() + 1, 68);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 8, 68);
    setbkcolor(BLUE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 3);
    setcolor(WHITE);
    int tw = textwidth((char*)"SMOOTH 3D AREA GRAPH");
    outtextxy(((getmaxx() + 1) - tw) / 2, 16, (char*)"SMOOTH 3D AREA GRAPH");
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

    /* Validate values and guard zero span */
    int maxVal = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] < 0) {
            setcolor(RED);
            settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
            outtextxy(xOff + 185, yOff + 300, (char*)"Area graph cannot use negative values.");
            setcolor(DARKGRAY);
            settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
            outtextxy(xOff + 215, yOff + 330, (char*)"Please enter only non-negative values.");
            return;
        }
        if (data[i] > maxVal) maxVal = data[i];
    }
    if (maxVal == 0) maxVal = 1;

    float ys = (bottom - top) / (float)maxVal;
    char buf[50];

    /* ── Y-axis tick labels ─────────────────────────────────────── */
    setcolor(BLACK);
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    for (int i = 0; i <= AREA_TICKS; i++) {
        int val = (maxVal * i) / AREA_TICKS;
        int py  = bottom - (int)(val * ys);
        line(left - 15, py, left - 10, py);
        sprintf(buf, "%d", val);
        outtextxy(left - 50, py - 5, buf);
    }

    const int depthX = 16;
    const int depthY = 12;

    /* Single point: simple 3D slab */
    if (n == 1) {
        int y1 = (int)(bottom - data[0] * ys);
        int pFront[8] = {
            left + 20, y1,
            right - 20, y1,
            right - 20, bottom,
            left + 20, bottom
        };
        int pBack[8] = {
            left + 20 + depthX, y1 - depthY,
            right - 20 + depthX, y1 - depthY,
            right - 20 + depthX, bottom - depthY,
            left + 20 + depthX, bottom - depthY
        };
        setfillstyle(SOLID_FILL, BLUE);
        fillpoly(4, pBack);
        setfillstyle(SOLID_FILL, MAGENTA);
        fillpoly(4, pFront);
        setcolor(LIGHTBLUE);
        line(left + 20, y1, left + 20 + depthX, y1 - depthY);
        line(right - 20, y1, right - 20 + depthX, y1 - depthY);
        sprintf(buf, "%d", data[0]);
        setcolor(BLACK);
        outtextxy((left + right) / 2 - 10, y1 - 20, buf);
        return;
    }

    float xs = (right - left) / (float)(n - 1);
    const int samplesPerSeg = 12;
    const int maxSamples = 900;
    int sxPts[maxSamples];
    int syPts[maxSamples];
    int sCount = 0;

    for (int i = 0; i < n - 1 && sCount < maxSamples; i++) {
        float p0 = (float)data[(i == 0) ? 0 : i - 1];
        float p1 = (float)data[i];
        float p2 = (float)data[i + 1];
        float p3 = (float)data[(i + 2 < n) ? i + 2 : n - 1];

        for (int s = 0; s < samplesPerSeg && sCount < maxSamples; s++) {
            float t = s / (float)samplesPerSeg;
            float fx = left + (i + t) * xs;
            float fyVal = areaCatmull(p0, p1, p2, p3, t);
            if (fyVal < 0.0f) fyVal = 0.0f;
            int fy = (int)(bottom - fyVal * ys + 0.5f);
            if (fy < top) fy = top;
            if (fy > bottom) fy = bottom;
            sxPts[sCount] = (int)(fx + 0.5f);
            syPts[sCount] = fy;
            sCount++;
        }
    }

    if (sCount < maxSamples) {
        sxPts[sCount] = right;
        syPts[sCount] = (int)(bottom - data[n - 1] * ys + 0.5f);
        if (syPts[sCount] < top) syPts[sCount] = top;
        if (syPts[sCount] > bottom) syPts[sCount] = bottom;
        sCount++;
    }

    int polyFront[2 * (maxSamples + 2)];
    int polyBack[2 * (maxSamples + 2)];
    int pFront = 0;
    int pBack = 0;

    polyBack[pBack++] = left + depthX;
    polyBack[pBack++] = bottom - depthY;
    for (int i = 0; i < sCount; i++) {
        polyBack[pBack++] = sxPts[i] + depthX;
        polyBack[pBack++] = syPts[i] - depthY;
    }
    polyBack[pBack++] = right + depthX;
    polyBack[pBack++] = bottom - depthY;

    polyFront[pFront++] = left;
    polyFront[pFront++] = bottom;
    for (int i = 0; i < sCount; i++) {
        polyFront[pFront++] = sxPts[i];
        polyFront[pFront++] = syPts[i];
    }
    polyFront[pFront++] = right;
    polyFront[pFront++] = bottom;

    /* Back area to create depth */
    setfillstyle(SOLID_FILL, BLUE);
    fillpoly(pBack / 2, polyBack);

    /* Front area */
    setfillstyle(SOLID_FILL, MAGENTA);
    fillpoly(pFront / 2, polyFront);

    /* Left and right side faces */
    int sideLeft[8] = {
        left, bottom,
        left, syPts[0],
        left + depthX, syPts[0] - depthY,
        left + depthX, bottom - depthY
    };
    int sideRight[8] = {
        right, bottom,
        right, syPts[sCount - 1],
        right + depthX, syPts[sCount - 1] - depthY,
        right + depthX, bottom - depthY
    };
    setfillstyle(SOLID_FILL, LIGHTMAGENTA);
    fillpoly(4, sideLeft);
    fillpoly(4, sideRight);

    /* Connector lines between front and back edges */
    setcolor(LIGHTBLUE);
    line(left, syPts[0], left + depthX, syPts[0] - depthY);
    line(right, syPts[sCount - 1], right + depthX, syPts[sCount - 1] - depthY);

    /* Smooth top lines */
    setcolor(BLUE);
    for (int i = 1; i < sCount; i++) {
        line(sxPts[i - 1], syPts[i - 1], sxPts[i], syPts[i]);
        line(sxPts[i - 1], syPts[i - 1] - 1, sxPts[i], syPts[i] - 1);
        line(sxPts[i - 1] + depthX, syPts[i - 1] - depthY,
             sxPts[i] + depthX, syPts[i] - depthY);
    }

    /* Data point indicators on the front curve */
    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    for (int i = 0; i < n; i++) {
        int px = (int)(left + i * xs + 0.5f);
        int py = (int)(bottom - data[i] * ys + 0.5f);
        if (py < top) py = top;
        if (py > bottom) py = bottom;

        setcolor(WHITE);
        setfillstyle(SOLID_FILL, WHITE);
        fillellipse(px, py, 4, 4);
        setcolor(BLUE);
        circle(px, py, 4);

        sprintf(buf, "%d", data[i]);
        setcolor(BLACK);
        outtextxy(px - 8, py - 18, buf);
    }
}

#endif
