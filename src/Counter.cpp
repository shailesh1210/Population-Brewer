#include "Counter.h"
#include "Parameters.h"
#include "ACS.h"

Counter::Counter()
{
}

Counter::Counter(std::shared_ptr<Parameters> param) : parameters(param)
{
}

Counter::~Counter()
{
}

void Counter::setParameters(std::shared_ptr<Parameters> p)
{
	parameters = p;
}

int Counter::getHouseholdCount(std::string hhType) const
{
	if(m_householdCount.count(hhType) > 0)
		return m_householdCount.at(hhType).size();
	else
		return -1;
}

int Counter::getPersonCount(std::string personType) const
{
	if(m_personCount.count(personType) > 0)
		return m_personCount.at(personType).size();
	else 
		return -1;
}

void Counter::initialize()
{
	initHouseholdCounter();
	initPersonCounter();

	switch(parameters->getSimType())
	{
	case EQUITY_EFFICIENCY:
		initRiskFacCounter();
		break;
	case MASS_VIOLENCE:
		initPtsdCounter();
		break;
	default:
		break;
	}
}


void Counter::output(std::string geoID)
{
	outputHouseholdCounts(geoID);
	outputPersonCounts(geoID);

	switch(parameters->getSimType())
	{
	case EQUITY_EFFICIENCY:
		outputRiskFactorPercent();
		break;
	case MASS_VIOLENCE:
		outputHealthOutcomes();
		break;
	default:
		break;
	}
	//outputRiskFactorPercent(population);
}

void Counter::initHouseholdCounter()
{
	clearMap(m_householdCount);

	std::vector<bool> hhCounts;
	hhCounts.reserve(0);
	
	const Pool *householdsPool = parameters->getHouseholdPool();

	for(size_t hh = 0; hh < householdsPool->size(); ++hh)
		m_householdCount.insert(std::make_pair(householdsPool->at(hh), hhCounts));

}

void Counter::initPersonCounter()
{
	clearMap(m_personCount);

	std::vector<bool> personCounts;
	personCounts.reserve(0);
	
	const Pool *indivPool = parameters->getPersonPool();
	for(size_t pp = 0; pp < indivPool->size(); ++pp)
		m_personCount.insert(std::make_pair(indivPool->at(pp), personCounts));
}

void Counter::initRiskFacCounter()
{
	clearMap(m_riskFacCount);

	std::vector<bool> riskFacCounts;
	riskFacCounts.reserve(0);

	const int num_risk_strata = 16;
	const Pool *nhanesPool = parameters->getNhanesPool();
	for(size_t nh = 0; nh < nhanesPool->size(); ++nh)
		for(size_t rf = 1; rf <= num_risk_strata; ++rf)
			m_riskFacCount.insert(std::make_pair(std::to_string(rf)+nhanesPool->at(nh), riskFacCounts));			
}

void Counter::initPtsdCounter()
{
	clearCounter();
	int num_steps = parameters->getViolenceParam()->tot_steps;
	for(int i = 0; i < NUM_TREATMENT; ++i)
	{
		for(int j = 0; j < NUM_PTSD; ++j)
		{
			for(int k = 0; k < num_steps; ++k)
			{
				m_ptsdCount[i][j].insert(std::make_pair(k, 0));
				m_ptsdResolvedCount[i][j].insert(std::make_pair(k, 0));

				if(j == 0)
				{
					m_cbtReach[i].insert(std::make_pair(k, 0));
					m_sprReach[i].insert(std::make_pair(k, 0));
				}
			}	
		}
	}

	initTreatmentCounter(num_steps);
	//initOutcomeCounter(num_steps);
}

void Counter::initTreatmentCounter(int steps)
{
	for(int i = 0; i < steps; ++i)
	{
		m_cbtCount.insert(std::make_pair(i, 0));
		m_sprCount.insert(std::make_pair(i, 0));
		m_ndCount.insert(std::make_pair(i, 0));
	}
}


void Counter::addHouseholdCount(std::string hhType)
{
	if (m_householdCount.count(hhType) > 0)
		m_householdCount[hhType].push_back(true);
}

void Counter::addPersonCount(std::string personType)
{
	if(m_personCount.count(personType) > 0)
		m_personCount[personType].push_back(true);
}

