#include "CardioModel.h"
#include "Metro.h"
#include "PersonPums.h"
#include "HouseholdPums.h"
#include "ACS.h"
#include "Random.h"
#include "Parameters.h"
#include "Counter.h"

CardioModel::CardioModel() 
{
	
}

CardioModel::~CardioModel()
{
	delete count;
}

void CardioModel::start()
{
	if(parameters != NULL)
		count = new Counter(parameters);
	else{
		std::cout << "Error: Parameters are not initialized!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	
	Metro *curMSA = &metroAreas.at("10180");
	createPopulation(curMSA);
	setRiskFactors();
	clearList();
	//curMSA->createPopulation(this);

	/*for(auto metro = metroAreas.begin(); metro != metroAreas.end(); ++metro)
	{
		createPopulation(&metro->second);
		setRiskFactors();

		if(parameters->writeToFile())
			count->output(metro->second.getGeoID());

		clearList();
	}*/
}

void CardioModel::createPopulation(Metro *metro)
{
	std::cout << "Creating Population for " << metro->getMetroName() << std::endl;
	metro->createAgents(this);
}

void CardioModel::addHousehold(const HouseholdPums *h, int)
{
}

void CardioModel::addAgent(const PersonPums *p)
{
	short int a_ageCat, a_sex, a_org, a_edu;
	std::string key_map;
	if(p->getAge() >= 35)
	{
		if(p->getOrigin() == ACS::Origin::WhiteNH || p->getOrigin() == ACS::Origin::BlackNH)
		{
			CardioAgent *agent = new CardioAgent(p, parameters);

			a_ageCat = agent->getNHANESAgeCat();
			a_sex = agent->getSex();
			a_org = agent->getNHANESOrigin();
			a_edu = agent->getNHANESEduCat();

			key_map = std::to_string(a_org)+std::to_string(a_sex)+std::to_string(a_ageCat)+std::to_string(a_edu);

			agentList.push_back(*agent);
			agentsPtrMap.insert(std::make_pair(key_map, &agentList[agentList.size()-1]));

			count->addPersonCount(a_org, a_sex);
					
			delete agent;
		}
	}
}

void CardioModel::setRiskFactors()
{
	std::cout << "Assigning NHANES Risk Factors....\n" << std::endl;

	ProbMapRf riskStrataMap = parameters->getRiskStrataProbability();
	
	double pop_size;
	double adj = 0;

	//calculates frequency of risk strata by person type(age, race, gender, and education)
	for(auto map_itr = riskStrataMap.begin();  map_itr != riskStrataMap.end(); ++map_itr)
	{
		pop_size = agentsPtrMap.count(map_itr->first);
		for(auto rf = map_itr->second.begin(); rf != map_itr->second.end(); ++rf)
			rf->first = pop_size*rf->first;

		rounding(map_itr->second, adj);
	}
	
	//assigns risk strata to agents
	int risk_count, risk_type;
	std::vector<PairDD> risk_pair;
	
	int counter = 0;
	Random random;
	for(auto map_itr = riskStrataMap.begin(); map_itr != riskStrataMap.end(); ++map_itr)
	{
		std::cout << "Risk factor assignment for person type: " << map_itr->first << std::endl;

		auto pop_range = agentsPtrMap.equal_range(map_itr->first);
		risk_pair = map_itr->second;

		std::vector<int> pop_risk_strata(map_itr->second.size());
		for(auto agent = pop_range.first; agent != pop_range.second; ++agent)
		{
			counter++;
			boost::range::random_shuffle(map_itr->second);
			for(auto riskIdx = map_itr->second.begin(); riskIdx != map_itr->second.end();)
			{
				if(riskIdx->first == 0)
				{
					riskIdx = map_itr->second.erase(riskIdx);
				}
				else
				{
					risk_count = (int)riskIdx->first;
					risk_type = (int)riskIdx->second;

					agent->second->setRiskFactors(random, risk_type);

					count->sumRiskFactors(agent->second);
					count->addRiskFactorCount(std::to_string(risk_type)+agent->first);

					risk_count--;
					riskIdx->first = risk_count;

					if(agent->second->getRiskStrata() > 0)
						pop_risk_strata[risk_type-1]++;
				
					break;
				}
			}
		}

		for(size_t i = 0; i < risk_pair.size(); ++i)
		{
			if(risk_pair[i].first != pop_risk_strata[i])
			{
				std::cout << "Error: Population totals doesn't match for strata: " << i+1 << " ! " << std::endl;
				exit(EXIT_SUCCESS);
			}
		}
	}
	
	std::cout << "Assignment Complete! " << std::endl;
	setFraminghamRiskScore();
	
}

void CardioModel::setFraminghamRiskScore()
{
	MapDbl mean_age, mean_tchols, mean_hdl, mean_bp, per_smoking;
	EET::Framingham male(getBeta(NHANES::Sex::Male)), female(getBeta(NHANES::Sex::Female));

	for(auto org : NHANES::Org::_values())
	{
		std::cout << std::endl;
		for(auto sex : NHANES::Sex::_values())
		{
			std::cout << std::endl; 
			std::string agentType = std::to_string(org)+std::to_string(sex);
			
			mean_age.insert(std::make_pair(agentType, count->getMeanAge(agentType)));
			mean_tchols.insert(std::make_pair(agentType, count->getMeanTchols(agentType)));
			mean_hdl.insert(std::make_pair(agentType, count->getMeanHChols(agentType)));
			mean_bp.insert(std::make_pair(agentType, count->getMeanBP(agentType)));
			per_smoking.insert(std::make_pair(agentType, count->getPercentSmoking(agentType)));
			
			std::cout << "Person Type: " << agentType << std::endl;
			
			std::cout << "Mean age: " << mean_age[agentType] << std::endl; 
			std::cout << "Mean Tchols : " << mean_tchols[agentType] << std::endl;
			std::cout << "Mean HDLchols : " << mean_hdl[agentType] << std::endl;
			std::cout << "Mean SysBP : " << mean_bp[agentType] << std::endl;
			std::cout << "% Smoking: " << per_smoking[agentType] << std::endl;

		}

		std::cout << std::endl;
	}

}

void CardioModel::rounding(std::vector<PairDD> &popCount, double & adj)
{
	//double adj = 0;
	for(auto it = popCount.begin(); it != popCount.end(); ++it)
	{
		double diff = adj+(it->first-floor(it->first));
		 if(diff >= 0.5){
			 adj = diff-1;
			 it->first = ceil(it->first);
		 }
		 else{
			 adj = diff;
			 it->first = floor(it->first);
		 }
	}
}

Counter * CardioModel::getCounter() const
{
	return count;
}

EET::Framingham CardioModel::getBeta(int gender)
{
	switch(gender)
	{
	case NHANES::Sex::Male:
		return parameters->getCardioParam()->male_coeff;
		break;
	case NHANES::Sex::Female:
		return parameters->getCardioParam()->female_coeff;
		break;
	default:
		break;
	}
}

void CardioModel::setSize(int pop)
{
	agentList.reserve(pop);
}

void CardioModel::clearList()
{
	agentList.clear();
	agentList.shrink_to_fit();

	agentsPtrMap.clear();
}