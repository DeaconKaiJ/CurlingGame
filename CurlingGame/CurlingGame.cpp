// Curling Game.cpp : Defines the entry point for the console application.

//Necessary Includes
#include <stdlib.h>
#include "stdafx.h"
#include <glut.h>
#include "simulation.h"
#include <string.h>
#include <thread>

//Glide variables
bool gGlideControl[6] = { false,false,false,false,false,false };
bool gDoPlay = true;

//Glide Power
float gGlideAngle = 0.0;
float gGlidePower = 0.25;
float gGlidePowerSpeed = 0.25f;
float gGlidePowerMax = 0.75;
float gGlidePowerMin = 0.1;
float gGlideBallFactor = 3.0;

//Glide Spin
float gGlideSpin = 0.0;
float gGlideSpinMax = 1.0;
float gGlideSpinMin = -1.0;

//Local multiplayer variables
int currentPlayer = 0;
int activeSlate = 0;
int* closestBallPtr = 0;
int winningTeam=0;

//camera variables, camera moves in a circle around the ball a fixed radius away
float initpos = 90.0;
float r = 1.0;
float ballX = 0.0;
float ballZ = 0.75;
float theta = TWO_PI * float(initpos) / float(360);
float x = r * cosf(theta) + ballX;//calculate the x component 
float z = r * sinf(theta) + ballZ;//calculate the z component
vec3 gCamPos(x, BALL_RADIUS + 0.5, z);
vec3 gCamLookAt(0.0, BALL_RADIUS, 0.75);

//controls/scoreboard variables
const char* input[7] = {"HitBall: ", "Change Slate: ","Add Power: ","Decrease Power: ","Move Left: ","Move Right: ","Add Spin: "};
const char* inputMeaning[7] = {"Enter","NumberKeys","UpArrow","DownArrow","LeftArrow","RightArrow","Q/E"};

bool notGame = false;

//Function used to write text to the screen
void writetoScreen(float x, float y, float z, char* string, int len) 
{
	glColor3f(1.0, 1.0, 1.0);
	glRasterPos3f(x,y,z);
	len = strlen(string);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, string[i]);
	}

}

//Function to apply a spin to the ball, done using multithreading
void rotation(void)
{
	while (true)
	{
		//delay to allow for actions on the main thread to update before this thread
		std::this_thread::sleep_for(std::chrono::microseconds(100000));
		if (gGlideSpin != 0)
		{
			//Sets an approriate force
			float rotatational = gGlideSpin / 10000000;
			while (gGame.AnyBallsMoving() == true)
			{
				//Apply for to the velocity of the x axis in direction
				gGame.cs.stones[gGame.cs.num - 1]->velocity(0) -= rotatational;
				//Reduces the force over time
				rotatational -= rotatational / 10000000;
			}
		}
	}
}