void Counter::addRiskFactorCount(std::string rfType)
{
	if(m_riskFacCount.count(rfType) > 0)
		m_riskFacCount[rfType].push_back(true);
	
}

void Counter::addPtsdCount(int treatment, int ptsd_type, int tick)
{
	if(m_ptsdCount[treatment][ptsd_type].count(tick) > 0)
		m_ptsdCount[treatment][ptsd_type].at(tick) += 1;
}

void Counter::addPtsdResolvedCount(int treatment, int ptsd_type, int tick)
{
	if(m_ptsdResolvedCount[treatment][ptsd_type].count(tick) > 0)
		m_ptsdResolvedCount[treatment][ptsd_type].at(tick) += 1;
}

void Counter::addCbtReach(int treatment, int tick)
{
	if(m_cbtReach[treatment].count(tick) > 0)
		m_cbtReach[treatment].at(tick) += 1;
}

void Counter::addSprReach(int treatment, int tick)
{
	if(m_sprReach[treatment].count(tick) > 0)
		m_sprReach[treatment].at(tick) += 1;
}

void Counter::addCbtCount(int tick)
{
	if(m_cbtCount.count(tick) > 0)
		m_cbtCount[tick] += 1;
}

void Counter::addSprCount(int tick)
{
	if(m_sprCount.count(tick) > 0)
		m_sprCount[tick] += 1;
}

void Counter::addNaturalDecayCount(int tick)
{
	if(m_ndCount.count(tick) > 0)
		m_ndCount[tick] += 1;
}

void Counter::computeOutcomes(int tick, int totPop)
{
	computePrevalence(tick, totPop);
	computeRecovery(tick, totPop);
}

void Counter::computePrevalence(int tick, int totPop)
{
	Outcomes prevalence;
	double ptsd_count[NUM_TREATMENT] = {0, 0};
	double non_ptsd_count[NUM_TREATMENT];

	for(int i = 0; i < NUM_TREATMENT; ++i)
	{
		for(int j = 0; j < NUM_PTSD; ++j)
		{
			double pts_count = m_ptsdCount[i][j].at(tick);
			double prev = (double)pts_count/totPop;

			if(m_totPrev[i][j].count(tick) > 0)
				m_totPrev[i][j].at(tick) += prev;
			else
				m_totPrev[i][j].insert(std::make_pair(tick, prev));

			ptsd_count[i] += pts_count;
		}
		
		non_ptsd_count[i] = (double)totPop - ptsd_count[i];
		prevalence.value[i] = ptsd_count[i]/totPop;
	}

	Pair stdErr(computeError(ptsd_count, non_ptsd_count));

	prevalence.diff = computeDiff(prevalence.value, stdErr.first);
	prevalence.ratio = computeRatio(prevalence.value, stdErr.second);

	if(m_prevalence.count(tick) > 0)
	{
		for(int i = 0; i < NUM_TREATMENT; ++i)
			m_prevalence[tick].value[i] += prevalence.value[i];

		m_prevalence[tick].diff += prevalence.diff;
		m_prevalence[tick].ratio += prevalence.ratio;
	}
	else
	{
		m_prevalence.insert(std::make_pair(tick, prevalence));
	}
	
}

void Counter::computeRecovery(int tick, int totPop)
{
	Outcomes recovery;
	double ptsd_count[NUM_TREATMENT] = {0,0};
	double ptsd_resolved[NUM_TREATMENT] = {0,0};

	for(int i = 0; i < NUM_TREATMENT; ++i)
	{
		for(int j = 0; j < NUM_PTSD; ++j)
		{
			double pts_count = m_ptsdCount[i][j].at(tick);
			double res_count = m_ptsdResolvedCount[i][j].at(tick);
			double recov = pts_count/(pts_count+res_count);

			if(m_totRecovery[i][j].count(tick) > 0)
				m_totRecovery[i][j].at(tick) += recov;
			else
				m_totRecovery[i][j].insert(std::make_pair(tick, recov));

			ptsd_count[i] += pts_count;
			ptsd_resolved[i] += res_count;
		}

		recovery.value[i] = ptsd_count[i]/(ptsd_count[i]+ptsd_resolved[i]);
	}

	Pair stdErr(computeError(ptsd_resolved, ptsd_count));
	recovery.diff = computeDiff(recovery.value, stdErr.first);
	recovery.ratio = computeRatio(recovery.value, stdErr.second);

	if(m_recovery.count(tick) > 0)
	{
		for(int i = 0; i < NUM_TREATMENT; ++i)
			m_recovery[tick].value[i] += recovery.value[i];

		m_recovery[tick].diff += recovery.diff;
		m_recovery[tick].ratio += recovery.ratio;
	}
	else
	{
		m_recovery.insert(std::make_pair(tick, recovery));
	}
	
}

