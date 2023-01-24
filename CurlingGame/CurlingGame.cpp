// Pool Game.cpp : Defines the entry point for the console application.
//

#include <stdlib.h>
#include "stdafx.h"
#include <glut.h>
#include "simulation.h"
#include <string.h>


//cue variables
float gCueAngle = 0.0;
float gCuePower = 0.25;
bool gCueControl[6] = { false,false,false,false,false,false };
float gCueAngleSpeed = 2.0f; //radians per second
float gCuePowerSpeed = 0.25f;
float gCuePowerMax = 0.75;
float gCuePowerMin = 0.1;
float gCueBallFactor = 3.0;
bool gDoCue = true;

float gCueRotation = 0.0;
float gCueRotationPowerMax = 1.0;
float gCueRotationPowerMin = -1.0;

int currentPlayer = 0;
int activeSlate = 0;
char textScore[50];
char textPower[20];
char textRotation[20];
int len;
int* closestBallPtr = 0;
int winningTeam=0;

//camera variables
float initpos = 90.0;
float r = 1.0;
float ballX = 0.0;
float ballZ = 0.75;
float theta = TWO_PI * float(initpos) / float(360);
float x = r * cosf(theta) + ballX;//calculate the x component 
float z = r * sinf(theta) + ballZ;//calculate the z component

vec3 gCamPos(x, BALL_RADIUS + 0.5, z);
vec3 gCamLookAt(0.0, BALL_RADIUS, 0.75);
/*-----------------------------------------------------------
  Start of Setup for game, these are initial setups that are affected if and when needed
  -----------------------------------------------------------*/

