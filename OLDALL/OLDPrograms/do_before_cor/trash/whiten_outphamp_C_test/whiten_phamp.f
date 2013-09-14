c ==========================================================
c Function filter4. Broadband filreting.
c ==========================================================
c Parameters for filter4 function:
c Input parameters:
c f1,f2   - low corner frequences, f2 > f1, Hz, (double)
c f3,f4   - high corner frequences, f4 > f3, Hz, (double)
c npow    - power of cosine tapering,  (int)
c dt      - sampling rate of seismogram in seconds, (double)
c n       - number of input samples, (int)
c seis_in - input array length of n, (float)
c Output parameters:
c seis_out - output array length of n, (float)
c ==========================================================

      subroutine filter4(f1,f2,f3,f4,npow,dt,n,seis_in,
     1  seissm,seis_out,seis_outamp,seis_outph,ns,dom)
      implicit none
      include 'fftw3.h'
      integer*4 npow,n
      real*8    f1,f2,f3,f4,dt
      real*4    seis_in(7500000),seis_out(7500000)
      real*4    seissm(750000), seis_outamp(1000000), seis_outph(1000000)
c ---
      integer*4 k,ns,nk,i
      real*8    plan1,plan2
      real*8    dom
      double complex czero,s(7500000),sf(1000000)
c ---
      czero = (0.0d0,0.0d0)



c determin the power of FFT
      ns = 2**max0(int(dlog(dble(n))/dlog(2.0d0))+1,13)
      dom = 1.0d0/dt/ns

      do k = 1,ns
        s(k) = czero
      enddo

      do k = 1,n
        s(k) = seis_in(k)
      enddo

c make backward FFT for seismogram: s ==> sf
      call dfftw_plan_dft_1d(plan1,ns,s,sf,
     *                         FFTW_BACKWARD, FFTW_ESTIMATE)
      call dfftw_execute(plan1)
      call dfftw_destroy_plan(plan1)
c kill half spectra and correct ends

      nk = ns/2+1
      do k = nk+1,ns
        sf(k) = czero
      enddo
 
      sf(1) = sf(1)/2.0d0
      sf(nk) = dcmplx(dreal(sf(n)),0.0d0)

c  
       do k = 1,nk
          seis_outamp(k) = 0.0
          seis_outph(k)  = 0.0
       enddo

c=============================================================
c     do smoothing on sf equivalent to do " smooth mean h 20" in SAC

      call smooth(f1,f2,f3,f4,dom,nk,sf,seissm)
C=============================================================
c===============================================================
c   make tapering
      call flt4(f1,f2,f3,f4,dom,nk,npow,sf)

       do i = 1,nk
        seis_outamp(i)= real(dsqrt(dreal(sf(i))**2 +
     1                        dimag(sf(i))**2))
        seis_outph(i) = real(datan2(dimag(sf(i)),dreal(sf(i))))
       enddo


c make forward FFT for seismogram: sf ==> s
c      call dfftw_plan_dft_1d(plan2,ns,sf,s,
c     *                         FFTW_FORWARD, FFTW_ESTIMATE)
c      call dfftw_execute(plan2)
c      call dfftw_destroy_plan(plan2)
c forming final result

c      do k = 1,n
c        seis_out(k) = 2.0*real(dreal(s(k)))/ns
c      enddo



      return
      end



c===============================================================
c Tapering subroutine itself
c===============================================================
      subroutine flt4(f1,f2,f3,f4,dom,nk,npow,sf)
      real*8    f1,f2,f3,f4,dom
      integer*4 nk,npow
      double complex sf(7500000)
      real*8    d1,d2,f,dpi,ss,s(7500000)
      integer*4 i,j
c ---
      dpi = datan(1.0d0)*4.0d0
      do i = 1,nk
         s(i) = 0.0d0
      enddo
      do i = 1,nk
        f = (i-1)*dom
        if(f.le.f1) then
          goto 1
        else if(f.le.f2) then
          d1 = dpi/(f2-f1)
          ss = 1.0d0
          do j = 1,npow
            ss = ss*(1-dcos(d1*(f1-f)))/2.0d0
          enddo
          s(i) = ss
        else if(f.le.f3) then
           s(i) = 1.0d0
        else if(f.le.f4) then
          d2 = dpi/(f4-f3)
          ss = 1.0d0
          do j = 1,npow
            ss = ss*(1+dcos(d2*(f3-f)))/2.0d0
          enddo
          s(i) = ss
        endif
  1     continue
      enddo
      do i = 1,nk
        sf(i) = sf(i)*s(i)
      enddo
      return
      end


c===============================================================
c  smoothing routine      call smooth(f1,f2,f3,f4,dom,nk,sf,seissm)
c=s==============================================================
      subroutine smooth(f1,f2,f3,f4,dom,nk,sf,seissm)
      real*8    f1,f2,f3,f4
      integer*4 nk
      double complex sf(7500000)
      real*4    seissm(750000)
      real*8    sorig(7500000), sout(7500000),dom
      real*8    f,sum, avg
c ---
        do i = 1,nk
         sorig(i) = dsqrt(dreal(sf(i))**2+dimag(sf(i))**2)
        enddo
c        do i = 1,nk

c        f = (i-1)*dom
c        if( f .ge. f1 .and. f .le. f4 ) then
c            sum = 0. 
c          do jk = -number,number
c             ijk = i+jk
c             sum = sum + sorig(ijk)
c          enddo
c            sout(i) = seissm(i)+0.1
c        else
c            sout(i) = sorig(i)
c        endif

c       enddo

      do i = 1,nk
        f = (i-1)*dom
      if( f .ge. f1 .and. f .le. f4 ) then
         sf(i) = sf(i)*(1.0d0/seissm(i))
      else
         sf(i) = sf(i)*0.0d0
      endif
          
      enddo


c        do i = 1,nk
c           sf(i) = sf(i)*sout(i)
c        enddo

       return
 
       end


