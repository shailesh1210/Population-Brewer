#include "ViolenceAgent.h"
#include "PersonPums.h"
#include "ACS.h"
#include "Parameters.h"
#include "Random.h"
#include "Counter.h"

ViolenceAgent::ViolenceAgent()
{
}

ViolenceAgent::ViolenceAgent(std::shared_ptr<Parameters> param, const PersonPums *p, Random *rand, Counter *count, int hhCount, int countPersons) 
	: parameters(param), random(rand), counter(count)
{
	this->householdID = hhCount;
	this->puma = p->getPumaCode();

	this->age = p->getAge();
	this->sex = p->getSex();
	this->origin = p->getOrigin();

	this->education = p->getEducation();

	this->agentIdx = "Agent"+std::to_string(hhCount)+std::to_string(countPersons);
	
	for(int i = 0; i < NUM_TREATMENT; ++i)
	{
		this->ptsdx[i] = 0.0;
		this->ptsdTime[i] = 0;
		this->curCBT[i] = false;
		this->curSPR[i] = false;
		this->sessionsCBT[i] = 0;
		this->sessionsSPR[i] = 0;
		this->isResolved[i] = false;
		this->resolvedTime[i] = 0;
		this->numRelapse[i] = 0;
		this->relapseTimer[i] = 0;
	}
	
	this->initPtsdx = ptsdx[STEPPED_CARE];
	
	this->priPTSD = false;
	this->secPTSD = false;
	this->terPTSD = false;

	this->priorCBT = 0;
	this->priorSPR = 0;

	this->schoolName = "N/A";

	setAgeCat();
	setNewOrigin();
	setDummyVariables();
}

ViolenceAgent::~ViolenceAgent()
{
	
}

void ViolenceAgent::excecuteRules(int tick)
{
	if(initPtsdx != 0.0)
	{
		priorTreatmentUptake(tick);
		ptsdScreeningSteppedCare(tick);

		for(int i = 0; i < NUM_TREATMENT; ++i)
		{
			provideTreatment(tick, i);
			symptomResolution(tick, i);
			symptomRelapse(i);
		}
	}
}

void ViolenceAgent::priorTreatmentUptake(int tick)
{
	if(tick == 0)
	{
		//Random random;
		short int male = (sex == Violence::Sex::Male) ? 1 : 0;

		double logCBT = 0;
		double logSPR = 0;

		if(initPtsdx < getPtsdCutOff())
		{
			logCBT = -1.3360 + (-0.0881*male) + (-0.7250*black) + (-0.3133*hispanic) + (-0.7122*other) + (0.6201*age2) + (-0.6444*age3);
			logSPR = -1.095 + (-0.2119*male) + (-0.5343*black) + (-0.2377*hispanic) + (-0.7551*other) + (0.5693*age2) + (-0.5413*age3);
		}
		else if(initPtsdx >= getPtsdCutOff())
		{
			logCBT = 0.2887 + (-0.1323*male) + (-0.4647*black) + (-0.4502*hispanic) + (-1.2979*other) + (0.1119*age2) + (-0.1129*age3);
			logSPR = 0.6048 + (-0.3331*male) + (-0.5914*black) + (-0.4878*hispanic) + (-0.8119*other) + (0.2076*age2) + (-0.0720*age3);
		}

		double pCBT = exp(logCBT)/(1+exp(logCBT));
		double pSPR = exp(logSPR)/(1+exp(logSPR));

		double randomCBT = random->uniform_real_dist();
		priorCBT = (randomCBT < pCBT) ? 1 : 0;

		double randomSPR = random->uniform_real_dist();
		priorSPR = (randomSPR < pSPR) ? 1 : 0;
	}
}

