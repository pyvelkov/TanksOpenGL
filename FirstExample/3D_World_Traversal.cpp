using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "..\SOIL\src\SOIL.h"
#include <iostream>
#include <vector>

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer };
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 2;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
GLuint texture[5];	//Array of pointers to textrure data in VRAM. We use two textures in this example.
int timeSinceStart = 0;
float randomFloat(float a, float b);
const GLuint NumVertices = 28;

//Height of camera (player) from the level
float height = 0.8f;

//Player motion speed for movement and pitch/yaw
float travel_speed = 300.0f;		//Motion speed
float mouse_sensitivity = 0.01f;	//Pitch/Yaw speed

//Used for tracking mouse cursor position on screen
int x0 = 0;
int y_0 = 0;

glm::vec3 shoot_dir_vector = glm::vec3(0.0f, 0.0f, height);
//FOR VEHICLE
float wheel_rotation = 0.0f;
float vehicle_speed = randomFloat(0.001f, 0.003f);
glm::vec3 moving_dir = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 vehicle_move_dir = moving_dir;
void initializeVehicles();
int pointCounter = 0;
//FOR VEHICLE ABOVE

//Transformation matrices and camera vectors
glm::mat4 model_view;
glm::vec3 unit_z_vector = glm::vec3(0, 0, 1);	//Assigning a meaningful name to (0,0,1) :-)
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);	//Forward vector is parallel to the level at all times (No pitch)

//The direction which the camera is looking, at any instance
glm::vec3 looking_dir_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = unit_z_vector;
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);


//Used to measure time between two frames
int oldTimeSinceStart = 0;
int deltaTime;

//Creating and rendering bunch of objects on the scene to interact with
const int Num_Obstacles = 50;
float obstacle_data[Num_Obstacles][3];


//game objects class
class GameObject {
public:
private:
	char type = 'n';
	glm::vec3 location;
	glm::vec3 direction;
	bool isAlive = false;
public:
	char getType() { return type; }
	void setType(char t) { type = t; }

	glm::vec3 getlocation() { return location; }
	void setlocation(float locx, float locy, float locz) {
		location = { locx, locy, locz };
	}

	glm::vec3 getDirection() { return direction; }
	void setDirection(float locx, float locy, float locz) {
		direction = { locx, locy, locz };
	}

	bool getisAlive() { return isAlive; }
	void setisAlive(bool alive) {
		isAlive = alive;
	}
};

vector<GameObject> gameScene;

//Helper function to generate a random float number within a range
float randomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

