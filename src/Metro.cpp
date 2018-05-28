#include "Metro.h"
#include "County.h"
#include "Parameters.h"
#include "csv.h"
#include "ACS.h"
#include "Counter.h"
#include "IPF.h"
#include "NDArray.h"

#define PUMS_NUM_HEADERS 8

Metro::Metro(std::shared_ptr<Parameters>param) : parameters(param)
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

void Metro::setEstimates(const Columns &estimates, int type)
{
	switch(type)
	{
	case ACS::Estimates::estRace:
		m_metroRaceEst.insert(std::make_pair(geoID, estimates));
		break;					  
	case ACS::Estimates::estEducation:
		m_metroEduEst.insert(std::make_pair(geoID, estimates));
		break;
	case ACS::Estimates::estMarital:
		m_metroMaritalEst.insert(std::make_pair(geoID, estimates));
		break;
	case ACS::Estimates::estHHType:
		m_metroHHTypeEst.insert(std::make_pair(geoID, estimates));
	default:
		break;
	}
}

void Metro::createAgents()
{
	importHouseholdPUMS();
	importPersonPUMS();
	runIPF();
	//assignAgentAttributes();
}

std::string Metro::getGeoID() const
{
	return geoID;
}

std::string Metro::getMetroName() const
{
	return metroName;
}

void Metro::importHouseholdPUMS()
{
	std::cout << "Importing Household PUMS file..." << std::endl;

	io::CSVReader<5>householdPumsFile(parameters->getHouseholdPumsFile());
	householdPumsFile.read_header(io::ignore_extra_column, "SERIALNO", "PUMA", "NP", "HHT", "HINCP");

	std::string hhIdx = ""; 
	std::string puma = "";
	std::string hhSize = ""; 
	std::string hhType = "";
	std::string hhIncome = "";

	std::map<std::string, int> m_hhType, m_hhIncome;
	if(parameters->getACSCodeBook().count(ACS::PumsVar::HHT) > 0)
		m_hhType = parameters->getACSCodeBook().find(ACS::PumsVar::HHT)->second;

	if(parameters->getACSCodeBook().count(ACS::PumsVar::HINCP) > 0)
		m_hhIncome = parameters->getACSCodeBook().find(ACS::PumsVar::HINCP)->second;

	HouseholdPums *hhPums = new HouseholdPums(m_hhType, m_hhIncome);

	while(householdPumsFile.read_row(hhIdx, puma, hhSize, hhType, hhIncome))
	{
		int f_puma = std::stoi(puma);
		int puma_count = m_pumaCounty.count(f_puma);

		if(puma_count > 0)
		{
			hhPums->setPUMA(puma);
			hhPums->setHouseholds(hhIdx, hhType, hhSize, hhIncome);

			if(hhPums->getHouseholdSize() > 0 && hhPums->getHouseholdType() > 0)
				m_householdPUMS.insert(std::make_pair(hhPums->getHouseholdIndex(), *hhPums));	
		}
	}
	delete hhPums;
	std::cout << "Import Successful!\n" << std::endl;
}

void Metro::importPersonPUMS()
{
	std::cout << "Importing Person PUMS file..." << std::endl;

	io::CSVReader<PUMS_NUM_HEADERS>personPumsFile(parameters->getPersonPumsFile());
	personPumsFile.read_header(io::ignore_extra_column, "SERIALNO", "AGEP", "SEX", "HISP", "RAC1P", "SCHL", "MAR", "PUMA");

	std::string idx = ""; 
	std::string age = "", sex = "", hisp = "", race = "";
	std::string edu = "";
	std::string marital_status = "";
	std::string puma = "";

	PersonPums *pumsAgent = new PersonPums(parameters->getACSCodeBook());
	while(personPumsFile.read_row(idx, age, sex, hisp, race, edu, marital_status, puma))
	{
		int f_puma = std::stoi(puma);
		int puma_count = m_pumaCounty.count(f_puma);

		if(puma_count > 0)
		{
			pumsAgent->setDemoCharacters(puma, idx, age, sex, hisp, race);
			pumsAgent->setSocialCharacters(edu, marital_status);

			auto hhRange = m_householdPUMS.equal_range(pumsAgent->getPUMSID());
			for(auto it = hhRange.first; it != hhRange.second; ++it)
				it->second.addPersons(pumsAgent);
		}
	}
	delete pumsAgent;
	
	std::cout << "Import Successful!\n" << std::endl;
}

