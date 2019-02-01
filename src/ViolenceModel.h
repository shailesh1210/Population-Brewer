#ifndef __ViolenceModel_h__
#define __ViolenceModel_h__

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <list>
#include <cmath>
#include <fstream>
#include <boost/math/special_functions/round.hpp>
#include <iomanip>

#include "PopBrewer.h"
#include "ViolenceAgent.h"

class Counter;
class PersonPums;
class HouseholdPums;
class Metro;
class County;
class Random;

#define PARKLAND 1101
#define TAYLOR 2700

#define IN_SCHOOL_NETWORK 1
#define OUT_SCHOOL_NETWORK 0

//identifiers of P values
#define STUDENT 0
#define TEACHER 1
#define ORIGIN 2
#define EDUCATION 3
#define AGE 4
#define GENDER 5

#define PREVAL_STUDENTS 0
#define PREVAL_TEACHERS_FEMALE 1
#define PREVAL_TEACHERS_MALE 2
#define PREVAL_FAM_FEMALE 3
#define PREVAL_FAM_MALE 4
#define PREVAL_COMM_FEMALE 5
#define PREVAL_COMM_MALE 6


class ViolenceModel : public PopBrewer
{
public:
	typedef std::vector<int> VecInts;
	typedef std::vector<double>VecDbls;
	typedef std::vector<ViolenceAgent> Household;
	typedef std::vector<Household*>HouseholdListPtr;
	typedef std::vector<ViolenceAgent*>AgentListPtr;
	typedef std::map<std::string, AgentListPtr> AgentListMap;
	typedef std::map<int, std::vector<Household>> HouseholdMap;
	typedef std::map<std::string, int> MapInt;
	typedef std::map<std::string, double> MapDbl;
	typedef std::map<std::string, bool> MapBool;
	typedef std::pair<double, double> PairDD;
	typedef std::map<std::string, PairDD> MapPair;

	ViolenceModel();
	virtual ~ViolenceModel();

	void start();

	void addHousehold(const HouseholdPums *, int);
	void addAgent(const PersonPums *);

	Counter * getCounter() const;
	
	int getNumStudents(const std::vector<ViolenceAgent> &, int &) const;
	int getOriginKey(const ViolenceAgent *) const;
	int getEducationKey(const ViolenceAgent *)const;
	int getInnerDraws() const;
	int getOuterDraws() const;
	int getMinHouseholdsPuma() const;
	int getMeanFriendSize() const;
	int getNumTeachers() const;
	int getMinAge() const;
	int getAgeDiffStudents() const;
	int getAgeDiffOthers() const;
	int getAffectedStudents() const;
	int getAffectedTeachers() const;
	int getMaxWeeks() const;
	double getPval(int) const;
	double getPrevalence(int) const;

	void setSize(int);
	void setPtsdStatus(AgentListMap *, int, double, int);

	void clearList();

private:

	void createPopulation();
	void distributePtsdStatus();
	void runModel();

	void createSchool(std::vector<Household>*);
	void createAgentHashMap(AgentListMap *, ViolenceAgent *, int);

	void createSocialNetwork(std::vector<Household>*, int);
	void findFriends(AgentListMap *, ViolenceAgent *, int);

	void distPrimaryPtsd();
	void distSecondaryPtsd();
	void distTertiaryPtsd();

	void poolPrimaryRiskAgents(AgentListMap*, AgentListMap*, int);
	void poolSecondaryRiskAgents(AgentListMap*);
	void poolTertiaryRiskAgents(AgentListMap*);
	
	void schoolDemographics();
	void networkAnalysis();
	
	void initializeHouseholdMap(int);
	void resizeHouseholds();
	bool isAffectedHousehold(Household *);
	
	Counter *count;
	Random *random;

	std::string schoolName;

	HouseholdMap pumaHouseholds;
	HouseholdListPtr schoolHouseholds;
	
	AgentListMap studentsMap, teachersMap, othersMap;
	MapBool primaryRiskPool, secondaryRiskPool, tertiaryRiskPool;
	
	//std::multimap<int, County> countyMap;
	//std::multimap<std::string, ViolenceAgent> m_students;

};

#endif