// inititializing buffers, coordinates, setting up pipeline, etc.
void init(void)
{
	glEnable(GL_DEPTH_TEST);

	//Normalizing all vectors
	up_vector = glm::normalize(up_vector);
	forward_vector = glm::normalize(forward_vector);
	looking_dir_vector = glm::normalize(looking_dir_vector);
	side_vector = glm::normalize(side_vector);

	//Randomizing the position and scale of obstacles
	for (int i = 0; i < Num_Obstacles; i++)
	{
		obstacle_data[i][0] = randomFloat(-50, 50); //X
		obstacle_data[i][1] = randomFloat(-50, 50); //Y
		obstacle_data[i][2] = randomFloat(0.1, 10.0); //Scale
	}

	//INITIALIZE VEHICLE==
	initializeVehicles();

	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up


	//Since we use texture mapping, to simplify the task of texture mapping, 
	//and to clarify the demonstration of texture mapping, we consider 4 vertices per face.
	//Overall, we will have 24 vertices and we have 4 vertices to render the sky (a large square).
	//Therefore, we'll have 28 vertices in total.
	GLfloat vertices[NumVertices][3] = {

		{ -100.0, -100.0, 0.0 }, //Plane to walk on and a sky
		{ 100.0, -100.0, 0.0 },
		{ 100.0, 100.0, 0.0 },
		{ -100.0, 100.0, 0.0 },

		{ -0.45, -0.45 ,0.01 }, // bottom face
		{ 0.45, -0.45 ,0.01 },
		{ 0.45, 0.45 ,0.01 },
		{ -0.45, 0.45 ,0.01 },

		{ -0.45, -0.45 ,0.9 }, //top face
		{ 0.45, -0.45 ,0.9 },
		{ 0.45, 0.45 ,0.9 },
		{ -0.45, 0.45 ,0.9 },

		{ 0.45, -0.45 , 0.01 }, //left face
		{ 0.45, 0.45 , 0.01 },
		{ 0.45, 0.45 ,0.9 },
		{ 0.45, -0.45 ,0.9 },

		{ -0.45, -0.45, 0.01 }, //right face
		{ -0.45, 0.45 , 0.01 },
		{ -0.45, 0.45 ,0.9 },
		{ -0.45, -0.45 ,0.9 },

		{ -0.45, 0.45 , 0.01 }, //front face
		{ 0.45, 0.45 , 0.01 },
		{ 0.45, 0.45 ,0.9 },
		{ -0.45, 0.45 ,0.9 },

		{ -0.45, -0.45 , 0.01 }, //back face
		{ 0.45, -0.45 , 0.01 },
		{ 0.45, -0.45 ,0.9 },
		{ -0.45, -0.45 ,0.9 },
	};

	//These are the texture coordinates for the second texture
	GLfloat textureCoordinates[28][2] = {
		0.0f, 0.0f,
		200.0f, 0.0f,
		200.0f, 200.0f,
		0.0f, 200.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};


	//Creating our texture:
	//This texture is loaded from file. To do this, we use the SOIL (Simple OpenGL Imaging Library) library.
	//When using the SOIL_load_image() function, make sure the you are using correct patrameters, or else, your image will NOT be loaded properly, or will not be loaded at all.
	GLint width1, height1;
	unsigned char* textureData1 = SOIL_load_image("grass.png", &width1, &height1, 0, SOIL_LOAD_RGB);

	GLint width2, height2;
	unsigned char* textureData2 = SOIL_load_image("apple.png", &width2, &height2, 0, SOIL_LOAD_RGB);

	GLint width3, height3;
	unsigned char* textureData3 = SOIL_load_image("tank.png", &width3, &height3, 0, SOIL_LOAD_RGB);

	GLint width4, height4;
	unsigned char* textureData4 = SOIL_load_image("wheel.png", &width4, &height4, 0, SOIL_LOAD_RGB);

	GLint width5, height5;
	unsigned char* textureData5 = SOIL_load_image("ammo.png", &width5, &height5, 0, SOIL_LOAD_RGB);

	glGenBuffers(2, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vTexCoord");
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_matrix");
	cam_mat_location = glGetUniformLocation(program, "camera_matrix");
	proj_mat_location = glGetUniformLocation(program, "projection_matrix");

	///////////////////////TEXTURE SET UP////////////////////////

	//Allocating two buffers in VRAM
	glGenTextures(2, texture);		//MIGHT HAVE TO CHANGE to 3!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//First Texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, second texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData2);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, third texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[2]);

	//Loading the third texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width3, height3, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData3);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, fourth texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[3]);

	//Loading the fourth texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width4, height4, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData4);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, fifth texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[4]);

	//Loading the fifth texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width5, height5, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData5);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////
}

//Helper function to draw a cube
void drawCube(float scale)
{
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//Select the second texture (apple.png) when drawing the second geometry (cube)
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glDrawArrays(GL_QUADS, 4, 24);
}

//GameObject GO;
//should iterate through collection and draw the bullets
void drawBullets() {
	model_view = glm::scale(model_view, glm::vec3(0.2, 0.2, 0.2));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//model_view = glm::translate(model_view, glm::vec3(GO.location[0], GO.location[1], cam_pos.z));
	//model_view = glm::translate(model_view, glm::vec3(bullet_vertices[i][0], bullet_vertices[i][1], bullet_vertices[i][2]));
	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glDrawArrays(GL_QUADS, 4, 24);
}

void initializeVehicles()
{
	for (int i = 0; i < 2; i++)
	{
		GameObject vehicle;
		vehicle.setType('v');
		vehicle.setisAlive(true);
		vehicle.setlocation(randomFloat(-50, 50), randomFloat(-50, 50), 0.2f);
		vehicle.setDirection(-vehicle.getlocation()[0], -vehicle.getlocation()[1], 0.0f);
		gameScene.push_back(vehicle);
	}
}

void drawVehicle(float scale, glm::vec3 direction)
{
	//this is the base/box of the tank
	// this needs to be here otherwise vehicles are in floor
	model_view = glm::translate(model_view, glm::vec3(0.0f, 0.0f, 0.6f));
	model_view = glm::rotate(model_view, atan(direction.y / direction.x), unit_z_vector);
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glDrawArrays(GL_QUADS, 4, 24);

	//Drawing wheels
	//wheel 1
	glm::mat4 tmp_mv = model_view;
	model_view = glm::translate(model_view, glm::vec3(scale / 2.0, scale / 2.0, -0.5f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 2.0, scale / 2.0, scale / 2.0));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glDrawArrays(GL_QUADS, 4, 24);
	//drawCube(scale / 4);
	model_view = tmp_mv;
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//wheel 2
	model_view = glm::translate(model_view, glm::vec3(scale / 2.0 - 1, scale / 2.0, -0.5f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 2.0, scale / 2.0, scale / 2.0));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glDrawArrays(GL_QUADS, 4, 24);
	//drawCube(scale / 4);
	model_view = tmp_mv;
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//wheel 3
	model_view = glm::translate(model_view, glm::vec3(scale / 2.0, scale / 2.0 - 1, -0.5f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 2.0, scale / 2.0, scale / 2.0));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glDrawArrays(GL_QUADS, 4, 24);
	//drawCube(scale / 4);
	model_view = tmp_mv;
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//wheel 4
	model_view = glm::translate(model_view, glm::vec3(scale / 2.0 - 1, scale / 2.0 - 1, -0.5f));
	model_view = glm::rotate(model_view, wheel_rotation, glm::vec3(0, 1, 0));
	model_view = glm::scale(model_view, glm::vec3(scale / 2.0, scale / 2.0, scale / 2.0));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glDrawArrays(GL_QUADS, 4, 24);
	//drawCube(scale / 4);
	model_view = tmp_mv;
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
}



//return if two objects are colliding
bool isColliding(GameObject GO1, GameObject GO2) {
	bool collided = false;
	if (GO1.getisAlive() && GO2.getisAlive()) {
		glm::vec3 GO1Location = GO1.getlocation();
		glm::vec3 GO2Location = GO2.getlocation();
		float x_dist = abs(GO1Location[0] - GO2Location[0]);
		float y_dist = abs(GO1Location[1] - GO2Location[1]);

		if (x_dist <= 0.9 && y_dist <= 0.9) collided = true;
	}
	return collided;
}

// check if there are any collisions
void checkCollision() {
	for (int i = 0; i < gameScene.size(); i++) {
		for (int j = 0; j < gameScene.size(); j++) {
			if (isColliding(gameScene[i], gameScene[j]) && i != j) {
				gameScene[i].setisAlive(false);
				gameScene[j].setisAlive(false);

				if (gameScene[i].getType() != 'b' || gameScene[j].getType() != 'b') {
					initializeVehicles();
				}

				if (gameScene[i].getType() == 'b' || gameScene[j].getType() == 'b') {
					pointCounter++;
					std::cout << "Current points: " << pointCounter << std::endl;
				}
			}
		}
	}
}


void renderVehicle() {
	checkCollision();
	for (int i = 0; i < gameScene.size(); i++) {
		if (gameScene[i].getisAlive() == true) {
			if (gameScene[i].getType() == 'v') {
				wheel_rotation += 0.01f;
				moving_dir = glm::normalize(gameScene[i].getDirection());
				vehicle_move_dir = moving_dir;
				glm::vec3 tempDir = gameScene[i].getlocation();
				tempDir += vehicle_speed * vehicle_move_dir;
				gameScene[i].setlocation(tempDir[0], tempDir[1], tempDir[2]);


				model_view = glm::translate(model_view, gameScene[i].getlocation());
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
				drawVehicle(1.0, moving_dir);
				model_view = glm::mat4(1.0);
				glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

				glm::vec3 tempDirLocation = gameScene[i].getlocation();

				if (tempDirLocation[0] < 0.1 && tempDirLocation[1] < 0.1) {
					gameScene[i].setisAlive(false);
					initializeVehicles();
				}

				if (abs(tempDirLocation[0] - cam_pos.x) <= 0.45 && abs(tempDirLocation[1] - cam_pos.y) <= 0.45) {
					std::cout << "A Tank ran you over and you died." << std::endl;
					exit(0);
				}
			}
		}
	}
}


void updateScene() {
	for (int i = 0; i < gameScene.size(); i++) {
		if (gameScene[i].getType() == 'b') {
			glm::vec3 tempDir = gameScene[i].getlocation();
			tempDir += .1f * gameScene[i].getDirection();
			gameScene[i].setlocation(tempDir[0], tempDir[1], tempDir[2]);
		}
		else if (gameScene[i].getType() == 'v') {
			renderVehicle();
		}
	}
}

//Renders level
void draw_level()
{
	//Select the first texture (grass.png) when drawing the first geometry (floor)
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glDrawArrays(GL_QUADS, 0, 4);

	for (int i = 0; i < gameScene.size(); i++) {
		if (gameScene[i].getType() == 'b') {
			model_view = glm::translate(glm::vec3(gameScene[i].getlocation()));
			glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
			drawBullets();
			model_view = glm::mat4(1.0);
		}
	}

	//Rendering obstacles obstacles
	for (int i = 0; i < Num_Obstacles; i++)
	{
		model_view = glm::translate(model_view, glm::vec3(obstacle_data[i][0], obstacle_data[i][1], 0.0));
		glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
		drawCube(obstacle_data[i][2]);
		model_view = glm::mat4(1.0);
	}
}



//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	model_view = glm::mat4(1.0);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//The 3D point in space that the camera is looking
	glm::vec3 look_at = cam_pos + looking_dir_vector;

	glm::mat4 camera_matrix = glm::lookAt(cam_pos, look_at, up_vector);
	glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

	glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

	draw_level();

	updateScene();

	glFlush();
}


