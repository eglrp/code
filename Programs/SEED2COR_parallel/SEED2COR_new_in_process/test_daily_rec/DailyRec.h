#ifndef DAILYREC_H
#define DAILYREC_H

#include "SacRec.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <memory>

class DailyRec {
private:
   struct DRimpl;
   std::unique_ptr<DRimpl> pimpl;

public:
   SacRec sacT;

public:
   /* constructor: read in parameters through the input param file */
   DailyRec( const char *fname = NULL );
   /* copy constructor */
   DailyRec( DailyRec& DRin );
   /* destructor */
   ~DailyRec();
   /* Load in parameters from file fname */
   bool Load( const char *fname );
   /* set parameter by inputing [param_name param_value] as a char* */
   int Set( const char *input );
   /* Parameters checking */
   bool CheckPreExtract();
   bool CheckPreRmRESP();
   bool CheckPreTSNorm();
   /* extracting sac from the seed file. */
   bool ExtractSac( int fskipesac = 0, bool writeout = false ); // extract with reporting
   bool extractSac( int fskipesac, bool writeout, std::ostringstream& reports ); // extract without reporting
   //void RmRESP();
   //void TempSpecNorm ();
};


#endif