void Metro::runIPF()
{
	std::cout << "Starting IPF...\n" << std::endl;

	for(auto hh = m_householdPUMS.begin(); hh != m_householdPUMS.end(); ++hh)
	{
		std::vector<PersonPums>personList(hh->second.getPersons());
		for(auto pp = personList.begin(); pp != personList.end(); ++pp)
			pumsList.push_back(*pp);
	}

	std::map<int, Columns> m_raceEstimates(getRaceEstimatesMap());

	//computing probabilities for educational attainment by race/origin, age and gender
	pEduByRace = computeProportions(m_raceEstimates, ACS::Sex::_size(), ACS::EduAgeCat::_size(), ACS::Origin::_size(), ACS::Education::_size(), ACS::Estimates::estEducation);
	//computing probabilties for martial status by race/origin, age and gender
	pMaritalByRace = computeProportions(m_raceEstimates, ACS::Sex::_size(), ACS::MaritalAgeCat::_size(), ACS::Origin::_size(), ACS::Marital::_size(), ACS::Estimates::estMarital);
	
	pumsList.clear();

	std::cout << "IPF completed!\n" << std::endl;
}

void Metro::assignAgentAttributes()
{
	std::cout << "Creating agents..." << std::endl;

	std::map<int, Columns> m_raceEstimates(getRaceEstimatesMap());
	int agentID = 0;

	for(auto orgByRace : ACS::Origin::_values())
	{
		Columns ageGenderEst = m_raceEstimates.at(orgByRace);
		for(auto sex : ACS::Sex::_values())
		{
			for(auto ageCat : ACS::AgeCat::_values())
			{
				size_t idx = ((ACS::AgeCat::_size()*(sex-1)) + (ageCat-1));
				int popSize = std::stoi(ageGenderEst.at(idx));

				if(popSize == 0)
					continue;

				for(int i = 0; i < popSize; i++)
				{
					Agent *myAgent = new Agent;

					myAgent->setID(++agentID);
					myAgent->setDemoAttributes(ageCat, sex, orgByRace);
					myAgent->setEducation(pEduByRace);
					myAgent->setMaritalStatus(pMaritalByRace);

					agentList.push_back(*myAgent);
					delete myAgent;
				}
			}
		}
	}

	std::cout << "Agents successfully created!\n" << std::endl;

	//educationCounts();
	//maritalStatusCounts();
}

std::map<int, Metro::Columns> Metro::getRaceEstimatesMap()
{
	//Step1: Extract estimates of race by sex and age for Metro Area based on its GEO ID

	std::map<int, Columns> m_raceEst;
	std::unordered_map<std::string, int> all_origins(parameters->getOriginMapping());

	auto msa_range = m_metroRaceEst.equal_range(geoID);
	for(auto row = msa_range.first; row != msa_range.second; ++row)
	{
		Columns raceEst;
		Columns cols = row->second;
		std::string origin = cols.front();

		for(size_t i = 1; i < cols.size(); i++)
			raceEst.push_back(cols.at(i));

		m_raceEst.insert(std::make_pair(all_origins.at(origin), raceEst));
		all_origins.erase(origin);
	}

	//Step 2: Initialize estimates of remaining race by sex and age to 0
	for(auto org = all_origins.begin(); org != all_origins.end(); ++org)
	{
		Columns est;
		for(size_t i = 0; i < ACS::RaceMarginalVar::_size()-1; i++)
			est.push_back("0");

		m_raceEst.insert(std::make_pair(org->second, est));
	}

	return m_raceEst;
}