//Begin the setup of the game, all aspects above.
//Initial render of the scene, making all objects appear on tab, This is an idle Function so will be called when a change occurs
void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set camera
	glLoadIdentity();
	gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);

	//draw the ball
	//glColor3f(1.0,1.0,1.0);
	for (int i = 0; i < gTable.bc.num; i++)
	{
		glPushMatrix();
		if (i % 2) glColor3f(0.0, 0.0, 1.0); else glColor3f(1.0, 0.0, 0.0);
		glTranslatef(gTable.bc.balls[i]->position(0), BALL_RADIUS, gTable.bc.balls[i]->position(1));
		glutSolidSphere(gTable.bc.balls[i]->radius, 20, 20);
		glPopMatrix();
	}
	
	//draw the table
	for (int i = 0; i < NUM_SLATES; i++)
	{
		for (int j = 0; j < NUM_CUSHIONS; j++)
		{
			glColor3f(0.0, 0.0, 1.0);
			glBegin(GL_QUADS);
			glVertex3f(gTable.slate[i].slateWalls[j]->vertices[0](0), -0.00001, gTable.slate[i].slateWalls[j]->vertices[0](1));
			glVertex3f(gTable.slate[i].slateWalls[j]->vertices[0](0), 0.05, gTable.slate[i].slateWalls[j]->vertices[0](1));
			glVertex3f(gTable.slate[i].slateWalls[j]->vertices[1](0), 0.05, gTable.slate[i].slateWalls[j]->vertices[1](1));
			glVertex3f(gTable.slate[i].slateWalls[j]->vertices[1](0), -0.00001, gTable.slate[i].slateWalls[j]->vertices[1](1));
			glEnd();
		}
	}
	for (int i = 0; i < NUM_SLATES; i++)
	{
		//DRAW floor
		glBegin(GL_QUADS);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(TABLE_X+gTable.slate[i].floor, -0.00001, TABLE_Z);
		glVertex3f(TABLE_X + gTable.slate[i].floor, -0.00001, -TABLE_Z);
		glVertex3f(-TABLE_X + gTable.slate[i].floor, -0.00001, -TABLE_Z);
		glVertex3f(-TABLE_X + gTable.slate[i].floor, -0.00001, TABLE_Z);
		glEnd();
	}
	
	//draw particles
	for (int i = 0; i < gTable.parts.num; i++)
	{
		if (winningTeam == 0)glColor3f(1.0, 0.0, 0.0); else glColor3f(0.0, 0.0, 1.0);
		glPushMatrix();
		glTranslatef(gTable.parts.particles[i]->position(0), gTable.parts.particles[i]->position(1), gTable.parts.particles[i]->position(2));
		glutSolidSphere(0.005f, 32, 32);
		glPopMatrix();
	}
	//draw the pockets
	for (int i = 0; i < NUM_SLATES; i++){
		float y = 0.0001;
		for (int j = NUM_POCKETS - 1; j > -1; j--)
		{
			if (j == 3)glColor3f(0.0, 0.0, 1.0);
			if (j == 2)glColor3f(1.0, 1.0, 1.0);
			if (j == 1)glColor3f(1.0, 0.0, 0.0);
			if (j == 0)glColor3f(1.0, 1.0, 1.0);
			glBegin(GL_POLYGON);
			int num_segments = 360;
			float cx = gTable.slate[i].floor;
			float cz = -0.8;
			float r = gTable.pockets[j].radius;
			for (int k = 0; k < num_segments; k++) {
				float theta = TWO_PI * float(k) / float(num_segments);//get the current angle 
				float x = r * cosf(theta);//calculate the x component 
				float z = r * sinf(theta);//calculate the z component 
				glVertex3f(x + cx, y, z + cz);//output vertex 
			}
			y += 0.0001;
			glEnd();
		}
	}
	
	for (int i = 0; i < NUM_SLATES; i++)
	{
		//red team
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(gTable.slate[i].floor - 0.1, 0.25, -TABLE_Z);
		sprintf_s(textScore, "Score Red: %d", gTable.slate[i].redScore);
		len = strlen(textScore);
		for (int i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, textScore[i]);
		}
		//blue team
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(gTable.slate[i].floor - 0.1, 0.20, -TABLE_Z);
		sprintf_s(textScore, "Score Blue: %d", gTable.slate[i].blueScore);
		len = strlen(textScore);
		for (int i = 0; i < len; i++) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, textScore[i]);
		}
	}
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		float x = 0.18;
		glColor3f(1.0, 1.0, 1.0);
		glRasterPos3f(-0.35+(x*i), 0.10, -TABLE_Z);
		sprintf_s(textScore, "P%d: %d", i + 1, gTable.player[i].score);
		len = strlen(textScore);
		for (int j = 0; j < len; j++) {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, textScore[j]);
		}
	}
	//power
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3f(gTable.slate[activeSlate].floor -TABLE_X-0.05, 0.20, -TABLE_Z);
	int gcp = gCuePower * 100;
	sprintf_s(textPower, "Power: %d", gcp); 
	len = strlen(textPower);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, textPower[i]);
	}
	//rotation
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3f(gTable.slate[activeSlate].floor + TABLE_X-0.05, 0.20, -TABLE_Z);
	int gcr = gCueRotation * 100;
	int gcrt;
	if (gcr < 0)gcrt = gcr * -1, sprintf_s(textRotation, "Rotate Right: %d", gcrt), len = strlen(textRotation);
	if (gcr > 0)sprintf_s(textRotation, "Rotate Left: %d", gcr), len = strlen(textRotation);
	if (gcr == 0)sprintf_s(textRotation, "Rotate: %d", gcr), len = strlen(textRotation);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, textRotation[i]);
	}
	//currentPlayer
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3f(gTable.slate[activeSlate].floor - TABLE_X - 0.05, 0.30, -TABLE_Z);
	sprintf_s(textPower, "CurrentPlayer: %d", currentPlayer+1); //gcp
	len = strlen(textPower);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, textPower[i]);
	}

	//glPopMatrix();

	glFlush();
	glutSwapBuffers();
}

//Keyboard function for when key IS pressed down (Arrow keys used for aiming cue, new camera movement)

void SpecKeyboardFunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
	{
		gCueControl[0] = true;
		break;
	}
	case GLUT_KEY_RIGHT:
	{
		gCueControl[1] = true;
		break;
	}
	case GLUT_KEY_UP:
	{
		gCueControl[2] = true;
		break;
	}
	case GLUT_KEY_DOWN:
	{
		gCueControl[3] = true;
		break;
	}
	}
}

//Keyboard function for when key IS NOT pressed down (Arrow keys used for aiming cue, new camera movement)

void SpecKeyboardUpFunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
	{
		gCueControl[0] = false;
		break;
	}
	case GLUT_KEY_RIGHT:
	{
		gCueControl[1] = false;
		break;
	}
	case GLUT_KEY_UP:
	{
		gCueControl[2] = false;
		break;
	}
	case GLUT_KEY_DOWN:
	{
		gCueControl[3] = false;
		break;
	}
	}
}

//Keyboard function for when key IS pressed down (Hitting ball, reseting game, moving camera(old))

void KeyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	//Enter key
	case(13):
	{
		if (gDoCue)
		{
			vec2 imp((-sin(gCueAngle) * gCuePower * gCueBallFactor),
				(-cos(gCueAngle) * gCuePower * gCueBallFactor));
			//Stage y
			gTable.bc.balls[gTable.bc.num-1]->ApplyImpulse(imp);
		}
		break;
	}
	
	case('q'):
	{
		gCueControl[4] = true;
		break;
	}
	case('e'):
	{
		gCueControl[5] = true;
		break;
	}
	}
}

//Keyboard function for when key IS NOT pressed down (Hitting ball, reseting game, moving camera(old))

void KeyboardUpFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case(48):
	{	
		vec2 pos(gTable.slate[0].floor, 0.75);
		if (gTable.AnyBallsMoving() == false)gTable.bc.gReset(), activeSlate = 0, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gTable.bc.AddBall(pos); break; 
	}
	case(49):
	{
		vec2 pos(gTable.slate[1].floor, 0.75);
		if (gTable.AnyBallsMoving() == false)gTable.bc.gReset(), activeSlate = 1, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gTable.bc.AddBall(pos); break;
	}
	case(50):
	{
		vec2 pos(gTable.slate[2].floor, 0.75);
		if (gTable.AnyBallsMoving() == false)gTable.bc.gReset(), activeSlate = 2, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gTable.bc.AddBall(pos); break;
	}
	case(51):
	{	
		vec2 pos(gTable.slate[3].floor, 0.75);
		if (gTable.AnyBallsMoving() == false)gTable.bc.gReset(), activeSlate = 3, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gTable.bc.AddBall(pos); break;
	}
	case(52):
	{
		vec2 pos(gTable.slate[4].floor, 0.75);
		if (gTable.AnyBallsMoving() == false)gTable.bc.gReset(), activeSlate = 4, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gTable.bc.AddBall(pos); break;
	}
	case('q'):
	{
		gCueControl[4] = false;
		break;
	}
	case('e'):
	{
		gCueControl[5] = false;
		break;
	}
	}
}

/*-----------------------------------------------------------
End of Setup for game, these are initial setups that are later affected if and when needed
-----------------------------------------------------------*/

/*-----------------------------------------------------------
Start of Setups for game that can be affectd once the game starts
-----------------------------------------------------------*/

//The ability to change the size of the application screen and the game continues to look appropriate
void ChangeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0) h = 1;
	float ratio = 1.0 * w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45, ratio, 0.2, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0.0,0.7,2.1, 0.0,0.0,0.0, 0.0f,1.0f,0.0f);
	gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);
}

//Lights for the game

void InitLights(void)
{
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	GLfloat light_ambient[] = { 2.0, 2.0, 2.0, 1.0 };
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);
}

/*-----------------------------------------------------------
End of Setups for game that can be affected once the game starts
-----------------------------------------------------------*/
#include <thread>