void ViolenceAgent::ptsdScreeningSteppedCare(int tick)
{
	int screening_time = parameters->getViolenceParam()->screening_time;
	if(tick == screening_time)
	{
		if(initPtsdx >= getPtsdCutOff())
		{
			double randomP1 = random->uniform_real_dist();
			double sensitivity = parameters->getViolenceParam()->sensitivity;
			if(randomP1 < sensitivity)
			{
				cbtReferred = true;
				sprReferred = false;
			}
			else
			{
				cbtReferred = false;
				sprReferred = true;
			}
		}
		else if(initPtsdx < getPtsdCutOff())
		{
			double randomP2 = random->uniform_real_dist();
			double specificity = 1 - (parameters->getViolenceParam()->specificity);
			if(randomP2 < specificity)
			{
				cbtReferred = true;
				sprReferred = false;
			}
			else
			{
				cbtReferred = false;
				sprReferred = true;
			}
		}
	}
}

void ViolenceAgent::provideTreatment(int tick, int treatment)
{
	int screening_time = parameters->getViolenceParam()->screening_time;
	if(tick >= screening_time)
	{
		if(treatment == STEPPED_CARE)
		{
			if(cbtReferred && !sprReferred)
				provideCBT(tick, STEPPED_CARE);
			else if(!cbtReferred && sprReferred)
				provideSPR(tick, STEPPED_CARE);
		}
		else if(treatment == USUAL_CARE)
		{
			provideSPR(tick, USUAL_CARE);
		}
	}
}

void ViolenceAgent::symptomResolution(int tick, int treatment)
{
	double strength = 0;
	int max_sessions = -1;
	
	if(curCBT[treatment] && !curSPR[treatment])
	{
		strength = parameters->getViolenceParam()->cbt_coeff;
		max_sessions = parameters->getViolenceParam()->max_cbt_sessions;
	
		if(ptsdx[treatment] >= getPtsdCutOff() && treatment == STEPPED_CARE)
			counter->addCbtCount(tick);

		sessionsCBT[treatment] += 1;
		curCBT[treatment] = false;
	}
	else if(!curCBT[treatment] && curSPR[treatment])
	{
		strength = parameters->getViolenceParam()->spr_coeff;
		max_sessions =  parameters->getViolenceParam()->max_spr_sessions;

		if(ptsdx[treatment] >= getPtsdCutOff() && treatment == STEPPED_CARE)
			counter->addSprCount(tick);

		sessionsSPR[treatment] += 1;
		curSPR[treatment] = false;
	}
	else if(!curCBT[treatment] && !curSPR[treatment])
	{
		double pNatDecay = parameters->getViolenceParam()->percent_nd;
		double randomP = random->uniform_real_dist();

		max_sessions = parameters->getViolenceParam()->nd_dur;
		if(randomP < pNatDecay)
		{
			if(ptsdx[treatment] >= getPtsdCutOff() && treatment == STEPPED_CARE)
				counter->addNaturalDecayCount(tick);
			strength = parameters->getViolenceParam()->nd_coeff;
		}
	}

	if(ptsdx[treatment] > 0)
	{
		ptsdx[treatment] = ptsdx[treatment] - (strength/max_sessions)*ptsdx[treatment];

		if(ptsdx[treatment] < 0)
			ptsdx[treatment] = 0;

		if(initPtsdx >= getPtsdCutOff())
		{
			if(ptsdx[treatment] >= getPtsdCutOff())
			{
				ptsdTime[treatment] += 1;
				isResolved[treatment] = false;

				counter->addPtsdCount(treatment, getPtsdType(), tick);
			}
			else if(ptsdx[treatment] < getPtsdCutOff() && !isResolved[treatment])
			{
				isResolved[treatment] = true;
			}

			if(isResolved[treatment])
			{
				counter->addPtsdResolvedCount(treatment, getPtsdType(), tick);
				resolvedTime[treatment] += 1;
			}
		}
	}
}

