#ifndef __Agent_h__
#define __Agent_h__

class Agent
{
public:
	Agent();
	~Agent();

	void setHouseholdID(double);
	void setPUMA(int);
	void setAge(short int);
	void setSex(short int);
	void setOrigin(short int);
	void setEducation(short int);

	double getHouseholdID() const;
	int getPUMA() const;
	short int getAge() const;
	short int getSex() const;
	short int getOrigin() const;
	short int getEducation() const;

protected:
	double householdID;
	int puma;
	short int age, sex;
	short int origin, education;
};

#endif