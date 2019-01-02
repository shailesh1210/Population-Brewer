/**
*	@file	 Metro.cpp	
*	@author	 Shailesh Tamrakar
*	@date	 7/24/2018
*	@version 1.0
*
*	@section DESCRIPTION
*	This source file contains variables and methods that makes up Metro class. It 
*	contains all the necessary information on particular MSA which includes its name,
*	geoID, population count, list of counties and PUMA codes within MSA and list of 
*	sythetically generated population with IPU.
*/

#include "Metro.h"
#include "County.h"
#include "Counter.h"
#include "Parameters.h"
#include "csv.h"
#include "IPUWrapper.h"
#include "ElapsedTime.h"
#include "Random.h"
#include "CardioModel.h"
#include "ViolenceModel.h"


template void Metro::createAgents<CardioModel>(CardioModel *);
template void Metro::createAgents<ViolenceModel>(ViolenceModel *);

Metro::Metro()
{
}

Metro::Metro(std::shared_ptr<Parameters>param) : parameters(param), ipuWrapper(NULL)
{
	
}

Metro::~Metro()
{
	
}

void Metro::setMetroIDandName(const std::string &ID, const std::string &name)
{
	this->geoID = ID;
	this->metroName = name;
}

void Metro::setPopulation(const int &pop)
{
	this->population = pop;
}

void Metro::setCounties(const County &cnty)
{
	m_pumaCounty.insert(std::make_pair(cnty.getPumaCode(), cnty));
}