Risk Counter::computeDiff(double *arr, double stdErr)
{
	Risk diff;

	diff.val = arr[STEPPED_CARE]-arr[USUAL_CARE];
	diff.lowLim = diff.val - 1.96*stdErr;
	diff.upLim = diff.val + 1.96*stdErr;
	
	return diff;
}

Risk Counter::computeRatio(double *arr, double stdErr)
{
	Risk ratio;

	ratio.val = arr[STEPPED_CARE]/arr[USUAL_CARE];

	ratio.lowLim = exp(log(ratio.val) - 1.96*stdErr);
	ratio.upLim =  exp(log(ratio.val) + 1.96*stdErr);
	
	return ratio;
}


Counter::Pair Counter::computeError(double *cases, double *non_cases)
{
	Pair stdErr;
	double a = cases[STEPPED_CARE];
	double b = non_cases[STEPPED_CARE];

	double c = cases[USUAL_CARE];
	double d = non_cases[USUAL_CARE];
	
	stdErr.first = sqrt(((a*b)/(pow((a+b),3))) + ((c*d)/(pow((c+d),3)))); //RD standard error
	stdErr.second = (a != 0 && c != 0) ? sqrt((1/a)-(1/(a+b))+(1/c)-(1/(c+d))) : 0; //RR standard error

	return stdErr;
}

Counter::Pair Counter::getPrevalence(int tick, int totPop)
{
	Pair prevalence(0, 0);
	
	for(int j = 0; j < NUM_PTSD; ++j)
	{
		prevalence.first += 100*(double)m_ptsdCount[STEPPED_CARE][j].at(tick)/totPop;
		prevalence.second +=  100*(double)m_ptsdCount[USUAL_CARE][j].at(tick)/totPop;
	}

	return prevalence;
}

double Counter::getCbtUptake(int tick)
{
	int tot_ptsd = 0;
	for(int j = 0; j < NUM_PTSD; ++j)
		tot_ptsd += m_ptsdCount[STEPPED_CARE][j].at(tick);

	return 100*(double)m_cbtCount[tick]/tot_ptsd;
}

double Counter::getSprUptake(int tick)
{
	int tot_ptsd = 0;
	for(int j = 0; j < NUM_PTSD; ++j)
		tot_ptsd += m_ptsdCount[STEPPED_CARE][j].at(tick);

	return 100*(double)m_sprCount[tick]/tot_ptsd;
}

double Counter::getNaturalDecayUptake(int tick)
{
	int tot_ptsd = 0;
	for(int j = 0; j < NUM_PTSD; ++j)
		tot_ptsd += m_ptsdCount[STEPPED_CARE][j].at(tick);

	return 100*(double)m_ndCount[tick]/tot_ptsd;
}

