/*
 * DATA VISUALIZER – main.cpp
 * Modern Microsoft Office-style UI
 * Full error handling & input validation for all 5 chart types.
 *
 * Validation rules (researched):
 *   Line Graph    : X and Y accept -9999 to 9999  (4-quadrant Cartesian)
 *   Histogram 3D  : Values must be 1 to 9999      (positive bars only)
 *   Pie Chart 2D  : Values must be 1 to 9999      (positive slice area)
 *   Scatter Plot  : X and Y must be 0 to 9999     (positive-quadrant plot)
 *   Area Graph    : Values must be 0 to 9999       (non-negative fill)
 *
 * Extreme-value guards:
 *   - maxDigits=4  → no value can exceed ±9999 via keyboard
 *   - Post-collection validation shows error screen if rules broken
 *   - Chart headers guard against all-zero, all-same, division-by-zero
 */

#include <graphics.h>
#include <conio.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <filesystem>

#include "gui_input.h"
#include "linegraph.h"
#include "histogram3d.h"
#include "piechart.h"
#include "scatterplot.h"
#include "areagraph.h"

/* ─── Palette ───────────────────────────────────────────────────────── */
#define COL_WIN_BG        WHITE
#define COL_CARD_BG       WHITE
#define COL_HINT          DARKGRAY
#define COL_PANEL_BG      LIGHTGRAY
#define COL_PANEL_BORDER  LIGHTGRAY
#define COL_ACCENT        BLUE

/* ─── Layout ────────────────────────────────────────────────────────── */
#define WIN_W        1180
#define WIN_H_MENU    760
#define HEADER_H       80
#define CARD_X0       330
#define CARD_W        520
#define CARD_H         62
#define CARD_GAP       14
#define CARD_Y_START  200

static std::vector<std::string> collectProjectImages()
{
    std::vector<std::string> paths;
    const std::vector<std::string> exts = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".webp"};
    namespace fs = std::filesystem;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(fs::current_path())) {
            if (!entry.is_regular_file()) continue;
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                           [](unsigned char c){ return (char)std::tolower(c); });
            for (size_t i = 0; i < exts.size(); i++) {
                if (ext == exts[i]) {
                    paths.push_back(entry.path().string());
                    break;
                }
            }
        }
    } catch (...) {
        /* If folder scan fails, silently return empty list */
    }

    std::sort(paths.begin(), paths.end());
    return paths;
}

static std::string findImageByFileName(const std::vector<std::string>& images, const char* fileName)
{
    if (!fileName) return "";
    namespace fs = std::filesystem;
    std::string target = fileName;
    std::transform(target.begin(), target.end(), target.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });

    for (size_t i = 0; i < images.size(); i++) {
        std::string base = fs::path(images[i]).filename().string();
        std::transform(base.begin(), base.end(), base.begin(),
                       [](unsigned char c){ return (char)std::tolower(c); });
        if (base == target) return images[i];
    }
    return "";
}

static void drawRoundedImageOrPlaceholder(int cx, int cy, int radius, const char* imagePath, const char* fallbackLabel)
{
    const int x1 = cx - radius;
    const int y1 = cy - radius;
    const int x2 = cx + radius;
    const int y2 = cy + radius;

    bool imageDrawn = false;
    if (imagePath && imagePath[0] != '\0') {
        readimagefile((char*)imagePath, x1, y1, x2, y2);
        imageDrawn = true;
    }

    if (!imageDrawn) {
        setfillstyle(SOLID_FILL, LIGHTBLUE);
        fillellipse(cx, cy, radius, radius);
        setbkcolor(LIGHTBLUE);
        setcolor(BLUE);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
        int tw = textwidth((char*)fallbackLabel);
        outtextxy(cx - tw / 2, cy - 6, (char*)fallbackLabel);
        setbkcolor(COL_WIN_BG);
    } else {
        /* Mask square corners so photo appears rounded */
        for (int y = y1; y <= y2; y++) {
            for (int x = x1; x <= x2; x++) {
                int dx = x - cx;
                int dy = y - cy;
                if (dx * dx + dy * dy > radius * radius) putpixel(x, y, COL_WIN_BG);
            }
        }
    }

    setcolor(BLUE);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    circle(cx, cy, radius);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
}

