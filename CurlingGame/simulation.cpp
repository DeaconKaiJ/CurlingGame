/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include<stdio.h>
#include<stdlib.h>


  /*-----------------------------------------------------------
	macros
	-----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

table gTable;

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
  pocket class members
  -----------------------------------------------------------*/
int pocket::pocketIndexCnt = 0;
void pocket::radiusD(void)
{
	radius = BALL_RADIUS * (index + 1);
}

/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1] - vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre /= 2.0;
}

int slateController::slateIndexCnt = 0;

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;

//Stage y, called when enter key hit in poolgame.cpp
void ball::ApplyImpulse(vec2 imp)
{
	velocity = imp;
	impTrue = true;
}
//Stage 12
void ball::ApplyFrictionForce(int ms)
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
//Stage 8
void ball::DoBallCollision(ball& b)
{
	//test if ball has hit another ball
	if (HasHitBall(b)) HitBall(b);
}
//Stage 4
void ball::DoPlaneCollision(const cushion& b)
{
	//test if ball has hit plane
	if (HasHitPlane(b)) HitPlane(b);
}
//Stage 11
void ball::Update(int ms)
{
	//apply friction, Stage 12
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms) / 1000.0f);
	//set small velocities to zero
	if (velocity.Magnitude() < SMALL_VELOCITY) velocity = 0.0;
}

//Stage 5
bool ball::HasHitPlane(const cushion& c) const
{
	//if moving away from plane, cannot hit
	if (velocity.Dot(c.normal) >= 0.0) return false;

	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if (sep > radius) return false;
	return true;
}
//Stage 9
bool ball::HasHitBall(const ball& b) const
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
//Stage 6
void ball::HitPlane(const cushion& c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0 + gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta;
}
//Stage 10
void ball::HitBall(ball& b)
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

void ballController::AddBall(const vec2& pos)
{
	if (num >= NUM_BALLS) gameEnd = true;
	if (num >= NUM_BALLS) return;
	balls[num] = new ball;
	balls[num]->position = pos;
	num++;
}
void ballController::gReset(void)
{
	int i = 0;
	while (i < num)
	{
		delete balls[i];
		balls[i] = balls[num - 1];
		num--;
	}
	gameEnd = false;
}


/*-----------------------------------------------------------
  particle class members
  -----------------------------------------------------------*/
  //Stage 14
void particle::update(int ms)
{
	position += (velocity * ms) / 1000.0;
	velocity(1) -= (4.0 * ms) / 1000.0; //(9.8*ms)/1000.0;
}

/*-----------------------------------------------------------
  particle set class members
  -----------------------------------------------------------*/
  //stage x, called at the end of stage 6/10
void particleSet::AddParticle(const vec3& pos)
{
	if (num >= MAX_PARTICLES) return;
	particles[num] = new particle;
	particles[num]->position = pos;

	particles[num]->velocity(0) = ((rand() % 200) - 100) / 200.0;
	particles[num]->velocity(2) = ((rand() % 200) - 100) / 200.0;
	particles[num]->velocity(1) = 2.0 * ((rand() % 100) / 100.0);

	num++;
}
//Stage 13
void particleSet::update(int ms)
{
	int i = 0;
	while (i < num)
	{
		particles[i]->update(ms);
		if ((particles[i]->position(1) < 0.0) && (particles[i]->velocity(1) < 0.0))
		{
			delete particles[i];
			particles[i] = particles[num - 1];
			num--;
		}
		else i++;
	}
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
  //Called in the main function, in poolgame.cpp
void slateController::SetupCushions(void)
{
	float moveRight;
	float separate;
	moveRight = TABLE_X*2 * num;
	separate = 0.10*num;
	if (num == 0) separate = 0.0;
	floor = moveRight + separate;

	slateWalls[0] = new cushion;
    slateWalls[0]->vertices[0](0) = -TABLE_X+moveRight+separate;
	slateWalls[0]->vertices[0](1) = -TABLE_Z;
	slateWalls[0]->vertices[1](0) = -TABLE_X+moveRight+separate;
	slateWalls[0]->vertices[1](1) = TABLE_Z;

	slateWalls[1] = new cushion;
	slateWalls[1]->vertices[0](0) = -TABLE_X + moveRight + separate;
	slateWalls[1]->vertices[0](1) = TABLE_Z;
	slateWalls[1]->vertices[1](0) = TABLE_X + moveRight + separate;
	slateWalls[1]->vertices[1](1) = TABLE_Z;

	slateWalls[2] = new cushion;
	slateWalls[2]->vertices[0](0) = TABLE_X + moveRight + separate;
	slateWalls[2]->vertices[0](1) = TABLE_Z;
	slateWalls[2]->vertices[1](0) = TABLE_X + moveRight + separate;
	slateWalls[2]->vertices[1](1) = -TABLE_Z;

	slateWalls[3] = new cushion;
	slateWalls[3]->vertices[0](0) = TABLE_X + moveRight + separate;
	slateWalls[3]->vertices[0](1) = -TABLE_Z;
	slateWalls[3]->vertices[1](0) = -TABLE_X + moveRight + separate;
	slateWalls[3]->vertices[1](1) = -TABLE_Z;

	for (int i = 0; i < NUM_CUSHIONS; i++)
	{
		slateWalls[i]->MakeCentre();
		slateWalls[i]->MakeNormal();
	}
}

//Stage 2
void table::Update(int ms, int activeSlate)
{
	//check for collisions for each ball
	for (int i = 0; i < gTable.bc.num; i++)
	{
		for (int j = 0; j < NUM_CUSHIONS; j++)
		{
			//Stage 3
			gTable.bc.balls[i]->DoPlaneCollision(*gTable.slate[activeSlate].slateWalls[j]);
		}

		for (int j = (i + 1); j < gTable.bc.num; j++)
		{
			//Stage 7
			gTable.bc.balls[i]->DoBallCollision(*gTable.bc.balls[j]);
		}
	}

	//update all balls, Stage 11
	for (int i = 0; i < gTable.bc.num; i++) gTable.bc.balls[i]->Update(ms);

	//update particles, Stage 13
	parts.update(ms);
}

//Called in Stage 1
bool table::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for (int i = 0; i < gTable.bc.num; i++)
	{
		if (gTable.bc.balls[i]->velocity(0) != 0.0) return true;
		if (gTable.bc.balls[i]->velocity(1) != 0.0) return true;
	}
	return false;
}
#include <iostream>
#include <algorithm>
int* table::calculateScore(float x)
{
	static int returnBallID[NUM_BALLS];
	int ballID[NUM_BALLS];
	float allDist[NUM_BALLS];
	float tempDist[NUM_BALLS];

	for (int i = 0; i < NUM_BALLS; i++)
	{
		float currentDistance = sqrt(pow(bc.balls[i]->position(0) - x, 2.0) + pow(bc.balls[i]->position(1) - -0.8, 2.0));
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

float* table::calcX(int currentPlayer, float theta, float r)
{
	float ballX = bc.balls[currentPlayer]->position(0);
	float x = r * cosf(theta) + ballX;//calculate the x component 
	return &x;
}

float* table::calcZ(int currentPlayer, float theta, float r)
{
	float ballZ = bc.balls[currentPlayer]->position(1);
	float z = r * sinf(theta) + ballZ;//calculate the z component
	return &z;
}
