#ifndef __Parameters_h__
#define __Parameters_h__

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <numeric>
//#include <unordered_map>
#include <boost/tokenizer.hpp>
#include "ACS.h"


#define EQUITY_EFFICIENCY 1
#define MASS_VIOLENCE 2

//Violence Model Parameters
namespace MVS
{
	struct ViolenceParams
	{
		int inner_draws, outer_draws;
		int min_households_puma;
		int mean_friends_size;
		int num_teachers;
		int min_age;
		int age_diff_students, age_diff_others;
		double p_val_student, p_val_teacher, p_val_origin, p_val_edu, p_val_age, p_val_gender;
		int aff_students, aff_teachers;
		double prev_students_tot;
		double prev_teachers_female;
		double prev_teachers_male;
		double prev_fam_female;
		double	prev_fam_male;
		double	prev_comm_female;
		double prev_comm_male;
		double ptsd_cutoff;
		int screening_time;
		double sensitivity;
		double specificity;
		int tot_steps;
		int treatment_time;
		int num_trials;
		int cbt_dur_non_cases;
		int max_cbt_sessions;
		int max_spr_sessions;
		double cbt_coeff;
		double spr_coeff;
		int cbt_cost, spr_cost;
		double nd_coeff;
		double percent_nd;
		int nd_dur;
		double ptsdx_relapse;
		int time_relapse;
		int num_relapse;
		double percent_relapse;
		double dw_mild, dw_moderate, dw_severe;
		double discount;
	};
}

namespace EET
{
	struct Framingham
	{
		double beta_age, beta_tchols, beta_hdl;
		double beta_sbp, beta_trt_sbp;
		double beta_smoker, beta_age_tchols;
		double beta_age_smoker, beta_sq_age;
	};

	struct CardioParams
	{
		Framingham male_coeff, female_coeff;
	};
}

class Parameters
{
public:
	typedef std::vector<std::string> Columns;
	typedef std::vector<Columns> Rows;
	typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
	typedef std::multimap<std::string, Columns> MultiMapCSV;
	typedef std::pair<double, double> PairDD;

	typedef std::map<std::string, int> MapInt;
	typedef std::map<std::string, double>MapDbl;
	typedef std::multimap<int, MapInt> MultiMapCB;
	typedef std::map<std::string, std::vector<PairDD>> ProbMap;
	typedef std::map<std::string, PairDD> PairMap;
	typedef std::map<int,std::map<std::string, PairDD>> RiskFacMap;

	typedef std::vector<std::string> Pool;

	Parameters(const char*, const char*, const int);

	virtual ~Parameters();

	std::string getInputDir() const;
	std::string getOutputDir() const;
	
	const char* getMSAListFile();
	const char* getCountiesListFile();
	const char* getPUMAListFile();
	const char* getHouseholdPumsFile(std::string);
	const char* getPersonPumsFile(std::string);

	const char* getRaceMarginalFile();
	const char* getEducationMarginalFile();

	const char* getHHTypeMarginalFile();
	const char* getHHSizeMarginalFile();
	const char* getHHIncomeMarginalFile();
	const char* getGQMarginalFile();

	double getAlpha() const;
	double getMinSampleSize() const;
	int getMaxDraws() const;
	short int getSimType() const;
	bool writeToFile() const;

	const Pool *getHouseholdPool() const;
	const Pool *getPersonPool() const;
	const Pool *getNhanesPool() const;

	MultiMapCB getACSCodeBook() const;
	ProbMap getRiskStrataProbability() const;
	const PairMap *getRiskFactorMap(int);
	//const PairMap *getRiskFactorCI(int);

	std::multimap<int, int> getVariableMap(int) const;
	MapInt getOriginMapping() const;
	MapInt getSchoolDemographics();
	PairMap *getPtsdSymptoms();
	//double getMassViolenceParam(std::string);
	const MVS::ViolenceParams *getViolenceParam();
	const EET::CardioParams *getCardioParam();
	
private:

	void readACSCodeBookFile();
	void readAgeGenderMappingFile();
	void readHHIncomeMappingFile();
	void readOriginListFile();

	//Equity-efficiency model
	void readNHANESRiskFactors();
	void readFraminghamCoefficients();
	//void readNHANESRiskFactorsCI();
	void setRiskFactors(std::map<std::string, PairDD>&, const char*, const char*, std::string); 
	void setCardioParams(PairMap *);

	//Mass-violence model
	void readSchoolDemograhics();
	void readMassViolenceInputs();
	void readPtsdSymptoms();
	void setViolenceParams(MapDbl *);

	void createHouseholdPool();
	void createPersonPool();
	void createNhanesPool();
	std::string getNHANESpersonType(const char*, const char*, const char*, const char*);

	MapInt createCodeBookMap(ACS::PumsVar);
	const char* getFilePath(const char *);

	Rows readCSVFile(const char*);

	const char *inputDir;
	const char *outputDir;

	MultiMapCSV m_codeBook;
	MultiMapCB m_acsCodes;

	std::multimap<int, int> m_eduAgeGender;
	std::multimap<int, int> m_hhIncome;

	MapInt m_originByRace, m_schoolDemo;
	MVS::ViolenceParams vParams;
	PairMap m_ptsdx;

	ProbMap m_riskStrataProb;
	RiskFacMap m_risks;//, m_risk_ci;
	EET::CardioParams cardioParams;

	double alpha, minSampleSize;
	int max_draws;
	short int simType;
	bool output;

	Pool hhPool, personPool, nhanesPool;

};
#endif __Parameters_h__