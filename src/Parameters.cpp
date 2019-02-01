/**
*	@file	 Parameters.cpp	
*	@author	 Shailesh Tamrakar
*	@date	 7/23/2018
*	@version 1.0
*
*	@section DESCRIPTION
*	This source file has methods/functions that - 
*	1. Return path of input files PUMS dataset, household and population-level 
*	estimates, list of Metropolitan Statistical Areas.
*	2. Read and store ACS codebook by variable type (age, education, race...)
*	3. Create maps by pairing variable indexes with its variable types.   
*	3. Create and return all the possible combinations (pool) of households 
*	by type, size and income as well as all person types by age, sex, race 
*	and education. 
*	4. Store and return parameters for testing goodness-of-fit of synthetic 
*	population. 
*/

#include "Parameters.h"
#include "csv.h"


Parameters::Parameters(const char *inDir, const char *outDir, const int simModel) : 
	inputDir(inDir), outputDir(outDir), alpha(0.05), minSampleSize(1000.0), max_draws(200), simType(simModel), output(true)
{
	readACSCodeBookFile();
	readAgeGenderMappingFile();
	readHHIncomeMappingFile();
	readOriginListFile();

	createHouseholdPool();
	createPersonPool();
	createNhanesPool();

	if(simType == EQUITY_EFFICIENCY)
	{
		readNHANESRiskFactors();
		readFraminghamCoefficients();
	}
	else if(simType == MASS_VIOLENCE)
	{
		readSchoolDemograhics();
		readMassViolenceInputs();
		readPtsdSymptoms();
	}
		
}

Parameters::~Parameters()
{
	
}

std::string Parameters::getInputDir() const
{
	return inputDir;
}

std::string Parameters::getOutputDir() const
{
	return outputDir;
}

const char* Parameters::getMSAListFile() 
{
	return getFilePath("ACS_15_METRO_LIST_fullList.csv");
}

const char* Parameters::getCountiesListFile() 
{
	return getFilePath("ACS_15_Metro_Counties_List.csv");
}

const char* Parameters::getPUMAListFile() 
{
	return getFilePath("ACS_15_Counties_Puma10_List.csv");
}

const char* Parameters::getHouseholdPumsFile(std::string st) 
{
	std::string hhPumsFile = "pums/households/ss15h"+st+".csv";
	return getFilePath(hhPumsFile.c_str());
}

const char* Parameters::getPersonPumsFile(std::string st) 
{
	std::string perPumsFile = "pums/persons/ss15p"+st+".csv";
	return getFilePath(perPumsFile.c_str());
}

const char* Parameters::getRaceMarginalFile() 
{
	return getFilePath("marginals/2015/ACS_15_race_by_age_sex.csv");
}

const char* Parameters::getEducationMarginalFile() 
{
	return getFilePath("marginals/2015/ACS_15_edu_by_age_sex.csv");
}

const char* Parameters::getHHTypeMarginalFile() 
{
	return getFilePath("marginals/2015/ACS_15_household_type.csv");
}

const char* Parameters::getHHSizeMarginalFile() 
{
	return getFilePath("marginals/2015/ACS_15_household_size.csv");
}

const char* Parameters::getHHIncomeMarginalFile()
{
	return getFilePath("marginals/2015/ACS_15_household_income.csv");
}

const char* Parameters::getGQMarginalFile() 
{
	return getFilePath("marginals/2015/ACS_15_group_quarters.csv");
}

double Parameters::getAlpha() const
{
	return alpha;
}

double Parameters::getMinSampleSize() const
{
	return minSampleSize;
}

int Parameters::getMaxDraws() const
{
	return max_draws;
}

short int Parameters::getSimType() const
{
	return simType;
}

bool Parameters::writeToFile() const
{
	return output;
}

const Parameters::Pool * Parameters::getHouseholdPool() const
{
	return &hhPool;
}

const Parameters::Pool * Parameters::getPersonPool() const
{
	return &personPool;
}

const Parameters::Pool * Parameters::getNhanesPool() const
{
	return &nhanesPool;
}

//std::unordered_multimap<int, int> Parameters::getVariableMap(int type) const
std::multimap<int, int> Parameters::getVariableMap(int type) const
{
	//std::unordered_multimap<int, int> temp;
	std::multimap<int, int> temp;
	switch(type)
	{
	case ACS::Estimates::estEducation:
		return m_eduAgeGender;
		break;
	case ACS::Estimates::estHHIncome:
		return m_hhIncome;
		break;
	default:
		return temp;
		break;
	}
}