//Initial render of the scene, making all objects appear on tab, This is an idle Function so will be called when a change occurs
void RenderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set camera
	glLoadIdentity();
	gluLookAt(gCamPos(0), gCamPos(1), gCamPos(2), gCamLookAt(0), gCamLookAt(1), gCamLookAt(2), 0.0f, 1.0f, 0.0f);

	//Draw Stones
	for (int i = 0; i < gGame.cs.num; i++)
	{
		glPushMatrix();
		if (i % 2) glColor3f(0.0, 0.0, 1.0); else glColor3f(1.0, 0.0, 0.0);
		glTranslatef(gGame.cs.stones[i]->position(0), BALL_RADIUS, gGame.cs.stones[i]->position(1));
		glutSolidSphere(gGame.cs.stones[i]->radius, 20, 20);
		glPopMatrix();
	}
	
	//Draw Slates
	for (int i = 0; i < NUM_SLATES; i++)
	{
		for (int j = 0; j < NUM_WALLS; j++)
		{
			glColor3f(0.0, 0.0, 1.0);
			glBegin(GL_QUADS);
			glVertex3f(gGame.slate[i].slateWalls[j]->vertices[0](0), -0.00001, gGame.slate[i].slateWalls[j]->vertices[0](1));
			glVertex3f(gGame.slate[i].slateWalls[j]->vertices[0](0), 0.05, gGame.slate[i].slateWalls[j]->vertices[0](1));
			glVertex3f(gGame.slate[i].slateWalls[j]->vertices[1](0), 0.05, gGame.slate[i].slateWalls[j]->vertices[1](1));
			glVertex3f(gGame.slate[i].slateWalls[j]->vertices[1](0), -0.00001, gGame.slate[i].slateWalls[j]->vertices[1](1));
			glEnd();
		}
	}
	//Draw slate floors
	for (int i = 0; i < NUM_SLATES; i++)
	{	
		glBegin(GL_QUADS);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(TABLE_X+ gGame.slate[i].floor, -0.00001, TABLE_Z);
		glVertex3f(TABLE_X + gGame.slate[i].floor, -0.00001, -TABLE_Z);
		glVertex3f(-TABLE_X + gGame.slate[i].floor, -0.00001, -TABLE_Z);
		glVertex3f(-TABLE_X + gGame.slate[i].floor, -0.00001, TABLE_Z);
		glEnd();
	}
	
	//Draw Fireworks
	for (int i = 0; i < gGame.fworks.num; i++)
	{
		if (winningTeam == 0)glColor3f(1.0, 0.0, 0.0); else glColor3f(0.0, 0.0, 1.0);
		glPushMatrix();
		glTranslatef(gGame.fworks.fireworks[i]->position(0), gGame.fworks.fireworks[i]->position(1), gGame.fworks.fireworks[i]->position(2));
		glutSolidSphere(0.005f, 32, 32);
		glPopMatrix();
	}

	//Draw scoring grid
	for (int i = 0; i < NUM_SLATES; i++){
		//change the y on which the each ring is drawn to avoid problems
		float y = 0.0001;
		//iterate backwards so biggest ring is done first
		for (int j = NUM_SCOREGRID - 1; j > -1; j--)
		{
			if (j == 3)glColor3f(0.0, 0.0, 1.0);
			if (j == 2)glColor3f(1.0, 1.0, 1.0);
			if (j == 1)glColor3f(1.0, 0.0, 0.0);
			if (j == 0)glColor3f(1.0, 1.0, 1.0);
			glBegin(GL_POLYGON);
			int num_segments = 360;
			float cx = gGame.slate[i].floor;
			float cz = -0.8;
			float r = gGame.targets[j].radius;
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
	//variables used to display text
	char text[50];
	int len;

	//Write Teams
	for (int i = 0; i < NUM_SLATES; i++)
	{
		//red team
		sprintf_s(text, "Score Red: %d", gGame.slate[i].redScore);
		len = strlen(text);
		writetoScreen(gGame.slate[i].floor - 0.1, 0.25, -TABLE_Z, text, len);

		//blue team
		sprintf_s(text, "Score Blue: %d", gGame.slate[i].blueScore);
		len = strlen(text);
		writetoScreen(gGame.slate[i].floor - 0.1, 0.20, -TABLE_Z, text,len);
	}

	//Write controls/scoreboard
	sprintf_s(text, "For Controls/Scoreboard: %s", "press s");
	len = strlen(text);
	writetoScreen(gGame.slate[activeSlate].floor - TABLE_X - 0.05, 0.10, -TABLE_Z, text, len);

	//Write ScoreBoard
	float x = 0.5;
	float y = 1.0;
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		if ((i) % 8 == 0)y -= 0.1, x = 0.5;
		sprintf_s(text, "Player%d score: %d", i + 1, gGame.player[i].score);
		len = strlen(text);
		writetoScreen((-TABLE_X * 3) + (x += 0.5), y, TABLE_Z + 0.5, text, len);
	}
	//Write Controls
	x = 0.4;
	y = 1.2;
	for (int i = 0; i < 7; i++)
	{
		//Write Key/Meanings
		if ((i) % 4 == 0)y -= 0.1, x = 0.4;
		sprintf_s(text, "%s%s", input[i], inputMeaning[i]);
		len = strlen(text);
		writetoScreen((-TABLE_X * 3) + (x += 0.6), y, TABLE_Z + 0.5, text, len);
	}

	//Write Power
	int gcp = gGlidePower * 100;
	sprintf_s(text, "Power: %d", gcp);
	len = strlen(text);
	writetoScreen(gGame.slate[activeSlate].floor - TABLE_X - 0.05, 0.20, -TABLE_Z, text, len);

	//Write Spin
	int spinNormalised = gGlideSpin * 100;
	if (spinNormalised < 0)sprintf_s(text, "Rotate Right: %d", spinNormalised * -1), len = strlen(text);
	if (spinNormalised > 0)sprintf_s(text, "Rotate Left: %d", spinNormalised), len = strlen(text);
	if (spinNormalised == 0)sprintf_s(text, "Rotate: %d", spinNormalised), len = strlen(text);
	writetoScreen(gGame.slate[activeSlate].floor + TABLE_X - 0.05, 0.20, -TABLE_Z, text, len);

	//Write Current Player
	sprintf_s(text, "CurrentPlayer: %d", currentPlayer+1);
	len = strlen(text);
	writetoScreen(gGame.slate[activeSlate].floor - TABLE_X - 0.05, 0.30, -TABLE_Z, text, len);

	glFlush();
	glutSwapBuffers();
}

