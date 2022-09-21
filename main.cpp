/*******************************************************************
		   Multi-Part Model Construction and Manipulation
********************************************************************/


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include "Vector3D.h"
#include "QuadMesh.h"
#include "CubeMesh.h"
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define M_PI 3.1415926535897932384626433832795

const int meshSize = 64;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

static int m_xT = 0;
static int m_yT = 0;
static double c_x = 0.0;
static double c_y = 6.0;
static double c_z = 22.0;
static double perView = 90;
GLint leftMouseButton, rightMouseButton;
static double cRad = 12.0;
static double cTheta = 0.0;
static double cPhi = 2.0;
static int mode = 0;
struct Blob {
	double x;
	double y;
	double z;
	double width;
	double height;
};

struct torp {
	double x;
	double y;
	double z;
	double EyView;
};

std::vector<Blob> blobs;
Blob workingBlob;
std::vector<torp> torps;
torp workingTorp;


static int texWidth;
static int texHeight;
static int nrChannels;
static unsigned int sandTexture;
static unsigned int subTexture;
static unsigned int EsubTexture;
static unsigned int chainTexture;
static unsigned int steelTexture;
static unsigned int torpTexture;




// Lighting/shading and material properties for drone - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat drone_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat drone_mat_shininess[] = { 0.0F };

// A quad mesh representing the ground
static QuadMesh groundMesh;
static CubeMesh cube;

// Structure defining a bounding box
struct BoundingBox {
	Vector3D min;
	Vector3D max;
};
BoundingBox subBBox, groundBBox, BBox, BBox2, BBox3,BBox4, EsubBBox, torpBBox;



// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void moveCamera(void);
void updateMesh(void);
//movement Properties
//static GLfloat xView = 0; // used to keep up and down rotation of sub
static GLfloat yView = 0; // used to keep left and right rotation of sub
static GLfloat xPos = 0; // used to keep x position of sub
static GLfloat yPos = 0; // used to keep y position of sub
static GLfloat zPos = 0; // used to keep z position of sub
static GLfloat spin = 0; // used to keep spin angle of propeller
static GLfloat engine = 0; // used to keep state of engine ie on/off
static GLfloat crashed = 0; //state of sub
static GLfloat tmode = 0; //state of sub view


static GLfloat ExPos = -7; // used to keep x position of Esub
static GLfloat EzPos = -7; // used to keep y position of Esub
static int score = 0;

GLUquadric* quad = gluNewQuadric();





