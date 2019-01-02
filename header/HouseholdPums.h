#ifndef __HouseholdPums_h__
#define __HouseholdPums_h__

#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <map>
#include <cmath>

class Parameters;
class PersonPums;

class HouseholdPums
{
public:
	
	HouseholdPums(std::shared_ptr<Parameters>);
	virtual ~HouseholdPums();

	void setPUMA(std::string);
	void setHouseholds(std::string, std::string, std::string, std::string);
	void addPersons(PersonPums);

	int getPUMA() const;
	double getHouseholdIndex() const;
	short int getHouseholdType() const;
	short int getHouseholdSize() const;
	int getHouseholdIncome() const;
	short int getHouseholdIncCat() const;
	short int getHHTypeBySize() const;
	std::vector<PersonPums> getPersons() const;
	void clearPersonList();
	
private:

	void setHouseholdType(short int);
	void setHouseholdSize(short int);
	void setHouseholdIncome(int);

	template<class T>
	T to_number(const std::string &);
	bool is_number(const std::string);

	std::shared_ptr<Parameters> parameters;
	int puma;
	double hhIdx;
	short int hhSize, hhType, hhIncomeCat;
	int hhIncome;

	std::vector<PersonPums> hhPersons;
};

#endif __HouseholdPums_h__