void ViolenceAgent::symptomRelapse(int treatment)
{
	double relapse_ptsdx = parameters->getViolenceParam()->ptsdx_relapse;
	if(isResolved[treatment] && initPtsdx > relapse_ptsdx)
	{
		double relapse_time = parameters->getViolenceParam()->time_relapse;
		double max_relapses = parameters->getViolenceParam()->num_relapse;

		relapseTimer[treatment] += 1;
		if(relapseTimer[treatment] > relapse_time && numRelapse[treatment] < max_relapses)
		{
			double randomP = random->uniform_real_dist();
			double percent_relapse = parameters->getViolenceParam()->percent_relapse;

			if(randomP < percent_relapse)
			{
				ptsdx[treatment] = 0.8*initPtsdx;
			
				curCBT[treatment] = false;
				curSPR[treatment] = false;

				isResolved[treatment] = false;
				resolvedTime[treatment] = 0;

				numRelapse[treatment] += 1;
			}
		}
	}
}

void ViolenceAgent::provideCBT(int tick, int treatment)
{
	double pCBT = getTrtmentUptakeProbability(treatment, CBT);
	if(pCBT > 0.0)
	{
		double randomP = random->uniform_real_dist();
		int maxCBTsessions = parameters->getViolenceParam()->max_cbt_sessions;
	
		if(randomP < pCBT && tick <= getMaxCbtTime())
		{
			curSPR[treatment] = false;
			if(sessionsCBT[treatment] < 2*maxCBTsessions)
				curCBT[treatment] = true;
			else
				curCBT[treatment] = false;

			if(treatment == STEPPED_CARE)
			{
				if(sessionsCBT[treatment] == 0 && sessionsSPR[treatment] == 0 && ptsdx[treatment] >= getPtsdCutOff())
					counter->addCbtReach(treatment, tick);
			}
		}
		else
		{
			provideSPR(tick, treatment);
		}
	}
	else
	{
		curCBT[treatment] = false;
	}
}

void ViolenceAgent::provideSPR(int tick, int treatment)
{
	curCBT[treatment] = false;

	double pSPR = getTrtmentUptakeProbability(treatment, SPR);
	if(pSPR > 0)
	{
		double randomP = random->uniform_real_dist();
		int maxSPRsessions = parameters->getViolenceParam()->max_spr_sessions;

		if(randomP < pSPR && tick <= getMaxSprTime())
		{
			if(sessionsSPR[treatment] < maxSPRsessions)
				curSPR[treatment] = true;
			else
				curSPR[treatment] = false;

			if(sessionsCBT[treatment] == 0 && sessionsSPR[treatment] == 0 && ptsdx[treatment] >= getPtsdCutOff())
				counter->addSprReach(treatment, tick);
		}
		else
		{
			curSPR[treatment] = false;
		}
	}
	else
	{
		curSPR[treatment] = false;
	}
}

double ViolenceAgent::getTrtmentUptakeProbability(int treatment, int type)
{
	double log = 0.0; 
	double p = 0.0;
	
	int male = (sex == Violence::Sex::Male) ? 1 : 0;

	if(initPtsdx >= getPtsdCutOff() && ptsdx[treatment] >= getPtsdCutOff())
	{
		if(type == CBT)
			log = -2.11 + (-0.1941*male) + (-0.5237*black) + (-1.0845*hispanic) + (-0.2653*other) + (-0.1139*age2) + (0.2455*age3) + (1.8377*priorCBT);
		else if(type == SPR)
			log = -1.7778 + (0.00136*male) + (-0.6774*black) + (-0.8136*hispanic) + (-0.3118*other) + (-0.0549*age2) + (0.3746*age3) + (1.4076*priorSPR);
	}
	else if(initPtsdx < getPtsdCutOff() && ptsdx[treatment] != 0)
	{
		if(type == CBT)
			log = -4.1636 + (-0.6422*male) + (-0.8040*black) + (-0.6795*hispanic) + (0.1976*other) + (0.1453*age2) + (-0.4203*age3) + (2.4109*priorCBT);
		else if(type == SPR)
			log = -3.7355 + (-0.5319*male) + (-1.2562*black) + (-0.4632*hispanic) + (0.1427*other) + (0.1703*age2) + (-0.5289*age3) + (2.0696*priorSPR);
	}
	
	p = (log != 0.0) ? exp(log)/(1+exp(log)) : 0.0; 
	return p;
}

