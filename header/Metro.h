#ifndef __Metro_h__
#define __Metro_h__

#include <iostream>
#include <memory>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <boost/range/algorithm.hpp>

#include <boost/math/distributions/chi_squared.hpp>

#include "PersonPums.h"
#include "HouseholdPums.h"

class County;
class Parameters;
class Counter;
class IPUWrapper;
class CardioModel;

class Metro
{
public:

	typedef std::vector<std::string> Columns;
	typedef std::multimap<int, std::multimap<std::string, Columns>> ACSEstimates;
	typedef std::pair<double, double> PairDD;
	typedef std::map<std::string, std::map<double,std::vector<PairDD>>> ProbMap;
	typedef std::map<std::string, std::vector<PairDD>> ProbMapRf;
	typedef std::map<int,std::map<std::string, PairDD>> RiskFacMap;
	typedef std::map<std::string, PairDD> PairMap;
	typedef std::map<double, HouseholdPums> PUMSHouseholdsMap;
	typedef std::multimap<int, County> CountyMap;
	typedef std::vector<double> Marginal;
	typedef std::vector<std::string> Pool;

	Metro();
	Metro(std::shared_ptr<Parameters>);
	virtual ~Metro();

	void setMetroIDandName(const std::string &, const std::string &);
	void setPopulation(const int &);
	void setCounties(const County &);
	void setEstimates(const Columns &, int);

	std::string getGeoID() const;
	std::string getMetroName() const;
	int getPopulation() const;

	std::multimap<int, County> getPumaCountyMap() const;
	int getCountyNum(std::string) const;

	template <class T>
	void createAgents(T *);
	
protected:
	
	template <class T>
	void drawHouseholds(IPUWrapper *, T *);

	bool checkFit(const Marginal *, const Counter *, int);
	void gofLog(double, int, int);
	//void normalDistCurve();
	
	std::shared_ptr<Parameters> parameters;
	IPUWrapper *ipuWrapper;
	
	std::string geoID;
	std::string metroName;
	int population;

	CountyMap m_pumaCounty;
	ACSEstimates m_metroACSEst;
	
};

#endif __Metro_h__