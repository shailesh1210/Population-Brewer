#ifndef __Agent_h__
#define __Agent_h__

#include <iostream>
#include <list>
#include <string>
#include <ctime>
#include <map>
#include <vector>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/tokenizer.hpp>

class Agent
{
public:
	typedef boost::tokenizer<boost::char_separator<char>> TokenizerString;
	typedef std::map<int, std::map<int, std::vector<double>>> ProbabilityMap; //typedef to store probabilities by race, age and sex

	Agent();
	virtual ~Agent();

	void setID(int);
	void setDemoAttributes(int, int, int);
	void setEducation(const ProbabilityMap &);
	void setMaritalStatus(const ProbabilityMap &);

	int getAgeCat() const;
	int getAge() const;
	int getGender() const;
	int getRace() const;
	int getEducation() const;
	int getEduAgeCat() const;
	int getMaritalStatus() const;
	int getMaritalAgeCat() const;

private:
	
	void setEduAgeCat();
	void setMaritalAgeCat();
	int randInteger(int, int);
	int getValue(const ProbabilityMap&, int, int);
	double uniformRealDist();

	int ID;
	int ageCat, age, race, gender;
	int education, eduAgeCat;
	int maritalStatus, maritalAgeCat;
};
#endif __Agent_h__