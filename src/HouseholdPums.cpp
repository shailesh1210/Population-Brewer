#include "HouseholdPums.h"
#include "ACS.h"

HouseholdPums::HouseholdPums(const std::map<std::string, int>&m_hhType, const std::map<std::string, int> &m_hhIncome) :
	m_householdType(m_hhType), m_householdIncome(m_hhIncome)
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

	setHouseholdType(to_number<int>(hh_type));
	setHouseholdSize(to_number<int>(hh_size));
	setHouseholdIncome(to_number<int>(hh_income));
}

void HouseholdPums::setHouseholdType(int type)
{
	if(type == m_householdType.at("Male householder-living alone-nonfamily") || type == m_householdType.at("Female householder-living alone-nonfamily")){
		this->hhType = ACS::HHType::LivingAloneNonFam;
	}
	else if(type == m_householdType.at("Male householder-not living alone-nonfamily") || type == m_householdType.at("Female householder-not living alone-nonfamily")){
		this->hhType = ACS::HHType::NotLivingAloneFam;
	}
	else{
		this->hhType = type;
	}
}

void HouseholdPums::setHouseholdSize(int size)
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

void HouseholdPums::addPersons(PersonPums* person)
{
	hhPersons.push_back(*person);
}

int HouseholdPums::getPUMA() const
{
	return puma;
}

double HouseholdPums::getHouseholdIndex() const
{
	return hhIdx;
}

int HouseholdPums::getHouseholdType() const
{
	return hhType;
}

int HouseholdPums::getHouseholdSize() const
{
	return hhSize;
}

int HouseholdPums::getHouseholdIncome() const
{
	return hhIncome;
}

std::vector<PersonPums> HouseholdPums::getPersons() const
{
	return hhPersons;
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