//std::unordered_map<std::string, int> Parameters::getOriginMapping() const
Parameters::MapInt Parameters::getOriginMapping() const
{
	return m_originByRace;
}

Parameters::MapInt Parameters::getSchoolDemographics() 
{
	if(simType == MASS_VIOLENCE)
		return m_schoolDemo;
	else{
		std::cout << "Error: Wrong simulation model selected!" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

Parameters::PairMap * Parameters::getPtsdSymptoms() 
{
	if(simType == MASS_VIOLENCE)
		return &m_ptsdx;
	else{
		std::cout << "Error: Wrong simulation model selected!" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

//double Parameters::getMassViolenceParam(std::string var)
//{
//	if(simType == MASS_VIOLENCE)
//	{
//		if(m_massViolenceParams.count(var) > 0)
//			return m_massViolenceParams[var];
//		else
//		{
//			std::cout << "Error: Invalid variable!" << std::endl;
//			exit(EXIT_SUCCESS);
//		}
//	}
//	else
//	{
//		std::cout << "Error: Wrong simulation model selected!" << std::endl;
//		exit(EXIT_SUCCESS);
//	}
//}

const MVS::ViolenceParams* Parameters::getViolenceParam()
{
	if(simType == MASS_VIOLENCE)
	{
		return &vParams;
	}
	else
	{
		std::cout << "Error: Wrong simulation model selected!" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

const EET::CardioParams* Parameters::getCardioParam()
{
	if(simType == EQUITY_EFFICIENCY)
	{
		return &cardioParams;
	}
	else
	{
		std::cout << "Error: Wrong simulation model selected!" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

Parameters::MultiMapCB Parameters::getACSCodeBook() const
{
	return m_acsCodes;
}

Parameters::ProbMap Parameters::getRiskStrataProbability() const
{
	return m_riskStrataProb;
}

const Parameters::PairMap * Parameters::getRiskFactorMap(int type)
{
	if(m_risks.count(type) > 0)
		return &m_risks[type];
	else{
		std::cout << "Error: Risk factor type = " << type << " doesn't exist!" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

//const Parameters::PairMap * Parameters::getRiskFactorCI(int type)
//{
//	if(m_risk_ci.count(type) > 0)
//		return &m_risk_ci[type];
//	else{
//		std::cout << "Error: Risk factor type = " << type << " doesn't exist!" << std::endl;
//		exit(EXIT_SUCCESS);
//	}
//}

/**
*	@brief Reads and stores PUMS data dictionary for housing and
*	population records
*	@section DESCRIPTION
*	Iterates and stores each line of ACS Codebook file until eof is reached.
*	Create a multimap paired with PUMS variable type and a map paired with
*	ACS integer codes for attributes and its definitions.
*	@param none
*	@return void
*/
void Parameters::readACSCodeBookFile()
{
	//const char* codeBookFile = getFilePath("pums\\ACS_2010_PUMS_codebook.csv");
	const char* codeBookFile = getFilePath("pums/ACS_2015_PUMS_codebook.csv");
	std::ifstream ifs;

	ifs.open(codeBookFile, std::ios::in);

	if(!ifs.is_open()){
		std::cout << "Error: Cannot open " << codeBookFile << "!" << std::endl;
		exit(EXIT_SUCCESS);
	}
	
	Columns col;
	Rows row;
	std::string line;
	
	while(std::getline(ifs, line))
	{
		Tokenizer temp(line);
		col.assign(temp.begin(), temp.end());
		row.push_back(col);

		if(col.empty())
		{
			std::string codeACS = row.front().at(0);
			for(auto it = row.begin()+1; it != row.end(); ++it)
			{
				if(!it->empty())
					m_codeBook.insert(std::make_pair(codeACS, *it));
			}
			col.clear();
			row.clear();
		}
	}

	m_acsCodes.insert(make_pair(ACS::PumsVar::RAC1P, createCodeBookMap(ACS::PumsVar::RAC1P)));
	m_acsCodes.insert(make_pair(ACS::PumsVar::SCHL, createCodeBookMap(ACS::PumsVar::SCHL)));
	m_acsCodes.insert(make_pair(ACS::PumsVar::AGEP, createCodeBookMap(ACS::PumsVar::AGEP)));

	m_acsCodes.insert(make_pair(ACS::PumsVar::HHT, createCodeBookMap(ACS::PumsVar::HHT)));
	m_acsCodes.insert(make_pair(ACS::PumsVar::HINCP, createCodeBookMap(ACS::PumsVar::HINCP)));

}

/**
*	@brief Creates a multimap by pairing "Sex-EduAge" type ((Male, 18-24),...) with "Sex-Age"
*	variable index (POP_M_18_19,...POP_M_22_24,...). 
*	@param none
*	@return void
*/
void Parameters::readAgeGenderMappingFile() 
{
	io::CSVReader<4>age_gender_map_file(getFilePath("variables/age_gender_map.csv"));
	age_gender_map_file.read_header(io::ignore_extra_column, "Gender", "Edu_Age_Range", "Mar_Age_Range", "Race_Variable");

	const char* gender = NULL;
	const char* edu_age_range = NULL;
	const char* marital_age_range = NULL;
	const char* var_name = NULL;

	while(age_gender_map_file.read_row(gender, edu_age_range, marital_age_range, var_name))
	{
		int sex = ACS::Sex::_from_string(gender);
		int raceVarIdx = ACS::RaceMarginalVar::_from_string(var_name);

		std::string r_eduAge(edu_age_range);
		std::string r_marAge(marital_age_range);

		if(r_eduAge != "NULL")
		{
			int eduAge = ACS::EduAgeCat::_from_string(edu_age_range);
			m_eduAgeGender.insert(std::make_pair(10*sex+eduAge, raceVarIdx-1));
		}

	}
}

/**
*	@brief Creates a multimap by pairing hhIncome type (hhInc1..hhInc10) with hhIncome 
*	variable index (HH_INC1,.....HHINC16).
*	@param none
*	@return void
*/
void Parameters::readHHIncomeMappingFile()
{
	io::CSVReader<2>hhIncome_map_file(getFilePath("variables/hhIncome_map.csv"));
	hhIncome_map_file.read_header(io::ignore_extra_column, "HHIncome", "Variable");

	const char* hhIncStr = NULL;
	const char* var_name = NULL;

	while(hhIncome_map_file.read_row(hhIncStr, var_name))
	{
		int hhIncCat = ACS::HHIncome::_from_string(hhIncStr);
		int hhIncIdx = ACS::HHIncMarginalVar::_from_string(var_name);

		m_hhIncome.insert(std::make_pair(hhIncCat, hhIncIdx));
	}

}

/**
*	@brief Creates a multimap by pairing race/origin type (Hispanic, White alone..) with
*	its corresponding PUMS code(1,...,8).
*	@param none
*	@return void
*/
void Parameters::readOriginListFile()
{
	Rows originList = readCSVFile(getFilePath("variables/race_by_origin.csv"));
	originList.erase(originList.begin());

	for(auto row = originList.begin(); row != originList.end(); ++row)
		m_originByRace.insert(std::make_pair(row->front(), std::stoi(row->back())));
}


void Parameters::readNHANESRiskFactors()
{
	io::CSVReader<13>nhanes_risks(getFilePath("risk_factors/nhanes_risk_factors.csv"));
	nhanes_risks.read_header(io::ignore_extra_column, "Risk_Score", "Race", 
		"Gender", "Age_Cat", "Edu_Cat", "Freq_nhanes", "Mean_Tchols", "Err_Tchols",
		"Mean_HDL", "Err_HDL", "Mean_bp", "Err_bp", "smoking_stat");

	const char *risk_score = NULL; const char *race = NULL;
	const char *sex = NULL; const char *age_cat = NULL; const char *edu_cat = NULL;
	const char *freq = NULL; 
	const char *tchols = NULL; const char *err_tchols = NULL;
	const char *hdlChols = NULL; const char *err_hdl = NULL;
	const char *sysBp = NULL; const char *err_sysBp = NULL;
	const char *smokingStat = NULL;

	std::map<std::string, std::vector<double>> m_tempFreq;
	std::map<std::string, PairDD> m_tchols, m_hdlChols, m_sysBp, m_smoking;

	while(nhanes_risks.read_row(risk_score, race, 
		sex, age_cat, edu_cat, freq, tchols, err_tchols,
		hdlChols, err_hdl, sysBp, err_sysBp, smokingStat))
	{
		double freq_ = std::stod(freq);
		int risk_strata = std::stoi(risk_score);

		std::string key_person_type = getNHANESpersonType(race, sex, age_cat, edu_cat);;
		std::string key_risk = std::to_string(risk_strata)+key_person_type;

		if(m_riskStrataProb.count(key_person_type) == 0)
		{
			std::vector<PairDD> temp_pair;
			std::vector<double> temp_prob;

			temp_pair.push_back(PairDD(freq_, risk_strata));
			temp_prob.push_back(freq_);

			m_riskStrataProb.insert(std::make_pair(key_person_type, temp_pair));
			m_tempFreq.insert(std::make_pair(key_person_type, temp_prob));
		}
		else
		{
			m_riskStrataProb[key_person_type].push_back(PairDD(freq_, risk_strata));
			m_tempFreq[key_person_type].push_back(freq_);
		}

		setRiskFactors(m_tchols, tchols, err_tchols, key_risk);
		setRiskFactors(m_hdlChols, hdlChols, err_hdl, key_risk);
		setRiskFactors(m_sysBp, sysBp, err_sysBp, key_risk);

		if(key_person_type == "1111")
			setRiskFactors(m_smoking, smokingStat, NULL, std::to_string(risk_strata));
	}

	std::vector<PairDD> vec_pairs;
	for(auto itr = m_tempFreq.begin(); itr != m_tempFreq.end(); ++itr)
	{
		double sum_pop = std::accumulate(itr->second.begin(), itr->second.end(), 0.0);
		
		vec_pairs = m_riskStrataProb[itr->first];
		for(size_t i = 0; i < vec_pairs.size(); ++i)
			vec_pairs[i].first = vec_pairs[i].first/sum_pop;

		m_riskStrataProb[itr->first] = vec_pairs;

	}

	m_risks.insert(std::make_pair(NHANES::RiskFac::totalChols, m_tchols));
	m_risks.insert(std::make_pair(NHANES::RiskFac::HdlChols, m_hdlChols));
	m_risks.insert(std::make_pair(NHANES::RiskFac::SystolicBp, m_sysBp));
	m_risks.insert(std::make_pair(NHANES::RiskFac::SmokingStat, m_smoking));

	//readNHANESRiskFactorsCI();
}

void Parameters::readFraminghamCoefficients()
{
	io::CSVReader<3>framingham_file(getFilePath("risk_factors/framingham_params.csv"));
	framingham_file.read_header(io::ignore_extra_column, "Variable", "Male", "Female");

	const char* var = NULL;
	const char* male_val = NULL;
	const char* female_val = NULL;

	PairMap m_framingham;
	while(framingham_file.read_row(var, male_val, female_val))
	{
		PairDD gender;
		gender.first = std::stod(male_val);
		gender.second = std::stod(female_val);

		m_framingham.insert(std::make_pair(var, gender));
	}

	setCardioParams(&m_framingham);
}

//void Parameters::readNHANESRiskFactorsCI()
//{
//	std::string dir = "risk_factors/";
//	std::map<int, std::string> m_files;
//
//	m_files.insert(std::make_pair(NHANES::RiskFac::totalChols, "Tchols_CI.csv"));
//	m_files.insert(std::make_pair(NHANES::RiskFac::LdlChols, "LDL_CI.csv"));
//	m_files.insert(std::make_pair(NHANES::RiskFac::SystolicBp, "Systolic_CI.csv"));
//
//	const char *race = NULL; const char *gender = NULL;
//	const char *ageCat = NULL; const char *edu = NULL;
//	const char *riskStrata = NULL; const char *ll = NULL; 
//	const char *ul = NULL;
//
//	std::map<std::string, PairDD> ci_tchols, ci_ldl, ci_systolic;
//
//	for(size_t i = NHANES::RiskFac::totalChols; i <= NHANES::RiskFac::SystolicBp; ++i)
//	{
//		std::string full_path = dir+m_files[i];
//		io::CSVReader<7> nhanes_rf_ci(getFilePath(full_path.c_str()));
//
//		nhanes_rf_ci.read_header(io::ignore_extra_column,"Race", "Gender", "Education", "Age", "Rf", "ll", "ul");
//
//		while(nhanes_rf_ci.read_row(race, gender, edu, ageCat, riskStrata, ll, ul))
//		{
//			std::string person_type = getNHANESpersonType(race, gender, ageCat, edu);
//			std::string key_risk = std::to_string(std::stoi(riskStrata))+person_type;
//
//			switch(i)
//			{
//			case NHANES::RiskFac::totalChols:
//				setRiskFactors(ci_tchols, ll, ul, key_risk);
//				break;
//			case NHANES::RiskFac::LdlChols:
//				setRiskFactors(ci_ldl, ll, ul, key_risk);
//				break;
//			case NHANES::RiskFac::SystolicBp:
//				setRiskFactors(ci_systolic, ll, ul, key_risk);
//				break;
//			default:
//				break;
//			}
//		}
//	}
//
//	m_risk_ci.insert(std::make_pair(NHANES::RiskFac::totalChols, ci_tchols));
//	m_risk_ci.insert(std::make_pair(NHANES::RiskFac::LdlChols, ci_ldl));
//	m_risk_ci.insert(std::make_pair(NHANES::RiskFac::SystolicBp, ci_systolic));
//}

void Parameters::setRiskFactors(std::map<std::string, PairDD> &map_risks, const char* var1, const char *var2, std::string key)
{
	if(var1 != NULL && var2 != NULL)
	{
		PairDD pair_risk(std::stod(var1), std::stod(var2));
		map_risks.insert(std::make_pair(key, pair_risk));
	}
	else if(var1 != NULL)
	{
		PairDD pair_risk(std::stod(var1), 0.0);
		map_risks.insert(std::make_pair(key, pair_risk));
	}
	else
	{
		std::cout << "Error: Mean and Std Error values cannot be NULL!" << std::endl;
		exit(EXIT_SUCCESS);
	}
}

void Parameters::setCardioParams(PairMap *m_params)
{
	cardioParams.male_coeff.beta_age = m_params->at("beta_age").first;
	cardioParams.female_coeff.beta_age = m_params->at("beta_age").second;

	cardioParams.male_coeff.beta_tchols = m_params->at("beta_tchols").first;
	cardioParams.female_coeff.beta_tchols = m_params->at("beta_tchols").second;

	cardioParams.male_coeff.beta_hdl = m_params->at("beta_hdl").first;
	cardioParams.female_coeff.beta_hdl = m_params->at("beta_hdl").second;

	cardioParams.male_coeff.beta_sbp = m_params->at("beta_sbp").first;
	cardioParams.female_coeff.beta_sbp = m_params->at("beta_sbp").second;

	cardioParams.male_coeff.beta_trt_sbp = m_params->at("beta_trt_sbp").first;
	cardioParams.female_coeff.beta_trt_sbp = m_params->at("beta_trt_sbp").second;

	cardioParams.male_coeff.beta_smoker = m_params->at("beta_smoker").first;
	cardioParams.female_coeff.beta_smoker = m_params->at("beta_smoker").second;

	cardioParams.male_coeff.beta_age_tchols = m_params->at("beta_age_tchols").first;
	cardioParams.female_coeff.beta_age_tchols = m_params->at("beta_age_tchols").second;

	cardioParams.male_coeff.beta_age_smoker = m_params->at("beta_age_smoker").first;
	cardioParams.female_coeff.beta_age_smoker = m_params->at("beta_age_smoker").second;

	cardioParams.male_coeff.beta_sq_age = m_params->at("beta_sq_age").first;
	cardioParams.female_coeff.beta_sq_age = m_params->at("beta_sq_age").second;

}

/**
*	@brief reads demographics of students by gender and origin for Mass-violence model
*	@param none
*	@return none
*/
void Parameters::readSchoolDemograhics()
{
	io::CSVReader<3>school_demo(getFilePath("mass_violence/stoneman_demo.csv"));
	//io::CSVReader<3>school_demo(getFilePath("mass_violence/cooper_hs_demo.csv"));
	school_demo.read_header(io::ignore_extra_column, "Gender", "Origin", "Count");

	const char *gender = NULL;
	const char *origin = NULL;
	const char *count = NULL;

	while(school_demo.read_row(gender, origin, count))
	{
		int i_gender = ACS::Sex::_from_string(gender);
		int i_origin = ACS::Origin::_from_string(origin);
		
		std::string key_school_demo = std::to_string(i_gender)+std::to_string(i_origin);
		m_schoolDemo.insert(std::make_pair(key_school_demo, std::stoi(count)));
	}
}

void Parameters::readMassViolenceInputs()
{
	io::CSVReader<2>social_network_params(getFilePath("mass_violence/mass_violence_input.csv"));
	social_network_params.read_header(io::ignore_extra_column, "Variable", "Value");

	const char* var = NULL;
	const char* val = NULL;

	MapDbl m_massViolenceParams;
	while(social_network_params.read_row(var, val))
		m_massViolenceParams.insert(std::make_pair(var, std::stod(val)));

	setViolenceParams(&m_massViolenceParams);
}

void Parameters::readPtsdSymptoms()
{
	io::CSVReader<5>ptsdx_file(getFilePath("mass_violence/ptsdx_strata.csv"));
	ptsdx_file.read_header(io::ignore_extra_column, "Gender", "Age_Cat", "ptsd", "mean", "std_err");

	const char* s_gender = NULL;
	const char* s_age = NULL;
	const char* s_ptsd = NULL;
	const char* mean = NULL;
	const char* std_err = NULL;

	std::string key = "";
	PairDD ptsdx;
	while(ptsdx_file.read_row(s_gender, s_age, s_ptsd, mean, std_err))
	{
		int i_gender = Violence::Sex::_from_string(s_gender);
		int i_age = Violence::AgeCat::_from_string(s_age);

		ptsdx.first = std::stod(mean);
		ptsdx.second = std::stod(std_err);

		key = std::to_string(i_gender)+std::to_string(i_age)+s_ptsd;
		m_ptsdx.insert(std::make_pair(key, ptsdx));
	}
}

void Parameters::setViolenceParams(MapDbl *m_param)
{
	vParams.inner_draws = (int)m_param->at("inner_draws");
	vParams.outer_draws = (int)m_param->at("outer_draws");
	vParams.min_households_puma = (int)m_param->at("min_households_puma");
	vParams.mean_friends_size = (int)m_param->at("mean_friends_size");
	vParams.num_teachers = (int)m_param->at("num_teachers");
	vParams.min_age = (int)m_param->at("min_age");
	vParams.age_diff_students =(int)m_param->at("age_diff_students");
	vParams.age_diff_others = (int)m_param->at("age_diff_others");
	vParams.p_val_student = m_param->at("p_val_student");
	vParams.p_val_teacher = m_param->at("p_val_teacher");
	vParams.p_val_origin = m_param->at("p_val_origin");
	vParams.p_val_edu = m_param->at("p_val_edu");
	vParams.p_val_age = m_param->at("p_val_age");
	vParams.p_val_gender = m_param->at("p_val_gender");
	vParams.aff_students = (int)m_param->at("aff_students");
	vParams.aff_teachers = (int)m_param->at("aff_teachers");
	vParams.prev_students_tot = m_param->at("prev_students_tot");
	vParams.prev_teachers_female = m_param->at("prev_teachers_female");
	vParams.prev_teachers_male = m_param->at("prev_teachers_male");
	vParams.prev_fam_female = m_param->at("prev_fam_female");
	vParams.prev_fam_male = m_param->at("prev_fam_male");
	vParams.prev_comm_female = m_param->at("prev_comm_female");
	vParams.prev_comm_male = m_param->at("prev_comm_male");
	vParams.ptsd_cutoff = m_param->at("ptsd_cutoff");
	vParams.screening_time = (int)m_param->at("screening_time");
	vParams.sensitivity = m_param->at("sensitivity");
	vParams.specificity = m_param->at("specificity");
	vParams.tot_steps = (int)m_param->at("tot_steps");
	vParams.treatment_time = (int)m_param->at("treatment_time");
	vParams.num_trials = (int)m_param->at("num_trials");
	vParams.cbt_dur_non_cases = (int)m_param->at("cbt_dur_non_cases");
	vParams.max_cbt_sessions = (int)m_param->at("max_cbt_sessions");
	vParams.max_spr_sessions = (int)m_param->at("max_spr_sessions");
	vParams.cbt_coeff = m_param->at("cbt_coeff");
	vParams.spr_coeff = m_param->at("spr_coeff");
	vParams.nd_coeff = m_param->at("nd_coeff");
	vParams.cbt_cost = (int)m_param->at("cbt_cost");
	vParams.spr_cost = (int)m_param->at("spr_cost");
	vParams.percent_nd = m_param->at("percent_nd");
	vParams.nd_dur = (int)m_param->at("nd_dur");
	vParams.ptsdx_relapse = m_param->at("ptsdx_relapse");
	vParams.time_relapse = (int)m_param->at("time_relapse");
	vParams.num_relapse = (int)m_param->at("num_relapse");
	vParams.percent_relapse = m_param->at("percent_relapse");
	vParams.dw_mild = m_param->at("dw_mild");
	vParams.dw_moderate = m_param->at("dw_moderate");
	vParams.dw_severe = m_param->at("dw_severe");
	vParams.discount = m_param->at("discount");
}

/**
*	@brief Creates pool of group-quarters and households by type, size and income.
*	@param none
*	@return void
*/
void Parameters::createHouseholdPool()
{
	std::string gqType = "-1";
	std::string gqSize = "1";
	std::string gqInc = "-1";

	hhPool.push_back(gqType+gqSize+gqInc);

	for(auto hhType : ACS::HHType::_values())
		for(auto hhSize : ACS::HHSize::_values())
			for(auto hhInc : ACS::HHIncome::_values())
				hhPool.push_back(std::to_string(hhType)+std::to_string(hhSize)+std::to_string(hhInc));

}

/**
*	@brief Creates possible pool of persons by age, sex, race and educational attaiment.
*	@param none
*	@return void
*/
void Parameters::createPersonPool()
{
	std::string dummy = "0";
	//pool of persons by sex, age and origin
	for(auto sex : ACS::Sex::_values())
		for(auto ageCat : ACS::AgeCat::_values())
			for(auto org : ACS::Origin::_values())
				personPool.push_back(dummy+std::to_string(sex)+std::to_string(ageCat)+std::to_string(org));

	//pool of person by sex, education age cat, origin and education attainment
	for(auto sex : ACS::Sex::_values())
		for(auto eduAge : ACS::EduAgeCat::_values())
			for(auto org : ACS::Origin::_values())
				for(auto edu : ACS::Education::_values())
					personPool.push_back(std::to_string(sex)+std::to_string(eduAge)+std::to_string(org)+std::to_string(edu));
}

void Parameters::createNhanesPool()
{
	for(auto org : NHANES::Org::_values())
		for(auto sex : NHANES::Sex::_values())
			for(auto age : NHANES::AgeCat::_values())
				for(auto edu : NHANES::Edu::_values())
					nhanesPool.push_back(std::to_string(org)+std::to_string(sex)+std::to_string(age)+std::to_string(edu));
}

std::string Parameters::getNHANESpersonType(const char *race, const char *sex, const char *age, const char *edu)
{
	std::string race_ = std::to_string(NHANES::Org::_from_string(race));
	std::string sex_ = std::to_string(NHANES::Sex::_from_string(sex));
	std::string age_cat_ = std::to_string(NHANES::AgeCat::_from_string(age));
	std::string edu_cat_ = std::to_string(NHANES::Edu::_from_string(edu));

	return (race_+sex_+age_cat_+edu_cat_);
}

/**
*	@brief Pairs ACS codes with their respective definitions
*	@param var is PUMS variable type (race, education, hhIncome,...)
*	@return map paired with PUMS ACS codes and its definition
*/
Parameters::MapInt Parameters::createCodeBookMap(ACS::PumsVar var)
{
	MapInt map;
	std::string str = var._to_string();

	auto its = m_codeBook.equal_range(str);

	for(auto it = its.first; it != its.second; ++it)
	{
		std::string key = it->second.back();
		int val = std::stoi(it->second.front());
		map.insert(std::make_pair(key, val));
	}

	m_codeBook.erase(str);

	return map;
}

/**
*	@param CSVfileName is name of file to be imported
*	@return result is complete file path for the file to be imported
*/
const char* Parameters::getFilePath(const char *CSVfileName)
{
	size_t bufferSize = strlen(inputDir) + strlen(CSVfileName) + 1;

	char *temp = new char[bufferSize];
	strcpy(temp, inputDir);
	strcat(temp, CSVfileName);

	char *result = temp;
	temp = NULL;
	delete [] temp;

	return result;
	
}

Parameters::Rows Parameters::readCSVFile(const char* file)
{
	Columns col;
	Rows row;

	std::ifstream ifs;
	ifs.open(file, std::ios::in);

	std::string line;
	while(std::getline(ifs, line))
	{
		Tokenizer temp(line);
		col.assign(temp.begin(), temp.end());
		row.push_back(col);
	}

	return row;
}


