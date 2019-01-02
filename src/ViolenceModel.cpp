#include "ViolenceModel.h"
#include "PersonPums.h"
#include "HouseholdPums.h"
#include "Parameters.h"
#include "Counter.h"
#include "Metro.h"
#include "County.h"
#include "ACS.h"
#include "Random.h"
#include "ElapsedTime.h"

ViolenceModel::ViolenceModel() : schoolName("Stoneman HS")
{
	
}

ViolenceModel::~ViolenceModel()
{
	delete count;
	delete random;
}

void ViolenceModel::start()
{
	if(parameters != NULL)
	{
		count = new Counter(parameters);
		random = new Random;
	}
	else
	{
		std::cout << "Error: Parameters are not initialized!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	int num_trials = parameters->getViolenceParam()->num_trials;
	for(int i = 0; i < num_trials; ++i)
	{
		std::cout << "Simulation no: " << i+1 << std::endl;

		createPopulation();
		distributePtsdStatus();
		runModel();

		clearList();
	}

	if(parameters->writeToFile())
		count->output("miami");
}

void ViolenceModel::addHousehold(const HouseholdPums *hh, int countHH)
{
	if(pumaHouseholds.size() == 0)
		exit(EXIT_SUCCESS);

	int puma_code = hh->getPUMA();
	std::vector<PersonPums> tempPersons;
	int countPersons;

	if(hh->getHouseholdType() >= ACS::HHType::MarriedFam)
	{
		Household tempHH;
		tempHH.reserve(hh->getHouseholdSize());

		tempPersons = hh->getPersons();
		countPersons = 0;
		for(auto pp = tempPersons.begin(); pp != tempPersons.end(); ++pp)
		{
			ViolenceAgent *agent = new ViolenceAgent(parameters, &(*pp), random, count, countHH, countPersons);
			
			agent->setFriendSize(random->poisson_dist(getMeanFriendSize()));
			agent->setPTSDx(parameters->getPtsdSymptoms(), false);

			tempHH.push_back(*agent);

			countPersons++;
			delete agent;
		}

		if(pumaHouseholds.count(puma_code) > 0)
			pumaHouseholds[puma_code].push_back(tempHH);
	}
}

void ViolenceModel::addAgent(const PersonPums *p)
{
	//do nothing here
}

void ViolenceModel::createPopulation()
{
	//Metro *metro = &metroAreas.at("10180");
	Metro *metro = &metroAreas.at("33100");
	//countyMap = metro->getPumaCountyMap();
	
	initializeHouseholdMap(PARKLAND);

	std::cout << std::endl;
	std::cout << "Creating Population for " << metro->getMetroName() << std::endl;
	metro->createAgents(this);

	createSchool(&pumaHouseholds[PARKLAND]);
}

void ViolenceModel::distributePtsdStatus()
{
	distPrimaryPtsd();
	distSecondaryPtsd();
	distTertiaryPtsd();
}

void ViolenceModel::runModel()
{
	std::cout << std::endl;
	std::cout << "Running mass violence model..." << std::endl;
	int curTick = 0;
	int countPersons;
	PairDD prevalence;

	std::vector<Household>*households;
	for(auto map = pumaHouseholds.begin(); map != pumaHouseholds.end(); ++map)
	{
		households = &map->second;
		//count->reset();
		while(curTick < getMaxWeeks())
		{
			countPersons = 0;

			for(auto hh = households->begin(); hh != households->end(); ++hh)
			{
				for(auto pp = hh->begin(); pp != hh->end(); ++pp)
				{
					ViolenceAgent *agent = (&(*pp));
					if(agent->getAge() < getMinAge())
						continue;

					agent->excecuteRules(curTick);
					countPersons++;
				}
			}
			prevalence = count->getPrevalence(curTick, countPersons);
			count->computeOutcomes(curTick, countPersons);

			curTick++;
		
			std::cout << std::fixed;
			std::cout << "Current tick: " << curTick  << std::setprecision(4) 
				<< ", Prev(SC): " << prevalence.first <<", Prev(UC): " << prevalence.second
				<< " | " << "CBT: " << count->getCbtUptake(curTick-1) 
				<< "," << "SPR: " << count->getSprUptake(curTick-1) 
				<< "," << "ND: " << count->getNaturalDecayUptake(curTick-1) << std::endl;
		}
	}

}

/**
*	@brief Creates a population for High School based on its demographics. Households are
*	added to the list if they have agents who are less than 18 years old and attends high school
*	(9th to 12th grade).
*	@param households is vector containing list of households in designated PUMA area
*	@return void
*/
void ViolenceModel::createSchool(std::vector<Household>*households)
{
	MapInt schoolDemoMap = parameters->getSchoolDemographics();
	resizeHouseholds();
	//std::vector<Household> *households(&pumaHouseholds[puma_area]);
	std::random_shuffle(households->begin(), households->end());

	std::string student_type;
	int students_per_hh, num_teachers;
	num_teachers = getNumTeachers();

	int countPersons = 0;
	for(auto hh = households->begin(); hh != households->end(); ++hh)
	{
		students_per_hh = getNumStudents(*hh, countPersons);
		if(students_per_hh > 0)
		{
			std::vector<ViolenceAgent>prosp_students;
			for(auto pp = hh->begin(); pp != hh->end(); ++pp)
			{
				if(pp->isStudent())
				{
					student_type =std::to_string(pp->getSex())+std::to_string(pp->getOrigin());
					//m_students.insert(std::make_pair(student_type, *pp));
					if(schoolDemoMap.count(student_type) > 0)
					{
						int student_count = schoolDemoMap.at(student_type);
						if(student_count > 0)
						{
							prosp_students.push_back(*pp);
							student_count--;
							schoolDemoMap.at(student_type) = student_count;
						}
					}
				}
			}

			if(students_per_hh == prosp_students.size())
			{
				for(auto pp = hh->begin(); pp != hh->end(); ++pp)
				{
					if(pp->getAge() < getMinAge())
						continue;

					if(pp->isStudent())
					{
						pp->setSchoolName(schoolName);
						createAgentHashMap(&studentsMap, &(*pp), IN_SCHOOL_NETWORK);
					}
					else
					{
						createAgentHashMap(&othersMap, &(*pp), OUT_SCHOOL_NETWORK);
					}
				}
				schoolHouseholds.push_back(&(*hh));
			}
			else
			{
				for(auto pp = hh->begin(); pp != hh->end(); ++pp)
				{
					if(pp->getAge() >= getMinAge())
					{
						createAgentHashMap(&othersMap, &(*pp), OUT_SCHOOL_NETWORK);
					}
				}

				for(auto pp = prosp_students.begin(); pp != prosp_students.end(); ++pp)
				{
					student_type = std::to_string(pp->getSex())+std::to_string(pp->getOrigin());
					schoolDemoMap.at(student_type)++;
				}
			}
		}
		else
		{
			if(num_teachers == 0)
			{
				for(auto pp = hh->begin(); pp != hh->end(); ++pp)
				{
					if(pp->getAge() >= getMinAge())
					{
						createAgentHashMap(&othersMap, &(*pp), OUT_SCHOOL_NETWORK);
					}
				}
			}
			else
			{
				std::random_shuffle(hh->begin(), hh->end());
				int count_teacher = 0;
				for(auto pp = hh->begin(); pp != hh->end(); ++pp)
				{
					if(pp->getAge() < getMinAge())
						continue;

					if(pp->isTeacher() && count_teacher == 0)
					{
						pp->setSchoolName(schoolName);
						createAgentHashMap(&teachersMap, &(*pp), IN_SCHOOL_NETWORK);
						count_teacher++;
						//num_teachers--;
					}
					else
					{
						createAgentHashMap(&othersMap, &(*pp), OUT_SCHOOL_NETWORK);
					}
				}

				if(count_teacher > 0 && num_teachers > 0){
					schoolHouseholds.push_back(&(*hh));
					num_teachers--;
				}
			}
		}
	}

	schoolDemographics();
	createSocialNetwork(households, countPersons);
}

void ViolenceModel::createAgentHashMap(AgentListMap *agentsMap, ViolenceAgent *a, int network_type)
{
	std::string key = "";
	if(network_type == OUT_SCHOOL_NETWORK)
		key = std::to_string(a->getOrigin())+std::to_string(a->getEducation());
	else if(network_type == IN_SCHOOL_NETWORK)
		key = std::to_string(a->getOrigin());

	std::vector<ViolenceAgent*>tempAgentPtr;
	if(agentsMap->count(key) == 0)
	{
		tempAgentPtr.push_back(a);
		agentsMap->insert(std::make_pair(key, tempAgentPtr));
	}
	else
	{
		agentsMap->at(key).push_back(a);
	}
}


/**
*	@brief Creates a social network of friends and families for agents who are 14 years or older.
*	Matching is done based on age, race, gender and education of agents. Each agent is assigned a 
*	predefined friends size from a poisson distribution with a mean friend size of 3.0. Social network
*	is built for students, teachers and other agents. Students can have friends from same/different schools. 
*	Teachers can have friends from school or other agents. 
*	@param househoulds is vector containing list of households
*	@param totalPersons is total number of agents who are 14 years or older
*	@return void
*/
void ViolenceModel::createSocialNetwork(std::vector<Household>*households, int totalPersons)
{
	std::cout << std::endl;
	std::cout << "Building Social Network...\n" << std::endl;
	
	int count, draws;
	double pSelection, waitTime;
	count = 0;
	waitTime = 2000; //milliseconds

	ElapsedTime timer;
	for(auto hh = households->begin(); hh != households->end(); ++hh)
	{
		for(auto pp = hh->begin(); pp != hh->end(); ++pp)
		{
			ViolenceAgent *a = (&(*pp));
			if(a->getAge() < getMinAge())
				continue;

			draws = 0;
			while(a->getTotalFriends() < a->getFriendSize() && draws < getOuterDraws())
			{
				draws++;
				if(a->getSchoolName() == schoolName)
				{
					if(a->isStudent())
					{
						pSelection = random->uniform_real_dist();
						if(pSelection > getPval(STUDENT))
							findFriends(&studentsMap, a, IN_SCHOOL_NETWORK);
						else
							findFriends(&othersMap, a, OUT_SCHOOL_NETWORK);
					}
					else if(a->isTeacher())
					{
						pSelection = random->uniform_real_dist();
						if(pSelection > getPval(TEACHER))
							findFriends(&teachersMap, a, IN_SCHOOL_NETWORK);
						else
							findFriends(&othersMap, a, OUT_SCHOOL_NETWORK);
					}
				}
				else
				{
					findFriends(&othersMap, a, OUT_SCHOOL_NETWORK);
				}
			}

			timer.stop();
			if(timer.elapsed_ms() > waitTime)
			{
				std::cout << "Social network building progress: " << 100*(double)count/totalPersons << "% completed!" << std::endl;
				timer.start();
			}

			++count;
		}
	}

	std::cout << "Social network building progress: " << 100*(double)count/totalPersons << "% completed!\n" << std::endl;
	
	std::cout << "Total Pop (14 or older): " << totalPersons << std::endl;
	std::cout << std::endl;
	//networkAnalysis();
}


void ViolenceModel::findFriends(AgentListMap *agentsMap, ViolenceAgent *a, int network_type)
{
	bool ageMatch, genderMatch;
	double ageChoice, genderChoice;
	int origin, edu;
	std::string new_key, old_key;

	new_key = old_key = "";

	ViolenceAgent *b = NULL;

	bool match_found = false;
	int inner_draws = 0;
	while(!match_found)
	{
		inner_draws++;
		if(inner_draws >= getInnerDraws())
			break;

		origin = getOriginKey(a);
		edu = getEducationKey(a);

		if(origin < 0 || edu < 0)
			exit(EXIT_SUCCESS);
		
		if(network_type == IN_SCHOOL_NETWORK)
			new_key = std::to_string(origin);
		else if(network_type == OUT_SCHOOL_NETWORK)
			new_key = std::to_string(origin)+std::to_string(edu);

		if(old_key == new_key && !old_key.empty() && !new_key.empty())
			break;

		if(agentsMap->count(new_key) > 0)
		{
			std::random_shuffle(agentsMap->at(new_key).begin(), agentsMap->at(new_key).end());
			for(size_t i = 0; i < agentsMap->at(new_key).size(); ++i)
			{
				b = agentsMap->at(new_key).at(i);
				if(!(a->isCompatible(b)))
					continue;
			
				ageMatch = genderMatch = true;
				if(a->isStudent() || b->isStudent())
				{
					if(abs(a->getAge()-b->getAge()) > getAgeDiffStudents())
						ageMatch = false;
				}
				else
				{
					ageChoice = random->uniform_real_dist();
					if(ageChoice > getPval(AGE))
					{
						if(abs(a->getAge()-b->getAge()) > getAgeDiffOthers())
							ageMatch = false;
					}
				}

				genderChoice = random->uniform_real_dist();
				if(genderChoice > getPval(GENDER))
				{
					if(a->getSex() != b->getSex())
						genderMatch = false;
				}

				if(ageMatch && genderMatch)
				{
					a->setFriend(b);
					b->setFriend(a);

					match_found = true;
					break;
				}
			}

			old_key = new_key;
		}
	}
}


/**
*	@brief Draws directly affected agents (students and teachers) from list of students/teachers 
*	within school based on prevalence of affected agents and assigns PTSD status agents.
*	@param none
*	@return void
*/
void ViolenceModel::distPrimaryPtsd()
{
	std::cout << "Distributing primary PTSD status!" << std::endl;

	AgentListMap aff_students, aff_teachers;
	poolPrimaryRiskAgents(&studentsMap, &aff_students, STUDENT); 
	setPtsdStatus(&aff_students, 0, getPrevalence(PREVAL_STUDENTS), PRIMARY);
	
	poolPrimaryRiskAgents(&teachersMap, &aff_teachers, TEACHER);
	for(auto sex : ACS::Sex::_values())
	{
		int a_sex = sex._to_integral();
		double preval =  (a_sex == ACS::Sex::Male) ? getPrevalence(PREVAL_TEACHERS_MALE) :
			getPrevalence(PREVAL_TEACHERS_FEMALE);

		setPtsdStatus(&aff_teachers, sex, preval, PRIMARY);
	}

	std::cout << "At primary risk: " << primaryRiskPool.size() << std::endl;
	std::cout << "Distribution complete!\n" << std::endl;
}

/**
*	@brief Creates a pool of friends and families of agents affected by shooting and assigns	
*	secondary PTSD status to agents based on the prevalence of PTSD.
*	@param none
*	@return void
*/
void ViolenceModel::distSecondaryPtsd()
{
	std::cout << "Distributing Secondary PTSD status!" << std::endl;
	AgentListMap aff_fam_friends;
	AgentListPtr tempAgents;
	
	for(auto sex : ACS::Sex::_values())
		aff_fam_friends.insert(std::make_pair(std::to_string(sex), tempAgents));

	poolSecondaryRiskAgents(&aff_fam_friends);

	//assign secondary PTSD status to close friends and families of directly affected agents
	for(auto sex : ACS::Sex::_values())
	{
		int a_sex = sex._to_integral();
		double preval =  (a_sex == ACS::Sex::Male) ? getPrevalence(PREVAL_FAM_MALE) :
			getPrevalence(PREVAL_FAM_FEMALE);

		setPtsdStatus(&aff_fam_friends, sex, preval, SECONDARY);
	}

	std::cout << "At secondary risk: " << secondaryRiskPool.size() << std::endl;
	std::cout << "Distribution complete!\n" << std::endl;
}

void ViolenceModel::distTertiaryPtsd()
{
	std::cout << "Distributing Tertiary PTSD status!" << std::endl;

	AgentListMap community;
	AgentListPtr tempAgents;
	
	for(auto sex : ACS::Sex::_values())
		community.insert(std::make_pair(std::to_string(sex), tempAgents));

	poolTertiaryRiskAgents(&community);

	for(auto sex : ACS::Sex::_values())
	{
		int a_sex = sex._to_integral();
		double preval =  (a_sex == ACS::Sex::Male) ? getPrevalence(PREVAL_COMM_MALE) :
			getPrevalence(PREVAL_COMM_FEMALE);

		setPtsdStatus(&community, sex, preval, TERTIARY);
	}

	std::cout << "At Tertiary risk: " << tertiaryRiskPool.size() << std::endl;
	std::cout << "Distribution complete!\n" << std::endl;
}

void ViolenceModel::poolPrimaryRiskAgents(AgentListMap *agentsMap, AgentListMap *aff_agents, int agent_type)
{
	VecInts origins;
	for(auto map = agentsMap->begin(); map != agentsMap->end(); ++map)
		origins.push_back(std::stoi(map->first));

	int affected_count = 0;
	if(agent_type == STUDENT)
		affected_count = getAffectedStudents();
	else if(agent_type == TEACHER)
		affected_count = getAffectedTeachers();

	if(affected_count <= 0)
	{
		std::cout << "Error: Directly affected agents cannot be less than or equal to zero!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	int totalSize = 0;
	for(auto it = agentsMap->begin(); it != agentsMap->end(); ++it)
		totalSize += it->second.size();

	if(totalSize < affected_count)
	{
		std::cout << "Error: Insufficient agents in the container (total agents < affected agents)!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	while(affected_count > 0)
	{
		std::string rand_origin = std::to_string(origins[random->random_int(0, origins.size()-1)]);
		if(agentsMap->count(rand_origin) > 0)
		{
			std::random_shuffle(agentsMap->at(rand_origin).begin(), agentsMap->at(rand_origin).end());

			int size = agentsMap->at(rand_origin).size();
			int idx = random->random_int(0, size-1);

			ViolenceAgent *agent = agentsMap->at(rand_origin).at(idx);
			std::string key_gender = (agent_type == TEACHER) ? std::to_string(agent->getSex()) : "0";

			if(agent->isPrimaryRisk(&primaryRiskPool))
				continue;

			if(aff_agents->count(key_gender) > 0)
			{
				aff_agents->at(key_gender).push_back(agent);
			}
			else
			{
				AgentListPtr temp;
				temp.push_back(agent);
				aff_agents->insert(std::make_pair(key_gender, temp));
			}
			
			primaryRiskPool.insert(std::make_pair(agent->getAgentIdx(), true));
			affected_count--;
		}
	}
}

void ViolenceModel::poolSecondaryRiskAgents(AgentListMap *aff_fam_friends)
{
	std::string key_gender = "";
	for(auto hh = schoolHouseholds.begin(); hh != schoolHouseholds.end(); ++hh)
	{
		Household *household = *hh; 
		if(!isAffectedHousehold(household))
		{
			for(auto pp = household->begin(); pp != household->end(); ++pp)
			{
				ViolenceAgent *agent = (&(*pp));
				if(agent->getAge() < getMinAge())
					continue;

				key_gender = std::to_string(agent->getSex());
				if(!agent->isPrimaryRisk(&primaryRiskPool)  && !agent->isSecondaryRisk(&secondaryRiskPool))
				{
					if(agent->getSchoolName() == schoolName)
					{
						aff_fam_friends->at(key_gender).push_back(agent);
						secondaryRiskPool.insert(std::make_pair(agent->getAgentIdx(), true));
					}
				}
			}
		}
		else
		{
			for(auto pp = household->begin(); pp != household->end(); ++pp)
			{
				ViolenceAgent *agent = (&(*pp));
				if(agent->getAge() < getMinAge())
					continue;

				if(agent->isPrimaryRisk(&primaryRiskPool))
				{
					AgentListPtr *friendList = agent->getFriendList();
					for(auto ff = friendList->begin(); ff != friendList->end(); ++ff)
					{
						ViolenceAgent *frnd = *ff;
						key_gender = std::to_string(frnd->getSex());
						if(!frnd->isPrimaryRisk(&primaryRiskPool) && !frnd->isSecondaryRisk(&secondaryRiskPool))
						{
							aff_fam_friends->at(key_gender).push_back(frnd);
							secondaryRiskPool.insert(std::make_pair(frnd->getAgentIdx(), true));
						}
					}
				}
				else
				{
					key_gender = std::to_string(agent->getSex());
					if(!agent->isSecondaryRisk(&secondaryRiskPool))
					{
						aff_fam_friends->at(key_gender).push_back(agent);
						secondaryRiskPool.insert(std::make_pair(agent->getAgentIdx(), true));
					}
				}
			}
		}
	}
}

void ViolenceModel::poolTertiaryRiskAgents(AgentListMap *community)
{
	std::string key_gender = "";
	for(auto map = othersMap.begin(); map != othersMap.end(); ++map)
	{
		AgentListPtr agentList = map->second;
		for(auto agent = agentList.begin(); agent != agentList.end(); ++agent)
		{
			ViolenceAgent *a = *agent;
			key_gender = std::to_string(a->getSex());
			if(!a->isPrimaryRisk(&primaryRiskPool) && !a->isSecondaryRisk(&secondaryRiskPool))
			{
				community->at(key_gender).push_back(a);
				tertiaryRiskPool.insert(std::make_pair(a->getAgentIdx(), true));
			}
		}
	}
}

void ViolenceModel::schoolDemographics()
{
	std::multimap<std::string, bool> demoStudents, demoTeachers;
	for(auto hh = schoolHouseholds.begin(); hh != schoolHouseholds.end(); ++hh)
	{
		Household *person = *hh;
		for(auto pp = person->begin(); pp != person->end(); ++pp)
		{
			if(pp->getSchoolName() == schoolName)
			{
				if(pp->isStudent())
					demoStudents.insert(std::make_pair(std::to_string(pp->getSex())+std::to_string(pp->getOrigin()), true));
				else if(pp->isTeacher())
					demoTeachers.insert(std::make_pair(std::to_string(pp->getSex())+std::to_string(pp->getOrigin()), true));
			}
		}
	}

	std::cout << "School Demo: " << std::endl;
	for(auto sex : ACS::Sex::_values())
	{
		for(auto origin : ACS::Origin::_values())
		{
			std::string key_demo = std::to_string(sex)+std::to_string(origin);
			std::cout <<  sex << "," << origin << "," << demoStudents.count(key_demo)
				<<"," << demoTeachers.count(key_demo) << std::endl;
		}
		std::cout << std::endl;
	}
}


void ViolenceModel::networkAnalysis()
{
	typedef std::vector<ViolenceAgent *> FriendsPtr;

	std::cout << std::endl;
	std::cout << "Social Network Analysis: " << std::endl;
	int count = 0;
	std::vector<Household>*parkland(&pumaHouseholds[TAYLOR]);

	for(auto hh = parkland->begin(); hh != parkland->end(); ++hh)
	{
		for(auto pp1 = hh->begin(); pp1 != hh->end(); ++pp1)
		{
			ViolenceAgent *agent1 = &(*pp1);
			if(agent1->getTotalFriends() != agent1->getFriendSize())
				std::cout << ++count << "," << agent1->getAgentIdx() << "," << agent1->getTotalFriends() << "," << agent1->getFriendSize() << std::endl;

			FriendsPtr *friends = agent1->getFriendList();
			for(auto pp2 = friends->begin(); pp2 != friends->end(); ++pp2) 
			{
				ViolenceAgent *agent2 = *pp2;
				if(agent1->getAgentIdx() == agent2->getAgentIdx())
				{
					std::cout << "Error: Cannot have self as a friend!" << std::endl;
					exit(EXIT_SUCCESS);
				}

				if(agent1->getHouseholdID() == agent2->getHouseholdID())
				{
					std::cout << "Error: Cannot have family member as a friend!" << std::endl;
					exit(EXIT_SUCCESS);
				}

				if(agent1->isStudent())
				{
					if(abs(agent1->getAge()-agent2->getAge()) > getAgeDiffStudents())
						std::cout << agent1->getAgentIdx() << std::endl;
				}

			}
		}
	}

}

void ViolenceModel::initializeHouseholdMap(int puma_area)
{
	////including 1101-1105 puma regions
	//if(puma_area == PARKLAND)
	//{
	//	auto it = countyMap.find(1106);
	//	countyMap.erase(it, countyMap.end());
	//}

	//std::vector<Household>vecHouseholds;
	//for(auto it = countyMap.begin(); it != countyMap.end(); ++it)
	//{
	//	pumaHouseholds.insert(std::make_pair(it->first, vecHouseholds));
	//	pumaHouseholds[it->first].reserve(getMinHouseholdsPuma());
	//}

	std::vector<Household>vecHouseholds;
	pumaHouseholds.insert(std::make_pair(puma_area, vecHouseholds));
	pumaHouseholds[puma_area].reserve(getMinHouseholdsPuma());
}

void ViolenceModel::resizeHouseholds()
{
	for(auto it = pumaHouseholds.begin(); it != pumaHouseholds.end(); ++it)
		pumaHouseholds[it->first].shrink_to_fit();
}


bool ViolenceModel::isAffectedHousehold(Household *hh)
{
	bool affected = false;
	for(auto pp = hh->begin(); pp != hh->end(); ++pp)
	{
		ViolenceAgent *agent = &(*pp);
		if(agent->isPrimaryRisk(&primaryRiskPool))
		{
			affected = true;
			break;
		}
	}
	return affected;
}


Counter * ViolenceModel::getCounter() const
{
	return count;
}


int ViolenceModel::getNumStudents(const std::vector<ViolenceAgent>&agents, int &countPersons) const
{
	int num_students = 0; 
	for(auto pp = agents.begin(); pp != agents.end(); ++pp)
	{
		if(pp->isStudent())
			num_students++;

		if(pp->getAge() >= getMinAge())
			countPersons++;
	}

	return num_students;
}

int ViolenceModel::getOriginKey(const ViolenceAgent *a) const
{
	int origin = -1;
	double originChoice = random->uniform_real_dist();
	if(originChoice > getPval(ORIGIN))
	{
		origin = a->getOrigin();
	}
	else
	{
		origin = random->random_int(ACS::Origin::Hisp, ACS::Origin::TwoNH);
		if(origin == a->getOrigin())
		{
			while(true)
			{
				origin = random->random_int(ACS::Origin::Hisp, ACS::Origin::TwoNH);
				if(origin != a->getOrigin())
					break;
			}
		}
	}

	return origin;
}

int ViolenceModel::getEducationKey(const ViolenceAgent *a) const
{
	int edu = -1;
	double eduChoice = random->uniform_real_dist();
	if(eduChoice > getPval(EDUCATION))
	{
		edu = a->getEducation();
	}
	else
	{
		edu = random->random_int(ACS::Education::Less_9th_Grade, ACS::Education::Graduate_Degree);
		if(edu == a->getEducation())
		{
			while(true)
			{
				edu = random->random_int(ACS::Education::Less_9th_Grade, ACS::Education::Graduate_Degree);
				if(edu != a->getEducation())
					break;
			}
		}
	}

	return edu;
}

int ViolenceModel::getInnerDraws() const
{
	return parameters->getViolenceParam()->inner_draws;
}

int ViolenceModel::getOuterDraws() const
{
	return parameters->getViolenceParam()->outer_draws;
}

int ViolenceModel::getMinHouseholdsPuma() const
{
	return parameters->getViolenceParam()->min_households_puma;
}

int ViolenceModel::getMeanFriendSize() const
{
	return parameters->getViolenceParam()->mean_friends_size;
}

int ViolenceModel::getNumTeachers() const
{
	return parameters->getViolenceParam()->num_teachers;
}

int ViolenceModel::getMinAge() const
{
	return parameters->getViolenceParam()->min_age;
}

int ViolenceModel::getAgeDiffStudents() const
{
	return parameters->getViolenceParam()->age_diff_students;
}

int ViolenceModel::getAgeDiffOthers() const
{
	return parameters->getViolenceParam()->age_diff_others;
}

int ViolenceModel::getAffectedStudents() const
{
	return parameters->getViolenceParam()->aff_students;
}

int ViolenceModel::getAffectedTeachers() const
{
	return parameters->getViolenceParam()->aff_teachers;
}

int ViolenceModel::getMaxWeeks() const
{
	return parameters->getViolenceParam()->tot_steps;
}

double ViolenceModel::getPval(int type) const
{
	switch(type)
	{
	case STUDENT:
		return parameters->getViolenceParam()->p_val_student;
		break;
	case TEACHER:
		return parameters->getViolenceParam()->p_val_teacher;
		break;
	case ORIGIN:
		return parameters->getViolenceParam()->p_val_origin;
		break;
	case EDUCATION:
		return parameters->getViolenceParam()->p_val_edu;
		break;
	case AGE:
		return parameters->getViolenceParam()->p_val_age;
		break;
	case GENDER:
		return parameters->getViolenceParam()->p_val_gender;
		break;
	default:
		return -1;
		break;
	}
}

double ViolenceModel::getPrevalence(int p_type) const
{
	switch(p_type)
	{
	case PREVAL_STUDENTS:
		return parameters->getViolenceParam()->prev_students_tot;
		break;
	case PREVAL_TEACHERS_FEMALE:
		return  parameters->getViolenceParam()->prev_teachers_female;
		break;
	case PREVAL_TEACHERS_MALE:
		return parameters->getViolenceParam()->prev_teachers_male;
		break;
	case PREVAL_FAM_FEMALE:
		return parameters->getViolenceParam()->prev_fam_female;
		break;
	case PREVAL_FAM_MALE:
		return parameters->getViolenceParam()->prev_fam_male;
		break;
	case PREVAL_COMM_FEMALE:
		return parameters->getViolenceParam()->prev_comm_female;
		break;
	case PREVAL_COMM_MALE:
		return  parameters->getViolenceParam()->prev_comm_male;
		break;
	default:
		return -1;
		break;
	}
}


void ViolenceModel::setSize(int pop)
{
	//do nothing here
}

void ViolenceModel::setPtsdStatus(AgentListMap *affectedAgents, int sex, double preval, int type)
{
	if(preval < 0)
		exit(EXIT_SUCCESS);

	//MapDbl *m_ptsdx = parameters->getPtsdSymptoms();
	MapPair *m_ptsdx = parameters->getPtsdSymptoms();

	std::string key_sex = std::to_string(sex);
	size_t prev_count = boost::math::round(preval*affectedAgents->at(key_sex).size());
	while(prev_count > 0)
	{
		std::random_shuffle(affectedAgents->at(key_sex).begin(), affectedAgents->at(key_sex).end());

		int sample_size = affectedAgents->at(key_sex).size();
		int randIdx = random->random_int(0, sample_size-1);

		ViolenceAgent *affAgent = affectedAgents->at(key_sex).at(randIdx);
		if(affAgent->getPTSDstatus(type))
			continue;

		affAgent->setPTSDstatus(true, type);
		affAgent->setPTSDx(m_ptsdx, true);

		prev_count--;
	}
}

void ViolenceModel::clearList()
{
	for(auto it = pumaHouseholds.begin(); it != pumaHouseholds.end(); ++it)
	{
		it->second.clear();
		it->second.shrink_to_fit();
	}

	schoolHouseholds.clear();

	studentsMap.clear();
	teachersMap.clear();
	othersMap.clear();

	primaryRiskPool.clear();
	secondaryRiskPool.clear();
	tertiaryRiskPool.clear();
}

