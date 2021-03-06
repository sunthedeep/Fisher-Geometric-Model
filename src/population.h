#ifndef POPULATION_H
#define POPULATION_H
#include <vector>
#include "randomv.h"
#include "modelFunctions.h"
#include "environment.h"
#include "allele.h"

/*
	main class which keeps track of populations and conducts the operations involved with simulating evolution (mutation, propogation, selection)
*/

using namespace std;
//! version = 3
class population{
 public:
  population(randomv &r, modelFunctions &myModelRef, vector<allele> &init, int ploidy, environment &envRef, ofstream &fsr);
  int            isFixed(int x);
  int            ispseudoFixed(double rmin);
  int            isAdapted(modelFunctions &myModelRef, environment &envRef);
  int            getSimTime(void) { return simTime; }
  int            getN(void) { return N; }
  int            getAllelesNumber(void) { return alleles.size(); }
  double         getWm(void) { return wm; }
  double         getU(void) { return u; }
  int            getAlleleCounter(void) { return alleleCounter;}
  int            getPloidy(void) { return ploidy; }
  double         getWildW(void) { return wildW; }
  double         getMaxGenW(void) { return maxGenW; }
  //   int            getSample(randomv &r, modelFunctions &myModelRef, unsigned int sampleSize, ofstream &fsb, ofstream &fsr);
  //  int            getDegree(int x);
  //  int            getBalancedPairs(void) {return balancedPairs; }
  //  double         getBalancedRatio(void);
  double         getR0(void){ return r0; }
  void           setSimTime (int x){ simTime = x; }
  void           setN (int x){ N = x; }
  void           setWm (double x){ wm = x; }
  void           setU (double x){ u = x; }
  void           setAlleleCounter(int x) { alleleCounter = x;}
  void           setPloidy(int x) { ploidy = x; }
  void           setR0(double x) { r0 = x; }

  double         FindNeutralSphere(modelFunctions &myModelRef);
  void           evolve(randomv &r, modelFunctions &myModelRef, environment &envRef, ofstream &fse);
  void           printStatus(modelFunctions &myModelRef, ofstream &fp_out, ofstream &fts, environment &envRef, ofstream &fsr);
  bool           areBalanced(modelFunctions &myModelRef, environment &envRef);
  //  void           addb(int a, int b); //add a pair of balancing alleles to table
  //  void           printTable(void); //output table
  int            printStates(modelFunctions &myModelRef, environment &envRef, ofstream &fsr);  //print allele pairs interaction terms
  

  /*//not used
   *  double getHetA(void) { return hetA; }
   *  double getHetB(void) { return hetB; }
   *  void   setHetA(double x) { hetA = x; }
   *  void   setHetB(double x) { hetB = x; }
   */
 private:
  int            simTime;           //! generations
  int            N;                 //! population size (number of individuals not chromosomes!!!)
  double         wm;                //! mean fitness
  double         maxWij;            //! maximum heterozygote fitness
  vector<double> phm;               //! mean phenotype (coordinates)
  int            updatePhm(modelFunctions &myModelRef);   //! function to update the mean phenotype
  double         u;                 //! mutation rate
  vector<allele> alleles;           //! a vector of alleles
  void           mutate (randomv &r, modelFunctions &myModelRef, environment &envRef, ofstream &fse); //! performs the mutation step
  void           propagate(randomv &r, modelFunctions &myModelRef, environment &envRef);              //! propagates the population (drift and selection)
  double         updatewm(modelFunctions &myModelRef, environment &envRef);                           //! update mean fitness
  int            alleleCounter;     //! gives each allele a unique identifier
  int            ploidy;            //! ploidy
  double         wildW;             //! maximum allele's fitness each generation "current wild type"
  double         maxGenW;           //! maximum fitness of genotype (individual)
  //  int    balancedPairs;  //number of balanced pairs of alleles
                         // pairs formed with the ancestral are also counted
  //  vector<int>    curBalanced;   // alleles balanced in current generation
  //  vector< vector <int> > table; //table for keeping track of pairs 
                                // of balancing alleles
  double         var;               //! var(wij) variance in individuals' fitness during current generation
  vector<double>         phenotypeVar;              //! variance in phenotypes along each axis
  bool           stableEquilibrium; //! true if the present generation in a balanced state (for all alleles with p>d and and only if w>1/N) 
  int            polymorphisms;     //! number of alleles with p>d 
  double         r0;                //! radius of the "neutral sphere" around the optimum
  //unused
  /*  
   *
   *"For the evolutionist or geneticist, the comparison of the hybrid with
   *the parental mean for a character would signify heterosis, but a
   *breeder is hardly interested in a hybrid that is not superior to both
   *parents."
   *
   *Basra, A. S. (1999). Heterosis and hybrid seed production in agronomic
   *                     crops. New York: Food Products Press. p.3
   *
   *		       */
  /*  double hetA;           //Heterosis A (wab - (waa+wbb)/2)/wab
   *  double hetB;           //Heterosis B (wab - max(waa,wbb))/wab
   */
};
#endif