//Note: Arguments definition in computeProportions(.....)
//		1. size_var1 = ACS::Sex::_size(), var1 = sex
//		2. size_var2 = ACS::EduAgeCat::_size() / ACS::MaritalAgeCat::_size(), var = eduAge/maritalAge
//		3. size_var3 = ACS::Origin::_size(), var3 = origin
//		4. size_var4 = ACS::Education::_size() / ACS::Marital::_size(), var4 = education, marital
//		5. type = ACS::Estimates::Education / ACS::Estimates::Marital
Metro::ProbabilityMap Metro::computeProportions(const std::map<int, Columns>&m_raceEstimates, size_t size_var1, size_t size_var2, size_t size_var3, size_t size_var4, int type)
{
	std::vector<Marginal> tempMarginal(getEstimatesVector(m_raceEstimates, size_var1, size_var2, type));

	Marginal mar_var3 = tempMarginal[0];
	Marginal mar_var4 = tempMarginal[1];

	std::map<int, Marginal> m_estByRace;
	ProbabilityMap pr;

	for(size_t var1 = 1; var1 <= size_var1; ++var1)
	{
		for(size_t var2 = 1; var2 <= size_var2; ++var2)
		{
			std::cout << "Running IPF for Sex: " << toString(var1, ACS::Estimates::estGender) << " and Age Category: " << toString(var2, type) << "..\n" << std::endl;
			m_estByRace = getIPFestimates(var1, var2, mar_var3, mar_var4, size_var3, size_var4, type); 
			convertToProportions(m_estByRace);
			pr.insert(std::make_pair(10*var1+var2, m_estByRace));
		}
	}

	return pr;	
}


std::vector<Metro::Marginal> Metro::getEstimatesVector(const std::map<int, Columns>&m_raceEstimates, size_t size_var1, size_t size_var2, int type)
{
	std::vector<Marginal> tempMarginal;
	std::unordered_multimap<int, int>m_ageGender(parameters->getAgeGenderMapping(type));
	if(m_ageGender.size() == 0)
		throw::std::runtime_error("Invalid data type!");

	Marginal m_race = getRaceEstimateVector(m_raceEstimates, m_ageGender, size_var1, size_var2);
	tempMarginal.push_back(m_race);

	switch(type)
	{
	case ACS::Estimates::estEducation:
		{
			Marginal m_education = getEducationEstimateVector();
			tempMarginal.push_back(m_education);
			break;
		}
	case ACS::Estimates::estMarital:
		{
			Marginal m_marital = getMaritalStatusEstimateVector();
			tempMarginal.push_back(m_marital);
			break;
		}
	default:
		break;
	}

	if(tempMarginal.size() < 2)
		throw::std::runtime_error("Invalid number of marginals!");

	return tempMarginal;
}

Metro::Marginal Metro::getRaceEstimateVector(const std::map<int, Columns>&m_raceEstimates, const std::unordered_multimap<int, int>&m_ageGender, size_t totSexType, size_t totAgeType)
{
	if(!isValidGeoID("Race"))
		exit(EXIT_SUCCESS);

	Marginal raceMarginals;
	for(size_t sex = 1; sex <= totSexType; ++sex)
	{
		for(size_t age = 1; age <= totAgeType; ++age)
		{
			int key = 10*sex+age; 
			for(auto org = m_raceEstimates.begin(); org != m_raceEstimates.end(); ++org)
			{
				double sum = 0;
				for(auto idx = m_ageGender.lower_bound(key); idx != m_ageGender.upper_bound(key); ++idx)
					sum += std::stod(org->second.at(idx->second));
				
				raceMarginals.push_back(sum);
			}
		}
	}

	return raceMarginals;
}


Metro::Marginal Metro::getEducationEstimateVector() 
{
	if(!isValidGeoID("Education"))
		exit(EXIT_SUCCESS);

	Marginal eduMarginal;
	
	auto msa_range = m_metroEduEst.equal_range(geoID);
	for(auto row = msa_range.first; row != msa_range.second; ++row)
	{
		Columns col = row->second;
		for(size_t i = 0; i < col.size(); i++)
			eduMarginal.push_back(std::stod(col.at(i)));
	}

	return eduMarginal;
}

Metro::Marginal Metro::getMaritalStatusEstimateVector()
{
	if(!isValidGeoID("Marital Status"))
		exit(EXIT_SUCCESS);

	Marginal marMarginal;
	auto msa = m_metroMaritalEst.equal_range(geoID);
	for(auto row = msa.first; row != msa.second; ++row)
	{
		Columns col = row->second;
		for(size_t i = 0; i < col.size();)
		{
			int popAgeBySex = std::stoi(col.at(i));
			for(size_t j = 0; j < ACS::Marital::_size(); ++j)
			{
				double pMarital = std::stod(col.at(i+j+1))/100;
				marMarginal.push_back(pMarital*popAgeBySex);
			}
			i += (ACS::Marital::_size()+1);
		}

	}
	
	return marMarginal;
}

