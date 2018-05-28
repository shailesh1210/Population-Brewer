#include "PopBrewer.h"
#include "Parameters.h"
#include "ACS.h"
#include "County.h"
#include "Metro.h"
#include "csv.h"


PopBrewer::PopBrewer(Parameters *param) : parameters(param)
{
}

PopBrewer::~PopBrewer()
{
}

void PopBrewer::generate()
{
	createMetroArea();
	importEstimates();
	
	Metro *curMSA = &metroAreas.at("10180");
	curMSA->createAgents();
	//importPUMS(curMSA);
}

void PopBrewer::createMetroArea()
{
	std::cout << "Creating Metropolitan Statistical Areas..." << std::endl;
	//mapping between metro area and their respective counties 
	std::multimap<std::string, std::string> msa_county_map;
	mapMetroToCounties(parameters->getCountiesListFile(), msa_county_map);

	//mapping between counties and their respective puma codes
	std::multimap<std::string, County> county_puma_map;
	mapCountiesToPUMA(parameters->getPUMAListFile(), county_puma_map);

	io::CSVReader<3> msaList(parameters->getMSAListFile());
	msaList.read_header(io::ignore_extra_column, "GEO_ID_2", "MSA_NAME", "TOT_POP");

	std::string geoID = "";
	std::string msaName = "";
	std::string totPop = "";

	int num_counties = 0;

	while(msaList.read_row(geoID, msaName, totPop))
	{
		Metro *metro = new Metro(parameters);
		metro->setMetroIDandName(geoID, msaName);
		metro->setPopulation(std::stoi(totPop));

		auto range_msa = msa_county_map.equal_range(msaName);
		if(range_msa.first == range_msa.second){
			std::cout << msaName << " doesn't exist in County List File!" << std::endl;
			exit(EXIT_SUCCESS);
		}
		for(auto countyByMSA = range_msa.first ; countyByMSA != range_msa.second; ++countyByMSA)
		{
			std::string county_msa = countyByMSA->second;
			auto range_county = county_puma_map.equal_range(county_msa);
			if(range_county.first == range_county.second){
				std::cout << county_msa << " of " << msaName << " doesn't exist in PUMA List file! " << std::endl;
				exit(EXIT_SUCCESS);
			}

			for(auto pumaByCounty = range_county.first; pumaByCounty != range_county.second; ++pumaByCounty)
					metro->setCounties(pumaByCounty->second);

			num_counties += (metro->getCountyNum(county_msa) >=1) ? 1 : 0;

			if(metro->getCountyNum(county_msa) == 0)
				std::cout << "No counties in " << msaName << " !" << std::endl;

			county_puma_map.erase(county_msa);
		}


		msa_county_map.erase(msaName);
		//metroList.push_back(*metro);
		metroAreas.insert(std::make_pair(metro->getGeoID(), *metro));
		delete metro;
	}

	county_puma_map.clear();
	msa_county_map.clear();

	std::cout << "Successfully Created!\n" << std::endl;
}

void PopBrewer::importEstimates()
{
	importRaceEstimates();
	importEducationEstimates();
	importMaritalStatusEstimates();

	importHHTypeEstimates();
}

void PopBrewer::importRaceEstimates()
{
	std::cout << "Importing Race Estimates by Age and Sex for MSAs...." << std::endl;
	
	Rows raceEst = readCSVFile(parameters->getRaceMarginalFile());
	Columns raceHdr = raceEst.front();
	raceEst.pop_front();

	std::list<ACS::RaceMarginalVar> raceVarsListMale;
	for(auto var : ACS::RaceMarginalVar::_values())
		raceVarsListMale.push_back(var);

	std::map<int, int> m_raceIdx(getColumnIndexMap(&raceVarsListMale, &raceHdr));
	setEstimates(raceEst, m_raceIdx, ACS::RaceMarginalVar::_size(), ACS::Estimates::estRace);
	
	std::cout << "Import Complete!\n" << std::endl;
}


void PopBrewer::importEducationEstimates()
{
	std::cout << "Importing Education Estimates by Age and Sex for MSAs..." << std::endl;
	Rows eduEst = readCSVFile(parameters->getEducationMarginalFile());
	Columns eduHdr = eduEst.front();
	eduEst.pop_front();

	std::list<ACS::EduMarginalVar1>eduVarListMale;
	for(auto val_male : ACS::EduMarginalVar1::_values())
		eduVarListMale.push_back(val_male);

	std::list<ACS::EduMarginalVar2> eduVarListFemale;
	for(auto val_female : ACS::EduMarginalVar2::_values())
		eduVarListFemale.push_back(val_female);

	std::map<int, int> m_eduIdx(getColumnIndexMap(&eduVarListMale, &eduHdr));
	std::map<int, int> m_female_eduIdx(getColumnIndexMap(&eduVarListFemale, &eduHdr));

	m_eduIdx.insert(m_female_eduIdx.begin(), m_female_eduIdx.end());
	setEstimates(eduEst, m_eduIdx, ACS::EduMarginalVar1::_size()+ACS::EduMarginalVar2::_size(), ACS::Estimates::estEducation);

	std::cout << "Import Complete!\n" << std::endl;
}

