#include "HouseholdPums.h"
#include "ACS.h"
#include "Parameters.h"
#include "PersonPums.h"

HouseholdPums::HouseholdPums(std::shared_ptr<Parameters> param) :
	parameters(param)
{
}

HouseholdPums::~HouseholdPums()
{
	
}

void HouseholdPums::setPUMA(std::string hh_puma)
{
	this->puma = to_number<int>(hh_puma);
}

void HouseholdPums::setHouseholds(std::string hh_idx, std::string hh_type, std::string hh_size, std::string hh_income)
{
	this->hhIdx = to_number<double>(hh_idx);

	setHouseholdSize(to_number<short int>(hh_size));
	setHouseholdType(to_number<short int>(hh_type));
	setHouseholdIncome(to_number<int>(hh_income));
}

void HouseholdPums::setHouseholdType(short int type)
{
	std::map<std::string, int> m_householdType, tempHHType;
	m_householdType = (parameters->getACSCodeBook().find(ACS::PumsVar::HHT)->second);
	
	if(type == m_householdType.at("Male householder-living alone-nonfamily") || type == m_householdType.at("Female householder-living alone-nonfamily")){
		this->hhType = ACS::HHType::NonFamily;
	}
	else if(type == m_householdType.at("Male householder-not living alone-nonfamily") || type == m_householdType.at("Female householder-not living alone-nonfamily")){
		this->hhType = ACS::HHType::NonFamily;
	}
	else{
		this->hhType = type;
	}
}

void HouseholdPums::setHouseholdSize(short int size)
{
	if(size >= ACS::HHSize::HHsize7){
		this->hhSize = ACS::HHSize::HHsize7;
	}
	else{
		this->hhSize = size;
	}
}

void HouseholdPums::setHouseholdIncome(int income)
{
	this->hhIncome = income;

	std::map<std::string, int> m_householdIncome(parameters->getACSCodeBook().find(ACS::PumsVar::HINCP)->second);
	
	for(auto incCat : ACS::HHIncome::_values())
	{
		if(income < 0){
			this->hhIncomeCat = -1;
			break;
		}
		std::string inc_str = incCat._to_string();
		if(income < m_householdIncome.at(inc_str)){
			this->hhIncomeCat = incCat;
			break;
		}
	}

}

void HouseholdPums::addPersons(PersonPums person)
{
	hhPersons.push_back(person);
}

int HouseholdPums::getPUMA() const
{
	return puma;
}

double HouseholdPums::getHouseholdIndex() const
{
	return hhIdx;
}

short int HouseholdPums::getHouseholdType() const
{
	return hhType;
}

short int HouseholdPums::getHouseholdSize() const
{
	return hhSize;
}

int HouseholdPums::getHouseholdIncome() const
{
	return hhIncome;
}

short int HouseholdPums::getHouseholdIncCat() const
{
	return hhIncomeCat;
}

short int HouseholdPums::getHHTypeBySize() const
{
	if(hhType > 0)
		return ((hhType-1)*ACS::HHSize::_size()+hhSize);
	else
		return -1;
}

std::vector<PersonPums> HouseholdPums::getPersons() const
{
	return hhPersons;
}

void HouseholdPums::clearPersonList()
{
	hhPersons.clear();
	hhPersons.shrink_to_fit();
}


template<class T>
T HouseholdPums::to_number(const std::string &data)
{
	if(!is_number(data) || data.empty())
		return -1;

	std::istringstream ss(data);
	T num;
	ss >> num;
	return num;
}

bool HouseholdPums::is_number(const std::string data)
{
	char *end = 0;
	double val = std::strtod(data.c_str(), &end);
	bool flag = (end != data.c_str() && val != HUGE_VAL);
	return flag;
}