int main(int argc, char** argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 2");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);
	moveCamera();

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{

	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	//Textures
	unsigned char* sandData = stbi_load("sandText.jpg", &texWidth, &texHeight, &nrChannels, 0);
	if (sandData)
	{
		glGenTextures(1, &sandTexture);
		glBindTexture(GL_TEXTURE_2D, sandTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, sandData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		printf("Didnt load texture sand");
	}
	unsigned char* subData = stbi_load("subText.jpg", &texWidth, &texHeight, &nrChannels, 0);
	if (subData)
	{
		glGenTextures(1, &subTexture);
		glBindTexture(GL_TEXTURE_2D, subTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, subData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		printf("Didnt load texture Esub");
	}
	unsigned char* EsubData = stbi_load("EsubText.jpg", &texWidth, &texHeight, &nrChannels, 0);
	if (EsubData)
	{
		glGenTextures(1, &EsubTexture);
		glBindTexture(GL_TEXTURE_2D, EsubTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, EsubData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		printf("Didnt load texture sub");
	}
	unsigned char* steelData = stbi_load("steelText.jpg", &texWidth, &texHeight, &nrChannels, 0);
	if (steelData)
	{
		glGenTextures(1, &steelTexture);
		glBindTexture(GL_TEXTURE_2D, steelTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, steelData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		printf("Didnt load texture steel");
	}
	unsigned char* chainData = stbi_load("chainText.jpg", &texWidth, &texHeight, &nrChannels, 0);
	if (chainData)
	{
		glGenTextures(1, &chainTexture);
		glBindTexture(GL_TEXTURE_2D, chainTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, chainData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		printf("Didnt load texture chain");
	}
	unsigned char* torpData = stbi_load("torpText.jpg", &texWidth, &texHeight, &nrChannels, 0);
	if (torpData)
	{
		glGenTextures(1, &torpTexture);
		glBindTexture(GL_TEXTURE_2D, torpTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, torpData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		printf("Didnt load texture torp");
	}
	
	gluQuadricTexture(quad, true);

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	

	// Set up ground quad mesh
	Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh, meshSize, origin, 16.0, 16.0, dir1v, dir2v);
	//setup blobs && their bounding box
	workingBlob.x = -2;
	workingBlob.z = -4;
	workingBlob.y = 1;
	workingBlob.height = 4.0;
	workingBlob.width = 10.0;
	Set(&BBox.min, -2.5, 0, -5);
	Set(&BBox.max,  -1.5, 4, -3);
	blobs.push_back(workingBlob);
	workingBlob.x = 2;
	workingBlob.z = 3;
	workingBlob.y = 1;
	workingBlob.height = 2.0;
	workingBlob.width = 10.0;
	Set(&BBox2.min, 1.5, 0, 2);
	Set(&BBox2.max, 2.5, 2, 4);
	blobs.push_back(workingBlob);
	workingBlob.x = -3;
	workingBlob.z = 7;
	workingBlob.y = 1;
	workingBlob.height = 6.0;
	workingBlob.width = 10.0;
	Set(&BBox3.min, -3.5, 0, -7.5);
	Set(&BBox3.max, -2.5, 6, -6.5);
	blobs.push_back(workingBlob);
	updateMesh();

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);
	

	//Set up the bounding boxs for ground
	
	Set(&groundBBox.min, -8.0f, -1.0, -8.0);
	Set(&groundBBox.max, 8.0f, 0.0,  8.0);

	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}
void moveCamera() {
	c_x = cRad * sinf(cTheta) * sinf(cPhi);
	c_z = cRad * -cosf(cTheta) * sinf(cPhi);
	c_y = cRad * -cosf(cPhi);
	if (c_y < 0.3) c_y = 0.3;
	glutPostRedisplay();
}


	bool colliding(const BoundingBox & tBox1, const BoundingBox & tBox2)
	{
		Set(&subBBox.min, xPos-0.25, yPos, zPos-0.25);
		Set(&subBBox.max, xPos+0.25, yPos+1, zPos+0.25);
		
		return(tBox1.max.x > tBox2.min.x&&
			tBox1.min.x < tBox2.max.x &&
			tBox1.max.y > tBox2.min.y&&
			tBox1.min.y < tBox2.max.y &&
			tBox1.max.z > tBox2.min.z&&
			tBox1.min.z < tBox2.max.z);


	}




double distance(double x, double y, double x2, double y2) {
	return sqrt((pow((x2 - x) ,2)) + (pow((y2 - y),2)));
}
double gaussian(double x, double z) {
	int sum = 0;
	for (auto blob : blobs) {
		double r = distance(blob.x, blob.z, x, z);
		sum += blob.height * (exp((-blob.width * pow(r, 2))));
	}
	return sum;
}
void updateMesh(void) {

	for (int i = 0; i < meshSize*meshSize + 1; i++) {
		groundMesh.vertices[i].position.y = gaussian(groundMesh.vertices[i].position.x, groundMesh.vertices[i].position.z);
	}
	ComputeNormalsQM(&groundMesh);
	
	glutPostRedisplay();
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis

	glLoadIdentity();
	if (tmode == 0)
	{
		gluLookAt(c_x, c_y, c_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}
	else if (tmode == 1) {
		double xlook = (xPos - (5 * sinf((perView * M_PI) / 180)));
		double zlook = (zPos - (5 * cosf((perView * M_PI) / 180)));
		gluLookAt(xPos - xlook, yPos + 1, zPos - zlook, xlook, yPos + 1, zlook, 0.0, 1, 0.0);
	}

	else if (tmode == 2) {
		double xlook = (xPos - (5 * sinf((perView * M_PI) / 180)));
		double zlook = (zPos - (5 * cosf((perView * M_PI) / 180)));
		gluLookAt(xPos, yPos + 1, zPos, xlook, yPos, zlook, 0.0, 1, 0.0);
	}
	


	// Draw ground mesh
	DrawMeshQM(&groundMesh, meshSize, sandTexture);

	//extra Object
	glPushMatrix();
	glTranslatef(4.5, 1.0, 4.5);
	glScalef(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, steelTexture);
	gluSphere(quad,0.4, 20, 20);
	glDisable(GL_TEXTURE_2D);
	glRotatef(90.0, 1.0, 0.0, 0.0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, chainTexture);
	gluCylinder(quad,0.05,0.05, 1, 20, 20);
	glDisable(GL_TEXTURE_2D);
	Set(&BBox4.min, 3.5, 0, 3.5);
	Set(&BBox4.max, 4.5, 2, 4.5);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(xPos, yPos, zPos);
									//xPos,yPos are the coordinates of the sub saved globally
	glRotatef(yView, 0.0, 1.0, 0.0);
									//yView is subs left and right rotation

	glScalef(0.2, 0.2, 0.2);
	if (crashed == 0) {
		//Hull
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, subTexture);
		glTranslatef(0.0, 2.0, 0.0);
		glPushMatrix();
		glScalef(1, 1, 5);
		gluSphere(quad,1, 20, 20);
		glPopMatrix();
		//bridge
		glPushMatrix();
		glTranslatef(0.0, 0.8, 1.0);
		glRotatef(90.0, -1.0, 0.0, 0.0);
		glPushMatrix();
		glScalef(0.25, 0.6, 0.5);
		gluCylinder(quad,1,1, 2, 20, 20);
		glPopMatrix();
		
		if (engine == 0) {
		glScalef(0.2, 0.2, 2);
		gluCylinder(quad,0.5,0.5, 1, 20, 20);
		glPopMatrix();
		
		
			glPushMatrix();
			glTranslatef(0.0, 2.5, 1);
			glRotatef(perView, 0.0, 1.0, 0.0);
			glScalef(0.2, 0.2, 0.4);
			gluCylinder(quad,0.5,0.5, 1, 20, 20);
			glPopMatrix();

			
		}
		else {
			glPopMatrix();
		}
			

		//front wings
		glPushMatrix();
		glTranslatef(0.0, 0.0, 1.5);
		glScalef(0.8, 0.05, 0.5);
		gluCylinder(quad,2,2, 1, 22, 20);
		glPopMatrix();
		//back propeller
		glPushMatrix();
		glRotatef(spin, 0.0, 0.0, 1.0);
		glTranslatef(0.0, 0.0, -4.8);
		glPushMatrix();
		glScalef(1.0, 1.0, 5.0);
		glutSolidTorus(0.1, 0.4, 20, 20);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.0, 0.0, -0.3);
		glPushMatrix();
		glScalef(0.6, 0.05, 0.5);
		gluCylinder(quad,2,2, 1, 22, 2);
		glPopMatrix();
		glPushMatrix();
		glScalef(0.05, 0.6, 0.5);
		gluCylinder(quad,2,2, 1, 22, 2);
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
	}
	else {
		glPopMatrix();
	}
	glDisable(GL_TEXTURE_2D);
	//enemy
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, EsubTexture);
	glPushMatrix();
	glTranslatef(ExPos, 0, EzPos);
	//xPos,yPos are the coordinates of the enemy sub saved globally
	glScalef(0.2, 0.2, 0.2);
	
		//Hull
		glTranslatef(0.0, 2.0, 0.0);
		glPushMatrix();
		glScalef(1, 1, 5);
		gluSphere(quad,1, 20, 20);
		glPopMatrix();
		//bridge
		glPushMatrix();
		glTranslatef(0.0, 0.8, 1.0);
		glRotatef(90.0, -1.0, 0.0, 0.0);
		glPushMatrix();
		glScalef(0.25, 0.6, 0.5);
		gluCylinder(quad,1,1, 2, 20, 20);
		glPopMatrix();
		glPopMatrix();
		//front wings
		glPushMatrix();
		glTranslatef(0.0, 0.0, 1.5);
		glScalef(0.8, 0.05, 0.5);
		gluCylinder(quad,2,2, 1, 22, 20);
		glPopMatrix();
		//back propeller
		glPushMatrix();
		glTranslatef(0.0, 0.0, -4.8);
		glPushMatrix();
		glScalef(1.0, 1.0, 5.0);
		glutSolidTorus(0.1, 0.4, 20, 20);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.0, 0.0, -0.3);
		glPushMatrix();
		glScalef(0.6, 0.05, 0.5);
		gluCylinder(quad,2,2, 1, 22, 2);
		glPopMatrix();
		glPushMatrix();
		glScalef(0.05, 0.6, 0.5);
		gluCylinder(quad,2,2, 1, 22, 2);
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);

		std::vector<torp> hold;
		for (auto torpe : torps) {
			torp temp;
			temp.x = torpe.x;
			temp.y = torpe.y;
			temp.z = torpe.z;
			temp.EyView = torpe.EyView;
			torps.pop_back();

			temp.x = temp.x + 0.5 * sinf(temp.EyView * M_PI / 180);
			temp.z = temp.z + 0.5 * cosf(temp.EyView * M_PI / 180);


			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, torpTexture);
			glPushMatrix();
			glTranslatef(temp.x, temp.y, temp.z);
			glRotatef(temp.EyView, 0.0, 1.0, 0.0);

			glScalef(0.2, 0.2, 0.2);
			glPushMatrix();
			glScalef(0.3, 0.3, 0.3);
			glTranslatef(0.0, 2.0, 0.0);
			glPushMatrix();
			glScalef(1, 1, 5);
			gluSphere(quad, 1, 20, 20);
			glPopMatrix();
			glPushMatrix();
			glTranslatef(0.0, 0.0, -4.8);
			glPushMatrix();
			glTranslatef(0.0, 0.0, -0.3);
			glPushMatrix();
			glScalef(0.6, 0.05, 0.5);
			gluCylinder(quad, 2, 2, 1, 22, 2);
			glPopMatrix();
			glPushMatrix();
			glScalef(0.05, 0.6, 0.5);
			gluCylinder(quad, 2, 2, 1, 22, 2);
			glPopMatrix();
			glPopMatrix();
			glPopMatrix();
			glPopMatrix();
			glPopMatrix();
			hold.push_back(temp);
			glDisable(GL_TEXTURE_2D);
		}
		for (auto torpe : hold) {
			int alive = 1;
			torp temp;
			temp.x = torpe.x;
			temp.y = torpe.y;
			temp.z = torpe.z;
			temp.EyView = torpe.EyView;
			hold.pop_back();
			Set(&torpBBox.min, temp.x-0.25, temp.y-0.5, temp.z-0.25);
			Set(&torpBBox.max, temp.x+0.25, temp.y+0.5, temp.z+0.25);
			if (colliding(torpBBox, EsubBBox)) {
				EzPos = -7;
				Set(&EsubBBox.min, ExPos - 0.75, 0, EzPos - 0.25);
				Set(&EsubBBox.max, ExPos + 0.75, 0 + 1, EzPos + 0.25);
				score += 1;
				printf("hit score is now :%d\n",score);
					continue;
			}
			else if (colliding(torpBBox, BBox)) {
				continue;
			}
			else if (colliding(torpBBox, BBox2)) {
				continue;
			}
			else if (colliding(torpBBox, BBox3)) {
				continue;
			}
			else if (colliding(torpBBox, BBox4)) {
				continue;
			}
			else if (temp.x > 8 || temp.x < -8 || temp.z>8 || temp.z < -8) {
				continue;
			}

				torps.push_back(temp);
		}

	glutSwapBuffers();   // Double buffering, swap buffers

	if (EzPos < 7) {
		EzPos += 0.2;
	}
	else {
		EzPos = -7;
	}
	Set(&EsubBBox.min, ExPos - 0.75, 0, EzPos - 0.25);
	Set(&EsubBBox.max, ExPos + 0.75, 0 + 1, EzPos + 0.25);

	if (colliding(EsubBBox, subBBox)) {
		crashed = 1;
		EzPos = -7;
	}



}


// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(c_x, c_y, c_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y){
if (crashed == 0) {
	
		switch (key)
		{
		case 't':
			if (tmode == 0)
				tmode = 1;
			else if (tmode == 1)
				tmode = 2;
			else
				tmode = 0;
			break;
		case 'd':
			if (engine == 1) {
				yView -= 2.5;
			}
			break;
		case 'a':
			if (engine == 1) {
				yView += 2.5;
			}
			break;
		case 'z':
			if (engine == 1) {
				yPos += 0.5;
				if (colliding(subBBox, groundBBox)) {
					printf("crashed");
					crashed = 1;
				}
			}
			break;
		case 'c':
			if (engine == 1) {
				yPos -= 0.5;
				if (colliding(subBBox, BBox) || colliding(subBBox, BBox2) || colliding(subBBox, BBox3) || colliding(subBBox, BBox4) || colliding(subBBox, groundBBox)) {

					printf("crashed");
					crashed = 1;
				}
			}
			break;
		case 's':
			if (engine == 1) {
				xPos -= 0.5 * sinf(yView * M_PI / 180);// drive backwards based on rotation of sub
				zPos -= 0.5 * cosf(yView * M_PI / 180);
				spin += 20; // Update rotation angle of propeller
				if (colliding(subBBox, BBox) || colliding(subBBox, BBox2) || colliding(subBBox, BBox3) || colliding(subBBox, BBox4)) {
					printf("crashed");
					crashed = 1;
				}
			}
			break;
		case 'w':
			if (engine == 1) {
				xPos += 0.5 * sinf(yView * M_PI / 180);// drive forward based on rotation of sub
				zPos += 0.5 * cosf(yView * M_PI / 180);
				spin -= 20; //Update rotation angle of propeller
				if (colliding(subBBox, BBox) || colliding(subBBox, BBox2) || colliding(subBBox, BBox3) || colliding(subBBox, BBox4)) {
					printf("crashed");
					crashed = 1;
				}
			}
			break;
		case 'x': // switch state of engine on each press
			if (engine == 0) {
				engine = 1;
			}
			else {
				engine = 0;
			}
			break;

		case 'f':
			workingTorp.x = xPos;
			workingTorp.z = yPos;
			workingTorp.y = zPos;
			workingTorp.EyView = yView;
			torps.push_back(workingTorp);
			break;

		case 'g':
			if (tmode != 0) {
				yView = perView + 180;
				moveCamera();

			}
			break;
		}
		glutPostRedisplay();
	}
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	if (crashed == 0) {
		// Help key
		if (key == GLUT_KEY_F1)
		{
			printf("----------Controlls----------\n\nChange modes\n\nx:start/stop engine\nwhen engine is off periscope is raised\nt:change view\n\n-----------Driving-----------\n\nengine must be on\nz:raise sub\nc:lower sub\na:turn left\nd:turn right\nw:drive forward\ns:drive backwards\nf:fire torpedo\n\n---------Normal View---------\n\nUp arrow: zoom out\nDown arrow: zoom in\nClick and drag mouse to turn camera\n\n--------periscope View------ - \n\nleft arrow : turn view right\nright arrow : turn view left\ng : turn sub to face current direction\n\n");

		}
		// Do transformations with arrow keys
		else if (key == GLUT_KEY_RIGHT)
		{
			perView += 2.5;
		}
		else if (key == GLUT_KEY_LEFT)
		{
			perView -= 2.5;
		}
		else if (key == GLUT_KEY_UP && cRad < 22.0)
		{
			cRad += 1.0;

			moveCamera();

		}
		else if (key == GLUT_KEY_DOWN && cRad > 1.0)
		{
			cRad -= 1.0;
			moveCamera();

		}

		glutPostRedisplay();   // Trigger a window redisplay
	}
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	//update the left and right mouse button states
	if (button == GLUT_LEFT_BUTTON)
		leftMouseButton = state;
	else if (button == GLUT_RIGHT_BUTTON)
		rightMouseButton = state;

	//update coordinates of the mouse
	m_xT = x;
	m_yT = y;



	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)

{
	if (mode == 0) {
		cTheta += (xMouse - m_xT) * 0.005;
		cPhi += (yMouse - m_yT) * 0.005;
		if (cPhi <= 0)
			cPhi = 0 + 0.001;
		if (cPhi >= 3.141)
			cPhi = 3.141 - 0.001;
		moveCamera();
		m_xT = xMouse;
		m_yT = yMouse;
		xMouse = c_x;
		yMouse = c_y;
		glutPostRedisplay();   // Trigger a window redisplay
	}
	if (mode == 1) {
		glutPostRedisplay();
	}
}


Vector3D ScreenToWorld(int x, int y)
{
	// you will need to finish this if you use the mouse
	return NewVector3D(0, 0, 0);
}