/**
*	@file	 main.cpp	
*	@author	 Shailesh Tamrakar
*	@date	 7/23/2018
*	@version 1.0
*
*	@section DESCRIPTION
*	The main purpose of this program is to generate a synthetic population
*	to assess the impact of tax intervention (soda-tax) on Cardio-Vascular
*	Disease (CVD) among economically derived population across US MSAs.
*	It imports publicily available PUMS (Public Use Microdata Sample) 
*	dataset as well as ACS (American Community Survey) estimates of socio-
*	demographic variables as input parameters, followed by execution
*	of IPU algorithm to create a synthetic population by simultaneously 
*	matching both household and person-level attributes/estimates.
*	 
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Parameters.h"
#include "PopBrewer.h"
#include "CardioModel.h"
#include "ViolenceModel.h"
#include "csv.h"
#include "IPU.h"
#include "ACS.h"


int main(int argc, const char* argv[])
{
	std::cout << "***********************************************************************\n";
	std::cout << "************Welcome to Population Brewer*******************************\n";
	std::cout << "This software generates synthetic population for US Metropolitan Areas\n";
	std::cout << "***********************************************************************\n";

	std::cout << std::endl;

	std::vector<const char*> arguments;
	const int NUM_ARGUMENTS = 4;

	if(argc < NUM_ARGUMENTS)
	{
		std::cout << "Program usage format\n";
		std::cout << "Program name[Synthetic Pop] Input Directory[input] Output Directory[output] Simulation Type[MVS=2 or EET=1] Interactive[0 or 1]"
			<< std::endl;
		exit(EXIT_SUCCESS);
	}

	for(int i = 0; i < argc; i++)
		arguments.push_back(argv[i]);

	int simType;
	bool interactive = (std::stoi(arguments.back()) != 0) ? true : false;

	if(interactive)
	{
		std::cout << "****Available simulation models****" << std::endl;
		std::cout << "1. Equity Efficiency Model" << std::endl;
		std::cout << "2. Mass Violence Model\n" << std::endl;

		std::cout << "Please select simulation model (Enter 1 or 2) : ";
		std::cin >> simType; 

		if(std::cin.eof())
			exit(EXIT_SUCCESS);

		while(!std::cin.eof() && !std::cin.good() || simType < EQUITY_EFFICIENCY || simType > MASS_VIOLENCE)
		{
			std::cout << "Invalid input!" << std::endl;
			std::cin.clear();
			std::cin.ignore(256,'\n');

			std::cout << "Please re-enter valid value: ";
			std::cin >> simType;

			if(std::cin.eof())
			exit(EXIT_SUCCESS);
		}
	}
	else
	{
		simType = std::stoi(arguments[3]);
	}

	std::cout << std::endl;
	Parameters *param = new Parameters(arguments[1], arguments[2], simType);
	
	switch(param->getSimType())
	{
	case EQUITY_EFFICIENCY:
		{
			CardioModel *cvdModel = new CardioModel;

			cvdModel->setParameters(*param);
			cvdModel->import();
			cvdModel->start();

			delete cvdModel;
			break;
		}

	case MASS_VIOLENCE:
		{
			ViolenceModel *massViolence = new ViolenceModel;

			massViolence->setParameters(*param);
			massViolence->import();
			massViolence->start();

			delete massViolence;
			break;
		}
	default:
		break;
	}
	
	//PopBrewer *pop = new PopBrewer(param);
	//pop->generate();

	//system("PAUSE");
	
	//delete pop;
	delete param;

}