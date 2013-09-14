      subroutine rmresponse(f1,f2,f3,f4,sei,freq,phase_res,amp_res)
      implicit none
      include 'fftw3.h'
      real*8    plan1,plan2, freq(10480576),phase_res(10480576), 
     *                                          amp_res(10480576)
      real*4    sei(10480576)
      double complex s(10480576),sf(10480576)
      double complex czero
      integer*4 k,nk,ns,n2
      real*8    dt2,dom
      real*8    f1,f2,f3,f4
      integer*4 npow
      common    /core/sf,dt2,ns,n2
     
      czero = (0.0d0,0.0d0)
      
      dom = 1.0d0/dt2/ns
      nk = ns/2+1
      npow=1
    
c===============================================================
c   make tapering & remove response
      call flt4(f1,f2,f3,f4,dom,nk,npow,sf,phase_res,amp_res,freq)
c===============================================================
c make forward FFT for seismogram: sf ==> s
      call dfftw_plan_dft_1d(plan2,ns,sf,s,
     *                         FFTW_FORWARD, FFTW_ESTIMATE)
      call dfftw_execute(plan2)
      call dfftw_destroy_plan(plan2)
      do k =1,n2
c         write(*,*) s(k)
         sei(k) = 2.0*real(dreal(s(k)))/ns
c         write(*,*) sei(k)
      enddo
      return
      end

c===============================================================
c Tapering subroutine itself
c===============================================================
      subroutine flt4(f1,f2,f3,f4,dom,nk,npow,sf,phase_res,amp_res,freq)
      real*8    freq(10480576),phase_res(10480576), amp_res(10480576)
      real*8    f1,f2,f3,f4,dom,dphdf, damdf, temp_ph, temp_amp
      integer*4 nk,npow
      double complex sf(10480576), res(10480576)
      real*8    d1,d2,f,dpi,ss,  sss(10480576)
      integer*4 i,j,k
      double complex czero
c ---
c      write(*,*) f1,f2,f3,f4,dom,nk,npow
      czero = (0.0d0,0.0d0)
      dpi = datan(1.0d0)*4.0d0
      do i = 1,nk
         sss(i) = 0.0d0
         res(i)=0.0d0
      enddo
c      write(*,*) "TEST!!", sss(1)
      k=1
      dphdf=(phase_res(k+1)-phase_res(k))/(log(freq(k+1))-log(freq(k)))
      damdf=(log(amp_res(k+1))-log(amp_res(k)))/
     *      (log(freq(k+1))-log(freq(k)))
      
      do i = 1,nk
c         write(*,*) "TEST1!!",i, sss(1)
         f = (i-1)*dom
        if(f.le.f1) then
c           write(*,*) "f le f1"
           res(i)=0.0d0
           goto 1
        endif
        if(f.gt.f4) then
c           write(*,*) "f le f1"
           res(i)=0.0d0
           goto 1
        endif
        do while(f.gt.freq(k+1))
           k = k+1
           dphdf=(phase_res(k+1)-phase_res(k))/
     *           (log(freq(k+1))-log(freq(k)))
           damdf=(log(amp_res(k+1))-log(amp_res(k)))/
     *           (log(freq(k+1))-log(freq(k)))
        enddo
        temp_ph=phase_res(k)+dphdf*(log(f)-log(freq(k)))
        temp_amp=amp_res(k)*exp(damdf*(log(f)-log(freq(k))))
        res(i)=temp_amp*exp((0.0d0,-1.0d0)*temp_ph)
c        write(*,*) i,res(i),freq(k),amp_res(k),phase_res(k),dphdf,damdf
        if(f.le.f1) then
           goto 1
        else if(f.le.f2) then
          d1 = dpi/(f2-f1)
          ss = 1.0d0
          do j = 1,npow
            ss = ss*(1-dcos(d1*(f1-f)))/2.0d0
          enddo
c          write(*,*) "TEST1" sss(i)
          sss(i) = ss
c          write(*,*) "TEST1", sss(i)
        else if(f.le.f3) then
           sss(i) = 1.0d0
c           write(*,*) "TEST2", sss(i)
        else if(f.le.f4) then
          d2 = dpi/(f4-f3)
          ss = 1.0d0
          do j = 1,npow
             ss = ss*(1+dcos(d2*(f3-f)))/2.0d0
         enddo
          sss(i) = ss
c          write(*,*) "TEST3", sss(i)
       endif
 1           continue
      enddo
c      write(*,*) "TEST!!", sss(1)
      do i = 1,nk
c         write(*,*) i, sss(i)
c         write(*,*) sf(i),sss(i),res(i)
         sf(i) = sf(i)*sss(i)*res(i)
c         write (*,*) i,sf(i), sss(i), res(i)
c         sf(i) = sf(i)*sss(i)/res(i)
c         write(*,*) sf(i),sss(i),res(i)
      enddo
      return
      end
