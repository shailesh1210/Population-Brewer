#include "PersonPums.h"
#include "ACS.h"
#include "Parameters.h"


PersonPums::PersonPums(std::shared_ptr<Parameters>param) : parameters(param)
{

}


PersonPums::~PersonPums()
{
	
}


void PersonPums::setDemoCharacters(std::string p_puma, std::string p_idx, std::string p_age, std::string p_sex, std::string p_eth, std::string p_race)
{
	personID = to_number<double>(p_idx);
	pumaCode = to_number<int>(p_puma);

	setAge(to_number<short int>(p_age));
	setSex(to_number<short int>(p_sex));
	setEthnicity(to_number<short int>(p_eth));
	setRace(to_number<short int>(p_race));
}

void PersonPums::setSocialCharacters(std::string p_education, std::string p_marital)
{
	setEduAgeCat();
	setEducation(to_number<short int>(p_education));
}

void PersonPums::setAge(short int p_age)
{
	this->age = p_age;
	Map ageMap = parameters->getACSCodeBook().find(ACS::PumsVar::AGEP)->second;

	std::string mid_key = std::to_string(1+ageMap.size()/2);

	if(age <= ageMap[mid_key])
		setAgeCat(ageMap, ACS::AgeCat::Age_0_4, ACS::AgeCat::Age_40_44);
	else if(age > ageMap[mid_key])
		setAgeCat(ageMap, ACS::AgeCat::Age_45_49, ACS::AgeCat::Age_85_100);
}

void PersonPums::setAgeCat(Map &ageMap, short int ageLowLim, short int ageUpLim)
{
	for(short int i = ageLowLim; i <= ageUpLim; ++i)
	{
		if(age <= ageMap[std::to_string(i)])
		{
			this->ageCat = i;
			break;
		}
	}
}

void PersonPums::setSex(short int p_sex)
{
	this->sex = p_sex;
}

void PersonPums::setEthnicity(short int p_ethnicity)
{
	this->ethnicity = p_ethnicity;
	if(p_ethnicity != ACS::Ethnicity::Not_Hispanic)
	{
		this->ethnicity = ACS::Ethnicity::Hispanic;
	}
	else
	{
		this->ethnicity = p_ethnicity;
	}
}

void PersonPums::setRace(short int p_race)
{
	Map raceMap = parameters->getACSCodeBook().find(ACS::PumsVar::RAC1P)->second;

	if(p_race == raceMap.at("White alone") || p_race == raceMap.at("Black alone")){
		this->race = p_race;
	}
	else if(p_race >= raceMap.at("American Indian alone") && p_race <= raceMap.at("American Indian & Alaska Native")){
		this->race = ACS::Race::American_Indian_Alaska_Native;
	}
	else if(p_race == raceMap.at("Asian alone")){
			this->race = ACS::Race::Asian;
	}
	else if(p_race == raceMap.at("Native Hawaiian & Pacific Islander")){
			this->race = ACS::Race::Hawaiian_Pacific;
	}
	else if(p_race == raceMap.at("Some other")){
			this->race = ACS::Race::Some_Other;
	}
	else{
			this->race = ACS::Race::Two_Or_More;
	}

	setOrigin();
}

void PersonPums::setOrigin()
{
	
	if(ethnicity == ACS::Ethnicity::Not_Hispanic)
	{
		switch(race)
		{
		case ACS::Race::White:
			this->originByRace = ACS::Origin::WhiteNH;
			break;
		case ACS::Race::Black:
			this->originByRace = ACS::Origin::BlackNH;
			break;
		case ACS::Race::American_Indian_Alaska_Native:
			this->originByRace = ACS::Origin::AmericanAlaskanNH;
			break;
		case ACS::Race::Asian:
			this->originByRace = ACS::Origin::AsianNH;
			break;
		case ACS::Race::Hawaiian_Pacific:
			this->originByRace = ACS::Origin::HawaiianNH;
			break;
		case ACS::Race::Some_Other:
			this->originByRace = ACS::Origin::SomeOtherNH;
			break;
		case ACS::Race::Two_Or_More:
			this->originByRace = ACS::Origin::TwoNH;
			break;
		default:
			break;
		}
	}
	else
	{
		this->originByRace = ACS::Origin::Hisp;
	}

}

void PersonPums::setEducation(short int p_education)
{
	Map eduMap = parameters->getACSCodeBook().find(ACS::PumsVar::SCHL)->second;

	if(p_education < eduMap.at("Grade 9")){
		this->education = ACS::Education::Less_9th_Grade;
	}
	else if(p_education >= eduMap.at("Grade 9") && p_education <= eduMap.at("12th grade")){
		this->education = ACS::Education::_9th_To_12th_Grade;
	}
	else if(p_education == eduMap.at("High School") || p_education == eduMap.at("GED")){
		this->education = ACS::Education::High_School;
	}
	else if(p_education == eduMap.at("Some college-Less than a year") || p_education == eduMap.at("Some College-More than a year")){
		this->education = ACS::Education::Some_College;
	}
	else if(p_education == eduMap.at("Associate's degree")){
		this->education = ACS::Education::Associate_Degree;
	}
	else if(p_education == eduMap.at("Bachelor's degree")){
		this->education = ACS::Education::Bachelors_Degree;
	}
	else{
		this->education = ACS::Education::Graduate_Degree;
	}
}

void PersonPums::setEduAgeCat()
{

	if(age >= 18 && age <= 24){
		this->eduAgeCat = ACS::EduAgeCat::Age_18_24;
	}
	else if(age >= 25 && age <= 34){
		this->eduAgeCat = ACS::EduAgeCat::Age_25_34;
	}
	else if(age >= 35 && age <= 44){
		this->eduAgeCat = ACS::EduAgeCat::Age_35_44;
	}
	else if(age >= 45 && age <= 64){
		this->eduAgeCat = ACS::EduAgeCat::Age_45_64;
	}
	else if(age >= 65){
		this->eduAgeCat = ACS::EduAgeCat::Age_65_Over;
	}
	else {
		this->eduAgeCat = -1;
	}
}


double PersonPums::getPUMSID() const
{
	return personID;
}

int PersonPums::getPumaCode() const
{
	return pumaCode;
}

short int PersonPums::getAge() const
{
	return age;
}

short int PersonPums::getAgeCat() const
{
	return ageCat;
}

short int PersonPums::getSex() const
{
	return sex;
}

short int PersonPums::getRace() const
{
	return race;
}

short int PersonPums::getEthnicity() const
{
	return ethnicity;
}

short int PersonPums::getOrigin() const
{
	return originByRace;
}

short int PersonPums::getEducation() const
{
	return education;
}

short int PersonPums::getEduAgeCat() const
{
	return eduAgeCat;
}


template<class T>
T PersonPums::to_number(const std::string &data)
{
	if(!is_number(data) || data.empty())
		return -1;

	std::istringstream ss(data);
	T num;
	ss >> num;
	return num;
}

bool PersonPums::is_number(const std::string data)
{
	char *end = 0;
	double val = std::strtod(data.c_str(), &end);
	bool flag = (end != data.c_str() && val != HUGE_VAL);
	return flag;
}




