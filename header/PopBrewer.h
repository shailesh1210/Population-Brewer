#ifndef __PopBrewer_h__
#define __PopBrewer_h__

#include <iostream>
#include <fstream>
#include <memory>
#include <iomanip>
#include <list>
#include <vector>
#include <map>
#include <boost/tokenizer.hpp>
#include <unordered_map>

class Parameters;
class Metro;
class County;

class PopBrewer
{
public:

	typedef boost::tokenizer<boost::escaped_list_separator<char>> TokenizerCSVFile;
	typedef std::vector<std::string> Columns;
	typedef std::list<Columns> Rows;
	typedef std::vector<double> Marginal;
	
	PopBrewer(Parameters *);
	virtual ~PopBrewer();

	void generate();

private:
	std::shared_ptr<Parameters>parameters;
	void createMetroArea();

	void importEstimates();

	void importRaceEstimates();
	void importEducationEstimates();
	void importMaritalStatusEstimates();

	void importHHTypeEstimates();

	void setEstimates(const Rows &, const std::map<int, int>&, const size_t, int);

	Rows readCSVFile(const char*);
	
	void mapMetroToCounties(const char*, std::multimap<std::string, std::string>&);
	void mapCountiesToPUMA(const char*, std::multimap<std::string, County>&);

	template<class T>
	std::map<int, int> getColumnIndexMap(std::list<T>*, Columns *);
	int getColumnIndex(Columns *, std::string);
	
	std::map<std::string, Metro> metroAreas;
	
};

#endif __PopBrewer_h__