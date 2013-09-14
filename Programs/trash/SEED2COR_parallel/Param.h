//global control
extern   SAC_DB *sdb;
extern   int NTHRDS;
extern   int imonth;
extern   int currevn;
extern   struct NOTE *reports;
extern   pthread_attr_t attr_j;
extern   pthread_mutex_t cevlock, fftlock;
//parameters
extern   char rdsexe[200];
extern   char sacexe[200];
extern   char stalst[200];
extern   char seedlst[200];
extern   char ch[4];
extern   int sps;
extern   float gapfrac;
extern   float t1;
extern   float tlen;
extern   float perl, perh;
extern   int tnorm_flag;
extern   float Eperl, Eperh;
extern   float timehlen;
extern   float frechlen;
extern   char fwname[200];
extern   int ftlen;
extern   int fprcs;
extern   float memomax;
extern   int lagtime;
extern   int mintlen;
extern   int fdel1;
extern   int fdel2;
extern   int fskip1;
extern   int fskip2;
extern   int fskip3;
extern   int fskip4;

/*
void PrintParams() {
   cout<<sdb<<endl;
   cout<<imonth<<endl;
   cout<<rdsexe<<endl;
   cout<<sacexe<<endl;
   cout<<stalst<<endl;
   cout<<seedlst<<endl;
   cout<<ch<<endl;
   cout<<sps<<endl;
   cout<<gapfrac<<endl;
   cout<<t1<<endl;
   cout<<tlen<<endl;
   cout<<perl<<" "<<perh<<endl;
   cout<<tnorm_flag<<endl;
   cout<<Eperl<<" "<<Eperh<<endl;
   cout<<timehlen<<endl;
   cout<<frechlen<<endl;
   cout<<fwname<<endl;
   cout<<ftlen<<endl;
   cout<<fprcs<<endl;
   cout<<memomax<<endl;
   cout<<lagtime<<endl;
   cout<<mintlen<<endl;
   cout<<fdel1<<endl;
   cout<<fdel2<<endl;
   cout<<fskip1<<endl;
   cout<<fskip2<<endl;
   cout<<fskip3<<endl;
   cout<<fskip4<<endl;
}
*/