/*-----------------------------------------------------------
The main class used to update the game in ms
-----------------------------------------------------------*/
//Stage 1
void UpdateScene(int ms)
{
	vec2 pos(gTable.slate[activeSlate].floor, 0.75);
	if (currentPlayer > NUM_PLAYERS_PER_SLATE * (activeSlate + 1) - 1) currentPlayer -= NUM_PLAYERS_PER_SLATE;
	//if (gTable.bc.num - 1 == -1) currentPlayer = 0; else currentPlayer = gTable.bc.num - 1;
	//tests if any balls are moving, if false, draw cue
	if (gTable.AnyBallsMoving() == false) gDoCue = true;
	//else balls are moving, dont draw cue
	else gDoCue = false;
	//If cue is true, activate movement of cue functions to be allowed
	if (gDoCue)
	{
		//Left control
		if (gCueControl[0]) {
			initpos -= 0.5;
			gCueAngle = -(initpos - 90) * TWO_PI / 360;
			theta = TWO_PI * float(initpos) / float(360);
			x = *gTable.calcX(gTable.bc.num - 1, theta, r);
			z = *gTable.calcZ(gTable.bc.num - 1, theta, r);
			gCamPos(0) = x;
			gCamPos(1) = BALL_RADIUS + 0.5;
			gCamPos(2) = z;
		}
		//Right control
		if (gCueControl[1]) {
			initpos += 0.5;
			gCueAngle = -(initpos - 90) * TWO_PI / 360;
			theta = TWO_PI * float(initpos) / float(360);
			x = *gTable.calcX(gTable.bc.num - 1, theta, r);
			z = *gTable.calcZ(gTable.bc.num - 1, theta, r);
			gCamPos(0) = x;
			gCamPos(1) = BALL_RADIUS + 0.5;
			gCamPos(2) = z;
		}
		//Increase power
		if (gCueControl[2]) gCuePower += ((gCuePowerSpeed * ms) / 1000);
		//Decrease power
		if (gCueControl[3]) gCuePower -= ((gCuePowerSpeed * ms) / 1000);
		//Control power to stay within bounds
		if (gCuePower > gCuePowerMax) gCuePower = gCuePowerMax;
		if (gCuePower < gCuePowerMin) gCuePower = gCuePowerMin;

		if (gCueControl[4]) gCueRotation += ((gCuePowerSpeed * ms) / 1000);
		if (gCueControl[5]) gCueRotation -= ((gCuePowerSpeed * ms) / 1000);
		if (gCueRotation > gCueRotationPowerMax) gCueRotation = gCueRotationPowerMax;
		if (gCueRotation < gCueRotationPowerMin) gCueRotation = gCueRotationPowerMin;
	}

	gCamLookAt(0) = gTable.bc.balls[gTable.bc.num - 1]->position(0);
	gCamLookAt(1) = BALL_RADIUS;
	gCamLookAt(2) = gTable.bc.balls[gTable.bc.num - 1]->position(1);

	if (gTable.bc.balls[gTable.bc.num - 1]->impTrue == true) if (gTable.AnyBallsMoving() == false) gTable.bc.AddBall(pos),
		gCueAngle = 0.0, gCuePower = 0.25, gCueRotation = 0.0, currentPlayer++;

	if (gTable.bc.gameEnd == true)
	{
		closestBallPtr = gTable.calculateScore(gTable.slate[activeSlate].floor);
		int playerScoreIndex = 1;
		if (closestBallPtr[0] % 2 == 0) winningTeam = 0, gTable.slate[activeSlate].redScore += 1; else winningTeam = 1, gTable.slate[activeSlate].blueScore += 1;
		for (int i = 1; i < NUM_BALLS; i++)
		{
			//test if even for first team
			if (winningTeam == 0) if (closestBallPtr[i] % 2 == 0) gTable.slate[activeSlate].redScore += 1, playerScoreIndex++; else break;
			//test if odd for second team
			if (winningTeam == 1) if (closestBallPtr[i] % 2 != 0) gTable.slate[activeSlate].blueScore += 1, playerScoreIndex++; else break;
		}
		for (int i = 0; i < playerScoreIndex; i++)
		{
			if (activeSlate == 0) if (closestBallPtr[i] > NUM_PLAYERS_PER_SLATE - 1) gTable.player[closestBallPtr[i] + (NUM_PLAYERS_PER_SLATE * (activeSlate - 1))].score += 1; else gTable.player[closestBallPtr[i]].score += 1;
			if (activeSlate != 0) if (closestBallPtr[i] > NUM_PLAYERS_PER_SLATE - 1) gTable.player[closestBallPtr[i] + (NUM_PLAYERS_PER_SLATE * (activeSlate - 1))].score += 1; else gTable.player[closestBallPtr[i] + (NUM_PLAYERS_PER_SLATE * (activeSlate))].score += 1;
		}
		vec3 posf(gTable.slate[activeSlate].floor, BALL_RADIUS, 0);
		for (int i = 0; i < 50; i++)
		{
			//stage x
			gTable.parts.AddParticle(posf);
		}
		gTable.bc.gReset(), gTable.bc.AddBall(pos),
			currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate;
	}
	

	//Call update function to table every x ms depening on variable, stage 1.5
	gTable.Update(ms, activeSlate);

	//Set UpdateScene to timer to run every x ms
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutPostRedisplay();
}
void rotation(void) 
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(100000));
		if (gCueRotation != 0)
		{
			float rotatational = gCueRotation / 10000000;
			while (gTable.AnyBallsMoving() == true)
			{
				gTable.bc.balls[gTable.bc.num - 1]->velocity(0) -= rotatational;
				rotatational -= rotatational / 10000000;
			}
		}	
	}
}

int _tmain(int argc, _TCHAR* argv[])
{	
	glutInit(&argc, ((char**)argv));
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(2000, 1000);
	glutCreateWindow("MSc Workshop : Pool Game");
	glutDisplayFunc(RenderScene);
	//Stage setup
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutReshapeFunc(ChangeSize);
	glutIdleFunc(RenderScene);

	std::thread r(rotation);
	r.detach();
	
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(KeyboardFunc);
	glutKeyboardUpFunc(KeyboardUpFunc);
	glutSpecialFunc(SpecKeyboardFunc);
	glutSpecialUpFunc(SpecKeyboardUpFunc);
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
}