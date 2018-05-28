#ifndef __Counter_h__
#define __Counter_h__

#include <iostream>
#include "PersonPums.h"
#include "Agent.h"

namespace Counter
{
	struct OriginBySexByAge
	{
		int sex, origin, ageCat;
	
		OriginBySexByAge(int p_sex, int p_origin, int p_ageCat) : sex(p_sex), origin(p_origin), ageCat(p_ageCat){}
		bool operator()(Agent &a){
			return (a.getGender() == sex && a.getRace() == origin && a.getAgeCat() == ageCat);
		}
	};

	struct EduByOrigin
	{
		int sex, origin, edu, eduAge;
	
		EduByOrigin(int p_sex, int p_origin, int p_edu, int p_eduAge) : sex(p_sex), origin(p_origin), edu(p_edu), eduAge(p_eduAge){}
		bool operator()(PersonPums &p){
			return (p.getSex() == sex && p.getOrigin() == origin && p.getEducation() == edu && p.getEduAgeCat() == eduAge);
		}
		bool operator()(Agent &a){
			return (a.getGender() == sex && a.getRace() == origin && a.getEducation() == edu && a.getEduAgeCat() == eduAge);
		}
	};

	struct MaritalStatusByOrigin
	{
		int sex, origin, maritalStatus, maritalAgeCat;
		MaritalStatusByOrigin(int p_sex, int p_origin, int p_marital, int p_maritalAge) : sex(p_sex), origin(p_origin), 
			maritalStatus(p_marital), maritalAgeCat(p_maritalAge){}

		bool operator()(Agent&a){
			return (a.getGender() == sex && a.getRace() == origin && a.getMaritalStatus() == maritalStatus && a.getMaritalAgeCat() == maritalAgeCat);
		}
	};

	struct EduByRace
	{
		int sex, race, edu, eduAge;
	
		EduByRace(int p_sex, int p_race, int p_edu, int p_eduAge) : sex(p_sex), race(p_race), edu(p_edu), eduAge(p_eduAge){}
		bool operator()(PersonPums &p){
			return (p.getSex() == sex && p.getRace() == race && p.getEducation() == edu && p.getEduAgeCat() == eduAge);
		}
	};

	struct EduByOriginByPUMA
	{
		int sex, eduAge, origin, edu, pumaCode;
	
		EduByOriginByPUMA(int p_sex, int p_eduAge,int p_origin, int p_edu,int p_puma) : sex(p_sex), eduAge(p_eduAge), origin(p_origin), edu(p_edu), pumaCode(p_puma){}
		bool operator()(PersonPums &p){
			return (p.getSex() == sex && p.getOrigin() == origin && p.getEducation() == edu && p.getEduAgeCat() == eduAge && p.getPumaCode() == pumaCode);
		}
	};

	struct MaritalStatusByPUMA
	{
		int sex, maritalAge, origin, maritalStatus, pumaCode;
		MaritalStatusByPUMA(int p_sex, int p_maritalAge, int p_origin, int p_marital, int p_puma) : 
			sex(p_sex), maritalAge(p_maritalAge), origin(p_origin), maritalStatus(p_marital), pumaCode(p_puma){}

		bool operator() (PersonPums &p){
			return (p.getSex() == sex && p.getOrigin() == origin && p.getMaritalStatus() == maritalStatus && p.getMaritalAgeCat() == maritalAge && p.getPumaCode() == pumaCode);
		}
	};
	
}
#endif __Counter_h__