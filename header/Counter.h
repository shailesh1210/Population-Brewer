#ifndef __Counter_h__
#define __Counter_h__

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <cmath>

#include "ViolenceAgent.h"

class Parameters;

struct Risk
{
	double val;
	double lowLim;
	double upLim;

	Risk &operator+=(const Risk &r)
	{
		this->val += r.val;
		this->lowLim += r.lowLim;
		this->upLim += r.upLim;

		return *this;
	}

	Risk &operator/=(const int num_trials)
	{
		this->val /= num_trials;
		this->lowLim /= num_trials;
		this->upLim /= num_trials;

		return *this;
	}

	friend std::ostream &operator<<(std::ostream &out, const Risk &r)
	{
		out << r.val << "," << r.lowLim << "," << r.upLim;
		return out;
	}

};

struct Outcomes
{
	double value[NUM_TREATMENT];
	Risk diff;
	Risk ratio;
};


class Counter
{
public:
	typedef std::map<std::string, std::vector<bool>> TypeMap;
	typedef std::map<int, int> MapInts;
	typedef std::map<int, double> MapDbls;
	typedef std::map<int, Outcomes> MapOutcomes;
	typedef std::vector<std::string> Pool;
	typedef std::pair<double, double> Pair;

	Counter();
	Counter(std::shared_ptr<Parameters>);
	virtual ~Counter();

	void setParameters(std::shared_ptr<Parameters>);

	int getPersonCount(std::string) const;
	int getHouseholdCount(std::string) const;

	void initialize();
	//void reset();
	void output(std::string);

	void addHouseholdCount(std::string);
	void addPersonCount(std::string);

	void addRiskFactorCount(std::string);

	void addPtsdCount(int, int, int);
	void addPtsdResolvedCount(int, int, int);
	void addCbtReach(int, int);
	void addSprReach(int, int);
	void addCbtCount(int);
	void addSprCount(int);
	void addNaturalDecayCount(int);

	void computeOutcomes(int, int);

	Pair getPrevalence(int, int);
	double getCbtUptake(int);
	double getSprUptake(int);
	double getNaturalDecayUptake(int);

	void outputHouseholdCounts(std::string);
	void outputPersonCounts(std::string);

	void outputRiskFactorPercent();
	void outputHealthOutcomes();

	
private:

	void initHouseholdCounter();
	void initPersonCounter();
	void initRiskFacCounter();

	void initPtsdCounter();
	void initTreatmentCounter(int);

	void computePrevalence(int, int);
	void computeRecovery(int, int);
	
	Pair computeError(double *, double *);
	Risk computeDiff(double *, double);
	Risk computeRatio(double *, double);

	void clearMap(TypeMap &);
	void clearCounter();

	std::shared_ptr<Parameters> parameters;

	TypeMap m_personCount, m_householdCount;
	TypeMap m_riskFacCount;

	//Counters for Mass Violence Model(PTSD and PTSD resolved)
	MapInts m_ptsdCount[NUM_TREATMENT][NUM_PTSD], m_ptsdResolvedCount[NUM_TREATMENT][NUM_PTSD];
	MapDbls m_totPrev[NUM_TREATMENT][NUM_PTSD], m_totRecovery[NUM_TREATMENT][NUM_PTSD];

	MapInts m_cbtReach[NUM_TREATMENT], m_sprReach[NUM_TREATMENT];
	MapInts m_cbtCount, m_sprCount, m_ndCount;
	MapOutcomes m_prevalence, m_recovery;
	

};
#endif __Counter_h__