void Counter::outputHouseholdCounts(std::string geoID)
{
	std::ofstream hhFile;
	std::string fileName = "households/" + geoID + "_households.csv";
	hhFile.open(parameters->getOutputDir()+fileName);

	if(!hhFile.is_open()){
		std::cout << "Error: Cannot create " << fileName << "!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	hhFile << ",";
	for(auto hhSizeStr : ACS::HHSize::_values())
			hhFile << hhSizeStr._to_string() << ",";

	hhFile << std::endl;
	for(auto hhType : ACS::HHType::_values())
	{
		hhFile << hhType._to_string() << ",";
		for(auto hhSize : ACS::HHSize::_values())
		{
			int countHHSize = 0;
			for(auto hhInc : ACS::HHIncome::_values())
			{
				std::string var_type = std::to_string(hhType)+std::to_string(hhSize)+std::to_string(hhInc);
				//countHHSize += m_householdCount.count(var_type);
				if(m_householdCount.count(var_type) > 0)
					countHHSize += m_householdCount[var_type].size();
			}
			hhFile << countHHSize << ",";
		}
		hhFile << std::endl;
	}

	hhFile << std::endl;
	
	hhFile << ",";
	for(auto hhIncStr : ACS::HHIncome::_values())
		hhFile << hhIncStr._to_string() << ",";
	
	hhFile << std::endl;
	for(auto hhType : ACS::HHType::_values())
	{
		hhFile << hhType._to_string() << ",";
		for(auto hhInc : ACS::HHIncome::_values())
		{
			int countHHInc = 0;
			for(auto hhSize : ACS::HHSize::_values())
			{
				std::string var_type = std::to_string(hhType)+std::to_string(hhSize)+std::to_string(hhInc);
				//countHHInc += m_householdCount.count(var_type);
				if(m_householdCount.count(var_type) > 0)
					countHHInc += m_householdCount[var_type].size();
			}
			hhFile << countHHInc << ",";
		}
		hhFile << std::endl;
	}
	hhFile << std::endl;
}

void Counter::outputPersonCounts(std::string geoID)
{
	std::ofstream pFile;
	std::string fileName = "persons/" + geoID + "_persons.csv";
	//const char* fileName = "persons\\persons.csv";
	pFile.open(parameters->getOutputDir()+fileName);

	if(!pFile.is_open()){
		std::cout << "Error: Cannot create " << fileName << "!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	std::string dummy = "0";

	//Person counter for age and sex
	pFile << ",";
	for(auto ageCatStr : ACS::AgeCat::_values())
		pFile << ageCatStr._to_string() << ",";
	pFile << std::endl;
	for(auto sex : ACS::Sex::_values())
	{
		pFile << sex._to_string() << ",";
		for(auto ageCat : ACS::AgeCat::_values())
		{
			int countAge = 0;
			for(auto org : ACS::Origin::_values())
			{
				std::string type = dummy+std::to_string(sex)+std::to_string(ageCat)+std::to_string(org);
				//countAge += m_personCount.count(type);
				if(m_personCount.count(type) > 0)
					countAge += m_personCount[type].size();
			}
			pFile << countAge << ",";
		}
		pFile << std::endl;
	}
	pFile << std::endl;
	
	//Person counter for origin and sex
	pFile << ",";
	for(auto orgStr : ACS::Origin::_values())
		pFile << orgStr._to_string() << ",";
	pFile << std::endl;
	for(auto sex : ACS::Sex::_values())
	{
		pFile << sex._to_string() << ",";
		for(auto org : ACS::Origin::_values())
		{
			int countOrigin = 0;
			for(auto ageCat : ACS::AgeCat::_values())
			{
				std::string type = dummy+std::to_string(sex)+std::to_string(ageCat)+std::to_string(org);
				//countOrigin += m_personCount.count(type);
				if(m_personCount.count(type) > 0)
					countOrigin += m_personCount[type].size();
			}
			pFile << countOrigin << ",";
		}
		pFile << std::endl;
	}
	pFile << std::endl;

	//Person counter for education and sex
	pFile << ",";
	for(auto eduAge : ACS::EduAgeCat::_values())
		for(auto edu : ACS::Education::_values())
			pFile << eduAge._to_string() << " " << edu._to_string() << ",";

	pFile << std::endl;
	for(auto sex : ACS::Sex::_values())
	{
		pFile << sex._to_string() << ",";
		for(auto eduAge : ACS::EduAgeCat::_values())
		{
			for(auto edu : ACS::Education::_values())
			{
				int countEdu = 0;
				for(auto org : ACS::Origin::_values())
				{
					std::string type = std::to_string(sex)+std::to_string(eduAge)+std::to_string(org)+std::to_string(edu);
					if(m_personCount.count(type) > 0)
						countEdu += m_personCount[type].size();
				}
				pFile << countEdu << ",";
			}
		}
		pFile << std::endl;
	}
	
	pFile << std::endl;
}

void Counter::outputRiskFactorPercent()
{
	std::ofstream rfile;
	std::string fileName = "risk_factor_props.csv";

	rfile.open(parameters->getOutputDir()+fileName);

	if(!rfile.is_open())
	{
		std::cout << "Error: Cannot open file: " << fileName << " !" << std::endl;
		exit(EXIT_SUCCESS);
	}

	const int num_risk_strata = 16;
	const Pool *nhanesPool = parameters->getNhanesPool();
	
	std::map<std::string, double> popCountType;

	for(size_t nh = 0; nh < nhanesPool->size(); ++nh)
	{
		double count = 0;
		for(size_t i = 1; i <= num_risk_strata; ++i)
		{
			std::string key = std::to_string(i)+nhanesPool->at(nh);
			count += m_riskFacCount[key].size();
		}
		popCountType.insert(std::make_pair(nhanesPool->at(nh), count));
	}

	for(auto org:NHANES::Org::_values())
	{
		for(auto sex : NHANES::Sex::_values())
		{
			for(auto age : NHANES::AgeCat::_values())
			{
				for(auto edu : NHANES::Edu::_values())
				{
					std::string key = std::to_string(org)+std::to_string(sex)+std::to_string(age)+std::to_string(edu);
					double total = popCountType[key];

					for(size_t rf = 1; rf <= num_risk_strata; ++rf)
					{
						double count = m_riskFacCount[std::to_string(rf)+key].size();

						rfile << rf << "," << org._to_string() << "," << sex._to_string() << "," << age._to_string() << ","
							<< edu._to_string() << "," << 100*count/total << std::endl;
					}
				}
			}
		}
	}

}

void Counter::outputHealthOutcomes()
{
	std::ofstream file1, file2, file3, file4;
	std::string outcome_file1 = "overall_prevalence.csv";
	std::string outcome_file2 = "overall_recovery.csv";
	std::string outcome_file3 = "prevalence_ptsd_type.csv";
	std::string outcome_file4 = "recovery_ptsd_type.csv";

	file1.open(parameters->getOutputDir()+outcome_file1);
	file2.open(parameters->getOutputDir()+outcome_file2);
	file3.open(parameters->getOutputDir()+outcome_file3);
	file4.open(parameters->getOutputDir()+outcome_file4);
	
	if(!file1.is_open() && !file2.is_open() && !file3.is_open() && !file4.is_open())
	{
		std::cout << "Error: Cannot open file!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	int num_steps = parameters->getViolenceParam()->tot_steps;
	int num_trials = parameters->getViolenceParam()->num_trials;

	for(int i = 0; i < num_steps; ++i)
	{
		for(int j = 0; j < NUM_TREATMENT; ++j)
		{
			m_prevalence[i].value[j] /= num_trials;
			m_recovery[i].value[j] /= num_trials;

			for(int k = 0; k < NUM_PTSD; ++k)
			{
				m_totPrev[j][k].at(i) /= num_trials;
				m_totRecovery[j][k].at(i) /= num_trials;

				file3 << i+1 << "," << m_totPrev[j][k].at(i) << ",";
				file4 << i+1 << "," << m_totRecovery[j][k].at(i) << ",";
			}
		}

		file3 << std::endl;
		file4 << std::endl;

		m_prevalence[i].diff /= num_trials;
		m_recovery[i].diff /= num_trials;

		m_prevalence[i].ratio /= num_trials;
		m_recovery[i].ratio /= num_trials;
		

		file1 << i << "," << m_prevalence[i].value[STEPPED_CARE] << "," << m_prevalence[i].value[USUAL_CARE] << "," 
			<< m_prevalence[i].diff << "," << m_prevalence[i].ratio << std::endl;

		file2 << i << "," << m_recovery[i].value[STEPPED_CARE] << "," << m_recovery[i].value[USUAL_CARE] << "," 
			<< m_recovery[i].diff << "," << m_recovery[i].ratio << std::endl;
	}
}

void Counter::clearMap(TypeMap &type)
{
	for(auto map = type.begin(); map != type.end(); ++map)
	{
		map->second.clear();
		map->second.shrink_to_fit();
	}

	type.clear();

}

void Counter::clearCounter()
{
	for(int i = 0; i < NUM_TREATMENT; ++i)
	{
		for(int j = 0; j < NUM_PTSD; ++j)
		{
			m_ptsdCount[i][j].clear();
			m_ptsdResolvedCount[i][j].clear();
		}

		m_cbtReach[i].clear();
		m_sprReach[i].clear();
	}

	m_sprCount.clear();
	m_cbtCount.clear();
	m_ndCount.clear();
}
