#ifndef STABILITY_H
#define STABILITY_H
#include <gsl/gsl_matrix.h>
#include <vector>
using namespace std;

/*
	Code to calculate whether or not a set of alleles can be maintained at a stable polymorphism using the method of Kimrua 1956
*/

class stability{
 public:
  stability(vector<double> ws, int n); // constructor, takes in a vector of fitness values of all relevant genotypes and the number of alleles being considered. the input vector is essentially a row-wise ordering of the upper right half of the nxn fitness matrix (including the diagonal)
  int stable(void);
  double getMeanFitness(void);
  double getDeterminantA(void){return determinantA;}
  double getDeterminantSum(void){return determinantSum;}
  gsl_matrix* getEqFreqs(void);
 private:
  int negative_definite(gsl_matrix *M, int n);
  int delta_condition(gsl_matrix *M, int n);
  double meanFitness;
  double determinantA;
  double determinantSum;
  gsl_matrix *A;
  gsl_matrix *A2;
  gsl_matrix *eqFreqs;
  int n; //! dimensions
};
#endif
