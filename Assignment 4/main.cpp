// Name: Taegyun Kim
// CS 3388 
// Assignment 4

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "LoadBitmap.cpp"
using namespace glm;
using namespace std;


/* 
Class to store veretex data. Since colors are not defined in the ply file or since we are using images, 
set the color variables to 0. We still need them for buffers.
*/
class VertexData{
    public:
        GLfloat x, y, z;
        GLfloat nx, ny, nz;
        GLfloat red = 0.0f;
        GLfloat green = 0.0f;
        GLfloat blue = 0.0f;
        GLfloat u,v;

};


/* Class to store triangle faces */
class TriData{
    public:
        int vertex[3];
};


/* Method to read ply files. Read each file and store data into list/vertex.
   Store vertex and face objects separately.
*/
void readPLYFILE(string fileName, vector<VertexData>& vertices, vector<TriData>& faces){
    ifstream input(fileName);

    string fileLine;
    int vertexCount;
    int faceCount;
    bool header = true;

    while (getline (input, fileLine)){
        string item;
        string itemType;
        int amount;
        
        if(fileLine.find("end_header") == 0){
            header = false;
			continue;
        }else{ 
			// here if still in the header section    
            stringstream extract(fileLine);
            extract >> item >> itemType >> amount;

            if (item == "element"){
                if (itemType == "vertex"){
                    vertexCount = amount;
                } else if (itemType == "face"){
                    faceCount = amount;
                }
            }
        }

        /* here if finished reading the header section */
        if (!header){ 
            // here read each vertex lines and face lines depending on the amount
            if (vertexCount > 0) { 
                VertexData vertex;
                stringstream extract(fileLine);

                // read vertex position data & texture position data per line
                extract >> vertex.x >> vertex.y >> vertex.z >> vertex.nx >> vertex.ny >> vertex.nz 
                >> vertex.u >> vertex.v; 

                //add the vertex to the return list
                vertices.push_back(vertex); 
                vertexCount --;
            }
            
            // read each face lines for triangle
            else if (faceCount > 0) { 
                TriData triangleFace;
                stringstream extract(fileLine);
				int vertex;

				//add the face to the return list
                extract >> vertex >> triangleFace.vertex[0] >> triangleFace.vertex[1] >> triangleFace.vertex[2];
                faces.push_back(triangleFace);
                faceCount--;

            }
        }
    }
    input.close();
};


/* Class to create textured mesh based on vertices and
faces collected from ply files.
*/
class TexturedMesh{

	GLuint VBOvertexPositionID;
	GLuint VBOtextureCoordID;
	GLuint VBOfaceIndexID;
	GLuint TEXTUREbmImgID;
	GLuint VAOmeshID;
	GLuint ShaderRenderID;
	GLuint programID;

	vector<VertexData> vertices; 
    vector<TriData> faces;  

	unsigned int width;
	unsigned int height;
	unsigned char* bmpData;


	public:
		TexturedMesh(string plyPath, string bitMapPath){
			loadARGB_BMP(bitMapPath.data(), &bmpData, &width, &height);

			// read ply file and store data into vertices and faces vectors
			readPLYFILE(plyPath, vertices, faces);

			// create and bind VAO
			glGenVertexArrays(1, &VAOmeshID);    
			glBindVertexArray(VAOmeshID);

			// create and bind vertex VBO
			glGenBuffers(1, &VBOvertexPositionID);    
			glBindBuffer(GL_ARRAY_BUFFER, VBOvertexPositionID); 

			glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 11 * vertices.size(), &(vertices[0]), GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);   

			// for vertices
			glVertexAttribPointer( 
					0,              
					3,                  
					GL_FLOAT,           
					GL_FALSE,           
					sizeof(GL_FLOAT) * 11, 
					0               
			);

			glGenBuffers(1, &VBOtextureCoordID);
			glBindBuffer(GL_ARRAY_BUFFER, VBOtextureCoordID);    

			glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 11 * vertices.size(), &(vertices[0]), GL_STATIC_DRAW);
			glEnableVertexAttribArray(1);   

			// for u, v
			glVertexAttribPointer(
					1,                                
					2,                              
					GL_FLOAT,                        
					GL_FALSE,                         
					sizeof(GL_FLOAT) * 11,  

					// 9 coordinates of 4 bytes          
					(void*) 36 			              
			);

			glGenBuffers(1, &VBOvertexPositionID);  
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOvertexPositionID);  
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GL_UNSIGNED_INT) * 3 * faces.size(), &(faces[0]), GL_STATIC_DRAW);
			glBindVertexArray(0);  

			GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
			GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
			string VertexShaderCode = "\
			#version 330 core\n\
			// Input vertex data, different for all executions of this shader.\n\
			layout(location = 0) in vec3 vertexPosition;\n\
			layout(location = 1) in vec2 uv;\n\
			// Output data ; will be interpolated for each fragment.\n\
			out vec2 uv_out;\n\
			// Values that stay constant for the whole mesh.\n\
			uniform mat4 MVP;\n\
			void main(){ \n\
				// Output position of the vertex, in clip space : MVP * position\n\
				gl_Position =  MVP * vec4(vertexPosition,1);\n\
				// The color will be interpolated to produce the color of each fragment\n\
				uv_out = uv;\n\
			}\n";

			// Read the Fragment Shader code from the file
			string FragmentShaderCode = "\
			#version 330 core\n\
			in vec2 uv_out; \n\
			uniform sampler2D tex;\n\
			void main() {\n\
				gl_FragColor = texture(tex, uv_out);\n\
			}\n";

			char const * VertexSourcePointer = VertexShaderCode.c_str();
			glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
			glCompileShader(VertexShaderID);

			// Compile Fragment Shader
			char const * FragmentSourcePointer = FragmentShaderCode.c_str(); 
			glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
			glCompileShader(FragmentShaderID);

			programID = glCreateProgram();
			glAttachShader(programID, VertexShaderID);
			glAttachShader(programID, FragmentShaderID);
			glLinkProgram(programID);

			glDetachShader(programID, VertexShaderID);
			glDetachShader(programID, FragmentShaderID);

			glDeleteShader(VertexShaderID);
			glDeleteShader(FragmentShaderID); 

			glGenTextures(1, &TEXTUREbmImgID);  
			glBindTexture(GL_TEXTURE_2D, TEXTUREbmImgID);  
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bmpData);
			glGenerateMipmap(GL_TEXTURE_2D);   
			glBindTexture(GL_TEXTURE_2D, 0);    

		}

		/* Method to draw each mesh object*/
		void draw(glm::mat4 MVP){

			// use blending
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

			glActiveTexture(GL_TEXTURE0);   
			glEnable(GL_TEXTURE_2D);       
			glBindTexture(GL_TEXTURE_2D, TEXTUREbmImgID);

			GLuint matrixID = glGetUniformLocation(programID, "MVP");   
			glUseProgram(programID); 
			glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);  

			glBindVertexArray(VAOmeshID); 

			glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0); 
			glBindVertexArray(0);   
			glUseProgram(0);    
			glBindTexture(GL_TEXTURE_2D, 0);
		}

};


/* Method to control the camera.
   Use the arrow keys on the keyboard to move.
   return the matrix for camera
*/
glm::mat4 cameraControls(){
	static glm::vec3 cameraPos = glm::vec3(0.5f, 0.4f, 0.5f);  
	static glm::vec3 cameraDir = glm::vec3(0.0f, 0.0f, -1.0f);
	static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f,  0.0f);

	float movementFactor = 0.05f;
	float rotationFactor = 3.0f;

	// Check the type of arrow key pressed
	if (glfwGetKey(window,GLFW_KEY_UP) == GLFW_PRESS){
		cameraPos += cameraDir * movementFactor;
	}
	if (glfwGetKey(window,GLFW_KEY_DOWN) == GLFW_PRESS){
		cameraPos -= cameraDir * movementFactor;
	}
	if (glfwGetKey(window,GLFW_KEY_RIGHT) == GLFW_PRESS){
		cameraDir = glm::rotate(cameraDir, glm::radians(-rotationFactor), cameraUp);
	}
	if (glfwGetKey(window,GLFW_KEY_LEFT) == GLFW_PRESS){
		cameraDir = glm::rotate(cameraDir, glm::radians(rotationFactor), cameraUp);
	}

	return glm::lookAt(cameraPos, cameraPos + cameraDir, cameraUp);
}



int main( int argc, char* argv[])
{
	
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// Open a window and create its OpenGL context
	float screenW = 1400;
	float screenH = 1200;
	window = glfwCreateWindow(screenW, screenH, "3388 Assignment 4", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}


	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Dark blue background
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);

	glm::mat4 MVP;  //model view projection matrix
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), screenW/screenH, 0.001f, 1000.0f); //perspective projection matrix


	// store each mesh objects to be drawn into a vector
	// store transparent objects last - doorBG, metaloOjects, curtains
	vector<TexturedMesh> meshObjects;
	meshObjects.push_back(TexturedMesh("Walls.ply","walls.bmp"));
	meshObjects.push_back(TexturedMesh("Table.ply", "table.bmp"));
	meshObjects.push_back(TexturedMesh("Floor.ply", "floor.bmp"));
	meshObjects.push_back(TexturedMesh("Bottles.ply", "bottles.bmp"));
	meshObjects.push_back(TexturedMesh("Patio.ply", "patio.bmp"));
	meshObjects.push_back(TexturedMesh("WindowBG.ply", "windowbg.bmp"));
	meshObjects.push_back(TexturedMesh("WoodObjects.ply", "woodobjects.bmp"));
	meshObjects.push_back(TexturedMesh("DoorBG.ply", "doorbg.bmp"));
	meshObjects.push_back(TexturedMesh("MetalObjects.ply", "metalobjects.bmp"));
	meshObjects.push_back(TexturedMesh("Curtains.ply", "curtains.bmp"));

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();

		glLoadMatrixf(glm::value_ptr(Projection));

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glm::vec3 eye = {0.5f, 0.2f, 0.5f};
		glm::vec3 up = {0.0f, 1.0f, 0.0f};
		glm::vec3 center = {0.0f, 0.0f, 0.0f};

		glm::mat4 V = glm::lookAt(eye, center, up); 
		
		// apply camera control & update V
		V = cameraControls();

		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 MV = V * M;
		glLoadMatrixf(glm::value_ptr(V));

		MVP = Projection * V * M;
		
		// render each object in the vector
		for (int i = 0; i < meshObjects.size(); i++){
			meshObjects[i].draw(MVP);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	 // Check if the ESC key was pressed or the window was closed
	}while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

