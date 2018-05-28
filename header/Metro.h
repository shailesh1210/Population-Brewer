#ifndef __Metro_h__
#define __Metro_h__

#include <iostream>
#include <memory>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <numeric>
#include "PersonPums.h"
#include "HouseholdPums.h"
#include "Agent.h"

class County;
class Parameters;
//class PUMS;
class Agent;

class Metro
{
public:

	typedef std::vector<std::string> Columns;
	typedef std::list<Columns> Rows;
	typedef std::vector<double> Marginal;
	typedef std::map<int, std::map<int, std::vector<double>>> ProbabilityMap; //typedef to store probabilities by race, age and sex

	Metro(std::shared_ptr<Parameters>);
	//Metro(std::string, std::string);
	virtual ~Metro();

	void setMetroIDandName(const std::string &, const std::string &);
	void setPopulation(const int &);
	void setCounties(const County &);
	void setEstimates(const Columns &, int);

	void createAgents();

	std::string getGeoID() const;
	std::string getMetroName() const;
	std::multimap<int, County> getPumaCountyMap() const;
	int getCountyNum(std::string) const;
	
private:

	void importHouseholdPUMS();
	void importPersonPUMS();
	void runIPF();
	void assignAgentAttributes();

	//Marginal getRaceEstimates();
	std::map<int, Columns> getRaceEstimatesMap();
	ProbabilityMap computeProportions(const std::map<int, Columns>&, size_t, size_t, size_t, size_t, int);

	std::vector<Marginal>getEstimatesVector(const std::map<int, Columns>&, size_t, size_t, int);
	Marginal getRaceEstimateVector(const std::map<int, Columns>&, const std::unordered_multimap<int, int>&, size_t, size_t);
	Marginal getEducationEstimateVector();
	Marginal getMaritalStatusEstimateVector();

	std::map<int, Marginal> getIPFestimates(int, int, Marginal &, Marginal &, size_t, size_t, int);
	void convertToProportions(std::map<int, Marginal>&);

	void createSeedMatrix(int, int, size_t, size_t, int);
	void setMarginals(Marginal &, int);
	void adjustMarginals(Marginal &, Marginal &);
	void clear();
	bool isValidGeoID(std::string);
	int getCount(int, int, int, int, int, int);
	void educationCounts();
	void maritalStatusCounts();
	std::string toString(int, int);

	std::shared_ptr<Parameters> parameters;
	
	std::string geoID;
	std::string metroName;
	int population;
	std::multimap<int, County> m_pumaCounty;

	std::multimap<std::string, Columns> m_metroRaceEst;
	std::multimap<std::string, Columns> m_metroEduEst;
	std::multimap<std::string, Columns> m_metroMaritalEst;

	std::multimap<std::string, Columns> m_metroHHTypeEst;

	std::list<PersonPums> pumsList;
	//std::unordered_multimap<std::string, PersonPums> m_personPUMS;
	std::unordered_map<double, HouseholdPums> m_householdPUMS;

	ProbabilityMap pEduByRace;
	ProbabilityMap pMaritalByRace;

	std::vector<double> seed;
	std::vector<Marginal> marginals;
	std::vector<int> m_size;

	std::vector<Agent> agentList;
};

#endif __Metro_h__