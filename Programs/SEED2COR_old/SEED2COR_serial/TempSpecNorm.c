#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include "64_sac_db.h"
#include "mysac64.h"
#include "Param.h"
using namespace std;

extern "C"{
void whiten_(double *f1,double *f2,double *f3,double *f4,int *npow,double *dt, int *n,float *hlen,float seis_in[],float seissm[],float seis_out[], float seis_outamp[],float seis_outph[],int *ns,double *dom,int *flag_whiten);

void filter4_(double *f1,double *f2,double *f3,double *f4,int *npow,double *dt,int *n,float *seis_in,float *seis_out);
}

SAC_HD *read_sac (char *fname, float **sig, SAC_HD *SHD);

void write_sac (char *fname, float *sig, SAC_HD *SHD);

void UpdateRec(char *name, int *rec_b, int *rec_e, int nrec);

int read_rec(int rec_flag, char *fname, int len, int *rec_b, int *rec_e, int *nrec);
void UpdateRec(char *name, int *rec_b, int *rec_e, int nrec) {
   int rec_b1[1000], rec_e1[1000], nrec1;
   FILE *frec;
   int irec, irec1;
   if( ! read_rec(1, name, 0, rec_b1, rec_e1, &nrec1) ) {
      fprintf(stderr, "*** Warning: cannot open record file %s ***", name);
      frec = fopen(name, "w");
      for(irec=0; irec<nrec; irec++)
         fprintf(frec, "%d %d\n", rec_b[irec], rec_e[irec]);
      fclose(frec);
   }
   int recB, recE;
   char name2[100];
   sprintf(name2, "%s2", name);
   frec = fopen(name2, "w");
   for(irec=0; irec<nrec; irec++)
      for(irec1=0;irec1<nrec1;irec1++){
         if(rec_b[irec]>=rec_e1[irec1]) continue;
         if(rec_e[irec]<=rec_b1[irec1]) break;
         recB = max(rec_b[irec],rec_b1[irec1]);
         recE = min(rec_e[irec],rec_e1[irec1]);
         fprintf(frec, "%d %d\n", recB, recE);
      }
   fclose(frec);
}

void OneBit(float *sig, SAC_HD *shd) {
   int i;
   for(i=0;i<shd->npts;i++) {
      if(sig[i]>0) sig[i] = 1.;
      else sig[i] = -1.;
   }
}

void RunAvg(float *sig, SAC_HD *shd) {

   int npow = 1;
   float sigw[shd->npts];
   double f2 = 1./Eperh, f1 = f2*0.6, f3 = 1./Eperl, f4 = f3*1.4;
//   double f1 = 1./60., f2 = 1./50., f3 = 1./10., f4 = 1./7.5;
   double dtd = (double)(shd->delta);
   if( Eperl == -1 ) memcpy(sigw, sig, shd->npts*sizeof(float));
   else filter4_(&f1,&f2,&f3,&f4,&npow,&dtd,&(shd->npts),sig,sigw);

   int i, j, wb, we;
   float dt = shd->delta;
   int n = shd->npts, half_l = (int)floor(timehlen/dt+0.5);
   float window_sum[n];

   // fast running average before whitening

   if(half_l*2>n-1) half_l = (n-1)/2;
   for(i=0,window_sum[0]=0.;i<=half_l;i++) window_sum[0] += fabs(sigw[i]);
   wb = 0; we = i;
   for(i=1;i<=half_l;i++) {
      window_sum[i] = window_sum[i-1] + fabs(sigw[we]);
      window_sum[i-1] /= we++;
   }
   for(j=we;i<n-half_l;i++) {
      window_sum[i] = window_sum[i-1] + fabs(sigw[we++]) - fabs(sigw[wb++]);
      window_sum[i-1] /= j;
   }
   for(;i<n;i++) {
      window_sum[i] = window_sum[i-1] - fabs(sigw[wb++]);
      window_sum[i-1] /= we-wb;
   }
   window_sum[n-1] /= we-wb;
//   write_sac("temp.sac", window_sum, shd );
//   exit(0);
   for(i=0;i<n;i++) if(sig[i]!=0.) sig[i] /= window_sum[i];
}

