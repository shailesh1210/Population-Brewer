#include "CardioAgent.h"
#include "ACS.h"
#include "PersonPums.h"
#include "Random.h"
#include "Parameters.h"

CardioAgent::CardioAgent()
{
}

CardioAgent::CardioAgent(const PersonPums *person, std::shared_ptr<Parameters> param) : parameters(param)
{
	this->householdID = person->getPUMSID();
	this->puma = person->getPumaCode();

	this->age = person->getAge();
	this->sex = person->getSex();

	this->origin = person->getOrigin();
	this->nhanes_org = (origin == ACS::Origin::WhiteNH) ? NHANES::Org::WhiteNH : NHANES::Org::BlackNH;;

	this->education = person->getEducation();
	this->nhanes_edu = (education <= ACS::Education::High_School) ? NHANES::Edu::HS_or_less : NHANES::Edu::some_coll_;

	this->rfStrata = -1;

	setNHANESAgeCat();
}

CardioAgent::~CardioAgent()
{
}

void CardioAgent::setNHANESAgeCat()
{
	if(age >= 35 && age <= 44){
		this->nhanes_ageCat = NHANES::AgeCat::Age_35_44;
	}
	else if(age >= 45 && age <= 54){
		this->nhanes_ageCat = NHANES::AgeCat::Age_45_54;
	}
	else if(age >= 55 && age <= 64){
		this->nhanes_ageCat = NHANES::AgeCat::Age_55_64;
	}
	else if(age >= 65 && age <= 74){
		this->nhanes_ageCat = NHANES::AgeCat::Age_65_74;
	}
	else if(age >= 75){
		this->nhanes_ageCat = NHANES::AgeCat::Age_75_;
	}
	else {
		this->nhanes_ageCat = -1;
	}
}


void CardioAgent::setRiskFactors(Random &random, short int rfs)
{
	this->rfStrata = rfs;

	std::string agentType = std::to_string(rfStrata)+std::to_string(nhanes_org)
		+std::to_string(sex)+std::to_string(nhanes_ageCat)+std::to_string(nhanes_edu);

	setTotalCholesterol(random, agentType);
	setLDLCholesterol(random, agentType);
	setSystolicBp(random, agentType);
	setSmokingStatus();
}

void CardioAgent::setTotalCholesterol(Random & random, std::string agentType)
{
	const PairMap *tcholsMap = parameters->getRiskFactorMap(NHANES::RiskFac::totalChols);
	if(tcholsMap->count(agentType) > 0)
	{
		PairDD tchols_pair = tcholsMap->at(agentType);
		chart.tchols = random.normal_dist(tchols_pair.first, tchols_pair.second);
	}
}

void CardioAgent::setLDLCholesterol(Random & random, std::string agentType)
{
	const PairMap *ldlCholsMap = parameters->getRiskFactorMap(NHANES::RiskFac::LdlChols);
	if(ldlCholsMap->count(agentType) > 0)
	{
		PairDD ldl_pair = ldlCholsMap->at(agentType);
		chart.ldlChols = random.normal_dist(ldl_pair.first, ldl_pair.second);
	}
}

void CardioAgent::setSystolicBp(Random & random, std::string agentType)
{
	const PairMap *sysBpMap = parameters->getRiskFactorMap(NHANES::RiskFac::SystolicBp);
	if(sysBpMap->count(agentType) > 0)
	{
		PairDD bp_pair = sysBpMap->at(agentType);
		chart.systolicBp = random.normal_dist(bp_pair.first, bp_pair.second);
	}
}

void CardioAgent::setSmokingStatus()
{
	const PairMap *smokeStatMap = parameters->getRiskFactorMap(NHANES::RiskFac::SmokingStat);
	std::string riskStrta = std::to_string(rfStrata);

	if(smokeStatMap->count(riskStrta) > 0)
	{
		PairDD smokeStat = smokeStatMap->at(riskStrta);
		chart.smokingStatus = (smokeStat.first == 1) ? true : false;
	}
}


short int CardioAgent::getNHANESAgeCat() const
{
	return nhanes_ageCat;
}

short int CardioAgent::getNHANESOrigin() const
{
	return nhanes_org;
}

short int CardioAgent::getNHANESEduCat() const
{
	return nhanes_edu;
}

short int CardioAgent::getRiskStrata() const
{
	return rfStrata;
}

RiskFactors CardioAgent::getRiskChart() const
{
	return chart;
}