//Note: Arguments definition in getIPFEstimates(.....)
//		1. size_var1 = ACS::Sex::_size(), var1 = sex
//		2. size_var2 = ACS::EduAgeCat::_size() / ACS::MaritalAgeCat::_size(), var = eduAge/maritalAge
//		3. size_var3 = ACS::Origin::_size(), var3 = origin
//		4. size_var4 = ACS::Education::_size() / ACS::Marital::_size(), var4 = education, marital
//		5. type = ACS::Estimates::Education / ACS::Estimates::Marital
std::map<int, Metro::Marginal> Metro::getIPFestimates(int var1, int var2, Marginal &mar_var3, Marginal &mar_var4, size_t size_var3, size_t size_var4, int type)
{
	createSeedMatrix(var1, var2, size_var3, size_var4, type);

	setMarginals(mar_var3, size_var3);
	setMarginals(mar_var4, size_var4);
	
	adjustMarginals(marginals[0], marginals[1]);

	m_size.push_back(marginals[0].size());
	m_size.push_back(marginals[1].size());

	NDArray<double>seed1D(m_size);
	seed1D.assign(seed);
	deprecated::IPF ipf(seed1D, marginals);
	
	//Marginal est(ipf.solve(seed1D));
	std::map<int, Marginal> m_estimates(ipf.solve(seed1D));

	clear();

	return m_estimates;
}

void Metro::convertToProportions(std::map<int, Marginal> &m_est)
{
	for(auto row = m_est.begin(); row != m_est.end(); ++row)
	{
		double sum = std::accumulate(row->second.begin(), row->second.end(), 0.0);
		for(size_t i = 0; i < row->second.size(); i++)
			row->second.at(i) = (sum != 0) ? row->second.at(i)/sum : 0.0;	

		std::partial_sum(row->second.begin(), row->second.end(), row->second.begin());
	}
}

//Note: Arguments definition in createSeedMatrix(.....)
//		1. size_var1 = ACS::Sex::_size(), var1 = sex
//		2. size_var2 = ACS::EduAgeCat::_size() / ACS::MaritalAgeCat::_size(), var = eduAge/maritalAge
//		3. size_var3 = ACS::Origin::_size(), var3 = origin
//		4. size_var4 = ACS::Education::_size() / ACS::Marital::_size(), var4 = education, marital
//		5. type = ACS::Estimates::Education / ACS::Estimates::Marital
void Metro::createSeedMatrix(int var1, int var2, size_t size_var3, size_t size_var4, int type)
{
	for(size_t var3 = 1; var3 <= size_var3; ++var3)
	{
		for(size_t var4 = 1; var4 <= size_var4; ++var4)
		{
			double frequency = 0;
			for(auto cnty = m_pumaCounty.begin(); cnty != m_pumaCounty.end(); ++cnty)
			{
				int pumaCode = cnty->first;
				int count = getCount(var1, var2, var3, var4, pumaCode, type); 
				frequency += cnty->second.getPopulationWeight() * count;
			}
			if(frequency == 0)
			{
				seed.push_back(0.001);
			}
			else
				seed.push_back(frequency);
		}
	}
}

void Metro::setMarginals(Marginal &mar, int size)
{
	int count = 0;
	std::vector<double> temp_marginal;
	for(auto it1 = mar.begin(); it1 != mar.begin()+size; ++it1)
	{
		temp_marginal.push_back(*it1);
		count++;
	}
	
	marginals.push_back(temp_marginal);
	mar.erase(mar.begin(), mar.begin()+count);
}

