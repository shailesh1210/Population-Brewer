#ifndef __ViolenceAgent_h__
#define __ViolenceAgent_h__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Agent.h"

#define NUM_PTSD 3
#define PRIMARY 0
#define SECONDARY 1
#define TERTIARY 2

//PTSD treatments
#define NUM_TREATMENT 2
#define STEPPED_CARE 0
#define USUAL_CARE 1

#define CBT 0
#define SPR 1

#define MIN_PTSDX 0
#define MAX_PTSDX 17

class PersonPums;
class Parameters;
class Random;
class Counter;

class ViolenceAgent : public Agent
{
public:
	typedef std::vector<ViolenceAgent*>FriendsPtr;
	typedef std::pair<double, double> PairDD;
	typedef std::map<std::string, PairDD> MapPair;
	typedef std::map<std::string, double> MapDbl;
	typedef std::map<std::string, bool> MapBool;

	ViolenceAgent();
	ViolenceAgent(std::shared_ptr<Parameters>, const PersonPums *, Random *rand, Counter *count, int, int);

	virtual ~ViolenceAgent();

	void excecuteRules(int);

	void setAgentIdx(std::string);
	void setAgeCat();
	void setNewOrigin();
	void setPTSDx(MapPair *, bool);
	void setPTSDstatus(bool, int);
	void setSchoolName(std::string);
	void setFriendSize(int);
	void setFriend(ViolenceAgent *);
	void setDummyVariables();

	std::string getAgentIdx() const;
	double getPTSDx(int) const;
	double getPTSDx(PairDD) const;
	double getPtsdCutOff() const;
	bool getPTSDstatus(int) const;
	int getPtsdType() const;
	std::string getSchoolName() const;
	FriendsPtr *getFriendList();
	int getFriendSize() const;
	int getTotalFriends() const;
	int getMaxCbtTime() const;
	int getMaxSprTime() const;

	bool isStudent() const;
	bool isTeacher() const;
	bool isCompatible(const ViolenceAgent *) const;
	bool isFriend(const ViolenceAgent *) const;
	bool isPrimaryRisk(const MapBool *) const;
	bool isSecondaryRisk(const MapBool *) const;

private:
	
	void priorTreatmentUptake(int);
	void ptsdScreeningSteppedCare(int);
	void provideTreatment(int, int);
	void symptomResolution(int, int);
	void symptomRelapse(int);

	void provideCBT(int, int);
	void provideSPR(int, int);
	double getTrtmentUptakeProbability(int, int);

	Random *random;
	Counter *counter;
	std::shared_ptr<Parameters>parameters;

	std::string schoolName, agentIdx;
	short int ageCat, newOrigin;
	int friendSize;
	FriendsPtr friendList;

	//ptsd variables
	double initPtsdx, ptsdx[NUM_TREATMENT];
	bool priPTSD, secPTSD, terPTSD;
	short int ptsdTime[NUM_TREATMENT];

	//treatment variables
	short int priorCBT, priorSPR;
	bool cbtReferred, sprReferred;
	bool curCBT[NUM_TREATMENT], curSPR[NUM_TREATMENT];
	short int sessionsCBT[NUM_TREATMENT], sessionsSPR[NUM_TREATMENT];
	
	//relapse variables
	short int numRelapse[NUM_TREATMENT], relapseTimer[NUM_TREATMENT];

	//resolution variables
	bool isResolved[NUM_TREATMENT];
	short int resolvedTime[NUM_TREATMENT];

	//dummy variables
	short int age1, age2, age3;
	short int white, black, hispanic, other;
};
#endif