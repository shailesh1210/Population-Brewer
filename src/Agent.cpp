#include "Agent.h"
#include "ACS.h"

#define AGE_RANGE_SIZE 2

Agent::Agent() : ID(-1), ageCat(-1), age(-1), gender(-1), race(-1), education(-1), eduAgeCat(-1), maritalStatus(-1), maritalAgeCat(-1)
{
}

Agent::~Agent()
{
}

void Agent::setID(int idx)
{
	this->ID = idx;
}

void Agent::setDemoAttributes(int p_ageCat, int p_gender, int p_race)
{
	this->ageCat = p_ageCat;
	this->gender = p_gender;
	this->race = p_race;

	boost::char_separator<char> sep("_");
	std::list<std::string> ageRange;

	std::string ageCatStr = ACS::AgeCat::_from_integral_unchecked(ageCat)._to_string();
	TokenizerString tok(ageCatStr, sep);
	ageRange.assign(tok.begin(), tok.end());
	ageRange.pop_front();

	if(ageRange.size() < AGE_RANGE_SIZE)
		this->age = std::stoi(ageRange.front());
	else
	{
		int age1 = std::stoi(ageRange.front());
		int age2 = std::stoi(ageRange.back());

		this->age = randInteger(age1, age2);
	}

}

void Agent::setEducation(const ProbabilityMap& pEduByRace)
{
	setEduAgeCat();
	this->education = getValue(pEduByRace, gender, eduAgeCat);
	/*size_t key = 10*gender+eduAgeCat;

	if(pEduByRace.count(key) > 0)
	{
		std::vector<double> pEducation(pEduByRace.at(key).at(race));
		double randomP = uniformRealDist();

		for(size_t eduIdx = 0; eduIdx < pEducation.size(); ++eduIdx)
		{
			if(randomP < pEducation.at(eduIdx))
			{
				this->education = eduIdx+1;
				break;
			}
		}
	}*/
}

void Agent::setMaritalStatus(const ProbabilityMap &pMaritalByRace)
{
	setMaritalAgeCat();
	this->maritalStatus = getValue(pMaritalByRace, gender, maritalAgeCat);

	/*size_t key = 10*gender+maritalAgeCat;
	if(pMaritalByRace.count(key) > 0)
	{
		std::vector<double> pMarital(pMaritalByRace.at(key).at(race));
		double randomP = uniformRealDist();

		for(size_t marIdx = 0; marIdx < pMarital.size(); ++marIdx)
		{
			if(randomP < pMarital.at(marIdx))
			{
				this->maritalStatus = marIdx+1;
				break;
			}
		}
	}*/
}

void Agent::setEduAgeCat() 
{
	if(age >= 18 && age <= 24){
		this->eduAgeCat = ACS::EduAgeCat::Age_18_24;
	}
	else if(age >= 25 && age <= 34){
		this->eduAgeCat = ACS::EduAgeCat::Age_25_34;
	}
	else if(age >= 35 && age <= 44){
		this->eduAgeCat = ACS::EduAgeCat::Age_35_44;
	}
	else if(age >= 45 && age <= 64){
		this->eduAgeCat = ACS::EduAgeCat::Age_45_64;
	}
	else if(age >= 65){
		this->eduAgeCat = ACS::EduAgeCat::Age_65_Over;
	}

}

void Agent::setMaritalAgeCat()
{
	if(age >= 15 && age <= 19){
		this->maritalAgeCat = ACS::MaritalAgeCat::Age_15_19;
	}
	else if(age >= 20 && age <= 34){
		this->maritalAgeCat = ACS::MaritalAgeCat::Age_20_34;
	}
	else if(age >= 35 && age <= 44){
		this->maritalAgeCat = ACS::MaritalAgeCat::Age_35_44;
	}
	else if(age >= 45 && age <= 54){
		this->maritalAgeCat = ACS::MaritalAgeCat::Age_45_54;
	}
	else if(age >= 55 && age <= 64){
		this->maritalAgeCat = ACS::MaritalAgeCat::Age_55_64;
	}
	else if(age >= 65){
		this->maritalAgeCat = ACS::MaritalAgeCat::Age_65_Over;
	}
}

int Agent::getAgeCat() const
{
	return ageCat;
}

int Agent::getAge() const
{
	return age;
}

int Agent::getGender() const
{
	return gender;
}

int Agent::getRace() const
{
	return race;
}

int Agent::getEducation() const
{
	return education;
}

int Agent::getEduAgeCat() const
{
	return eduAgeCat;
}

int Agent::getMaritalStatus() const
{
	return maritalStatus;
}

int Agent::getMaritalAgeCat() const
{
	return maritalAgeCat;
}

int Agent::getValue(const ProbabilityMap &prByRace, int gender, int ageCat)
{
	int val = -1;
	size_t key = 10*gender+ageCat;
	if(prByRace.count(key) > 0)
	{
		std::vector<double> pr(prByRace.at(key).at(race));
		double randomP = uniformRealDist();

		for(size_t idx = 0; idx < pr.size(); ++idx)
		{
			if(randomP < pr.at(idx))
			{
				val = idx+1;
				break;
			}
		}
	}

	return val;
}
int Agent::randInteger(int min, int max)
{
	static boost::mt19937 rng((uint32_t)time(NULL));

	boost::random::uniform_int_distribution<>dist(min, max);
	boost::random::variate_generator<boost::mt19937&, boost::random::uniform_int_distribution<>>generator(rng, dist);

	int randVal = generator();

	return randVal;
}

double Agent::uniformRealDist()
{
	static boost::mt19937 rng((uint32_t)time(NULL));

	boost::random::uniform_real_distribution<>dist(0, 1);
	boost::random::variate_generator<boost::mt19937&, boost::random::uniform_real_distribution<>>generator(rng, dist);

	double randVal = generator();

	return randVal;
}