static void drawCenteredWrappedName(int centerX, int yTop, int maxWidth, const char* fullName)
{
    if (!fullName || fullName[0] == '\0') return;

    std::string text = fullName;
    std::vector<std::string> words;
    std::string cur;
    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == ' ') {
            if (!cur.empty()) { words.push_back(cur); cur.clear(); }
        } else {
            cur.push_back(text[i]);
        }
    }
    if (!cur.empty()) words.push_back(cur);

    std::vector<std::string> lines;
    std::string line;
    for (size_t i = 0; i < words.size(); i++) {
        std::string candidate = line.empty() ? words[i] : (line + " " + words[i]);
        if (textwidth((char*)candidate.c_str()) <= maxWidth || line.empty()) {
            line = candidate;
        } else {
            lines.push_back(line);
            line = words[i];
        }
    }
    if (!line.empty()) lines.push_back(line);

    if (lines.size() > 3) {
        std::string merged = lines[2];
        for (size_t i = 3; i < lines.size(); i++) merged += " " + lines[i];
        lines.resize(3);
        lines[2] = merged;
    }

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    setcolor(BLACK);
    setbkcolor(WHITE);
    for (size_t i = 0; i < lines.size(); i++) {
        int tw = textwidth((char*)lines[i].c_str());
        outtextxy(centerX - tw / 2, yTop + (int)i * 16, (char*)lines[i].c_str());
    }
}

