#ifndef __IPU_h__
#define __IPU_h__

#include <iostream>
#include <iomanip>
#include <armadillo>

class IPU
{
public:
	IPU(const arma::mat&, const arma::vec&, const arma::vec&, double, bool);
	virtual ~IPU();
	void solve();
	
private:

	void roundWeights();

	arma::mat inp;
	arma::vec cons;
	arma::vec weights;
	double eps;
	bool printOutput;
};

#endif __IPU_h__