void ViolenceAgent::setAgentIdx(std::string idx)
{
	this->agentIdx = idx;
}

void ViolenceAgent::setAgeCat()
{
	if(age >= 14 && age <= 34)
		ageCat = Violence::AgeCat::Age_14_34;
	else if(age >= 35 && age <= 64)
		ageCat = Violence::AgeCat::Age_35_64;
	else if(age >= 65)
		ageCat = Violence::AgeCat::Age_65_;
	else 
		ageCat = -1;
}

void ViolenceAgent::setNewOrigin()
{
	if(origin != ACS::Origin::Hisp && origin != ACS::Origin::WhiteNH && origin != ACS::Origin::BlackNH)
		newOrigin = Violence::Origin::OtherNH;
	else
		newOrigin = origin;
}

void ViolenceAgent::setPTSDx(MapPair *m_ptsdx, bool isPtsd)
{
	if(age >= 14)
	{
		std::string ptsd_status = (isPtsd) ? "1" : "0";
		std::string key_ptsd = std::to_string(sex)+std::to_string(ageCat)+ptsd_status;
	
		if(m_ptsdx->count(key_ptsd) > 0)
		{
			PairDD pair_ptsdx = m_ptsdx->at(key_ptsd);
			double ptsdx_ = getPTSDx(pair_ptsdx);

			for(int i = 0; i < NUM_TREATMENT; ++i)
				this->ptsdx[i] = ptsdx_;

			this->initPtsdx = ptsdx[STEPPED_CARE];
		}
		else
		{
			std::cout << "Error: Strata " << key_ptsd << " doesn't exist!" << std::endl;
			exit(EXIT_SUCCESS);
		}
	}

}

void ViolenceAgent::setPTSDstatus(bool ptsdStatus, int type)
{
switch(type)
	{
	case PRIMARY:
		this->priPTSD = ptsdStatus;
		break;
	case SECONDARY:
		this->secPTSD = ptsdStatus;
		break;
	case TERTIARY:
		this->terPTSD = ptsdStatus;
		break;
	default:
		break;
	}
}

void ViolenceAgent::setSchoolName(std::string name)
{
	this->schoolName = name;
}

void ViolenceAgent::setFriendSize(int size)
{
	if(size > 0 && age >= 14)
	{
		this->friendSize = size;
		friendList.reserve(size);
	}
	else
	{
		this->friendSize = 0;
		friendList.reserve(0);
	}
}

void ViolenceAgent::setFriend(ViolenceAgent *frnd)
{
	if((int)friendList.size() <= friendSize)
		friendList.push_back(frnd);
}

void ViolenceAgent::setDummyVariables()
{
	age1 = age2 = age3 = 0;
	if(ageCat == Violence::AgeCat::Age_14_34)
		age1 = 1;
	else if(ageCat == Violence::AgeCat::Age_35_64)
		age2 = 1;
	else if(ageCat == Violence::AgeCat::Age_65_)
		age3 = 1;

	white = black = hispanic = other = 0;
	if(newOrigin == Violence::Origin::WhiteNH)
		white = 1;
	else if(newOrigin == Violence::Origin::BlackNH)
		black = 1;
	else if(newOrigin == Violence::Origin::Hisp)
		hispanic = 1;
	else if(newOrigin == Violence::Origin::OtherNH)
		other = 1;
}

std::string ViolenceAgent::getAgentIdx() const
{
	return agentIdx;
}


double ViolenceAgent::getPTSDx(int treatment) const
{
	switch(treatment)
	{
	case STEPPED_CARE:
		return ptsdx[STEPPED_CARE];
		break;
	case USUAL_CARE:
		return ptsdx[USUAL_CARE];
		break;
	default:
		return -1;
		break;
	}
}

