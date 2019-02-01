#ifndef __Counter_h__
#define __Counter_h__

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <cmath>

#include "ViolenceAgent.h"

class Parameters;
class CardioAgent;

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

#define WEEKS_IN_YEAR 52
#define DAYS_IN_WEEK 7

class Counter
{
public:
	typedef std::map<std::string, std::vector<bool>> TypeMap;
	typedef std::map<int, int> MapInts;
	typedef std::map<int, double> MapDbls;
	typedef std::map<int, Outcomes> MapOutcomes;
	typedef std::vector<std::string> Pool;
	typedef std::pair<double, double> Pair;
	typedef std::map<std::string, std::map<int, double>> RiskFacMap;
	typedef std::vector<ViolenceAgent*> AgentListPtr;
	typedef std::vector<double> VectorDbls;

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
	void addPersonCount(int, int);

	//CVD model
	void addRiskFactorCount(std::string);
	void sumRiskFactors(CardioAgent *);

	//MV model
	void addPtsdCount(int, int, int);
	void addCbtReferredNonPtsd();
	void addPtsdResolvedCount(int, int, int);
	void addCbtReach(int, int);
	void addSprReach(int, int);
	void addCbtCount(ViolenceAgent*, int, int);
	void addSprCount(ViolenceAgent*, int, int);
	void addNaturalDecayCount(int);

	void computeOutcomes(int, int);
	void computeCostEffectiveness(AgentListPtr *);

	//MV model
	VectorDbls getPrevalence(int, int);
	double getCbtUptake(int);
	double getSprUptake(int);
	double getNaturalDecayUptake(int);
	double getYLD(double, int);
	double getTotalCost(int);
	
	//CVD model
	double getMeanAge(std::string) const;
	double getMeanTchols(std::string) const;
	double getMeanHChols(std::string) const;
	double getMeanBP(std::string) const;
	double getPercentSmoking(std::string) const;

	void outputHouseholdCounts(std::string);
	void outputPersonCounts(std::string);

	void outputRiskFactorPercent();
	void outputHealthOutcomes();
	void outputCostEffectiveness();

	
private:

	void initHouseholdCounter();
	void initPersonCounter();
	void initRiskFacCounter();

	void initPtsdCounter();
	void initTreatmentCounter(int);

	void accumulateRiskFacs(CardioAgent *);

	void computePrevalence(int, int);
	void computeRecovery(int, int);

	void computeDALYs(AgentListPtr *);
	void computeTotalCost();
	void computeAverageCost();
	
	Pair computeError(double *, double *);
	Risk computeDiff(double *, double);
	Risk computeRatio(double *, double);

	void clearMap(TypeMap &);
	void clearCounter();

	std::shared_ptr<Parameters> parameters;

	TypeMap m_personCount, m_householdCount;
	
	RiskFacMap m_sumRiskFac;
	TypeMap m_riskFacCount;
	
	//Counters for Mass Violence Model(PTSD and PTSD resolved)
	int nonPtsdCountSC;
	MapInts m_ptsdCount[NUM_TREATMENT][NUM_PTSD], m_ptsdResolvedCount[NUM_TREATMENT][NUM_PTSD];
	MapDbls m_totPrev[NUM_TREATMENT][NUM_PTSD], m_totRecovery[NUM_TREATMENT][NUM_PTSD];
	
	MapInts m_cbtReach[NUM_TREATMENT], m_sprReach[NUM_TREATMENT];
	MapInts m_cbtCount, m_sprCount, m_ndCount; //overall count of CBT and SPR treatment
	MapInts m_totCbt[NUM_TREATMENT][NUM_CASES], m_totSpr[NUM_TREATMENT][NUM_CASES];
	
	MapOutcomes m_prevalence, m_recovery;
	MapDbls m_totDalys, m_totPtsdFreeWeeks, m_totCost, m_avgCost;
	

};
#endif __Counter_h__