#include "IPF.h"
#include "NDArray.h"
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <boost\tokenizer.hpp>
#include <fstream>
#include <numeric>

#define EDU_CAT 7
#define RACE_CAT 7
#define AGE_CAT 5

namespace type
{
	typedef std::vector<std::string> Column;
	typedef std::vector<Column> Row;
	typedef boost::tokenizer<boost::escaped_list_separator<char>> TokenizerCSVFile;
}

type::Row readCSVFile(std::ifstream &file);

int main()
{
	std::ifstream file1, file2, file3;
	file1.open("input\\baton_rouge_15\\sex_by_age\\age_marginal.csv", std::ios::in);
	if(!file1.is_open()){
		std::cout << "Cannot open File1!" << std::endl;
		exit(EXIT_SUCCESS);
	}
		
	file2.open("input\\baton_rouge_15\\sex_by_age\\gender_marginal.csv", std::ios::in);
	if(!file2.is_open()){
		std::cout << "Cannot open File2!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	file3.open("input\\baton_rouge_15\\sex_by_age\\seed.csv", std::ios::in);
	if(!file3.is_open()){
		std::cout << "Cannot open Seed matrix file!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	//extracts education marginals from Education file
	type::Row eduFile = readCSVFile(file1);
	eduFile.erase(eduFile.begin(), eduFile.begin()+1);
	std::vector<double>allEduMarginals;
	for(auto it1 = eduFile.begin(); it1 != eduFile.end(); ++it1)
		allEduMarginals.push_back(std::stod(it1->back()));

	//double totEduMarginals = std::accumulate(eduMarginals.begin(), eduMarginals.end(), 0);

	//extracts race marginals from Race File
	type::Row raceFile = readCSVFile(file2);
	raceFile.erase(raceFile.begin(), raceFile.begin()+1);
	std::vector<double>allRaceMarginals;
	for(auto it2 = raceFile.begin(); it2 != raceFile.end(); ++it2)
		allRaceMarginals.push_back(std::stod(it2->back()));

	//extract seed matrix
	type::Row seedMatrixFile = readCSVFile(file3);
	seedMatrixFile.erase(seedMatrixFile.begin(), seedMatrixFile.begin()+1);
	std::vector<double>allSeedVec;
	for(auto row = seedMatrixFile.begin(); row != seedMatrixFile.end(); ++row){
		for(auto col = row->begin()+1; col != row->end(); ++col)
			allSeedVec.push_back(std::stod(*col));
	}

	std::vector<std::vector<double>>marginals;
	marginals.push_back(allRaceMarginals);
	marginals.push_back(allEduMarginals);
	

	std::vector<int>m_size;
	m_size.push_back(marginals[0].size());
	m_size.push_back(marginals[1].size());

	NDArray<double>seedArr(m_size);
	seedArr.assign(allSeedVec);
	deprecated::IPF ipf(seedArr, marginals);
	ipf.solve(seedArr);

	
	/*int fromElems = 0;
	int toElems = 0;

	std::map<int, std::string>ageMap;
	ageMap.insert(std::make_pair(0, "Male, Age 18-24"));
	ageMap.insert(std::make_pair(1, "Male, Age 25-34"));
	ageMap.insert(std::make_pair(2, "Male, Age 35-44"));
	ageMap.insert(std::make_pair(3, "Male, Age 45-64"));
	ageMap.insert(std::make_pair(4, "Male, Age 65+"));

	ageMap.insert(std::make_pair(5, "Female, Age 18-24"));
	ageMap.insert(std::make_pair(6, "Female, Age 25-34"));
	ageMap.insert(std::make_pair(7, "Female, Age 35-44"));
	ageMap.insert(std::make_pair(8, "Female, Age 45-64"));
	ageMap.insert(std::make_pair(9, "Female, Age 65+"));

	for(size_t ageCat = 0; ageCat < 2*AGE_CAT; ++ageCat)
	{
		fromElems = ageCat*EDU_CAT;
		toElems = fromElems+EDU_CAT;
		std::vector<double>eduMarginal(allEduMarginals.begin()+fromElems, allEduMarginals.begin()+toElems);
		double popByEducation = std::accumulate(eduMarginal.begin(), eduMarginal.end(), 0);

		fromElems = ageCat*RACE_CAT;
		toElems = fromElems+RACE_CAT;
		std::vector<double>raceMarginal(allRaceMarginals.begin()+fromElems, allRaceMarginals.begin()+toElems);
		double popByRace = std::accumulate(raceMarginal.begin(), raceMarginal.end(), 0);

		if(popByEducation != popByRace){
			if(popByEducation > popByRace){
				double sumRace = 0;
				for(size_t i = 0; i < raceMarginal.size(); ++i){
					raceMarginal[i] = raceMarginal[i]*popByEducation/popByRace;
					sumRace += raceMarginal[i];
				}
				popByRace = sumRace;
			}
			else{
				double sumEdu = 0;
				for(size_t j = 0; j < eduMarginal.size(); ++j){
					eduMarginal[j] = eduMarginal[j]*popByRace/popByEducation;
					sumEdu += eduMarginal[j];
				}
				popByEducation = (int)sumEdu;
			}
		}

		std::vector<std::vector<double>>marginals;
		marginals.push_back(eduMarginal);
		marginals.push_back(raceMarginal);

		std::vector<int>m_size;
		m_size.push_back(marginals[0].size());
		m_size.push_back(marginals[1].size());

		fromElems = ageCat*EDU_CAT*RACE_CAT;
		toElems = fromElems+EDU_CAT*RACE_CAT;

		std::cout << "Starting IPF for " << ageMap[ageCat] << ".....\n" << std::endl;
		std::vector<double>seed(allSeedVec.begin()+fromElems, allSeedVec.begin()+toElems);
		NDArray<double>seedArr(m_size);
		seedArr.assign(seed);
		deprecated::IPF ipf(seedArr, marginals);
		ipf.solve(seedArr);
		
		std::cout << std::endl;

		eduMarginal.clear();
		raceMarginal.clear();
		marginals.clear();
		m_size.clear();
		seed.clear();
	}*/
	
	return 0;
}

type::Row readCSVFile(std::ifstream &file)
{
	type::Column col;
	type::Row row;

	std::string line;
	while(std::getline(file, line))
	{
		type::TokenizerCSVFile token(line);
		col.assign(token.begin(), token.end());
		row.push_back(col);
	}
	return row;
}