/**
*	@brief Sets pop/household estimates to MSA by variable type
*	@param estimates is a vector of strings containing pop/household estimates
*	@param type is estimates type (Education, Race...) of population/household
*	@return void
*/
void Metro::setEstimates(const Columns &estimates, int type)
{
	std::string typeName = "";
	for(auto val : ACS::Estimates::_values())
	{
		if(type == val._to_integral())
		{
			std::multimap<std::string, Columns> temp_est;
			temp_est.insert(std::make_pair(geoID, estimates));
			m_metroACSEst.insert(std::make_pair(type, temp_est));

			typeName = val._to_string();

			break;
		}
	}

	if(m_metroACSEst.size() == 0){
		std::cout << "Error: Person/Household type: " << typeName << " doesn't exist!" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

std::string Metro::getGeoID() const
{
	return geoID;
}

std::string Metro::getMetroName() const
{
	return metroName;
}

int Metro::getPopulation() const
{
	return population;
}

std::multimap<int, County> Metro::getPumaCountyMap() const
{
	return m_pumaCounty;
}

int Metro::getCountyNum(std::string countyName) const
{
	int count = 0;
	for(auto it1 = m_pumaCounty.begin(); it1 != m_pumaCounty.end(); ++it1)
	{
		if(it1->second.getCountyName() == countyName)
			count++;
	}
	return count;
}

template <class T>
void Metro::createAgents(T *model)
{
	if(ipuWrapper == NULL)
	{
		bool run = true;
		IPUWrapper *ipuWrap = new IPUWrapper(parameters, &m_metroACSEst, &m_pumaCounty);
		ipuWrap->startIPU(geoID, population, run);
	
		ipuWrapper = ipuWrap;
		if(!ipuWrap->successIPU())
		{
			std::cout << "IPU unsuccessful! Cannot Create Households!" << std::endl;
			exit(EXIT_SUCCESS);
		}
	}
	
	drawHouseholds(ipuWrapper, model);
}

template <class T>
void Metro::drawHouseholds(IPUWrapper *ipuWrap, T *model)
{
	std::cout << "Creating Households...\n" << std::endl;

	double waitTime = 4000; //4 seconds wait time
	ElapsedTime timer;

	const ProbMap *prHouseholds = ipuWrap->getHouseholdProbability();
	const PUMSHouseholdsMap* m_householdsPums = ipuWrap->getHouseholds();
	const Marginal *ipuCons = ipuWrap->getConstraints();

	double num_households, randomP, hhProb, hhIdx;
	std::string hhType;

	bool fit_pop = false;
	int num_draws = 0;
	
	std::vector<PersonPums> tempPersons;

	std::string sex, ageCat, origin, eduAgeCat, edu;
	std::string personType1, personType2;
	std::string dummy = "0";

	int pop_mem_size = (int)(1.02*population);

	Random random;

	while(!fit_pop)
	{
		int countHH = 0; int countPer = 0; 
		
		model->setSize(pop_mem_size);
		model->getCounter()->initialize();
	
		std::cout << "Drawing households - Attempt: " << ++num_draws << std::endl;

		for(auto hh = prHouseholds->begin(); hh != prHouseholds->end(); ++hh)
		{
			hhType = hh->first;
			num_households = ipuWrap->getHouseholdCount(hhType);
			while(num_households != 0)
			{
				randomP = random.uniform_real_dist();
				for(auto hash = hh->second.begin(); hash != hh->second.end(); ++hash)
				{
					PairDD last_elem = hash->second.back();
					double maxProb = last_elem.first;

					if(randomP < maxProb)
					{
						for(auto pr = hash->second.begin(); pr != hash->second.end(); ++pr)
						{
							hhProb = pr->first;
							hhIdx = pr->second;
						
							if(randomP < hhProb)
							{
								countHH++;
								model->getCounter()->addHouseholdCount(hhType);

								const HouseholdPums *hh = &m_householdsPums->at(hhIdx);

								if(parameters->getSimType() == MASS_VIOLENCE)
									model->addHousehold(hh, countHH);

								tempPersons = hh->getPersons();
								for(auto pp = tempPersons.begin(); pp != tempPersons.end(); ++pp)
								{
									sex = std::to_string(pp->getSex());
									origin = std::to_string(pp->getOrigin());
									ageCat = std::to_string(pp->getAgeCat());

									personType1 = dummy+sex+ageCat+origin;
									model->getCounter()->addPersonCount(personType1);
			
									if(pp->getAge() >= 18) 
									{
										eduAgeCat = std::to_string(pp->getEduAgeCat());
										edu = std::to_string(pp->getEducation());

										personType2 = sex+eduAgeCat+origin+edu;
										model->getCounter()->addPersonCount(personType2);
									}

									countPer++;
									if(parameters->getSimType() == EQUITY_EFFICIENCY)
										model->addAgent(&(*pp));
								}

								timer.stop();
								if(timer.elapsed_ms() > waitTime)
								{
									std::cout << "Households Count:" << countHH << " Person Count: " <<  countPer << std::endl;
									timer.start();
								}

								num_households--;
								break;
							}
						}
						break;
					}
				}
			}
		}

		std::cout << "Households Count:" << countHH << " Person Count: " <<  countPer << std::endl;

		fit_pop = checkFit(ipuCons, model->getCounter(), num_draws);
		if(!fit_pop)
			model->clearList();
	}

	std::cout << "Households successfully created!\n" << std::endl;
	//agentList.shrink_to_fit();
	//ipuWrap->clearHHPums();
}

bool Metro::checkFit(const Marginal *cons, const Counter *count, int num_draws)
{
	std::vector<double> obsFreq, estFreq;
	bool fit = false;

	Pool temp_person_pool = *(parameters->getPersonPool());

	int male_child_start = ACS::ChildAgeCat::_size()*ACS::Origin::_size();
	int male_child_end = ACS::AgeCat::_size()*ACS::Origin::_size();

	//erase male adults (over 18) from indvidual pool
	temp_person_pool.erase(temp_person_pool.begin()+male_child_start, temp_person_pool.begin()+male_child_end); 

	int female_child_start = 2*male_child_start;
	int female_child_end = 2*male_child_start+(male_child_end-male_child_start);
	//erase female adults from individual pool
	temp_person_pool.erase(temp_person_pool.begin()+female_child_start, temp_person_pool.begin()+female_child_end);

	for(size_t i = 0; i < temp_person_pool.size(); ++i)
		obsFreq.push_back(count->getPersonCount(temp_person_pool[i]));

	int num_hh_type = (ACS::HHType::_size()*ACS::HHSize::_size()*ACS::HHIncome::_size())+1;
	for(auto cts = cons->begin()+num_hh_type; cts != cons->end(); ++cts)
		estFreq.push_back(*cts);

	
	if(obsFreq.size() != estFreq.size())
	{
		std::cout << "Error: Size of Observed and Estimated Freq. doesn't match!" << std::endl;
		exit(EXIT_SUCCESS);
	}


	std::cout << std::endl;
	std::cout << "Starting chi-square test..." << std::endl;

	int df = 0;
	double sum_chi_sqr = 0;
	int size = estFreq.size();
	double diff = 0;
	//double min_sample_size = 1000.0;

	for(int i = 0; i < size; ++i)
	{
		if(estFreq[i] >= parameters->getMinSampleSize())
		{
			diff = obsFreq[i]-estFreq[i];
			sum_chi_sqr += ((diff*diff)/estFreq[i]);
			df++;
		}
	}

	boost::math::chi_squared dist(df-1);
	double p_val = 1-(boost::math::cdf(dist, sum_chi_sqr));

	std::cout << std::setprecision(6) << "df: " << df-1 << "|" << "chi-val: " << sum_chi_sqr << "|" << "p-val: " << p_val << std::endl << std::endl;
	
	//bool fit = (p_val < alpha) ? false : true;

	if(p_val > parameters->getAlpha())
		fit = true;
	else if(num_draws == parameters->getMaxDraws())
		fit = true;

	if(fit)
		gofLog(p_val, df, num_draws);
	
	return fit;
}

void Metro::gofLog(double pval, int df, int num_draws)
{
	std::ofstream logFile;
	logFile.open("gofLog.txt", std::ios::app);

	if(!logFile.is_open())
		exit(EXIT_SUCCESS);

	logFile << geoID << ", pvalue: " << pval << ", df: " << df << ", num_draws: " << num_draws << std::endl;

	logFile.close();

}

//void Metro::normalDistCurve()
//{
//	const Pool *nhanesPool = parameters->getNhanesPool();
//	const PairMap *cholsLevelMap = parameters->getRiskFactorMap(NHANES::RiskFac::totalChols);
//	const PairMap *ldlLevelMap = parameters->getRiskFactorMap(NHANES::RiskFac::LdlChols);
//	const PairMap *sysBpMap = parameters->getRiskFactorMap(NHANES::RiskFac::SystolicBp);
//
//	PairDD tchols_pair, ldlChols_pair, sysBp_pair;
//
//	std::ofstream file;
//	std::string fileName = "chols_level/tchols.csv";
//
//	file.open(parameters->getOutputDir()+fileName);
//	if(!file.is_open())
//		exit(EXIT_SUCCESS);
//
//	for(size_t i = 0; i < nhanesPool->size(); ++i)
//	{
//		std::string agent_type = nhanesPool->at(i);
//		auto agent_range = agentsPtrMap.equal_range(agent_type);
//
//		for(auto agent = agent_range.first; agent != agent_range.second; ++agent)
//		{
//			std::string risk_strata = std::to_string(agent->second->getRiskStrata());
//			tchols_pair = cholsLevelMap->at(risk_strata+agent_type);
//			ldlChols_pair = ldlLevelMap->at(risk_strata+agent_type);
//			sysBp_pair = sysBpMap->at(risk_strata+agent_type);
//
//			file << agent_type << "," << risk_strata << "," << agent->second->getRiskChart().tchols << "," << tchols_pair.first << "," << tchols_pair.second << ","
//				<< agent->second->getRiskChart().ldlChols << "," << ldlChols_pair.first << "," << ldlChols_pair.second << ","
//				<< agent->second->getRiskChart().systolicBp << "," << sysBp_pair.first << "," << sysBp_pair.second << std::endl;
//		}
//	}
//
//	file.close();
//}