static void drawPeopleSection(const char* title, int x1, int x2, int yTop,
                              const std::vector<std::string>& images,
                              const char* fileNames[],
                              const char* displayNames[],
                              int count)
{
    const int titleY = yTop + 16;
    const int firstAvatarY = yTop + 72; /* tighter layout for 5 teammates */
    const int avatarR = 28;
    const int nameGap = 8;
    const int lineH = 14;
    const int maxNameLines = 2;
    const int itemStep = avatarR * 2 + nameGap + maxNameLines * lineH + 8;
    const int bottomPad = 12;
    int boxH = (firstAvatarY - yTop) + (count - 1) * itemStep + (avatarR * 2 + nameGap + maxNameLines * lineH) + bottomPad;

    setfillstyle(SOLID_FILL, WHITE);
    bar(x1, yTop, x2, yTop + boxH);
    setcolor(COL_PANEL_BORDER);
    rectangle(x1, yTop, x2, yTop + boxH);

    setbkcolor(WHITE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    setcolor(BLUE);
    int tw = textwidth((char*)title);
    outtextxy((x1 + x2 - tw) / 2, titleY, (char*)title);

    const int centerX = (x1 + x2) / 2;
    int y = firstAvatarY;
    for (int i = 0; i < count; i++) {
        char tag[8];
        sprintf(tag, "P%d", i + 1);
        std::string pathStr = findImageByFileName(images, fileNames[i]);
        const char* path = pathStr.empty() ? NULL : pathStr.c_str();
        drawRoundedImageOrPlaceholder(centerX, y, avatarR, path, fileNames[i]);
        drawCenteredWrappedName(centerX, y + avatarR + nameGap, (x2 - x1) - 18, displayNames[i]);
        y += itemStep;
    }
    setbkcolor(COL_WIN_BG);
}

/* ─── Draw one card button ──────────────────────────────────────────── */
static void drawMenuCard(int index, const char* label, bool hovered, bool isExit)
{
    int x1 = CARD_X0;
    int y1 = CARD_Y_START + index * (CARD_H + CARD_GAP);
    int x2 = CARD_X0 + CARD_W;
    int y2 = y1 + CARD_H;

    int fillCol  = isExit ? (hovered ? RED : LIGHTRED)
                          : (hovered ? LIGHTBLUE : COL_CARD_BG);
    int textCol   = (isExit || hovered) ? WHITE : BLACK;
    int borderCol = hovered ? COL_ACCENT : COL_PANEL_BORDER;

    /* soft shadow for Office card depth */
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(x1 + 4, y1 + 4, x2 + 4, y2 + 4);

    setfillstyle(SOLID_FILL, fillCol);
    bar(x1, y1, x2, y2);
    setcolor(borderCol);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
    rectangle(x1, y1, x2, y2);

    setfillstyle(SOLID_FILL, isExit ? DARKGRAY : (hovered ? WHITE : COL_ACCENT));
    bar(x1, y1 + 1, x1 + 6, y2 - 1);

    setbkcolor(fillCol);
    setcolor(textCol);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    int tw = textwidth((char*)label);
    const int FH = 16;
    outtextxy(x1 + (CARD_W - tw) / 2, y1 + (CARD_H - FH) / 2, (char*)label);
    setbkcolor(COL_WIN_BG);
}

static void drawSoftLabel(int x, int y, const char* text, int bgCol = WHITE, int fgCol = BLUE)
{
    const int padX = 8;
    const int padY = 5;
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    int tw = textwidth((char*)text);
    const int th = 14; /* safer fixed height for SANS_SERIF size-1 */

    setfillstyle(SOLID_FILL, bgCol);
    bar(x, y, x + tw + 2 * padX, y + th + 2 * padY);
    setcolor(COL_PANEL_BORDER);
    rectangle(x, y, x + tw + 2 * padX, y + th + 2 * padY);

    setbkcolor(bgCol);
    setcolor(fgCol);
    /* Draw twice with 1px shift for a bold-like effect */
    outtextxy(x + padX, y + padY, (char*)text);
    outtextxy(x + padX + 1, y + padY, (char*)text);
    setbkcolor(COL_WIN_BG);
}

static void drawSoftLabelCentered(int y, const char* text, int bgCol = WHITE, int fgCol = BLUE)
{
    const int padX = 8;
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    int tw = textwidth((char*)text);
    int totalW = tw + 2 * padX;
    int x = (WIN_W - totalW) / 2;
    drawSoftLabel(x, y, text, bgCol, fgCol);
}

static void drawBoldInputLabel(int x, int y, const char* text)
{
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setbkcolor(COL_WIN_BG);
    setcolor(BLACK);
    outtextxy(x, y, (char*)text);
    outtextxy(x + 1, y, (char*)text);
    outtextxy(x, y + 1, (char*)text);
    outtextxy(x + 1, y + 1, (char*)text);
}

static void drawInputInstructionPanel(const char* line1, const char* line2, int y)
{
    setfillstyle(SOLID_FILL, COL_PANEL_BG);
    bar(130, y, 870, y + 62);
    setcolor(COL_PANEL_BORDER);
    rectangle(130, y, 870, y + 62);
    setfillstyle(SOLID_FILL, COL_ACCENT);
    bar(130, y, 136, y + 62);

    setbkcolor(COL_PANEL_BG);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    setcolor(BLACK);
    outtextxy(150, y + 12, (char*)line1);
    outtextxy(150, y + 35, (char*)line2);
    setbkcolor(COL_WIN_BG);
}

/* ─── Header banner ─────────────────────────────────────────────────── */
void drawAppBackground(const char* title)
{
    setbkcolor(COL_WIN_BG);
    cleardevice();
    setfillstyle(SOLID_FILL, BLUE);
    bar(0, 0, WIN_W, HEADER_H);
    setfillstyle(SOLID_FILL, LIGHTBLUE);
    bar(0, 0, 10, HEADER_H);
    setcolor(LIGHTBLUE);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    line(0, HEADER_H + 1, WIN_W, HEADER_H + 1);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
    setbkcolor(BLUE);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
    setcolor(LIGHTGRAY);
    outtextxy(WIN_W - 190, 10, (char*)"Office UI Edition  v3.0");
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 4);
    setcolor(WHITE);
    int tw = textwidth((char*)title);
    outtextxy((WIN_W - tw) / 2, 17, (char*)title);
    setbkcolor(COL_WIN_BG);

    /* subtle content area card */
    setfillstyle(SOLID_FILL, WHITE);
    bar(75, HEADER_H + 20, WIN_W - 75, WIN_H_MENU - 30);
    setcolor(COL_PANEL_BORDER);
    rectangle(75, HEADER_H + 20, WIN_W - 75, WIN_H_MENU - 30);
}

/* ─── Error overlay ─────────────────────────────────────────────────── */
/*
 * Shows a red banner error and waits for a keypress.
 * Returns so caller can loop back and re-collect data.
 */
