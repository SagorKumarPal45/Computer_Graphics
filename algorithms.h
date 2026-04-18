#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <graphics.h>
#include <cmath>

inline void DDA(int x1,int y1,int x2,int y2,int color=BLUE)
{
    int dx=x2-x1;
    int dy=y2-y1;

    int steps=abs(dx)>abs(dy)?abs(dx):abs(dy);

    float xinc=dx/(float)steps;
    float yinc=dy/(float)steps;

    float x=x1;
    float y=y1;

    for(int i=0;i<=steps;i++)
    {
        putpixel(round(x),round(y),color);
        x+=xinc;
        y+=yinc;
    }
}

inline void bresenhamLine(int x1, int y1, int x2, int y2, int color=BLUE)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        putpixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

inline void bresenhamLineAnimated(int x1, int y1, int x2, int y2, int color=BLUE, int speed=1)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int count = 0;

    while (true)
    {
        putpixel(x1, y1, color);
        if (count++ % speed == 0) delay(1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

inline void drawCirclePoints(int xc, int yc, int x, int y, int color)
{
    putpixel(xc+x, yc+y, color);
    putpixel(xc-x, yc+y, color);
    putpixel(xc+x, yc-y, color);
    putpixel(xc-x, yc-y, color);
    putpixel(xc+y, yc+x, color);
    putpixel(xc-y, yc+x, color);
    putpixel(xc+y, yc-x, color);
    putpixel(xc-y, yc-x, color);
}

inline void bresenhamCircle(int xc, int yc, int r, int color=BLUE)
{
    int x = 0, y = r;
    int d = 3 - 2 * r;
    drawCirclePoints(xc, yc, x, y, color);
    while (y >= x)
    {
        x++;
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
        drawCirclePoints(xc, yc, x, y, color);
    }
}

inline void drawCircleLines(int xc, int yc, int x, int y, int color)
{
    line(xc - x, yc + y, xc + x, yc + y);
    line(xc - x, yc - y, xc + x, yc - y);
    line(xc - y, yc + x, xc + y, yc + x);
    line(xc - y, yc - x, xc + y, yc - x);
}

inline void bresenhamCircleFilled(int xc, int yc, int r, int color=BLUE)
{
    int x = 0, y = r;
    int d = 3 - 2 * r;
    int prev_color = getcolor();
    setcolor(color);
    
    drawCircleLines(xc, yc, x, y, color);
    while (y >= x)
    {
        x++;
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
            d = d + 4 * x + 6;
        drawCircleLines(xc, yc, x, y, color);
    }
    setcolor(prev_color);
}

inline void bresenhamCircleAnimated(int xc, int yc, int r, int color=BLUE, int dly=5)
{
    for(int current_r = 1; current_r <= r; current_r++)
    {
        bresenhamCircleFilled(xc, yc, current_r, color);
        delay(dly);
    }
}


#endif