//Keyboard function for when key IS pressed down (Arrow keys used for aiming, new camera movement)
void SpecKeyboardFunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
	{
		notGame = false;
		gGlideControl[0] = true;
		break;
	}
	case GLUT_KEY_RIGHT:
	{
		notGame = false;
		gGlideControl[1] = true;
		break;
	}
	case GLUT_KEY_UP:
	{
		notGame = false;
		gGlideControl[2] = true;
		break;
	}
	case GLUT_KEY_DOWN:
	{
		notGame = false;
		gGlideControl[3] = true;
		break;
	}
	}
}

//Keyboard function for when key IS NOT pressed down (Arrow keys used for aiming, new camera movement)
void SpecKeyboardUpFunc(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
	{
		gGlideControl[0] = false;
		break;
	}
	case GLUT_KEY_RIGHT:
	{
		gGlideControl[1] = false;
		break;
	}
	case GLUT_KEY_UP:
	{
		gGlideControl[2] = false;
		break;
	}
	case GLUT_KEY_DOWN:
	{
		gGlideControl[3] = false;
		break;
	}
	}
}

//Keyboard function for when key IS pressed down (Hitting ball, applying spin)
void KeyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	//Enter key
	case(13):
	{
		notGame = false;
		if (gDoPlay)
		{
			vec2 imp((-sin(gGlideAngle) * gGlidePower * gGlideBallFactor),
				(-cos(gGlideAngle) * gGlidePower * gGlideBallFactor));
			//Stage y
			gGame.cs.stones[gGame.cs.num-1]->ApplyImpulse(imp);
		}
		break;
	}
	
	case('q'):
	{
		gGlideControl[4] = true;
		break;
	}
	case('e'):
	{
		gGlideControl[5] = true;
		break;
	}
	}
}