void keyboard(unsigned char key, int x, int y)
{
	if (key == 'a')
	{
		//Moving camera along opposit direction of side vector
		cam_pos += side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'd')
	{
		//Moving camera along side vector
		cam_pos -= side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 'w')
	{
		//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
		cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}
	if (key == 's')
	{
		//Moving camera along backward (negative forward) vector. To be more realistic, we use X=V.T equation in physics
		cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
	}

	if (key == 'f')
	{
		//set a static shoot vector on every bullet and push to collection via direction
		shoot_dir_vector = looking_dir_vector;
		GameObject GO;
		GO.setType('b');
		GO.setisAlive(true);
		GO.setlocation(cam_pos.x, cam_pos.y, cam_pos.z);
		GO.setDirection(shoot_dir_vector[0], shoot_dir_vector[1], shoot_dir_vector[2]);
		gameScene.push_back(GO);
	}
}

//Controlling Pitch with vertical mouse movement
void mouse(int x, int y)
{
	//Controlling Yaw with horizontal mouse movement
	int delta_x = x - x0;

	//The following vectors must get updated during a yaw movement
	forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	looking_dir_vector = glm::rotate(looking_dir_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	side_vector = glm::rotate(side_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	up_vector = glm::rotate(up_vector, -delta_x * mouse_sensitivity, unit_z_vector);
	x0 = x;

	//The following vectors must get updated during a pitch movement
	int delta_y = y - y_0;
	glm::vec3 tmp_up_vec = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
	glm::vec3 tmp_looking_dir = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);

	//The dot product is used to prevent the user from over-pitch (pitching 360 degrees)
	//The dot product is equal to cos(theta), where theta is the angle between looking_dir and forward vector
	GLfloat dot_product = glm::dot(tmp_looking_dir, forward_vector);

	//If the angle between looking_dir and forward vector is between (-90 and 90) degress 
	if (dot_product > 0)
	{
		up_vector = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
		looking_dir_vector = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);
	}
	y_0 = y;
}

void idle()
{
	//Calculating the delta time between two frames
	//We will use this delta time when moving forward (in keyboard function)
	timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	//std:cout << oldTimeSinceStart << std::endl;

	if (pointCounter == 10) {
		exit(0);
	}
	glutPostRedisplay();
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Camera and Projection");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();
}
