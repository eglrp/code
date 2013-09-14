//revised to retain the amplitude information.
//no time-domain normalization applied. earthquake signal picked and threw away instead
//frequency whiten by a fixed function
//output am, ph and rec (for keeping track of correlation time length) files


#define MAIN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <math.h>
//#incl64_mysac.h"
#include "/home/tianye/code/Programs/head/mysac.h"

#define NSIG 90000
#define Nf 400000

/* Function prorotypes */


void filter4_(double *f1,double *f2,double *f3,double *f4,int *npow,
              double *dt,int *n, float seis_in[],
              float seis_out[], float seis_outamp[],
              float seis_outph[],int *ns,double *dom);

void fft_phamp_(double *dt,int *n, float seis_in[],
                float seis_outamp[],
                float seis_outph[],int *ns,double *dom);

void swapn(unsigned char *b, int N, int n);




/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
	SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
/*..........................................................................*/
	fsac = fopen(fname, "rb");
	if ( !fsac )
	{
	 fclose (fsac);
	 return NULL;
	}

	if ( !SHD ) SHD = &SAC_HEADER;

	 fread(SHD,sizeof(SAC_HD),1,fsac);

	 if ( SHD->npts > nmax )
	 {
	  fprintf(stderr,
	   "ATTENTION !!! dans le fichier %s npts est limite a %d",fname,nmax);

	  SHD->npts = nmax;
	 }

	 fread(sig,sizeof(float),(int)(SHD->npts),fsac);

	fclose (fsac);

   /*-------------  calcule de t0  ----------------*/
   {
	int eh, em ,i;
	float fes;
	char koo[9];

	for ( i = 0; i < 8; i++ ) koo[i] = SHD->ko[i];
	koo[8] = 0;

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

	if ( !SHD ) SHD = &SAC_HEADER;


        SHD->iftype = (int)ITIME;
        SHD->leven = (int)TRUE;

        SHD->lovrok = (int)TRUE;
        SHD->internal4 = 6L;



  /*+++++++++++++++++++++++++++++++++++++++++*/
     SHD->depmin = sig[0];
     SHD->depmax = sig[0];
 
   for ( i = 0; i < SHD->npts ; i++ )
   {
    if ( SHD->depmin > sig[i] ) SHD->depmin = sig[i];
    if ( SHD->depmax < sig[i] ) SHD->depmax = sig[i];
   }

	 fwrite(SHD,sizeof(SAC_HD),1,fsac);

	 fwrite(sig,sizeof(float),(int)(SHD->npts),fsac);


	 fclose (fsac);
}



/*c/////////////////////////////////////////////////////////////////////////*/
 float sig[NSIG];
 SAC_HD shd1,shd_temp;


/*c/////////////////////////////////////////////////////////////////////////*/

