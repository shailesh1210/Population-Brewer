#ifndef __CardioModel_h__
#define __CardioModel_h__

#include <iostream>
#include <vector>
#include <map>

#include "CardioAgent.h"
#include "PopBrewer.h"

//class Parameters;
class Metro;
class PersonPums;
class HouseholdPums;
class Random;
class Counter;

class CardioModel : public PopBrewer
{
public:
	typedef std::pair<double, double> PairDD;
	typedef std::map<std::string, std::vector<PairDD>> ProbMapRf;
	typedef std::map<std::string, double> MapDbl;
	typedef std::vector<CardioAgent> AgentList;
	typedef std::multimap<std::string, CardioAgent *> AgentPtr;
	
	CardioModel();
	virtual ~CardioModel();

	void start();

	void addHousehold(const HouseholdPums *, int);
	void addAgent(const PersonPums *);

	Counter * getCounter() const;
	EET::Framingham getBeta(int);

	void setSize(int);
	void clearList();

private:
	void createPopulation(Metro *);
	void setRiskFactors();
	void setFraminghamRiskScore();

	void rounding(std::vector<PairDD>&, double &);
	Counter *count;

	AgentList agentList;
	AgentPtr agentsPtrMap;
	
};
#endif