int EqkCut(float *sig, SAC_HD *shd, char *recname) {

   int i, n = shd->npts, ninc = n/1000, npole=0;
   for(i=0;i<n;i+=ninc) if(fabs(sig[i])<1.e-20) npole++;
   if(npole>600) {
      cout<<"      Signal time length not long enough."<<endl;
      return 0;
   }

   int npow = 1;
   float sigw[n];
   double f2 = 1./Eperh, f1 = f2*0.8, f3 = 1./Eperl, f4 = f3*1.2;
   double dt = (double)(shd->delta);
   filter4_(&f1,&f2,&f3,&f4,&npow,&dt,&n,sig,sigw);

   //compute noise level
   int   ii, j, is, s1k, flag;
   s1k=(int)floor(1000./dt+0.5);
   int rec_i,rec_b[1000],rec_e[1000];

   double win_max[s1k+1],win_min,window_avg,window_std;
   for(i=0;i<n;i++) sigw[i] = fabs(sigw[i]);
   for( is=0, i=0; i<= n-s1k; is++,i+=s1k){
      win_max[is]=0;
      for( ii=i; ii<i+s1k; ii++ )
         if(win_max[is]<sigw[ii]) win_max[is]=sigw[ii];
   }
   flag=0;
   if(i<n) flag=1;
   for( ii=i;ii<n;ii++ )
      if(win_max[is]<sigw[ii]) win_max[is]=sigw[ii];

   window_avg=0;ii=0;
   win_min = 1e20;
   for(i=0; i<(int)(n/s1k); i++) if(win_max[i]>1e-20 && win_min>win_max[i]) win_min=win_max[i];

   for( i =0; i< (int)(n/s1k); i++){
      if(win_max[i]>win_min*2.0 || win_max[i]<1e-20) continue;
      window_avg+=win_max[i];
      ii+=1;
   }
   if( ii < 20 ) {
      cout<<"Time length not enough after throw away earthquakes"<<endl;
      return 0;
   }
   window_avg=window_avg/ii;
   window_std=0;
   for( i =0; i< (int)(n/s1k); i++){
      if(win_max[i]>win_min*2.0 || win_max[i]<1e-20)continue;
      window_std+=(window_avg-win_max[i])*(window_avg-win_max[i]);
   }
   window_std=sqrt(window_std/(ii-1));

   ii=0;
   for( i =0; i < (int)(n/s1k); i++){
      if(win_max[i]>window_avg+2.0*window_std){  // || win_max[i]<1e-20){
         for ( j=0; j<s1k; j++ ) sig[i*s1k+j]=0;
         sigw[i]=0;
      }
      else sigw[i]=1;
      ii+=1;
   }
   if(win_max[i]>window_avg+2.0*window_std) {
      for ( j=i*s1k; j<n; j++ ) sig[j]=0;
      sigw[i]=0;
   }
   else sigw[i]=1;

   if( ii < 20 ) {
      cout<<"Time length not enough after throw away earthquakes"<<endl;
      return 0;
   }

   rec_i=0;
   rec_b[0]=0;
   for( i=1; i<(int)(n/s1k)+flag;){
      if(sigw[i]-sigw[i-1]==1) rec_b[rec_i]=i*s1k;
      else if(sigw[i]-sigw[i-1]==-1) {
         rec_e[rec_i]=i*s1k;
         if ((rec_e[rec_i]-rec_b[rec_i])<2500/dt)
            for(ii=rec_b[rec_i];ii<rec_e[rec_i];ii++) sig[ii]=0;
         else rec_i++;
      }
      i++;
   }
   if(sigw[i-1]==1) {
      rec_e[rec_i]=n;
      if((rec_e[rec_i]-rec_b[rec_i])<1500/dt)
         for(ii=rec_b[rec_i];ii<rec_e[rec_i];ii++) sig[ii]=0;
      else rec_i++;
   }
   for(i=0;i<rec_i;i++) {
      if(rec_b[i]!=0){
         rec_b[i]+=300;
         for(ii=rec_b[i]-300;ii<rec_b[i];ii++) sig[ii]=0;
      }
      if(rec_e[i]!=n){
         rec_e[i]-=300;
         for(ii=rec_e[i]+1;ii<=rec_e[i]+300;ii++) sig[ii]=0;
      }
   }

   UpdateRec(recname, rec_b, rec_e, rec_i);

   return 1;
}

