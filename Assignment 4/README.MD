
## Contents:

### 1. main.cpp -> main code file to run the assignment
### 2. LoadBitmap.cpp -> cpp file given to read bmp files
### 3. "LinksHouse" folder -> contains all the ply & bmp files needed for texture mesh and rendering
### 4. README


Pre-requisites to run the program:

• OpenGL
• GLEW
• GLM
• LoadBitmap.cpp
----------------------------------------------

To run the program:

g++ main.cpp -GLEW -lGL  -lglfw -o main.out
./main.out
-----------------------------------------------------------------------


How the program works:

• The readPLYFile method reads the ply files to collect vertices and triangle faces coordinates. It also collects the number of vertices and faces.

• The collected data is then used in the TextureMesh class to start drawing for each furniture/object.

• Any camera changes depend on the arrow keys. You can move around in first-person view.
