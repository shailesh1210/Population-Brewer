#ifndef __PersonPums_h__
#define __PersonPums_h__

#include <iostream>
#include <memory>
#include <map>
#include <list>
#include <vector>
#include <sstream>
#include <boost/tokenizer.hpp>

class Parameters;

class PersonPums
{
public:
	typedef std::map<std::string, int> Map;
	typedef std::multimap<int, Map> MultiMapCB;

	typedef boost::tokenizer<boost::char_separator<char>>TokenizerString;
	typedef std::vector<std::string> Column;
	typedef std::list<Column> Row;

	PersonPums(MultiMapCB);
	virtual ~PersonPums();

	void setDemoCharacters(std::string, std::string, std::string, std::string, std::string, std::string);
	void setSocialCharacters(std::string, std::string);
	
	double getPUMSID() const;
	int getPumaCode() const;
	int getAge() const;
	
	int getSex() const;
	int getRace() const;
	int getEthnicity() const;
	int getOrigin() const;
	int getEducation() const;
	int getEduAgeCat() const;
	int getMaritalStatus() const;
	int getMaritalAgeCat() const;

private:

	void setAge(int);
	void setSex(int);
	void setEthnicity(int);
	void setRace(int);
	void setOrigin();
	void setEducation(int);
	void setEduAgeCat();
	void setMaritalStatus(int);
	void setMaritalAgeCat();

	template<class T>
	T to_number(const std::string &);
	bool is_number(const std::string);

	MultiMapCB m_acsCodeBook;
	int pumaCode;
	double personID;
	int age, sex;
	int race, ethnicity, originByRace; 
	int education, eduAgeCat;
	int maritalStatus, maritalAgeCat;
};

#endif __PersonPums_h__
