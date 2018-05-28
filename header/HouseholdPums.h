#ifndef __HouseholdPums_h__
#define __HouseholdPums_h__

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include "PersonPums.h"

class HouseholdPums
{
public:
	HouseholdPums(const std::map<std::string, int>&, const std::map<std::string, int>&);
	virtual ~HouseholdPums();

	void setPUMA(std::string);
	void setHouseholds(std::string, std::string, std::string, std::string);
	void addPersons(PersonPums*);

	int getPUMA() const;
	double getHouseholdIndex() const;
	int getHouseholdType() const;
	int getHouseholdSize() const;
	int getHouseholdIncome() const;
	std::vector<PersonPums> getPersons() const;

private:

	void setHouseholdType(int);
	void setHouseholdSize(int);
	void setHouseholdIncome(int);

	template<class T>
	T to_number(const std::string &);
	bool is_number(const std::string);

	int puma;
	double hhIdx;
	int hhType;
	int hhSize;
	int hhIncome, hhIncomeCat;

	std::vector<PersonPums> hhPersons;
	std::map<std::string, int> m_householdType, m_householdIncome;
};

#endif __HouseholdPums_h__