//Keyboard function for when key IS NOT pressed down (changing slate, stopping spin, moving camera to help sections)
void KeyboardUpFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case(48):
	{	
		notGame = false;
		vec2 pos(gGame.slate[0].floor, 0.75);
		if (gGame.AnyBallsMoving() == false)gGame.cs.gReset(), activeSlate = 0, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gGame.cs.AddBall(pos), gGlideAngle = 0.0, gGlidePower = 0.25, gGlideSpin = 0.0; break;
	}
	case(49):
	{
		notGame = false;
		vec2 pos(gGame.slate[1].floor, 0.75);
		if (gGame.AnyBallsMoving() == false)gGame.cs.gReset(), activeSlate = 1, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gGame.cs.AddBall(pos), gGlideAngle = 0.0, gGlidePower = 0.25, gGlideSpin = 0.0; break;
	}
	case(50):
	{
		notGame = false;
		vec2 pos(gGame.slate[2].floor, 0.75);
		if (gGame.AnyBallsMoving() == false)gGame.cs.gReset(), activeSlate = 2, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gGame.cs.AddBall(pos), gGlideAngle = 0.0, gGlidePower = 0.25, gGlideSpin = 0.0; break;
	}
	case(51):
	{	
		notGame = false;
		vec2 pos(gGame.slate[3].floor, 0.75);
		if (gGame.AnyBallsMoving() == false)gGame.cs.gReset(), activeSlate = 3, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gGame.cs.AddBall(pos), gGlideAngle = 0.0, gGlidePower = 0.25, gGlideSpin = 0.0; break;
	}
	case(52):
	{
		notGame = false;
		vec2 pos(gGame.slate[4].floor, 0.75);
		if (gGame.AnyBallsMoving() == false)gGame.cs.gReset(), activeSlate = 4, currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate,
			gGame.cs.AddBall(pos), gGlideAngle = 0.0, gGlidePower = 0.25, gGlideSpin = 0.0; break;
	}
	case('q'):
	{
		gGlideControl[4] = false;
		break;
	}
	case('e'):
	{
		gGlideControl[5] = false;
		break;
	}
	case('s'):
	{
		notGame = true;
		gCamPos(0) = 1.5; gCamPos(1) = 1.0; gCamPos(2) = -TABLE_Z;
		gCamLookAt(0) = 1.5; gCamLookAt(1) = 0.5; gCamLookAt(2) = TABLE_Z;
	}
	case('z'):
	{
		notGame = true;
	}
	}
}

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