void Metro::adjustMarginals(Marginal &mar1, Marginal &mar2)
{
	double pop_mar1 = std::accumulate(mar1.begin(), mar1.end(), 0.0);
	double pop_mar2 = std::accumulate(mar2.begin(), mar2.end(), 0.0);

	if(pop_mar1 == pop_mar2)
		return;

	if(pop_mar1 > pop_mar2)
	{
		double sum2 = 0;
		for(size_t i = 0; i < mar2.size(); ++i)
		{
			mar2[i] = mar2[i]*pop_mar1/pop_mar2;
			sum2 += mar2[i]; 
		}

	}
	else
	{
		double sum1 = 0;
		for(size_t j = 0; j < mar1.size(); ++j)
		{
			mar1[j] = mar1[j]*pop_mar2/pop_mar1;
			sum1 += mar1[j];
		}
	}
}

void Metro:: clear()
{
	seed.clear();
	marginals.clear();
	m_size.clear();
}

std::multimap<int, County> Metro::getPumaCountyMap() const
{
	return m_pumaCounty;
}

int Metro::getCountyNum(std::string countyName) const
{
	//return m_countyPuma.count(county);
	int count = 0;

	for(auto it1 = m_pumaCounty.begin(); it1 != m_pumaCounty.end(); ++it1){
		if(it1->second.getCountyName() == countyName)
			count++;
	}
	
	return count;
}

bool Metro::isValidGeoID(std::string str)
{
	bool flag = true;
	if(m_metroEduEst.count(geoID) == 0)
	{
		std::cout << "GEO ID: "<< geoID << " doesn't exist in the " << str << " estimates list! " << std::endl;
		std::cout << "Please re-check!" << std::endl;
		
		flag = false;
	}

	return flag;
}

int Metro::getCount(int var1, int var2, int var3, int var4, int pumaCode, int type)
{
	int count = 0;
	switch(type)
	{
	case ACS::Estimates::estEducation:
		count = std::count_if(pumsList.begin(), pumsList.end(), Counter::EduByOriginByPUMA(var1, var2, var3, var4, pumaCode));
		break;
	case ACS::Estimates::estMarital:
		count = std::count_if(pumsList.begin(), pumsList.end(), Counter::MaritalStatusByPUMA(var1, var2, var3, var4, pumaCode));
		break;
	default:
		break;
	}

	return count;
}

void Metro::educationCounts()
{
	std::ofstream file;
	const char* fileName = "education.csv";
	file.open(parameters->getOutputDir()+fileName);

	if(!file.is_open()){
		std::cout << "Error: Cannot create " << fileName << std::endl;
		exit(EXIT_SUCCESS);
	}
	
	for(auto sex : ACS::Sex::_values())
	{
		for(auto eduAge : ACS::EduAgeCat::_values())
		{
			for(auto edu : ACS::Education::_values())
			{
				file << sex._to_string() << "," << eduAge._to_string() << "," << edu._to_string() << ",";
				for(auto origin : ACS::Origin::_values())
				{
					int count = std::count_if(agentList.begin(), agentList.end(), Counter::EduByOrigin(sex, origin, edu, eduAge));
					file << count << ","; 
				}
				file << std::endl;
			}
		}
	}

	file.close();
}

void Metro::maritalStatusCounts()
{
	std::ofstream file;
	const char* fileName = "marital_status.csv";
	file.open(parameters->getOutputDir()+fileName);

	if(!file.is_open()){
		std::cout << "Error: Cannot create " << fileName << std::endl;
		exit(EXIT_SUCCESS);
	}
	
	for(auto sex : ACS::Sex::_values())
	{
		for(auto marAge : ACS::MaritalAgeCat::_values())
		{
			for(auto mar : ACS::Marital::_values())
			{
				file << sex._to_string() << "," << marAge._to_string() << "," << mar._to_string() << ",";
				for(auto origin : ACS::Origin::_values())
				{
					int count = std::count_if(agentList.begin(), agentList.end(), Counter::MaritalStatusByOrigin(sex, origin, mar, marAge));
					file << count << ","; 
				}
				file << std::endl;
			}
		}
	}

	file.close();
}

std::string Metro::toString(int val, int type)
{
	std::string str = "";
	switch(type)
	{
	case ACS::Estimates::estGender:
		str = ACS::Sex::_from_integral(val)._to_string();
		break;
	case ACS::Estimates::estEducation:
		str = ACS::EduAgeCat::_from_integral(val)._to_string();
		break;
	case ACS::Estimates::estMarital:
		str = ACS::MaritalAgeCat::_from_integral(val)._to_string();
	default:
		break;
	}
	return str;
}

