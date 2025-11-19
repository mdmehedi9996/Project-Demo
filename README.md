# Project-Demo
<br>
Author-Md. Mehedi Hasan.
“Student at Pabna University of Science and Technogoly” 
<br>
# OpenGL Electric App (FreeGLUT + C++)

This project is a simple OpenGL application built using **C++**, **FreeGLUT**, and **MinGW g++**.  
Anyone can download the project, build it, and run the `.exe` file easily.


## 📦 Requirements

To compile and run this project, you need:

- **MinGW (g++)**  
- **FreeGLUT** library (included in this repository under `libs/`)
- Windows (Tested on Win10/Win11)
- Visual Studio Code (optional)

## 📁 Project Structure
Project-Demo/
├── libs/ → header + library files
├── src/
│ ├── main.cpp
│ ├── main.exe → created after build
│ ├── freeglut.dll
│ ├── *.bmp images
├── .vscode/ → build/run configuration
└── README.md

## 🛠️ How to Build

Open a terminal inside the project folder:
commad directory is-> (like) E:\OpenGL Project\Project-Demo>
Then type in terminal: 
g++ src/main.cpp -Ilibs/include -Llibs/lib -lfreeglut -lopengl32 -lglu32 -o src/main.exe

## After built .exe file in src file 
commad directory is-> (like) E:\OpenGL Project\Project-Demo\src>
Then type in terminal:
.\main.exe



