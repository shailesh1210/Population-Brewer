#ifndef __Parameters_h__
#define __Parameters_h__

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <boost/tokenizer.hpp>
#include "ACS.h"

class Parameters
{
public:
	typedef std::vector<std::string> Columns;
	typedef std::vector<Columns> Rows;
	typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
	typedef std::multimap<std::string, Columns> MultiMapCSV;

	typedef std::map<std::string, int> Map;
	typedef std::multimap<int, Map> MultiMapCB;

	Parameters(const char*, const char*);

	virtual ~Parameters();

	std::string getInputDir() const;
	std::string getOutputDir() const;
	
	const char* getMSAListFile() const;
	const char* getCountiesListFile() const;
	const char* getPUMAListFile() const;
	const char* getHouseholdPumsFile() const;
	const char* getPersonPumsFile() const;

	std::string getAgeMarginalFile() const;
	std::string getSexMarginalFile() const;

	const char* getRaceMarginalFile() const;
	const char* getEducationMarginalFile() const;
	const char* getMaritalMarginalFile() const;

	const char* getHHTypeMarginalFile() const;
	const char* getHHSizeMarginalFile() const;
	const char* getHHIncomeMarginalFile() const;

	MultiMapCB getACSCodeBook() const;

	std::unordered_multimap<int, int> getAgeGenderMapping(int) const;
	std::unordered_map<std::string, int> getOriginMapping() const;
	

private:

	void readACSCodeBookFile();
	void readAgeGenderMappingFile();
	void readOriginListFile();

	Map createCodeBookMap(ACS::PumsVar);
	const char* getFilePath(char *) const;

	Rows readCSVFile(const char*);

	const char *inputDir;
	const char *outputDir;

	MultiMapCSV m_codeBook;
	MultiMapCB m_acsCodes;

	std::unordered_multimap<int, int> m_eduAgeGender;
	std::unordered_multimap<int, int> m_maritalAgeGender;
	std::unordered_map<std::string, int> m_originByRace;

};
#endif __Parameters_h__