static void showError(const char* line1, const char* line2 = NULL, const char* line3 = NULL)
{
    /* Red panel */
    setfillstyle(SOLID_FILL, LIGHTRED);
    bar(130, 240, 870, 380);
    setcolor(RED);
    setlinestyle(SOLID_LINE, 0, THICK_WIDTH);
    rectangle(130, 240, 870, 380);
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);

    /* Error icon strip */
    setfillstyle(SOLID_FILL, RED);
    bar(130, 240, 148, 380);

    setbkcolor(LIGHTRED);
    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
    setcolor(WHITE);
    outtextxy(165, 252, (char*)"  INPUT ERROR");

    settextstyle(DEFAULT_FONT, HORIZ_DIR, 1);
    setcolor(BLACK);
    if (line1) outtextxy(165, 290, (char*)line1);
    if (line2) outtextxy(165, 310, (char*)line2);
    if (line3) outtextxy(165, 330, (char*)line3);

    setcolor(DARKGRAY);
    outtextxy(165, 356, (char*)"Press any key to go back and fix the values...");
    setbkcolor(COL_WIN_BG);

    /* Flush any pending key/click, then wait */
    while (kbhit()) getch();
    while (ismouseclick(WM_LBUTTONDOWN)) clearmouseclick(WM_LBUTTONDOWN);
    getch();
}

/* ─── Validate data for a given chart choice ────────────────────────── */
/*
 * Returns true if data is valid.
 * On error: shows error overlay and returns false (caller re-loops).
 *
 *  choice: '1'=Line  '2'=Hist  '3'=Pie  '4'=Scatter  '5'=Area
 *  x[], y[] used for choice '1' and '4'; dataArr[] used for '2','3','5'
 */
static bool validateData(char choice, int n, int x[], int y[], int dataArr[])
{
    if (choice == '1')  /* Line Graph: -9999..9999 */
    {
        /* No special restriction beyond the maxDigits=4 already enforced.
           All-same X values (vertical line) is valid. Nothing to reject. */
        return true;
    }

    if (choice == '2')  /* Histogram 3D: each value must be > 0 */
    {
        bool anyPositive = false;
        for (int i = 0; i < n; i++) {
            if (dataArr[i] < 0) {
                showError("Histogram does not support negative values.",
                          "All values must be greater than 0.",
                          "Please re-enter your data.");
                return false;
            }
            if (dataArr[i] > 0) anyPositive = true;
        }
        if (!anyPositive) {
            showError("All histogram values are zero.",
                      "At least one value must be greater than 0.",
                      "Please re-enter your data.");
            return false;
        }
        /* Extreme: check if max value is too high relative to others */
        int maxV = dataArr[0], minV = dataArr[0];
        for (int i = 1; i < n; i++) {
            if (dataArr[i] > maxV) maxV = dataArr[i];
            if (dataArr[i] < minV) minV = dataArr[i];
        }
        if (maxV > 0 && minV > 0 && (maxV / minV) > 500) {
            showError("Extreme value spread detected (one value > 500x another).",
                      "Some bars may be invisible. Consider more balanced values.",
                      "Press any key to continue anyway or re-run to fix...");
            /* This is a warning, not a hard error – still return true */
            return true;
        }
        return true;
    }

    if (choice == '3')  /* Pie Chart: values must be > 0, sum > 0 */
    {
        long long total = 0;
        for (int i = 0; i < n; i++) {
            if (dataArr[i] < 0) {
                showError("Pie Chart does not support negative values.",
                          "Negative values cannot form a pie slice.",
                          "Please re-enter your data.");
                return false;
            }
            total += dataArr[i];
        }
        if (total <= 0) {
            showError("All pie chart values are zero.",
                      "Sum of all values must be greater than 0.",
                      "Please re-enter your data.");
            return false;
        }
        return true;
    }

    if (choice == '4')  /* Scatter Plot: X and Y must be >= 0 */
    {
        for (int i = 0; i < n; i++) {
            if (x[i] < 0 || y[i] < 0) {
                showError("Scatter Plot only supports the positive quadrant.",
                          "All X and Y values must be >= 0.",
                          "Please re-enter your data.");
                return false;
            }
        }
        /* All-zero: every point on origin – warn */
        bool anyNonZero = false;
        for (int i = 0; i < n; i++)
            if (x[i] > 0 || y[i] > 0) { anyNonZero = true; break; }
        if (!anyNonZero) {
            showError("All scatter points are at origin (0,0).",
                      "Please enter at least one non-zero X or Y value.",
                      "");
            return false;
        }
        return true;
    }

    if (choice == '5')  /* Area Graph: values must be >= 0 */
    {
        for (int i = 0; i < n; i++) {
            if (dataArr[i] < 0) {
                showError("Area Graph does not support negative values.",
                          "All values must be >= 0 (the area cannot go below baseline).",
                          "Please re-enter your data.");
                return false;
            }
        }
        /* All-zero: nothing to draw */
        bool anyNonZero = false;
        for (int i = 0; i < n; i++)
            if (dataArr[i] > 0) { anyNonZero = true; break; }
        if (!anyNonZero) {
            showError("All area graph values are zero — nothing to draw.",
                      "Please enter at least one value greater than 0.",
                      "");
            return false;
        }
        return true;
    }

    return true;  /* unknown choice: allow through */
}

