/*-----------------------------------------------------------
	Simulation Source File
-----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include<stdio.h>
#include<stdlib.h>
#include <iostream>
#include <algorithm>

/*-----------------------------------------------------------
	macros
-----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

game gGame;

static const float gRackPositionX[] = { 0.0f,0.0f,(BALL_RADIUS * 2.0f),(-BALL_RADIUS * 2.0f),(BALL_RADIUS * 4.0f) };
static const float gRackPositionZ[] = { 0.5f,0.0f,(-BALL_RADIUS * 3.0f),(-BALL_RADIUS * 3.0f) };

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;

/*-----------------------------------------------------------
	players class members
-----------------------------------------------------------*/
int players::playerIndexCnt = 0;

/*-----------------------------------------------------------
	target class members
-----------------------------------------------------------*/
int ScoreGrid::sgIndexCnt = 0;
void ScoreGrid::radiusD(void)
{
	radius = BALL_RADIUS * (index + 1);
}

/*-----------------------------------------------------------
	walls class members
-----------------------------------------------------------*/
void walls::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1] - vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void walls::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre /= 2.0;
}

/*-----------------------------------------------------------
	slate class members
-----------------------------------------------------------*/
int slateController::slateIndexCnt = 0;

void slateController::SetupCushions(void)
{
	float moveRight;
	float separate;
	moveRight = TABLE_X * 2 * num;
	separate = 0.10 * num;
	floor = moveRight + separate;

	slateWalls[0] = new walls;
	slateWalls[0]->vertices[0](0) = -TABLE_X + floor;
	slateWalls[0]->vertices[0](1) = -TABLE_Z;
	slateWalls[0]->vertices[1](0) = -TABLE_X + floor;
	slateWalls[0]->vertices[1](1) = TABLE_Z;

	slateWalls[1] = new walls;
	slateWalls[1]->vertices[0](0) = -TABLE_X + floor;
	slateWalls[1]->vertices[0](1) = TABLE_Z;
	slateWalls[1]->vertices[1](0) = TABLE_X + floor;
	slateWalls[1]->vertices[1](1) = TABLE_Z;

	slateWalls[2] = new walls;
	slateWalls[2]->vertices[0](0) = TABLE_X + floor;
	slateWalls[2]->vertices[0](1) = TABLE_Z;
	slateWalls[2]->vertices[1](0) = TABLE_X + floor;
	slateWalls[2]->vertices[1](1) = -TABLE_Z;

	slateWalls[3] = new walls;
	slateWalls[3]->vertices[0](0) = TABLE_X + floor;
	slateWalls[3]->vertices[0](1) = -TABLE_Z;
	slateWalls[3]->vertices[1](0) = -TABLE_X + floor;
	slateWalls[3]->vertices[1](1) = -TABLE_Z;

	for (int i = 0; i < NUM_WALLS; i++)
	{
		slateWalls[i]->MakeCentre();
		slateWalls[i]->MakeNormal();
	}
}

/*-----------------------------------------------------------
	stone class members
-----------------------------------------------------------*/
int stone::ballIndexCnt = 0;

void stone::ApplyImpulse(vec2 imp)
{
	velocity = imp;
	impTrue = true;
}

void stone::ApplyFrictionForce(int ms)
{
	if (velocity.Magnitude() <= 0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms) / 1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if (velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void stone::DoBallCollision(stone& b)
{
	//test if ball has hit another ball
	if (HasHitBall(b)) HitBall(b);
}

void stone::DoPlaneCollision(const walls& b)
{
	//test if ball has hit plane
	if (HasHitPlane(b)) HitPlane(b);
}

void stone::Update(int ms)
{
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms) / 1000.0f);
	//set small velocities to zero
	if (velocity.Magnitude() < SMALL_VELOCITY) velocity = 0.0;
}


bool stone::HasHitPlane(const walls& c) const
{
	//if moving away from plane, cannot hit
	if (velocity.Dot(c.normal) >= 0.0) return false;

	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if (sep > radius) return false;
	return true;
}

bool stone::HasHitBall(const stone& b) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - b.position;
	float dist = (float)relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - b.velocity;

	//if moving apart, cannot have hit
	if (relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if (dist > (radius + b.radius)) return false;
	return true;
}

void stone::HitPlane(const walls& c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0 + gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta;
}