int main (int argc, char *argv[])
{
static int n, ns,npow;
static double f1, f2, f3, f4, dt,dom;
static float seis_in[NSIG*2],seis_out[NSIG*2];
static float seis_outamp[Nf],seis_outph[Nf];
double t1,t2,t3,t4;
char  name[160], name1[160], name_flt[160];
char  nameamp[160],nameph[160],namesamp[160];
FILE  *in, *ff, *fsm;
int   i, ii, j, nn, ntest;
float sig_window[NSIG],sig1[NSIG],window_sum[NSIG];
double window_avg,window_std;
int window_b,window_e;
int half_l=(int)(40/1.);

  if( argc != 2) {
      printf("Usage: whiten_phamp  parameter_file\n");
      exit(-1);
  }

// open and read parameter file param.dat
  if((in = fopen(argv[1],"r")) == NULL) {
      printf("Can not find file %s.\n",argv[1]);
      exit(1);
  }
  while((nn = fscanf(in,"%lf %lf %lf %lf %lf %d %s %s",&t1,&t2,&t3,&t4,
            &dt,&npow,name,name_flt)) != EOF) { // start main loop
      if(nn == 0 || nn != 8) break;
      printf("Whiten: Corners periods. Low: %f - %f, High: %f - %f\n",t1, t2, t3, t4);
//      printf("Step: %f, Cosine power: %d\n",dt, npow);


// remove quotes from name
      j = 0;
      for(i = 0; i < strlen(name); i++) {
          if(name[i] == '\'' || name[i] == '\"') continue;
          name[j] = name[i]; j++;
      }
      name[j] = '\0';
  if ( !read_sac(name, sig, &shd1, NSIG ) )
    {
      fprintf(stderr,"file %s did not found\n", name );
      continue;
    }

  if ( !read_sac(name_flt, sig_window, &shd1, NSIG ) )
    {
      fprintf(stderr,"file %s did not found\n", name_flt );
      continue;
    }

    n  = shd1.npts;
    dt = shd1.delta;
    ntest=0;
    for( i =0; i< n; ) { if(fabs(sig[i])<1e-20) ntest+=1; i=i+100; }
    printf("ntest: %d\n",ntest);
    if(ntest > 600/dt) {
       printf("%s: Signal time length not long enough.\n",name);
       continue; 
      } 

/*     for( i =0; i< n; i++){
        if(sig[i]>0)sig[i]=1;
        if(sig[i]<0)sig[i]=-1;
       } */
	for(i=0;i<n;i++){
                window_sum[i]=0;
		window_b=i-half_l;
		if(window_b<0)window_b=0;
		window_e=i+half_l;
		if(window_e>n)window_e=n;
		for(j=window_b;j<window_e;j++)window_sum[i]+=fabs(sig_window[j]);
                window_sum[i]/=(window_e-window_b);
	}
	for(i=0;i<n;i++) sig[i]=sig[i]/window_sum[i];

     sprintf(namesamp,"%s_water",name);
     write_sac(namesamp,sig, &shd1);
     
//      write_sac(namesamp,sig_window, &shd1);


  // output am and ph files

     for( i =0; i< n; i++)
     {  
     seis_in[i] = sig[i];  
     //     printf(" seis_in1  %d %f\n", i,sig[i]);
     }

      printf(" Dt1= %f, Nsamples1= %d\n",dt, n);
	
  f1 = 1.0/t1; f2 = 1.0/t2; f3 = 1.0/t3; f4 = 1.0/t4;

//printf("Check !!! %f %f %f %f %d %f %d\n",f1,f2,f3,f4,npow,dt,n);

  filter4_(&f1,&f2,&f3,&f4,&npow,&dt,&n,seis_in,seis_out,seis_outamp,seis_outph,&ns,&dom);
	shd1.npts = n;
	shd1.delta = dt;
//        write_sac(name,seis_out, &shd1);
        strcpy(nameamp,name);
        strcpy(nameph,name);
        strcat(nameamp,".am");
        strcat(nameph, ".ph");
	shd1.npts = ns/2+1;
	shd1.delta = dom;
	shd1.b = 0;
        shd1.iftype = IXY;
        write_sac(nameamp,seis_outamp, &shd1 );
        write_sac(nameph, seis_outph,  &shd1 );

/*  // output rec.am and rec.ph files

     for( i =0; i< n; i++)
     {
     seis_in[i] = sig_window[i];
     }

  fft_phamp_(&dt,&n,seis_in,seis_outamp,seis_outph,&ns,&dom);
        shd1.npts = n;
        shd1.delta = dt;
        write_sac(name,seis_out, &shd1);
        strcpy(nameamp,name);
        strcpy(nameph,name);
        strcat(nameamp,"_rec.am");
        strcat(nameph, "_rec.ph");
        shd1.npts = ns/2+1;
        shd1.delta = dom;
        shd1.b = 0;
        shd1.iftype = IXY;
        write_sac(nameamp,seis_outamp, &shd1 );
        write_sac(nameph, seis_outph,  &shd1 );  */


  }
  
   return 0;
  }
