/*-----------------------------------------------------------
	Simulation Header File
-----------------------------------------------------------*/
#include"vecmath.h"

/*-----------------------------------------------------------
	Macros
-----------------------------------------------------------*/
#define TABLE_X					(0.3f) 
#define TABLE_Z					(1.2f)
#define TABLE_Y					(0.1f)
#define BALL_RADIUS				(0.05f)
#define BALL_MASS				(0.05f)
#define TWO_PI					(6.2832f)
#define	SIM_UPDATE_MS			(10)	
#define NUM_WALLS				(4)
#define NUM_SLATES				(5)
#define MAX_PARTICLES			(50)
#define NUM_SCOREGRID			(4)
#define NUM_PLAYERS_PER_SLATE	(6)
#define NUM_PLAYERS				(NUM_PLAYERS_PER_SLATE*NUM_SLATES)
#define NUM_BALLS				(NUM_PLAYERS_PER_SLATE*2)	

/*-----------------------------------------------------------
	player class
-----------------------------------------------------------*/
class players {
	static int playerIndexCnt;
public:
	int index;//Identifier for player
	int score;//Score for player
	//Constructor
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
	int index;
	
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
	walls* slateWalls[4];
	int num;
	int redScore;
	int blueScore;
	float floor;
	slateController() 
	{ 
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
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;
	bool	impTrue;

	stone() : position(0.0), velocity(0.0), radius(BALL_RADIUS),
		mass(BALL_MASS) { index = ballIndexCnt++; impTrue = false; }

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
	stone* stones[NUM_BALLS];
	bool gameEnd;
	int num;
	curlingStones()
	{
		for (int i = 0; i < NUM_BALLS; i++) stones[i] = 0;
		num = 0;
		gameEnd = false;
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
	vec3 position;
	vec3 velocity;

	firework() { position = 0; velocity = 0; }
	void update(int ms);
};

class fireworkSet
{
public:
	firework* fireworks[MAX_PARTICLES];
	int num;

	fireworkSet()
	{
		for (int i = 0; i < MAX_PARTICLES; i++) fireworks[i] = 0;
		num = 0;
	}

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
	float* calcX(int currentPlayer, float theta, float r);
	float* calcZ(int currentPlayer, float theta, float r);
};

/*-----------------------------------------------------------
	global table
-----------------------------------------------------------*/
extern game gGame;
