#include "SacRec.h"
#include "StaSacs.h"
#include "MyOMP.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
//#define DEBUG

int main (int argc, char *argv[]) {
   if( argc != 2) {
		// sac_type: 0=dis, 1=vel, 2=acc
      std::cout<<"Usage: "<<argv[0]<<" [sac-list (SAC_Z SAC_H1 SAC_H2 SAC_D sac_type SAC_Z_out)]"<<std::endl;
      exit(-1);
   }

	// read in sac list   
	std::ifstream flst( argv[1] );
	if( ! flst )
		throw std::runtime_error("Error(main): IO failed on file "+std::string(argv[1]));
	std::vector<std::string> flstV;
	for(std::string line; std::getline(flst, line); ) 
		flstV.push_back(std::move(line));

	// loop through the sac list
	float Eperl = 15., Eperu = 40.;
	//for( auto line : flstV ) {
	#pragma omp parallel for schedule(dynamic, 1)
	for(int i=0; i<flstV.size(); i++) {
		const auto &line = flstV[i];
		std::stringstream ss(line);
		std::string sacnameZ, sacnameH1, sacnameH2, sacnameD, outname;
		int sactype;
		if( ! (ss >> sacnameZ >> sacnameH1 >> sacnameH2 >> sacnameD >> sactype >> outname) ) {
			std::cerr<<"Error(main): format error within input line: "<<line<<std::endl;
			continue;
		}
		StaSacs stasac(sacnameZ, sacnameH1, sacnameH2, sacnameD, sactype);
		std::stringstream report(std::ios_base::app|std::ios_base::in|std::ios_base::out);
		report<<"Producing sac file "<<outname<<"\n";
		try {
			auto res = stasac.RemoveTiltCompliance(Eperl, Eperu, 2000.);
			report<<"direction & coh_t & coh_c = "<<res<<"\n";
		} catch( const std::exception& e ) {
			report<<"Warning(main): rmTiltCompliance failed ("<<e.what()<<") and no correction made.";
		}
		#pragma omp critical
		std::cout<<report.str()<<std::endl;
		stasac.Write(outname);
	}

   return 0;
}