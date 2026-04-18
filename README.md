# 📊 Modern Office-Style Data Visualizer

> A comprehensive, interactive 2D/3D Data Visualization application built from scratch using C++ and core Computer Graphics algorithms.

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![OpenGL](https://img.shields.io/badge/OpenGL-FFFFFF?style=for-the-badge&logo=opengl)
![Code::Blocks](https://img.shields.io/badge/Code::Blocks-4CAF50?style=for-the-badge)

## 📝 Overview
Most basic computer graphics lab projects rely on hardcoded values and lack a proper User Interface. **Modern Office-Style Data Visualizer** bridges the gap between abstract mathematical algorithms and real-world utility. It features a modern, interactive "Office-Style" UI dashboard where users can input numerical data and instantly see it represented in five different graphical formats.

This project was developed as a part of the **CSE412/422: Computer Graphics Lab** course at **Daffodil International University (DIU)**.

## ✨ Key Features
* **Modern Interactive UI:** A clean, blue/white color palette with clickable cards and a responsive dashboard.
* **Robust Input Validation:** Custom input boxes that capture keyboard strokes, handle backspace/deletion, and strictly validate digits (e.g., preventing negative numbers for Pie Charts to prevent crashes).
* **5 Dynamic Chart Types:**
  1. **Line Graph (4-Quadrant):** Handles both positive and negative values.
  2. **3D Histogram:** Features animated, growing bars with isometric 3D depth.
  3. **2D Pie Chart:** Automatically calculates percentages and 360-degree angles.
  4. **Scatter Plot:** Renders data points with a visually pleasing outer glow effect.
  5. **3D Area Graph:** Uses smooth splines for beautiful curve rendering.
* **Real-time Animations:** Incremental loops and delays are used to animate the drawing process (e.g., pie slice reveal, line drawing).

## 🧮 Core Algorithms Implemented
Instead of relying on pre-built charting libraries, the underlying geometry and rendering logic were built from scratch:
* **DDA (Digital Differential Analyzer) & Bresenham's Line Algorithm:** Used for drawing perfect grid axes, connecting data points, and rendering callout lines.
* **Bresenham's Circle Algorithm:** Used for drawing glowing data markers and pie chart outlines.
* **Catmull-Rom Splines:** Mathematical spline equations used to calculate smooth, curving lines for the 3D Area Graph instead of rigid, sharp angles.
* **2D Transformations:** Extensive use of **Translation** and **Scaling** to dynamically fit user data (ranging from 1 to 9999) perfectly inside the screen's viewport.
* **Isometric Math:** Used to project the top and side polygon faces of the 3D Histogram bars.

## 🛠️ Tech Stack & Tools
* **Language:** C++
* **Graphics Library:** `graphics.h` (WinBGIm)
* **IDE:** Code::Blocks
* **Version Control:** Git / GitHub
## 🚀 How to Run the Project
1. **Prerequisites:** Ensure you have **Code::Blocks** installed with the `graphics.h` (WinBGIm) library configured. 
   > *Note: Standard GCC compilers do not include `graphics.h` by default. You must install the WinBGIm package.*
2. **Clone the repository:**
   ```bash
   git clone [https://github.com/SagorKumarPal45/Computer_Graphics.git](https://github.com/SagorKumarPal45/Computer_Graphics.git)
