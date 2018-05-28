#include "IPU.h"

#define MAX_ITERATIONS 2000

IPU::IPU(const arma::mat& input, const arma::vec &con, const arma::vec &w, double epsilon, bool print) : 
	inp(input), cons(con), weights(w), eps(epsilon), printOutput(print)
{
}

IPU::~IPU()
{
}

void IPU::solve()
{
	int row_size = inp.n_rows;
	int col_size = inp.n_cols;

	if(col_size != cons.size()){
		std::cout << "Error: Column size of freq. matrix doesn't match constraints size!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	if(row_size != weights.size()){
		std::cout << "Error: Row size of freq. matrix doesn't match weights size!" << std::endl;
		exit(EXIT_SUCCESS);
	}

	arma::vec gamma_vals(col_size);
	arma::vec gamma_vals_new(col_size);

	double gamma, gamma_new, delta;

	for(int i = 0; i < col_size; ++i)
		gamma_vals[i] = (fabs(arma::sum(inp.col(i)%weights)-cons[i]))/cons[i];

	gamma = arma::mean(gamma_vals);

	bool run_ipu = true;
	int iterations = 0;

	while(run_ipu && iterations <= MAX_ITERATIONS)
	{
		iterations++;
		for(int j = 0; j < col_size; ++j)
		{
			double col_weighted_sum = arma::sum(inp.col(j)%weights);
			double ratio = cons[j]/col_weighted_sum;
			for(int i = 0; i < row_size; ++i)
			{
				if(inp.at(i, j) != 0)
					weights.at(i) = ratio * weights.at(i);
			}
		}

		for(int i = 0; i < col_size; ++i)
			gamma_vals_new[i] = (fabs(arma::sum(inp.col(i)%weights)-cons[i]))/cons[i];

		gamma_new = mean(gamma_vals_new);
		
		delta = fabs(gamma_new-gamma);

		if(printOutput)
			std::cout << "Improvement run in " << iterations << ":" << std::setprecision(8) 
			<< "|gamma_new = " << gamma_new << "|gamma = " << gamma << std::endl;

		if(delta < eps){
			if(printOutput)
				std::cout << "Ipu completed after " << iterations << "!\n" << std::endl; 
			run_ipu = false;
		}
		else{
			for(int i = 0; i < col_size; ++i)
				gamma_vals[i] = gamma_vals_new[i];

			gamma = gamma_new;
		}

		if(iterations > MAX_ITERATIONS)
			std::cout << "WARNING: Convergence not achieved!\n" << std::endl;
	}

	roundWeights();

}

void IPU::roundWeights()
{
	arma::vec roundedWeights(inp.n_rows);
	roundedWeights.fill(0);
	double adj = 0;
	for(size_t i = 0; i < weights.size(); ++i)
	{
		 double diff = adj+(weights.at(i)-floor(weights.at(i)));
		 if(diff >= 0.5){
			 adj = diff-1;
			 roundedWeights.at(i) = ceil(weights.at(i));
		 }
		 else{
			 adj = diff;
			 roundedWeights.at(i) = floor(weights.at(i));
		 }

	}
}



