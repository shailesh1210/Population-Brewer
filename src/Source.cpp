#include <iostream>
#include <string>
#include <vector>
#include "Parameters.h"
#include "PopBrewer.h"
#include "csv.h"
#include "IPU.h"
#include <armadillo>

int main(int argc, const char* argv[])
{
	arma::mat freq;
	arma::vec cons;

	freq << 1 << 0 << 1 << 1 << 1 << arma::endr
		 << 1 << 0 << 1 << 0 << 1 << arma::endr
		 << 1 << 0 << 2 << 1 << 0 << arma::endr
		 << 0 << 1 << 1 << 0 << 2 << arma::endr
		 << 0 << 1 << 0 << 2 << 1 << arma::endr
		 << 0 << 1 << 1 << 1 << 0 << arma::endr
		 << 0 << 1 << 2 << 1 << 2 << arma::endr
		 << 0 << 1 << 1 << 1 << 0 << arma::endr;

	cons << 35 << 65 << 91 << 65 << 104;
	arma::vec weights = arma::ones<arma::vec>(freq.n_rows);
	double epsilon = 1e-6;

	IPU ipu(freq, cons, weights, epsilon, true);
	ipu.solve();

	std::vector<const char*> arguments;

	if(argc < 3)
	{
		std::cout << "Program usage format\n";
		std::cout << "Program name[Synthetic Pop] Input Directory[input] Output Directory[output]" << std::endl;
		exit(EXIT_SUCCESS);
	}

	for(int i = 0; i < argc; i++)
		arguments.push_back(argv[i]);

	Parameters *param = new Parameters(arguments[1], arguments[2]);
	PopBrewer *pop = new PopBrewer(param);

	pop->generate();

}