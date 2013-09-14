#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "/home/linf/PROGRAMING_TOOL/NOISE_CODA_64/mysac64.h"
#define fMax 10000000

SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax);
void rmresponse_(int *n,double *dt,float *sei, double *freq, double *phase_res, double *amp_res, int *nf);


/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
     /*----------------------------------------------------------------------------
       ----------------------------------------------------------------------------*/
{
  FILE *fsac;
  /*..........................................................................*/
  if((fsac=fopen(fname, "rb")) == NULL) return NULL;

  //  if ( !SHD ) SHD = &SAC_HEADER;

  fread(SHD,sizeof(SAC_HD),1,fsac);

  if ( SHD->npts > nmax ) {
    fprintf(stderr,
	    "ATTENTION !!! in the file %s npts exceeds limit  %d",fname,nmax);
    SHD->npts = nmax;
  }

  fread(sig,sizeof(float),(int)(SHD->npts),fsac);

  fclose (fsac);

  /*-------------  calculate from t0  ----------------*/
  {
    int eh, em ,i;
    float fes;
    char koo[9];

    for ( i = 0; i < 8; i++ ) koo[i] = SHD->ko[i];
    koo[8] = '\0';

        SHD->o = SHD->b + SHD->nzhour*3600. + SHD->nzmin*60 +
	  SHD->nzsec + SHD->nzmsec*.001;

        sscanf(koo,"%d%*[^0123456789]%d%*[^.0123456789]%g",&eh,&em,&fes);

        SHD->o  -= (eh*3600. + em*60. + fes);
	/*-------------------------------------------*/}

  return SHD;
}




/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
void write_sac (char *fname, float *sig, SAC_HD *SHD)
     /*----------------------------------------------------------------------------
       ----------------------------------------------------------------------------*/
{
  FILE *fsac;
  int i;
  /*..........................................................................*/
  fsac = fopen(fname, "wb");

  //  if ( !SHD ) SHD = &SAC_HEADER;


  SHD->iftype = (int)ITIME;
  SHD->leven = (int)TRUE;

  SHD->lovrok = (int)TRUE;
  SHD->internal4 = 6L;



  /*+++++++++++++++++++++++++++++++++++++++++*/
  SHD->depmin = sig[0];
  SHD->depmax = sig[0];

  for ( i = 0; i < SHD->npts ; i++ ) {
    if ( SHD->depmin > sig[i] ) SHD->depmin = sig[i];
    if ( SHD->depmax < sig[i] ) SHD->depmax = sig[i];
  }
  fwrite(SHD,sizeof(SAC_HD),1,fsac);

  fwrite(sig,sizeof(float),(int)(SHD->npts),fsac);


  fclose (fsac);
}



int main (int argc, char *argv[])
{
  if(argc != 4) {
    printf("Usage: rm_instrument_response in_SAC_file  TF_file out_SAC_name\n");
    exit(-1);
  }
  static int n,nf;
  static double dt;
  static float sei[10480576];
  //  static int fMax=2000;
  int i,ii;
  static double freq[fMax], phase_res[fMax], amp_res[fMax],coh[fMax];
  double temp,pi,degtorad;
  pi=4*atan(1.0);
  degtorad=pi/180;
  FILE *f_ph, *f_am;
  if((f_ph = fopen(argv[2], "r"))==NULL) {
    fprintf(stderr,"%s file not found\n",argv[2]);
    exit(1);
  }
  //  if((f_am = fopen(argv[3], "r"))==NULL) {
  //  fprintf(stderr,"%s file not found\n",argv[3]);
  //exit(1);
  //}
  for(i=0;i<fMax;i++)
    {
      if(fscanf(f_ph,"%lf %lf %lf %lf",&freq[i],&coh[i],&amp_res[i],&phase_res[i])==EOF)
	break;
      //      phase_res[i]*=degtorad;
      //amp_res[i]*=0.000000001;
      //      fprintf(stderr,"%lf %lf %lf\n",freq[i],phase_res[i],amp_res[i]);
    }
  fclose(f_ph);
  //fclose(f_am);
  nf=i;
  if(freq[0]>0.001||freq[i-1]<0.25)
    {
      fprintf(stderr,"frequency does not cover 0.001 and 0.25 Hz!!\n");
      return 1;
    }
  SAC_HD shd;
  read_sac(argv[1],sei, &shd, 10480576 );
  n=shd.npts;
  dt=shd.delta;
  //for(ii=0;ii<1000;ii++)
  //{
  //  sei[ii]*=cos(pi*(1000.0-ii)/1000.0/2.0);
  //  sei[n-ii]*=cos(pi*(1000.0-ii)/1000.0/2.0);
  //}
  rmresponse_(&n,&dt,sei,freq,phase_res,amp_res,&nf);
  write_sac (argv[3], sei, &shd );

  return 0;  
}