double ViolenceAgent::getPTSDx(PairDD pair_ptsdx) const
{
	bool ptsd_status = (priPTSD || secPTSD || terPTSD) ? true : false;
	double ptsdx_ = random->normal_dist(pair_ptsdx.first, pair_ptsdx.second);

	if(ptsdx_ >= getPtsdCutOff())
	{
		if(ptsdx_ >  MAX_PTSDX)
			ptsdx_ = MAX_PTSDX;

		if(!ptsd_status)
			ptsdx_ = getPtsdCutOff()-1;
	}
	else
	{
		if(ptsdx_ < MIN_PTSDX)
			ptsdx_ = MIN_PTSDX;

		if(ptsd_status)
		{
			while(true)
			{
				ptsdx_ = random->normal_dist(pair_ptsdx.first, pair_ptsdx.second);
				if(ptsdx_ > MAX_PTSDX)
					ptsdx_ = MAX_PTSDX;
				
				if(secPTSD)
					ptsdx_ = 0.9*ptsdx_;
				else if(terPTSD)
					ptsdx_ = 0.85*ptsdx_;

				if(ptsdx_ >= getPtsdCutOff())
					break;
			}
		}
	}
	
	return ptsdx_;
}

double ViolenceAgent::getPtsdCutOff() const
{
	return parameters->getViolenceParam()->ptsd_cutoff;
}

bool ViolenceAgent::getPTSDstatus(int type) const
{
	switch(type)
	{
	case PRIMARY:
		return priPTSD;
		break;
	case SECONDARY:
		return secPTSD;
		break;
	case TERTIARY:
		return terPTSD;
		break;
	default:
		return false;
		break;
	}
}

int ViolenceAgent::getPtsdType() const
{
	int ptsd_type = -1;

	if(priPTSD)
		ptsd_type = PRIMARY;
	else if(secPTSD)
		ptsd_type = SECONDARY;
	else if(terPTSD)
		ptsd_type = TERTIARY;

	return ptsd_type;
}

std::string ViolenceAgent::getSchoolName() const
{
	return schoolName;
}

ViolenceAgent::FriendsPtr * ViolenceAgent::getFriendList() 
{
	return &friendList;
}

int ViolenceAgent::getFriendSize() const
{
	return friendSize;
}

int ViolenceAgent::getTotalFriends() const
{
	return friendList.size();
}

int ViolenceAgent::getMaxCbtTime() const
{
	int screening_time = parameters->getViolenceParam()->screening_time;
	if(initPtsdx >= getPtsdCutOff())
		return parameters->getViolenceParam()->tot_steps;
	else
		return screening_time+parameters->getViolenceParam()->cbt_dur_non_cases;

}

int ViolenceAgent::getMaxSprTime() const
{
	return parameters->getViolenceParam()->tot_steps;
}

bool ViolenceAgent::isStudent() const
{
	if(age >=14 && age <= 18 && (education == ACS::Education::_9th_To_12th_Grade))
		return true;
	else
		return false;
}

bool ViolenceAgent::isTeacher() const
{
	if(age >= 25 && age < 65 && education >= ACS::Education::Bachelors_Degree)
		return true;
	else
		return false;
}

bool ViolenceAgent::isCompatible(const ViolenceAgent *b) const
{
	bool valid = true;
	if(this->householdID == b->householdID || this->agentIdx == b->agentIdx || b->getTotalFriends() == b->friendSize || this->isFriend(b))
		valid = false;

	return valid;
}

bool ViolenceAgent::isFriend(const ViolenceAgent *b) const
{
	bool isFrnd = false;
	for(auto ff = friendList.begin(); ff != friendList.end(); ++ff)
	{
		ViolenceAgent *ref = *ff;
		if(ref->getAgentIdx() == b->getAgentIdx())
		{
			isFrnd = true;
			break;
		}
	}

	return isFrnd;
}

bool ViolenceAgent::isPrimaryRisk(const MapBool *primaryRiskPool) const
{
	if(primaryRiskPool->count(agentIdx) > 0)
		return true;
	else
		return false;
}

bool ViolenceAgent::isSecondaryRisk(const MapBool *secondRiskPool) const
{
	if(secondRiskPool->count(agentIdx) > 0)
		return true;
	else
		return false;
}