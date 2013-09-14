      subroutine rmresponse(n,dt,sei,freq,phase_res, amp_res,nf)
      implicit none
      include 'fftw3.h'
      real*8    plan1,plan2, freq(2000),phase_res(2000), amp_res(2000)
      real*4    sei(1048576)
      double complex s(1048576),sf(1048576)
      double complex dc2,czero
      integer*4 n,k,nk,ns,nf
      real*8    dt,dom
      real*8    f1,f2,f3,f4
      integer*4 npow
     
      czero = (0.0d0,0.0d0)
      dc2 = dcmplx(2.0d0,0.0d0)
      
      ns = 2**max0(int(dlog(dble(n))/dlog(2.0d0))+1,13)
      dom = 1.0d0/dt/ns
      do k = 1,ns
        s(k) = czero
      enddo
      do k = 1,n
        s(k) = sei(k)
      enddo

c make backward FFT for seismogram: s ==> sf
      call dfftw_plan_dft_1d(plan1,ns,s,sf,
     *                         FFTW_BACKWARD, FFTW_ESTIMATE)
      call dfftw_execute(plan1)
      call dfftw_destroy_plan(plan1)
      nk = ns/2+1
      do k = nk+1,ns
         sf(k) = czero
      enddo
      sf(1) = sf(1)/2.0d0
      sf(nk) = dcmplx(dreal(sf(nk)),0.0d0)
      f1=0.001
      f2=0.002
      f3=0.2
      f4=0.25
      npow=1
    
c===============================================================
c   make tapering & remove response
      call flt4(f1,f2,f3,f4,dom,nk,npow,sf,phase_res,amp_res,freq,nf)
c===============================================================
c make forward FFT for seismogram: sf ==> s
      call dfftw_plan_dft_1d(plan2,ns,sf,s,
     *                         FFTW_FORWARD, FFTW_ESTIMATE)
      call dfftw_execute(plan2)
      call dfftw_destroy_plan(plan2)
      do k =1,n
c         write(*,*) s(k)
         sei(k) = 2.0*real(dreal(s(k)))/ns
c         write(*,*) sei(k)
      enddo
      return
      end

c===============================================================
c Tapering subroutine itself
c===============================================================
      subroutine flt4(f1,f2,f3,f4,dom,nk,npow,sf,phase_res,amp_res,freq,nf)
      real*8    freq(2000),phase_res(2000), amp_res(2000)
      real*8    f1,f2,f3,f4,dom,dphdf, damdf, temp_ph, temp_amp
      integer*4 nk,npow,nf
      double complex sf(1048576), res(1048576)
      real*8    d1,d2,f,dpi,ss,  sss(1048576)
      integer*4 i,j,k
      double complex czero
c ---
      czero = (0.0d0,0.0d0)
      dpi = datan(1.0d0)*4.0d0
      do i = 1,nk
         sss(i) = 0.0d0
      enddo
      k=1
      dphdf=(phase_res(k+1)-phase_res(k))/(log(freq(k+1))-log(freq(k)))
      damdf=(log(amp_res(k+1))-log(amp_res(k)))/(log(freq(k+1))-log(freq(k)))
      do i = 1,nk
        f = (i-1)*dom
        if(f.le.f1) then
           res(i)=1
           goto 1
        endif
        do while(f.gt.freq(k+1))
           k = k+1
           dphdf=(phase_res(k+1)-phase_res(k))/(log(freq(k+1))-log(freq(k)))
           damdf=(log(amp_res(k+1))-log(amp_res(k)))/(log(freq(k+1))-log(freq(k)))
        enddo
        temp_ph=phase_res(k)+dphdf*(log(f)-log(freq(k)))
        temp_amp=amp_res(k)*exp(damdf*(log(f)-log(freq(k))))
        res(i)=temp_amp*exp((0.0d0,-1.0d0)*temp_ph)
        if(f.le.f1) then
           goto 1
        else if(f.le.f2) then
          d1 = dpi/(f2-f1)
          ss = 1.0d0
          do j = 1,npow
            ss = ss*(1-dcos(d1*(f1-f)))/2.0d0
          enddo
          sss(i) = ss
        else if(f.le.f3) then
           sss(i) = 1.0d0
        else if(f.le.f4) then
          d2 = dpi/(f4-f3)
          ss = 1.0d0
          do j = 1,npow
             ss = ss*(1+dcos(d2*(f3-f)))/2.0d0
         enddo
          sss(i) = ss
       endif
 1           continue
      enddo
      do i = 1,nk
c         write(*,*) sf(i),sss(i),res(i)
         sf(i) = sf(i)*sss(i)/res(i)
c         sf(i) = sf(i)*sss(i)/res(i)
c         write(*,*) sf(i),sss(i),res(i)
      enddo
      return
      end
