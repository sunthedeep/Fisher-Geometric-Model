/**
	tester.cpp
	Main file to process inputs and run simulation
	Sandeep Venkataram and Diamantis Sellis
	Petrov Lab | Stanford University


	 Assumptions:
	 - Infinite allele model (all new mutations lead to new alleles)
	 - Random mating (Hardy-Weinberg equilibrium)
	 - New Mutations r~E(lambda), r~N(1,1), r~Gamma(1,1) or r~U(0,4)and theta~U(-pi,pi)
	 - Fitness landscape s~N(b,c)
	 - Environment:
		 o Fixed
		 o Changing - not supported in current implementation
			. Periodic (Harmonic oscilator)
			. Random
				- Brownian motion
				- Levy flight

	make
	 ./bin/FGM initPopulation.dat initEnvironment.dat par.dat output

	output 
	 single runs
		   .table	one line per allele for each generation
		   .ts	   one line per genaration wm etc
		   .edges	mutation events parent,offspring and time
		   .b		summary statistics for each run

	file .b
	column variable 
	1 meanW average wm across run (excluding burn-in period)
	2 varianceW variance in wm across run (excluding burn-in period)
	3 MeanBalanced # of generations with a balanced state
	4 i total time of run
	5 adapted did the population adapt?
	6 tinyWm boolean true if the population is very far from the optimum

*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <sstream>
#include <limits>
#include <cmath>
#include <gsl/gsl_math.h>
#include <gsl/gsl_statistics.h>
#include "randomv.h"
#include "modelFunctions.h"
#include "environment.h"
#include "allele.h"
#include "population.h"
#include "runningStat.h"

/*
 * 
 * 
 */


//version = 36;
/*
  N dimentional FISHER GEOMETRIC MODEL

 
*/

