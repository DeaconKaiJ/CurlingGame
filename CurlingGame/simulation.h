/*-----------------------------------------------------------
	Simulation Header File
-----------------------------------------------------------*/
#include"vecmath.h"
#include<stdlib.h>
#include"stdafx.h"

/*-----------------------------------------------------------
	Game Macros
-----------------------------------------------------------*/
#define NUM_SLATES				(5)//Necessary to change when altering game
#define NUM_PLAYERS_PER_SLATE	(6)//Necessary to change when altering game
#define NUM_PLAYERS				(NUM_PLAYERS_PER_SLATE*NUM_SLATES)

/*-----------------------------------------------------------
	Slate Macros
-----------------------------------------------------------*/
#define SLATE_X					(0.3f) 
#define SLATE_Z					(1.2f)
#define NUM_SCOREGRID			(4)
#define NUM_WALLS				(4)

/*-----------------------------------------------------------
	Stone Macros
-----------------------------------------------------------*/
#define STONE_RADIUS			(0.05f)
#define STONE_MASS				(0.05f)
#define NUM_STONES				(NUM_PLAYERS_PER_SLATE*2)	

/*-----------------------------------------------------------
	Misc Macros
-----------------------------------------------------------*/
#define TWO_PI					(6.2832f)
#define	SIM_UPDATE_MS			(10)	
#define MAX_PARTICLES			(50)

/*-----------------------------------------------------------
	player class
-----------------------------------------------------------*/
class players {
	static int playerIndexCnt;
public:
	int index;//Identifier for player
	int score;//Score for player
	//constructor
	players() { index = playerIndexCnt++; score = 0; }
};

/*-----------------------------------------------------------
	target class
-----------------------------------------------------------*/
class ScoreGrid
{
	static int sgIndexCnt;
public:
	float radius;//Radius of scoringGrid
	int index;//index of target
	
	//contructor
	ScoreGrid()
	{ 
		index = sgIndexCnt++; radiusD(); 
	}

	void radiusD(void);
};

/*-----------------------------------------------------------
	walls class
-----------------------------------------------------------*/
class walls
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;

	void MakeNormal(void);
	void MakeCentre(void);
};

/*-----------------------------------------------------------
	slate class
-----------------------------------------------------------*/
class slateController
{
	static int slateIndexCnt;
public:
	walls* slateWalls[4]; //walls
	int num; //act as index
	int redScore; //score for red team
	int blueScore; //score for blue team
	float floor; //the middle point of the slate

	//constructor
	slateController() 
	{ 
		//make but set all to zero
		for (int i = 0; i < NUM_WALLS; i++) slateWalls[i] = 0;
		redScore = 0; blueScore = 0; floor = 0; num = slateIndexCnt++; SetupCushions(); 
	}

	void SetupCushions(void);
};

/*-----------------------------------------------------------
	stone class
-----------------------------------------------------------*/
class stone
{
	static int ballIndexCnt;
public:
	vec2	position; //position
	vec2	velocity; //movement
	float	radius; //radius
	float	mass; //mass
	int		index; //index
	bool	impTrue; //has been hit?

	//contructor
	stone() : position(0.0), velocity(0.0), radius(STONE_RADIUS),
		mass(STONE_MASS) { index = ballIndexCnt++; impTrue = false; }

	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const walls& c);
	void DoBallCollision(stone& b);
	void Update(int ms);

	bool HasHitPlane(const walls& c) const;
	bool HasHitBall(const stone& b) const;

	void HitPlane(const walls& c);
	void HitBall(stone& b);
};

class curlingStones
{
public:
	stone* stones[NUM_STONES]; //init stones
	bool gameEnd; //has game ended?
	int num; //act as index

	//constructor
	curlingStones()
	{
		for (int i = 0; i < NUM_STONES; i++) stones[i] = 0;
		num = 0;
		gameEnd = false;

		//add a ball at starting location
		vec2 pos(0.0, 0.75);
		AddBall(pos);
	}

	void AddBall(const vec2& pos);
	void gReset(void);
};

/*-----------------------------------------------------------
	Firework class
-----------------------------------------------------------*/
class firework
{
public:
	vec3 position; //position
	vec3 velocity; //movement

	//constructor
	firework() { position = 0; velocity = 0; }
	void update(int ms);
};

class fireworkSet
{
public:
	firework* fireworks[MAX_PARTICLES]; //init all fireworks
	int num; //act as index

	//constructor
	fireworkSet()
	{
		for (int i = 0; i < MAX_PARTICLES; i++) fireworks[i] = 0;
		num = 0;
	}

	//destructor
	~fireworkSet()
	{
		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			if (fireworks[i]) delete fireworks[i];
		}
	}

	void AddFirework(const vec3& pos);
	void update(int ms);
};

/*-----------------------------------------------------------
	game class
-----------------------------------------------------------*/
class game
{
public:
	slateController slate[NUM_SLATES];
	ScoreGrid targets[NUM_SCOREGRID];
	fireworkSet fworks;
	curlingStones cs;
	players player[NUM_PLAYERS];
	void Update(int ms, int activeSlate);
	bool AnyBallsMoving(void) const;
	int* calculateScore(float x);
	float* calcCam(int currentPlayer, float theta, float r);
};

/*-----------------------------------------------------------
	global table
-----------------------------------------------------------*/
extern game gGame;