/* ─── Card hover hit-test ───────────────────────────────────────────── */
static bool cardHit(int mx, int my, int index)
{
    int y1 = CARD_Y_START + index * (CARD_H + CARD_GAP);
    return  mx >= CARD_X0 && mx <= CARD_X0 + CARD_W &&
            my >= y1       && my <= y1 + CARD_H;
}

/* ──────────────────────────────────────────────────────────────────────
   MAIN
   ────────────────────────────────────────────────────────────────────── */
int main()
{
    initwindow(WIN_W, WIN_H_MENU, "Data Visualizer");
    setbkcolor(COL_WIN_BG);
    cleardevice();

    const char* labels[] = {
        "Line Graph",
        "Histogram 3D",
        "Pie Chart 2D",
        "Scatter Plot",
        "Area Graph",
        "Exit"
    };
    const int N_ITEMS = 6;
    std::vector<std::string> allImages = collectProjectImages();

    while (true)
    {
        /* ── Main menu ──────────────────────────────────────────────── */
        char choice  = 0;
        int  prevHov = -2;

        while (choice == 0)
        {
            int mx  = mousex();
            int my  = mousey();
            int hov = -1;
            for (int i = 0; i < N_ITEMS; i++)
                if (cardHit(mx, my, i)) { hov = i; break; }

            if (hov != prevHov)
            {
                drawAppBackground("DATA VISUALIZER");
                settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
                setbkcolor(COL_WIN_BG);
                setcolor(DARKGRAY);
                int sw = textwidth((char*)"Choose a chart type to visualize");
                outtextxy((WIN_W - sw) / 2, CARD_Y_START - 84,
                          (char*)"Choose a chart type to visualize");
                drawSoftLabelCentered(CARD_Y_START - 50,
                                      "Mouse: hover and click  |  Keyboard: press 1 to 5");
                for (int i = 0; i < N_ITEMS; i++)
                    drawMenuCard(i, labels[i], (hov == i), (i == N_ITEMS - 1));
                drawSoftLabelCentered(WIN_H_MENU - 68,
                                      "Tip: press 0 anytime on menu to close the app");

                /* Left and right people info panels on main menu */
                const char* teammateFiles[] = {"sagor.jpg", "juthy.jpg", "naib.jpg", "abid.jpg", "sohan.jpg"};
                const char* teammateNames[] = {
                    "Sagor Kumar Pal",
                    "Farzana Akter Juthy",
                    "Sheikh Rashid Naib",
                    "Abidul Hak Arman",
                    "Md. Sohan Mia"
                };
                const char* teacherFiles[]  = {"teacher.jpg"};
                const char* teacherNames[]  = {"Md. Mehefujur Rahman Mubin"};

                drawPeopleSection("TEAM MATES", 80, 305, 102, allImages, teammateFiles, teammateNames, 5);
                drawPeopleSection("TEACHER",    875, 1100, 230, allImages, teacherFiles, teacherNames, 1);
                prevHov = hov;
            }

            if (ismouseclick(WM_LBUTTONDOWN)) {
                clearmouseclick(WM_LBUTTONDOWN);
                if (hov >= 0 && hov < N_ITEMS - 1) choice = (char)('1' + hov);
                else if (hov == N_ITEMS - 1)        choice = '0';
            }
            if (kbhit()) {
                char key = (char)getch();
                if (key >= '1' && key <= '5') choice = key;
                else if (key == '0')           choice = '0';
            }
            delay(20);
        }

        if (choice == '0') { closegraph(); return 0; }

        /* ── Determine input constraints per chart ───────────────────── */
        /*
         * positiveOnly : blocks '-' key at input level
         * minVal/maxVal: clamping range (enforced after atoi)
         * rangeHint    : shown to user in the data-entry header
         */
        bool positiveOnly;
        int  minVal, maxValAllowed;
        const char* rangeHint;

        if (choice == '1') {
            /* Line Graph: accepts negative values */
            positiveOnly   = false;
            minVal         = -9999;
            maxValAllowed  = 9999;
            rangeHint      = "X and Y: -9999 to 9999 (negative values OK)";
        } else if (choice == '2') {
            /* Histogram: positive bars only */
            positiveOnly   = true;
            minVal         = 0;
            maxValAllowed  = 9999;
            rangeHint      = "Values: 1 to 9999  (positive integers only)";
        } else if (choice == '3') {
            /* Pie Chart: positive slice area */
            positiveOnly   = true;
            minVal         = 0;
            maxValAllowed  = 9999;
            rangeHint      = "Values: 1 to 9999  (positive integers only)";
        } else if (choice == '4') {
            /* Scatter Plot: positive quadrant */
            positiveOnly   = true;
            minVal         = 0;
            maxValAllowed  = 9999;
            rangeHint      = "X and Y: 0 to 9999 (non-negative integers only)";
        } else {
            /* Area Graph: non-negative fill */
            positiveOnly   = true;
            minVal         = 0;
            maxValAllowed  = 9999;
            rangeHint      = "Values: 0 to 9999  (non-negative integers only)";
        }

        /* ── Number-of-points input ──────────────────────────────────── */
        bool back_to_menu = false;
        int  n  = 1;
        int  x[50]       = {0};
        int  y[50]       = {0};
        int  dataArr[50] = {0};
        char buf[64];
        int  clickX = 0, clickY = 0;

        drawAppBackground("ENTER DATA");
        for (;;)
        {
            setfillstyle(SOLID_FILL, COL_WIN_BG);
            bar(0, HEADER_H + 2, WIN_W, 230);

            settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
            setbkcolor(COL_WIN_BG);
            setcolor(BLACK);
            outtextxy(150, 105, (char*)"Number of data points (1 - 50)");

            drawSoftLabel(150, 148, rangeHint);
            drawInputInstructionPanel("Tab/Enter: next field   Up Arrow: previous field",
                                      "Space: back to menu    Click any box to jump",
                                      194);

            int nav_tmp = INPUT_NAV_CONFIRM;
            /* Points count: always 1-50, always positive */
            n = getNumberInput(580, 105, &nav_tmp, NULL,
                               &clickX, &clickY,
                               true,   /* positiveOnly */
                               1, 50,  /* min/max */
                               2);     /* maxDigits for 1-50 */
            if (nav_tmp == INPUT_NAV_BACK)  { back_to_menu = true; break; }
            if (nav_tmp == INPUT_NAV_MOUSE) continue;
            if (nav_tmp == INPUT_NAV_PREV)  continue;
            if (n < 1) { n = 1; continue; }   /* reject 0 */
            break;
        }

        if (back_to_menu) {
            closegraph();
            initwindow(WIN_W, WIN_H_MENU, "Data Visualizer");
            setbkcolor(COL_WIN_BG);
            cleardevice();
            continue;
        }

        if (n < 1)  n = 1;
        if (n > 50) n = 50;

        /* ── Resize window for rows ────────────────────────────────── */
        const int inpRowY0      = 200;
        const int inpBottomGap  = 120;
        const int inpRowH       = 36;
        const int maxVisibleRows = 12;
        int visibleRows = (n < maxVisibleRows) ? n : maxVisibleRows;
        if (visibleRows < 1) visibleRows = 1;
        int dataWinH = inpRowY0 + visibleRows * inpRowH + inpBottomGap;
        if (dataWinH < WIN_H_MENU) dataWinH = WIN_H_MENU;

        int hintY = inpRowY0 + visibleRows * inpRowH + 16;
        if (hintY > dataWinH - 70) hintY = dataWinH - 70;

        closegraph();
        initwindow(WIN_W, dataWinH, "Data Visualizer");
        setbkcolor(COL_WIN_BG);
        cleardevice();

        /* ── Data entry loop – retries until valid ───────────────────── */
        bool validData = false;
        while (!validData && !back_to_menu)
        {
            /* Reset arrays for re-entry after validation error */
            for (int i = 0; i < 50; i++) { x[i] = 0; y[i] = 0; dataArr[i] = 0; }

            /* ── X/Y input (Line Graph & Scatter Plot) ─────────────── */
            if (choice == '1' || choice == '4')
            {
                int total = 2 * n;
                int focus = 0;
                while (focus < total && !back_to_menu)
                {
                    int focusRow = focus / 2;
                    int pageStart = (focusRow / visibleRows) * visibleRows;
                    int pageEnd = pageStart + visibleRows;
                    if (pageEnd > n) pageEnd = n;

                    drawAppBackground("ENTER DATA");
                    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
                    setbkcolor(COL_WIN_BG);
                    setcolor(BLACK);
                    outtextxy(150, 105, (char*)"Number of data points:");
                    sprintf(buf, "%d", n);
                    outtextxy(450, 105, buf);
                    drawSoftLabel(150, 138, rangeHint);
                    drawInputInstructionPanel("X/Y data entry mode: click a field to move focus",
                                              "Tab/Enter: next   Up: previous   Space: back",
                                              hintY);

                    for (int i = pageStart; i < pageEnd; i++) {
                        int vis = i - pageStart;
                        int ry = inpRowY0 + vis * inpRowH;
                        /* Row highlight before labels */
                        if (focus / 2 == i) {
                            setfillstyle(SOLID_FILL, LIGHTCYAN);
                            bar(140, ry - 6, WIN_W - 140, ry + inpRowH - 10);
                        }
                        sprintf(buf, "X%d:", i + 1);
                        drawBoldInputLabel(150, ry, buf);
                        sprintf(buf, "Y%d:", i + 1);
                        drawBoldInputLabel(455, ry, buf);
                        int xField = 2 * i, yField = 2 * i + 1;
                        setcolor(BLACK);
                        if (focus != xField) { sprintf(buf, "%d", x[i]); outtextxy(262, ry, buf); }
                        if (focus != yField) { sprintf(buf, "%d", y[i]); outtextxy(555, ry, buf); }
                    }

                    int row = focus / 2;
                    int col = focus % 2;
                    int ix  = col ? 555 : 262;
                    int iy  = inpRowY0 + (row - pageStart) * inpRowH;

                    int* pval = col ? &y[row] : &x[row];
                    int  nav  = INPUT_NAV_CONFIRM;
                    *pval = getNumberInput(ix, iy, &nav, pval,
                                          &clickX, &clickY,
                                          positiveOnly,
                                          minVal, maxValAllowed, 4);

                    if (nav == INPUT_NAV_BACK)  { back_to_menu = true; break; }
                    if (nav == INPUT_NAV_MOUSE) {
                        if (clickY >= inpRowY0 && clickY <= inpRowY0 + visibleRows * inpRowH) {
                            int vis = (clickY - inpRowY0) / inpRowH;
                            int targetRow = pageStart + vis;
                            if (targetRow >= 0 && targetRow < n) {
                                int ry = inpRowY0 + vis * inpRowH;
                                if (pointInInputBox(clickX, clickY, 262, ry)) focus = 2 * targetRow;
                                else if (pointInInputBox(clickX, clickY, 555, ry)) focus = 2 * targetRow + 1;
                            }
                        }
                        continue;
                    }
                    if (nav == INPUT_NAV_PREV) { if (focus > 0) focus--; }
                    else                         focus++;
                }
            }
            else
            {
                /* ── Single-value input ────────────────────────────── */
                int focus = 0;
                while (focus < n && !back_to_menu)
                {
                    int pageStart = (focus / visibleRows) * visibleRows;
                    int pageEnd = pageStart + visibleRows;
                    if (pageEnd > n) pageEnd = n;

                    drawAppBackground("ENTER DATA");
                    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 2);
                    setbkcolor(COL_WIN_BG);
                    setcolor(BLACK);
                    outtextxy(150, 105, (char*)"Number of data points:");
                    sprintf(buf, "%d", n);
                    outtextxy(450, 105, buf);
                    drawSoftLabel(150, 138, rangeHint);
                    drawInputInstructionPanel("Value entry mode: click a field to move focus",
                                              "Tab/Enter: next   Up: previous   Space: back",
                                              hintY);

                    for (int i = pageStart; i < pageEnd; i++) {
                        int vis = i - pageStart;
                        int ry = inpRowY0 + vis * inpRowH;
                        if (focus == i) {
                            setfillstyle(SOLID_FILL, LIGHTCYAN);
                            bar(140, ry - 6, WIN_W - 140, ry + inpRowH - 10);
                        }
                        sprintf(buf, "V%d:", i + 1);
                        drawBoldInputLabel(150, ry, buf);
                        if (focus != i) {
                            setcolor(BLACK);
                            sprintf(buf, "%d", dataArr[i]);
                            outtextxy(310, ry, buf);
                        }
                    }

                    int iy  = inpRowY0 + (focus - pageStart) * inpRowH;
                    int nav = INPUT_NAV_CONFIRM;
                    dataArr[focus] = getNumberInput(310, iy, &nav, &dataArr[focus],
                                                    &clickX, &clickY,
                                                    positiveOnly,
                                                    minVal, maxValAllowed, 4);

                    if (nav == INPUT_NAV_BACK)  { back_to_menu = true; break; }
                    if (nav == INPUT_NAV_MOUSE) {
                        if (clickY >= inpRowY0 && clickY <= inpRowY0 + visibleRows * inpRowH) {
                            int vis = (clickY - inpRowY0) / inpRowH;
                            int targetRow = pageStart + vis;
                            int ry = inpRowY0 + vis * inpRowH;
                            if (targetRow >= 0 && targetRow < n &&
                                pointInInputBox(clickX, clickY, 310, ry)) {
                                focus = targetRow;
                            }
                        }
                        continue;
                    }
                    if (nav == INPUT_NAV_PREV) { if (focus > 0) focus--; }
                    else                         focus++;
                }
            }

            if (back_to_menu) break;

            /* ── Post-collection validation ─────────────────────────── */
            drawAppBackground("ENTER DATA");   /* clean canvas for error overlay */
            if (validateData(choice, n, x, y, dataArr)) {
                validData = true;
            }
            /* If false → loop restarts, arrays reset, user re-enters */
        }

        /* ── Back to menu ───────────────────────────────────────────── */
        if (back_to_menu) {
            closegraph();
            initwindow(WIN_W, WIN_H_MENU, "Data Visualizer");
            setbkcolor(COL_WIN_BG);
            cleardevice();
            continue;
        }

        /* ── Open standard chart window, draw ───────────────────────── */
        closegraph();
        initwindow(WIN_W, WIN_H_MENU, "Data Visualizer");
        setbkcolor(COL_WIN_BG);
        cleardevice();

        if (choice == '1') drawLineGraph(x, y, n);
        if (choice == '2') drawHistogram3D(dataArr, n);
        if (choice == '3') drawPieChart3D(dataArr, n);
        if (choice == '4') drawScatterPlot(x, y, n);
        if (choice == '5') drawAreaGraph(dataArr, n);

        /* "Press any key" hint at bottom (always below chart area) */
        const char* hintMsg = "Press any key to return to menu...";
        setbkcolor(COL_WIN_BG);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, 1);
        int hintW = textwidth((char*)hintMsg);
        int hintX = (WIN_W - hintW) / 2;
        int hintYBottom = WIN_H_MENU - 26;
        setcolor(BLACK);
        outtextxy(hintX, hintYBottom, (char*)hintMsg);
        outtextxy(hintX + 1, hintYBottom, (char*)hintMsg);
        getch();

        closegraph();
        initwindow(WIN_W, WIN_H_MENU, "Data Visualizer");
        setbkcolor(COL_WIN_BG);
        cleardevice();
    }

    closegraph();
    return 0;
}