#include "Agent.h"

Agent::Agent()
{
}

Agent::~Agent()
{

}

void Agent::setHouseholdID(double h_idx)
{
	this->householdID = h_idx;
}

void Agent::setPUMA(int puma_code)
{
	this->puma = puma_code;
}


void Agent::setAge(short int p_age)
{
	this->age = p_age;
}

void Agent::setSex(short int p_sex)
{
	this->sex = p_sex;
}

void Agent::setOrigin(short int p_org)
{
	this->origin = p_org;
}

void Agent::setEducation(short int p_edu)
{
	this->education = p_edu;
}

double Agent::getHouseholdID() const
{
	return householdID;
}

int Agent::getPUMA() const
{
	return puma;
}

short int Agent::getAge() const
{
	return age;
}

short int Agent::getSex() const
{
	return sex;
}

short int Agent::getOrigin() const
{
	return origin;
}

short int Agent::getEducation() const
{
	return education;
}