//The main class used to update the game in ms
void UpdateScene(int ms)
{
	vec2 pos(gGame.slate[activeSlate].floor, 0.75);

	//Corrects current player variable to display correctly
	if (currentPlayer > NUM_PLAYERS_PER_SLATE * (activeSlate + 1) - 1) currentPlayer -= NUM_PLAYERS_PER_SLATE;
	
	//tests if any balls are moving, if false activate play, else balls are moving, dont activate play
	if (gGame.AnyBallsMoving() == false) gDoPlay = true; else gDoPlay = false;
	
	//If play is true, activate glide functions
	if (gDoPlay)
	{
		//Left control
		if (gGlideControl[0]) {
			initpos -= 0.5;
			gGlideAngle = -(initpos - 90) * TWO_PI / 360;
			theta = TWO_PI * float(initpos) / float(360);
			x = *gGame.calcX(gGame.cs.num - 1, theta, r);
			z = *gGame.calcZ(gGame.cs.num - 1, theta, r);
			gCamPos(0) = x;
			gCamPos(1) = BALL_RADIUS + 0.5;
			gCamPos(2) = z;
		}
		//Right control
		if (gGlideControl[1]) {
			initpos += 0.5;
			gGlideAngle = -(initpos - 90) * TWO_PI / 360;
			theta = TWO_PI * float(initpos) / float(360);
			x = *gGame.calcX(gGame.cs.num - 1, theta, r);
			z = *gGame.calcZ(gGame.cs.num - 1, theta, r);
			gCamPos(0) = x;
			gCamPos(1) = BALL_RADIUS + 0.5;
			gCamPos(2) = z;
		}
		//Increase power
		if (gGlideControl[2]) gGlidePower += ((gGlidePowerSpeed * ms) / 1000);
		//Decrease power
		if (gGlideControl[3]) gGlidePower -= ((gGlidePowerSpeed * ms) / 1000);
		//Control power to stay within bounds
		if (gGlidePower > gGlidePowerMax) gGlidePower = gGlidePowerMax;
		if (gGlidePower < gGlidePowerMin) gGlidePower = gGlidePowerMin;

		//Increase Spin
		if (gGlideControl[4]) gGlideSpin += ((gGlidePowerSpeed * ms) / 1000);
		//Decrease Spin
		if (gGlideControl[5]) gGlideSpin -= ((gGlidePowerSpeed * ms) / 1000);
		//Keep spin inside of bounds
		if (gGlideSpin > gGlideSpinMax) gGlideSpin = gGlideSpinMax;
		if (gGlideSpin < gGlideSpinMin) gGlideSpin = gGlideSpinMin;
	}
	//bool to decide gamestate
	if (notGame == false)
	{
		//Look at active ball
		gCamLookAt(0) = gGame.cs.stones[gGame.cs.num - 1]->position(0);
		gCamLookAt(1) = BALL_RADIUS;
		gCamLookAt(2) = gGame.cs.stones[gGame.cs.num - 1]->position(1);
	}
	//Test if ball has been hit and has stopped, to further play to new ball
	if (gGame.cs.stones[gGame.cs.num - 1]->impTrue == true) if (gGame.AnyBallsMoving() == false) gGame.cs.AddBall(pos),
		gGlideAngle = 0.0, gGlidePower = 0.25, gGlideSpin = 0.0, currentPlayer++;

	//If all balls have been played
	if (gGame.cs.gameEnd == true)
	{
		//Find distance of all balls and order them
		closestBallPtr = gGame.calculateScore(gGame.slate[activeSlate].floor);

		//Index used for finding individual scores
		int playerScoreIndex = 1;

		//Decides winning team
		if (closestBallPtr[0] % 2 == 0) winningTeam = 0, gGame.slate[activeSlate].redScore += 1; else winningTeam = 1, gGame.slate[activeSlate].blueScore += 1;
		
		//Iterate thhough all balls
		for (int i = 1; i < NUM_BALLS; i++)
		{
			//test if even for first team, add points until opposition ball is discovered
			if (winningTeam == 0) if (closestBallPtr[i] % 2 == 0) gGame.slate[activeSlate].redScore += 1, playerScoreIndex++; else break;

			//test if odd for second team, add points until opposition ball is discovered
			if (winningTeam == 1) if (closestBallPtr[i] % 2 != 0) gGame.slate[activeSlate].blueScore += 1, playerScoreIndex++; else break;
		}

		//iterate through player index
		for (int i = 0; i < playerScoreIndex; i++)
		{
			//Applies scores to players based on ball id
			if (activeSlate == 0) if (closestBallPtr[i] > NUM_PLAYERS_PER_SLATE - 1) gGame.player[closestBallPtr[i] + (NUM_PLAYERS_PER_SLATE * (activeSlate - 1))].score += 1; else gGame.player[closestBallPtr[i]].score += 1;
			if (activeSlate != 0) if (closestBallPtr[i] > NUM_PLAYERS_PER_SLATE - 1) gGame.player[closestBallPtr[i] + (NUM_PLAYERS_PER_SLATE * (activeSlate - 1))].score += 1; else gGame.player[closestBallPtr[i] + (NUM_PLAYERS_PER_SLATE * (activeSlate))].score += 1;
		}

		//position for fireworks
		vec3 posf(gGame.slate[activeSlate].floor, BALL_RADIUS, 0);

		//add 50 fireworks for the team who won in the middle of the slate
		for (int i = 0; i < 50; i++)
		{
			gGame.fworks.AddFirework(posf);
		} 

		//reset game
		gGame.cs.gReset(), gGame.cs.AddBall(pos),currentPlayer = NUM_PLAYERS_PER_SLATE * activeSlate;
	}

	//Call update function to table every x ms depening on variable, stage 1.5
	gGame.Update(ms, activeSlate);

	//Set UpdateScene to timer to run every x ms
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutPostRedisplay();
}

//Main for starting the game
int _tmain(int argc, _TCHAR* argv[])
{	
	glutInit(&argc, ((char**)argv));
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(2000, 1000);
	glutCreateWindow("MSc Workshop : Pool Game");
	glutDisplayFunc(RenderScene);
	glutTimerFunc(SIM_UPDATE_MS, UpdateScene, SIM_UPDATE_MS);
	glutReshapeFunc(ChangeSize);
	glutIdleFunc(RenderScene);

	//Identify thread and detach
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