void PopBrewer::importMaritalStatusEstimates()
{
	std::cout << "Importing Marital Status Estimates by Age and Sex for MSAs..." << std::endl;

	Rows maritalEst = readCSVFile(parameters->getMaritalMarginalFile());
	Columns maritalHdr = maritalEst.front();
	maritalEst.pop_front();

	std::list<ACS::MaritalMarginalVar1> maritalVarListMale;
	for(auto val_male : ACS::MaritalMarginalVar1::_values())
		maritalVarListMale.push_back(val_male);

	std::list<ACS::MaritalMarginalVar2> maritalValListFemale;
	for(auto val_female : ACS::MaritalMarginalVar2::_values())
		maritalValListFemale.push_back(val_female);

	std::map<int, int> m_maritalIdx(getColumnIndexMap(&maritalVarListMale, &maritalHdr));
	std::map<int, int> m_female_maritalIdx(getColumnIndexMap(&maritalValListFemale, &maritalHdr));

	m_maritalIdx.insert(m_female_maritalIdx.begin(), m_female_maritalIdx.end());
	setEstimates(maritalEst, m_maritalIdx, ACS::MaritalMarginalVar1::_size()+ACS::MaritalMarginalVar2::_size(), ACS::Estimates::estMarital);

	std::cout << "Import Complete!\n" << std::endl;
}

void PopBrewer::importHHTypeEstimates()
{
	std::cout << "Importing Household Type Estimates for MSAs..." << std::endl;

	Rows hhTypeEst = readCSVFile(parameters->getHHTypeMarginalFile());
	Columns hhTypeHdr = hhTypeEst.front();
	hhTypeEst.pop_front();

	std::list<ACS::HHTypeMarginalVar> hhTypeVarList;
	for(auto val : ACS::HHTypeMarginalVar::_values())
		hhTypeVarList.push_back(val);
	
	std::map<int, int> m_hhTypeIdx(getColumnIndexMap(&hhTypeVarList, &hhTypeHdr));
	setEstimates(hhTypeEst, m_hhTypeIdx, ACS::HHTypeMarginalVar::_size(), ACS::Estimates::estHHType);

	std::cout << "Import Complete!\n" << std::endl;
}

void PopBrewer::setEstimates(const Rows &rows, const std::map<int, int>&m_idx, const size_t size, int type)
{
	for(auto curRow = rows.begin(); curRow != rows.end(); ++curRow)
	{
		std::string geoID = curRow->front();
		if(metroAreas.count(geoID) == 0)
			continue;

		Metro *metro = &metroAreas.at(geoID);

		Columns est;
		for(size_t i = 0; i < size; ++i)
			est.push_back(curRow->at(m_idx.at(i)));

		metro->setEstimates(est, type);
	}
}


PopBrewer::Rows PopBrewer::readCSVFile(const char *file)
{
	Columns col;
	Rows row;

	std::ifstream ifs;
	ifs.open(file, std::ios::in);

	std::string line;
	while(std::getline(ifs, line))
	{
		TokenizerCSVFile temp(line);
		col.assign(temp.begin(), temp.end());
		row.push_back(col);
	}

	return row;
}

void PopBrewer::mapMetroToCounties(const char* countyFile, std::multimap<std::string, std::string> &m_map)
{	
	//const int num_vars = 2;
	io::CSVReader<2> countyList(countyFile);
	countyList.read_header(io::ignore_extra_column, "MSA", "Counties");

	std::string metro = "";
	std::string county = "";
	while(countyList.read_row(metro, county))
		m_map.insert(std::make_pair(metro, county));

}

void PopBrewer::mapCountiesToPUMA(const char* pumaFile, std::multimap<std::string, County> &m_map)
{
	io::CSVReader<4> pumaList(pumaFile);
	pumaList.read_header(io::ignore_extra_column, "cntyname", "State", "PUMA", "Pop14");

	std::string county = "";
	std::string state = ""; 
	std::string puma_code = "";
	std::string population = "";

	std::multimap<std::string, County> temp_puma_county_map;

	while(pumaList.read_row(county, state, puma_code, population)){

		County tempCnty;
		tempCnty.setCountyName(county);
		tempCnty.setPumaCode(std::stoi(puma_code));
		tempCnty.setPopulation(std::stoi(population));

		temp_puma_county_map.insert(std::make_pair(state+puma_code, tempCnty));
	}

	for(auto it1 = temp_puma_county_map.begin(); it1 != temp_puma_county_map.end();)
	{
		auto puma_range = temp_puma_county_map.equal_range(it1->first);

		int sum_pop = 0;
		for(auto it2 = puma_range.first; it2 != puma_range.second; ++it2)
			sum_pop += it2->second.getPopulation();

		for(auto it3 = puma_range.first; it3 != puma_range.second; ++it3){
			double weight = ((double)it3->second.getPopulation()/sum_pop);
			it3->second.setPopulationWeight(weight);

			m_map.insert(std::make_pair(it3->second.getCountyName(), it3->second));
		}

		it1 = puma_range.second;
	}
}

template<class T>
std::map<int, int> PopBrewer::getColumnIndexMap(std::list<T> *varList, Columns *col)
{
	std::map<int, int> m_temp;
	for(auto it = varList->begin(); it != varList->end(); ++it)
	{
		std::string varStr = it->_to_string();
		int colIdx = getColumnIndex(col, varStr);

		if(colIdx < 0)
			exit(EXIT_SUCCESS);

		m_temp.insert(std::make_pair(it->_to_integral(), colIdx));
	}
	
	return m_temp;
}


int PopBrewer::getColumnIndex(Columns *col, std::string varName2)
{
	int idx = 0;
	for(auto itr = col->begin(); itr != col->end(); ++itr)
	{
		std::string varName1 = *itr;
		if(varName1 == varName2)
			return idx;
		idx++;
	}
	std::cout <<"Error: " << varName2 << " does not exist in the Column!" << std::endl;
	return -1;
}

