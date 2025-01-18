# Simple C header file for creating OpenGL window and handling inputs on Windows

The windows are created in a sperate thread because windows blocks the thread that creates the
window whenever that window is moved or resized.