#ifndef __CardioAgent_h__
#define __CardioAgent_h__

#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "Agent.h"

class PersonPums;
class Random;
class Parameters;


struct RiskFactors
{
	double tchols, ldlChols;
	double systolicBp;
	bool smokingStatus;
};


class CardioAgent : public Agent
{
public:
	
	typedef std::pair<double, double> PairDD;
	typedef std::map<std::string, PairDD> PairMap;
	typedef std::map<int, std::map<std::string, PairDD>> RiskMap;

	CardioAgent();
	CardioAgent(const PersonPums *, std::shared_ptr<Parameters>);

	virtual ~CardioAgent();

	void setNHANESOrigin(short int);
	void setNHANESEduCat(short int);
	void setRiskFactors(Random &, short int);

	short int getNHANESAgeCat() const;
	short int getNHANESOrigin() const;
	short int getNHANESEduCat() const;
	short int getRiskStrata() const;
	RiskFactors getRiskChart() const;

private:

	void setNHANESAgeCat();

	void setTotalCholesterol(Random &, std::string);
	void setLDLCholesterol(Random &, std::string);
	void setSystolicBp(Random &, std::string);
	void setSmokingStatus();

	std::shared_ptr<Parameters>parameters;

	short int nhanes_ageCat;
	short int nhanes_org;
	short int nhanes_edu;

	short int rfStrata;

	RiskFactors chart;
};

#endif __CardioAgent_h__