void stone::HitBall(stone& b)
{
	//find direction from other ball to this ball
	vec2 relDir = (position - b.position).Normalised();

	//split velocities into 2 parts:  one component perpendicular, and one parallel to 
	//the collision plane, for both balls
	//(NB the collision plane is defined by the point of contact and the contact normal)
	float perpV = (float)velocity.Dot(relDir);
	float perpV2 = (float)b.velocity.Dot(relDir);
	vec2 parallelV = velocity - (relDir * perpV);
	vec2 parallelV2 = b.velocity - (relDir * perpV2);

	//Calculate new perpendicluar components:
	//v1 = (2*m2 / m1+m2)*u2 + ((m1 - m2)/(m1+m2))*u1;
	//v2 = (2*m1 / m1+m2)*u1 + ((m2 - m1)/(m1+m2))*u2;
	float sumMass = mass + b.mass;
	float perpVNew = (float)((perpV * (mass - b.mass)) / sumMass) + (float)((perpV2 * (2.0 * b.mass)) / sumMass);
	float perpVNew2 = (float)((perpV2 * (b.mass - mass)) / sumMass) + (float)((perpV * (2.0 * mass)) / sumMass);

	//find new velocities by adding unchanged parallel component to new perpendicluar component
	velocity = parallelV + (relDir * perpVNew);
	b.velocity = parallelV2 + (relDir * perpVNew2);
}

/*-----------------------------------------------------------
	stone controller class members
-----------------------------------------------------------*/
void curlingStones::AddBall(const vec2& pos)
{
	if (num >= NUM_BALLS) gameEnd = true;
	if (num >= NUM_BALLS) return;
	stones[num] = new stone;
	stones[num]->position = pos;
	num++;
}
void curlingStones::gReset(void)
{
	int i = 0;
	while (i < num)
	{
		delete stones[i];
		stones[i] = stones[num - 1];
		num--;
	}
	gameEnd = false;
}


/*-----------------------------------------------------------
	firwork class members
-----------------------------------------------------------*/
 
void firework::update(int ms)
{
	position += (velocity * ms) / 1000.0;
	velocity(1) -= (4.0 * ms) / 1000.0;
}

/*-----------------------------------------------------------
	firework set class members
-----------------------------------------------------------*/
void fireworkSet::AddFirework(const vec3& pos)
{
	if (num >= MAX_PARTICLES) return;
	fireworks[num] = new firework;
	fireworks[num]->position = pos;

	fireworks[num]->velocity(0) = ((rand() % 200) - 100) / 200.0;
	fireworks[num]->velocity(2) = ((rand() % 200) - 100) / 200.0;
	fireworks[num]->velocity(1) = 2.0 * ((rand() % 100) / 100.0);

	num++;
}

void fireworkSet::update(int ms)
{
	int i = 0;
	while (i < num)
	{
		fireworks[i]->update(ms);
		if ((fireworks[i]->position(1) < 0.0) && (fireworks[i]->velocity(1) < 0.0))
		{
			delete fireworks[i];
			fireworks[i] = fireworks[num - 1];
			num--;
		}
		else i++;
	}
}

/*-----------------------------------------------------------
	Game class members
-----------------------------------------------------------*/
void game::Update(int ms, int activeSlate)
{
	//check for collisions for each ball
	for (int i = 0; i < gGame.cs.num; i++)
	{
		for (int j = 0; j < NUM_WALLS; j++)
		{			
			gGame.cs.stones[i]->DoPlaneCollision(*gGame.slate[activeSlate].slateWalls[j]);
		}

		for (int j = (i + 1); j < gGame.cs.num; j++)
		{
			gGame.cs.stones[i]->DoBallCollision(*gGame.cs.stones[j]);
		}
	}

	for (int i = 0; i < gGame.cs.num; i++) gGame.cs.stones[i]->Update(ms);

	fworks.update(ms);
}


bool game::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for (int i = 0; i < gGame.cs.num; i++)
	{
		if (gGame.cs.stones[i]->velocity(0) != 0.0) return true;
		if (gGame.cs.stones[i]->velocity(1) != 0.0) return true;
	}
	return false;
}

int* game::calculateScore(float x)
{
	static int returnBallID[NUM_BALLS];
	int ballID[NUM_BALLS];
	float allDist[NUM_BALLS];
	float tempDist[NUM_BALLS];

	for (int i = 0; i < NUM_BALLS; i++)
	{
		float currentDistance = sqrt(pow(cs.stones[i]->position(0) - x, 2.0) + pow(cs.stones[i]->position(1) - -0.8, 2.0));
		ballID[i] = i, tempDist[i] = currentDistance, allDist[i] = currentDistance;
	}
	std::sort(tempDist, tempDist + NUM_BALLS);
	for (int i = 0; i < NUM_BALLS; i++)
	{
		for (int j = 0; j < NUM_BALLS; j++)
		{
			if (tempDist[i] == allDist[j]) returnBallID[i] = ballID[j];
		}
	}
	return returnBallID;
}

float* game::calcX(int currentPlayer, float theta, float r)
{
	float ballX = cs.stones[currentPlayer]->position(0);
	float x = r * cosf(theta) + ballX;//calculate the x component 
	return &x;
}

float* game::calcZ(int currentPlayer, float theta, float r)
{
	float ballZ = cs.stones[currentPlayer]->position(1);
	float z = r * sinf(theta) + ballZ;//calculate the z component
	return &z;
}