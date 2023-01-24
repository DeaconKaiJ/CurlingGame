/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"

/*-----------------------------------------------------------
	Macros
-----------------------------------------------------------*/
#define TABLE_X			(0.3f) 
#define TABLE_Z			(1.2f)
#define TABLE_Y			(0.1f)
#define BALL_RADIUS		(0.05f)
#define BALL_MASS		(0.05f)
#define TWO_PI			(6.2832f)
#define	SIM_UPDATE_MS	(10)	
#define NUM_CUSHIONS	(4)
#define NUM_SLATES		(5)
#define MAX_PARTICLES	(200)
#define NUM_POCKETS		(4)
#define NUM_PLAYERS_PER_SLATE	(6)
#define NUM_PLAYERS		(NUM_PLAYERS_PER_SLATE*NUM_SLATES)
#define NUM_BALLS		(NUM_PLAYERS_PER_SLATE*2)	


/*-----------------------------------------------------------
	player class
-----------------------------------------------------------*/
class players {
	static int playerIndexCnt;
public:
	int index;
	int score;
	players() { index = playerIndexCnt++; score = 0; }
};

/*-----------------------------------------------------------
	pocket class
-----------------------------------------------------------*/
class pocket
{
	static int pocketIndexCnt;
public:
	float radius;
	int index;
	
	pocket() 
	{ 
		index = pocketIndexCnt++; radiusD(); 
	}

	void radiusD(void);
};

/*-----------------------------------------------------------
	cushion class
-----------------------------------------------------------*/
class cushion
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;

	void MakeNormal(void);
	void MakeCentre(void);
};

class slateController
{
	static int slateIndexCnt;
public:
	cushion* slateWalls[4];
	int num;
	int redScore;
	int blueScore;
	float floor;
	slateController() 
	{ 
		for (int i = 0; i < NUM_CUSHIONS; i++) slateWalls[i] = 0;
		redScore = 0; blueScore = 0; floor = 0; num = slateIndexCnt++; SetupCushions(); 
	}

	void SetupCushions(void);
};

/*-----------------------------------------------------------
	ball class
-----------------------------------------------------------*/

class ball
{
	static int ballIndexCnt;
public:
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;
	bool	impTrue;

	ball() : position(0.0), velocity(0.0), radius(BALL_RADIUS),
		mass(BALL_MASS) { index = ballIndexCnt++; impTrue = false; }

	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const cushion& c);
	void DoBallCollision(ball& b);
	void Update(int ms);

	bool HasHitPlane(const cushion& c) const;
	bool HasHitBall(const ball& b) const;

	void HitPlane(const cushion& c);
	void HitBall(ball& b);
};

class ballController
{
public:
	ball* balls[NUM_BALLS];
	bool gameEnd;
	int num;
	ballController()
	{
		for (int i = 0; i < NUM_BALLS; i++) balls[i] = 0;
		num = 0;
		gameEnd = false;
		vec2 pos(0.0, 0.75);
		AddBall(pos);
	}
	void AddBall(const vec2& pos);
	void gReset(void);
};

/*-----------------------------------------------------------
	particle class
-----------------------------------------------------------*/
class particle
{
public:
	vec3 position;
	vec3 velocity;

	particle() { position = 0; velocity = 0; }
	void update(int ms);
};

class particleSet
{
public:
	particle* particles[MAX_PARTICLES];
	int num;

	particleSet()
	{
		for (int i = 0; i < MAX_PARTICLES; i++) particles[i] = 0;
		num = 0;
	}

	~particleSet()
	{
		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			if (particles[i]) delete particles[i];
		}
	}

	void AddParticle(const vec3& pos);
	void update(int ms);
};


/*-----------------------------------------------------------
	table class
-----------------------------------------------------------*/
class table
{
public:
	slateController slate[NUM_SLATES];
	pocket pockets[NUM_POCKETS];
	particleSet parts;
	ballController bc;
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
extern table gTable;