int TemperalNorm( char *fname, float **sig, SAC_HD *shd) {

   *sig = NULL;
   if( read_sac(fname, sig, shd) == NULL ) {
      fprintf(stderr, "*** Warning: Cannot open file %s ***", fname);
      return 0;
   }
   char recname[300];
   sprintf(recname,"%s_rec",fname);

   if( tnorm_flag==1 ) OneBit(*sig, shd);
   else if( tnorm_flag==2 ) RunAvg(*sig, shd);
   else if( tnorm_flag==3 ) { if(!EqkCut(*sig, shd, recname)) return 0; }
   else if( tnorm_flag!=0 ) {  
      cout<<"Undefined normalization method!"<<endl; 
      exit(0); 
   }

/*
   char ftmp[100], *fp;
   sprintf(ftmp, "%s", fname);
   strtok(ftmp, "_");
   fp = strtok(NULL, "\n");
   sprintf(ftmp, "temp/ft_%s", fp);
   write_sac(ftmp, *sig, shd );
*/
// exit(0);

   return 1;
}

int SpectralNorm(char *fname, float *sig, SAC_HD shd) {

   int ns, npow = 1, flag_whiten;
   double dom;
   double dt = (double)shd.delta;
   int n = shd.npts;
   //double f1 = 1./100., f2 = 1./80., f3 = 1./4., f4 = 1./3.;
   double f2 = 1./perh, f1 = f2*0.8, f3 = 1./perl, f4 = f3*1.2;
   float *sigw = NULL;
   //if(strcmp(fwname,"0") != 0) {
   if(frechlen==-1.) {
      SAC_HD shdw;
      if( read_sac(fwname, &sigw, &shdw) == NULL ) {
         cout<<"Cannot open file "<<fwname<<endl;
         free(sig);
         exit(0);
      }
      if(shdw.npts!=shd.npts || fabs(shdw.delta-shd.delta)>1.e-3) {
         cout<<"Smoothing spectrum is incompatibale with input signals!"<<endl;
         exit(0);
      }
   }
   float seis_out[n];
   int nf = (int)(log((double)n)/log(2.))+1;
   if(nf<13) nf = 13;
   nf = (int)pow(2,nf);
   float seis_outamp[nf], seis_outph[nf];
   //memset (seis_outamp,0,nf*sizeof(float));
   //memset (seis_outph,0,nf*sizeof(float));
   whiten_(&f1,&f2,&f3,&f4,&npow,&dt,&n,&frechlen,sig,sigw,seis_out,seis_outamp,seis_outph,&ns,&dom,&flag_whiten);
   if(flag_whiten==0) { 
      cout<<"      skipped due to probamatic spectrum."; 
      return 0;
   }

   char nameamp[200], nameph[200];
/*
   shd.npts = n;
   shd.delta = dt;
   char ftmp[100], *fp;
   sprintf(ftmp, "%s", fname);
   strtok(ftmp, "_");
   fp = strtok(NULL, "\n");
   sprintf(nameamp, "temp/ft_%s", fp);
   write_sac(nameamp,seis_out, &shd);
*/
//exit(0);
   sprintf(nameamp, "%s.am", fname);
   sprintf(nameph, "%s.ph", fname);
   shd.npts = ns/2 + 1;
   shd.delta = dom;
   shd.b = 0;
   shd.iftype = IXY;
   write_sac(nameamp,seis_outamp, &shd );
   write_sac(nameph, seis_outph,  &shd );

   free(sig);
   free(sigw);
  
   return 1;
}

void TempSpecNorm () {
   char amname[200], phname[200];
   int ist, iev, nst;
   float *sig;
   SAC_HD shd;

   for(iev=0;iev<NEVENTS;iev++) {
      if(strcmp(sdb->mo[imonth].seedf[iev],"0")==0) continue;
      nst = 0;
      fprintf(stderr, "### Preparing am and ph records for event %s: ", sdb->ev[iev].name);
      for( ist = 0; ist < sdb->nst; ist++ ) {
         sprintf(amname, "%s.am", sdb->rec[iev][ist].ft_fname);
         sprintf(phname, "%s.ph", sdb->rec[iev][ist].ft_fname);
         if( fskip3==2 || (fskip3==1 && access( amname, R_OK) != -1 && access( phname, R_OK) != -1) ) continue;
         if(sdb->rec[iev][ist].n <= 0) continue;
         if( !TemperalNorm( sdb->rec[iev][ist].ft_fname, &sig, &shd) ) 
            { sdb->rec[iev][ist].n = 0; continue; }
         if( !SpectralNorm( sdb->rec[iev][ist].ft_fname, sig, shd ) ) 
            { sdb->rec[iev][ist].n = 0; continue; }
	 if( nst%20 == 0 ) fprintf(stderr, "\n   ");
         fprintf(stderr, "%s ", sdb->st[ist].name);
         nst++;
      }
      cout<<endl<<"   "<<nst<<" stations processed. ###"<<endl;
   }

}

