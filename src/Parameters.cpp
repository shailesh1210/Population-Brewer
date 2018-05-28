#include "Parameters.h"
#include "csv.h"


//Parameters::Parameters(std::string inDir, std::string outDir) : inputDir(inDir), outputDir(outDir)
//{
//	//readACSCodeBookFile();
//}

Parameters::Parameters(const char *inDir, const char *outDir) : inputDir(inDir), outputDir(outDir)
{
	readACSCodeBookFile();
	readAgeGenderMappingFile();
	readOriginListFile();
}

Parameters::~Parameters()
{
}

//Public methods of class Parameters
std::string Parameters::getInputDir() const
{
	return inputDir;
}

std::string Parameters::getOutputDir() const
{
	return outputDir;
}

const char* Parameters::getMSAListFile() const
{
	return getFilePath("ACS_15_METRO_LIST_fullList.csv");
}

const char* Parameters::getCountiesListFile() const
{
	return getFilePath("ACS_15_Metro_Counties_List.csv");
}

const char* Parameters::getPUMAListFile() const
{
	return getFilePath("ACS_15_Counties_Puma_List.csv");
}


const char* Parameters::getHouseholdPumsFile() const
{
	return getFilePath("pums\\ss10htx.csv");
}
const char* Parameters::getPersonPumsFile() const
{
	//return getFilePath("pums_2010_baton_rouge.csv");
	//return getFilePath("pums\\ss10pla1.csv");
	return getFilePath("pums\\ss10ptx.csv");
}

std::string Parameters::getAgeMarginalFile() const
{
	return getFilePath("marginals\\age_marginals_2010.csv");
}

std::string Parameters::getSexMarginalFile() const
{
	return getFilePath("marginals\\gender_marginals_2010.csv");
}


const char* Parameters::getRaceMarginalFile() const
{
	return getFilePath("marginals\\all_msa\\ACS_10_race_by_age_sex.csv");
	//return getFilePath("marginals\\abilene\\race_marginals_2010_abilene.csv");
}

const char* Parameters::getEducationMarginalFile() const
{
	return getFilePath("marginals\\all_msa\\ACS_10_edu_by_age_sex.csv");
	//return getFilePath("marginals\\abilene\\education_marginals_2010_abilene.csv");
}

const char* Parameters::getMaritalMarginalFile() const
{
	return getFilePath("marginals\\all_msa\\ACS_10_marital_status_by_age_sex.csv");
}

const char* Parameters::getHHTypeMarginalFile() const
{
	return getFilePath("marginals\\all_msa\\ACS_10_household_type.csv");
}

const char* Parameters::getHHSizeMarginalFile() const
{
	return getFilePath("");
}

const char* Parameters::getHHIncomeMarginalFile() const
{
	return getFilePath("");
}

std::unordered_multimap<int, int> Parameters::getAgeGenderMapping(int type) const
{
	std::unordered_multimap<int, int> temp;
	switch(type)
	{
	case ACS::Estimates::estEducation:
		return m_eduAgeGender;
		break;
	case ACS::Estimates::estMarital:
		return m_maritalAgeGender;
		break;
	default:
		return temp;
		break;
	}
}

std::unordered_map<std::string, int> Parameters::getOriginMapping() const
{
	return m_originByRace;
}

Parameters::MultiMapCB Parameters::getACSCodeBook() const
{
	return m_acsCodes;
}

//private methods of class Parameters
void Parameters::readACSCodeBookFile()
{
	const char* codeBookFile = getFilePath("pums\\ACS_2010_PUMS_codebook.csv");
	//const char* codeBookFile = getFilePath("pums\\ACS_2015_PUMS_codebook.csv");
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

	m_acsCodes.insert(make_pair(ACS::PumsVar::HHT, createCodeBookMap(ACS::PumsVar::HHT)));
	m_acsCodes.insert(make_pair(ACS::PumsVar::HINCP, createCodeBookMap(ACS::PumsVar::HINCP)));

}

void Parameters::readAgeGenderMappingFile() 
{
	io::CSVReader<4>age_gender_map_file(getFilePath("variables\\age_gender_map.csv"));
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

		if(r_eduAge != "NULL"){
			int eduAge = ACS::EduAgeCat::_from_string(edu_age_range);
			m_eduAgeGender.insert(std::make_pair(10*sex+eduAge, raceVarIdx-1));
		}

		if(r_marAge != "NULL"){
			int marAge = ACS::MaritalAgeCat::_from_string(marital_age_range);
			m_maritalAgeGender.insert(std::make_pair(10*sex+marAge, raceVarIdx-1));
		}
	}
}

void Parameters::readOriginListFile()
{
	Rows originList = readCSVFile(getFilePath("variables\\race_by_origin.csv"));
	originList.erase(originList.begin());

	for(auto row = originList.begin(); row != originList.end(); ++row)
		m_originByRace.insert(std::make_pair(row->front(), std::stoi(row->back())));
}


Parameters::Map Parameters::createCodeBookMap(ACS::PumsVar var)
{
	Map map;
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


const char* Parameters::getFilePath(char *CSVfileName) const
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