using namespace std;
int main(int argc, char* argv[]){
  //parse input
  if(argc<5){
	cout<<"arguments missing"<<endl;
	cout<<"./command initPopulation.dat initEnvironment.dat parameters.dat output"<<endl;
	exit(1);
  }
  char*   file1name = argv[1]; //initial population description
  char*   file2name = argv[2]; //initial environment description
  char*   file3name = argv[3]; //population and model parameters
  string  out = argv[4];

  //run options
  /*
	to be transfered to parameter file or added as command line options
   */

  int	repeats		= 1;
  bool   breakCondition = false; //use some condition to stop simulation
  //configure output files
  string   filename = out;
  ofstream fp_out(out.append(".table").c_str(), ios::out);
  out = filename;
  ofstream fts(out.append(".ts").c_str(), ios::out);
  out = filename;
  ofstream fsb(out.append(".b").c_str(), ios::out); //balanced
  out = filename;
  ofstream fsr(out.append(".r").c_str(), ios::out); //rest
  out = filename;
  ofstream fse(out.append(".edges").c_str(),ios::out); //edges
  
	string  line;
  //use the same random object
  randomv		  myrand;
  randomv		 &myrandRef = myrand;

	//initialize objects
	modelFunctions   myModel(myrandRef);
	modelFunctions  &myModelRef = myModel;
	int			  generation;
	runningStat	  rs;
	int			  balancedGenerations = 0;
	//initial allele parameters
   
	
	//population parameters
	int	n;	   //population size
	double u;	   //mutation rate
	int	ploidy;  //ploidy
	int	maxTime; //simulation time
	char   f;	   //new mutation function
	char fw; // fitness function type
	
	//for fitness function
	double		 a;		   //mean
	double		 b;		   //shape
	double		 c;		   //maximum 
	double		 d;		   //shape of curve 
	double		 e;		   // error parameter
	vector<double> parameters;  //new mutation function parameters
	
	//environmental parameters
	double  F;		   //frequency of motion
	double  P;		   //phase
	bool	changing;	//changing environment
	double  stepProb;	//step probability
	bool	periodic;	//periodic change
	double  fixedTheta;  // theta for optimum
						 // (if not -PI<fixedTheta<=PI then randomized)
	bool	 walk;	   //random walk (0 Wiener process, 1 Levy flight)
	double   sigma;	  // sigma for gausian step in Wiener process
	double   alpha;	  // alpha (scale) for Pareto step in Levy flight
	double   beta;	   // beta (shape) for Pareto step in Levy flight
	
	//simulation run parameters
	int burnIn;		  //generations to run before starting to print status
	int step;			//print status every step generations
	
	int numDimensions;
	
	ifstream populationInit(file3name);
	if(populationInit.is_open()){
	  while(! populationInit.eof()){
	getline(populationInit, line);
	if (line.size() < 1 || line.at(0) == '#'){
	  continue; //skip comments and empty lines
	}
	stringstream sstr(line);
	char parameter;
	sstr>>parameter;
	switch (parameter){
	case 'N':
	  sstr >> n;
	  break;
	case 'u':
	  sstr >> u;
	  break;
	case 'p':
	  sstr >> ploidy;
	  break;
	case 't':
	  sstr >> maxTime;
	  break;
	case 'w':
	  sstr >> fw;
	  break;
	case 'a':
	  sstr >> a;
	  break;
	case 'b':
	  sstr >> b;
	break;
	case 'c':
	  sstr >> c;
	  break;
	case 'd':
	  sstr >> d;
	  break;
	case 'e':
	  sstr >> e;
	  break;
	case 'f':
	  sstr >> f;
	  break;
	case 'P':
	  sstr >>P;
	  break;
	case 'm':
	  {
		double myparameter;
		while(sstr >> myparameter){
		  parameters.push_back(myparameter);
		}
	  }
	  break;
	case 'F':
	  sstr >> F;
	break;
	case 'C':
	  sstr >> changing;
	  break;
	case 'S':
	  sstr >> stepProb;
	  break;
	case 'E':
	  sstr >> periodic;
	  break;
	case 'T':
	  sstr >> fixedTheta;
	  break;
	case 'W':
	  sstr >> walk;
	  break;
	case 's':
	  sstr >> sigma;
	  break;
	case 'A':
	  sstr >> alpha;
	  break;
	case 'B':
	  sstr >> beta;
	  break;
	case 'I':
	  sstr >> burnIn;
	  break;
	case 'J':
	  sstr >> step;
	  break;
	case 'D':
	  sstr >> numDimensions;
	  break;
	default:
	  {
		cout<<"unknown parameter: "<<parameter<<" in line "<<line<<endl;
		exit(1);
	  }
	}
	  }
	  populationInit.close();
	}else{
	  cout << "Unable to open file "<<file3name<<endl;
	  exit(1);
	}
	
	//read input files
	//environment
	vector<double> opt;
	vector< vector<double> > possibleMutations;
	ifstream envInit(file2name);
	if (envInit.is_open()){
	  while (! envInit.eof() ){
		getline (envInit,line);
		if (line.size() < 1 || line.at(0) == '#'){
			continue;	 //skip comments and empty lines
		}
		double dummy;
		//split line to words
		stringstream sstr(line);
		//use same order as printStatus output
		// so the end of a .ts file can be used
		for (int c = 0; c<numDimensions; c++){
			sstr>>dummy;
			opt.push_back(dummy);
		}
		break;
	  }
	 
	  while (! envInit.eof() ){
		  getline (envInit,line);
		  if (line.size() < 1 || line.at(0) == '#'){
			  continue;	 //skip comments and empty lines
		  }
		  double dummy;
		  //split line to words
		  stringstream sstr(line);
		  //use same order as printStatus output
		  // so the end of a .ts file can be used
		  vector<double> mutVec;
		  for (int c = 0; c<numDimensions; c++){
			  sstr>>dummy;
			  mutVec.push_back(dummy);
		  }
		  possibleMutations.push_back(mutVec);
	  }
	}else{
	  cout << "Unable to open file "<<file2name <<endl;
	  exit(1);
	}
	envInit.close();
	
	environment	  myEnvironment(opt);
	environment	 &envRef = myEnvironment;
	if(possibleMutations.size()>0){
		//cout<<"possible mutations in use"<<endl;
		envRef.setUsePredefinedMutations(true);
		envRef.setPredefinedMutations(possibleMutations);
	}
  //  cout<<"Here 1"<<endl;

   //  cout<<"Here 2"<<endl;
	//assign parameters

	myModelRef.setFitnessFunction(fw);
	//	myModelRef.setMaxR(4);
	myModelRef.setA(a);
	myModelRef.setB(b);
	myModelRef.setC(c);
	myModelRef.setD(d);
	myModelRef.setE(e);
	myModelRef.setMutationFunction(f);
	myModelRef.setParameters(parameters);
	myModelRef.setWalk(walk);
	myModelRef.setSigma(sigma);
	myModelRef.setAlpha(alpha);
	myModelRef.setBeta(beta);
	myModelRef.setNumDimensions(numDimensions);
	
	//  myModelRef.printTable	= false;
	envRef.setF(F);
	if(P < 0){
	  P = M_PI*2*myrandRef.sampleUniform();
	}
	envRef.setPhi(P);
	if(fixedTheta <= -1*M_PI || fixedTheta > M_PI){ //randomize
	  fixedTheta = myModelRef.fTheta(myrand);
	}
	envRef.setPeriodic(periodic);
	char pl;
	if (ploidy == 1){
	  pl = 'h';
	}else if(ploidy == 2){
	  pl = 'd';
	}else{
	  cout<<ploidy<<"-ploids are not supported (yet!)"<<endl;
	  exit(1);
	} 
	
	vector<allele> initial;
	ifstream alleleInit(file1name);
	if (alleleInit.is_open()){
	  while (! alleleInit.eof() ){
		getline (alleleInit,line);
		if (line.size() < 1 || line.at(0) == '#'){
		continue; //skip comments and empty lines
		}
		//split line to words
		stringstream sstr(line);
		//use same order as printStatus output
		// so the end of a .table file can be used
		double pi;	 //frequency
		double ri;	 //distance from (0,0)
		
		int	idi;	//allele id
		double w;	  //fitness
		int	agei;   //allele age
		bool   fixed;
		double s;
		sstr>>generation;
		sstr>>pi;
		sstr>>ri;
		vector<double> rv;
		rv.push_back(ri);
		//cout<<pi<<' '<<ri<<endl;
		for(int i=1; i<numDimensions;i++){
		//	cout<<thetai<<endl;
			rv.push_back(0);
		}	
		sstr>>idi;
		sstr>>w;
		sstr>>agei;
		sstr>>s;
		sstr>>fixed;
		
		
		allele ith(pi, rv, myModelRef);
		ith.setId(idi);
		ith.setBd(generation - agei);
		ith.setFixed(fixed);
		ith.setfW(envRef.fW(rv,myModelRef));
		initial.push_back(ith);
	  }
	  alleleInit.close();
	}else{
	  cout << "Unable to open file "<<file1name<<endl;
	  exit(1);
	}
	
   //  cout<<"Here 3"<<endl;
	//initialize population
	vector<allele> &initRef = initial;
	population	  we(myrandRef, myModelRef, initRef, ploidy, envRef, fsr);
  //  cout<<"Here 3a"<<endl;
	we.setN(n);
	we.setU(u);
	we.setSimTime(generation);
	we.FindNeutralSphere(myModelRef);
	//  int counter = 0; //make array for mean w
	//  double stat[maxTime];
	bool tinyWm = false; // if population is far away from
				  // the optimum small wm values can
				  // cause precission problems, so better
				  //end program with appropriate output
	double wm0 = we.getWm(); //initial population mean fitness
	int noChange = 0; //change environment every noChange genarations
	bool adapted = false;
  //  cout<<"Here 3b"<<endl;
	while(generation < maxTime){
	  if(we.isAdapted(myModelRef,envRef)){
	adapted = true;
	if(breakCondition){
	  break;
	}
	  }
	//  cout<<"Here 3c"<<endl;
	  if (changing == 1){
	if (stepProb > 1){
	  if (numeric_limits< double >::min() > stepProb - noChange){
		envRef.change(myrandRef, myModelRef, generation);
		noChange = 0;
	  }
	  noChange++;
	}else{
	  if (myrandRef.sampleUniform() < stepProb){
		envRef.change(myrandRef, myModelRef, generation);
	  }
	//  cout<<"Here 3d"<<endl;
	}
	  }
	 //  cout<<"Here 4"<<endl;
	  we.evolve(myrandRef, myModelRef, envRef, fse);
	//   cout<<"Here 5"<<endl;
	  if(ploidy == 2){
	if (we.areBalanced(myModelRef,envRef)){
	  balancedGenerations++;
	}
	  }
	  //check if wm is veeery small 
	  if (numeric_limits< double >::min()  > 0.000001 * we.getWm()){ //six orders of magnitude
	cerr<<" wm approaching precission limit!!!"<<endl;
	tinyWm = true;
	break;
	  }
	  
	  ///if(i == maxTime-1){
	  if(generation >= burnIn){
	rs.Push(we.getWm());
	if (repeats == 1 ){
	  if(generation%step == 0){
		we.printStatus(myModelRef, fp_out, fts, envRef, fsr);
		if(ploidy == 2){
		  ////		we.printStates(myModelRef, envRef, fsr); //do not print states
		}
	  }
	}
	  }
	  generation++;
	}
	if(repeats>1){
	  we.printStatus(myModelRef, fp_out, fts, envRef, fsr);
	}
	//summary statistics 
	double meanW		= rs.Mean();
	double varianceW	= rs.Variance();
	fsb<<meanW<<' '<<varianceW<<' '<<balancedGenerations<<' '<<generation<<' '<<adapted<<' '<<tinyWm<<endl;
  
	
  fp_out.close();
  fts.close();
  fsb.close();
  //  printf("\a"); //beep
  return EXIT_SUCCESS;
}
