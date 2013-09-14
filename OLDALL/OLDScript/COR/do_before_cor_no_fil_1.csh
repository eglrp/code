#!/bin/csh
#foreach  year  (2007 2008)
#set year = 2007
#需要调试，看看运行完成之后有哪些文件需要删除#
set year = 2008
set bp = 5to150
set mainpath = /utera/tianye/data_locate_src
#set sta_reverse_lst = ${mainpath}/sta_reverse.lst #provide absorlute path
set station_lst = ${mainpath}/station.lst
#set sta_unit_lst =  #provide absorlute path

cd ${mainpath}
#if($year == 2007) then
#foreach month ( JUL AUG SEP OCT NOV DEC )
foreach month ( JAN FEB MAR APR MAY JUN )
	if( $month == JAN ) @ nmonth =  1
	if( $month == FEB ) @ nmonth =  2
	if( $month == MAR ) @ nmonth =  3
	if( $month == APR ) @ nmonth =  4
	if( $month == MAY ) @ nmonth =  5
	if( $month == JUN ) @ nmonth =  6
	if( $month == JUL ) @ nmonth =  7
	if( $month == AUG ) @ nmonth =  8
	if( $month == SEP ) @ nmonth =  9
	if( $month == OCT ) @ nmonth = 10
	if( $month == NOV ) @ nmonth = 11
	if( $month == DEC ) @ nmonth = 12

        mkdir -p $year'.'$month
       # mv $year'.'$month'.'*'.SAC' $year$month
	cd $year'.'$month
        mv 'monthly_sac_files/'$year'.'$month'.'*'.SAC' .
######a detail of station.lst needs to be modified. More specificlly, province name should be added before station name, like "ProvStaName", eg. HLBAQ.#### 
	\rm -f event.dat
	\rm ft_20*.SAC

	foreach day1 ( 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31  )
#	foreach day1 ( 1 2 3 )
######if need to do de_resp, then need to remove the origional ft_fname files	
	
	echo $nmonth $day1 > temp.dat
	awk -v ytp=$year '{  printf "%-4s %2s %2s 000000.00\n",ytp,$1, $2}' temp.dat  >> event.dat
	end #foreach day1

set event_dat = ${mainpath}/$year'.'$month/event.dat	
######cut and trans, in the program the output ft_bit is different from the zy's version#
######modify the cut_tran_resp_bit to fit your use, we can just do the bit without doing transfer#	
	\rm sac_db.out event_station.tbl 

	echo "###############set_sacdb_CheckSamp_bit"
	/home/tianye/code/Programs/do_before_cor/de_resp/set_sacdb_CheckSamp_bit LHZ ${station_lst} ${event_dat}

        set i=1
        while( $i <= 31 )
        if( `ls $year'.'$month'.'$i | wc -l` == 0 ) rm -rf $year'.'$month'.'$i
        @ i += 1
        end

	echo "###############cut_trans_resp_bit"
	/home/tianye/code/Programs/do_before_cor/de_resp/cut_trans_resp_bit 0 86401

	\rm temp.dat sac_db.out
#######special cases: for the stations have pi shift, use the "reverse"; for those have pi/2 shift, use "unit"#
##	\rm sac_db_reverse.out  event_station_reverse.tbl
##	echo "################set_sacdb_China_reverse"
##	/Users/jiayixie/progs/jy/do_before_cor/de_resp/set_sacdb_China_reverse BHZ ${sta_reverse_lst}
##	echo "################reverse_used_with_db"
##	/Users/jiayixie/progs/jy/do_before_cor/de_resp/reverse_used_with_db
##	\rm sac_db_reverse.out 
#######for the unit, choose d (differentiate) or i (integrate)
##	/Users/jiayixie/progs/jy/do_before_cor/de_resp/set_sacdb_CheckSamp_unit BHZ ${sta_unit_lst}
##	/Users/jiayixie/progs/jy/do_before_cor/de_resp/unit_used_with_db d
##	/home/tianye/code/Programs/do_before_cor/de_resp/unit_used_with_db d		
        mkdir -p monthly_sac_files
        mv $year'.'$month'.'*'.SAC' monthly_sac_files
	mkdir -p $bp
	cd $bp


	foreach day ( 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31  )
#	foreach day ( 1 2 3 )
		set dirday =  ${year}.${month}.${day}
                rm -rf $dirday
		mkdir $dirday
		cd $dirday
		cp '../../'$dirday'/ft_'$year'.'$month'.'$day'.'* .
		ls ft_*SAC > temp.ft
                awk '{print "40 30 10 7.5 1 1  ", $1}' temp.ft > param.dat
                /home/tianye/code/Programs/do_before_cor/filter4/filter4 param.dat 1
		awk '{print "100 80 5 4 1 1  ", $1}' temp.ft > param.dat
		/home/tianye/code/Programs/do_before_cor/filter4/filter4 param.dat 0
                
#######here we can choose if include the filter in whiten process (whiten_rej_phamp_no/with_fil)
                awk '{print "80 20 5 4 1 1  ", $1, $1"_10.0_30.0"}' temp.ft > param.dat
		/home/tianye/code/Programs/do_before_cor/whiten_outphamp_C/whiten_rej_phamp_no_fil  param.dat /home/tianye/code/Programs/do_before_cor/whiten_outphamp_C/Stack.smooth
		\rm ft_*.SAC ft_*.SAC_10.0_30.0
#		\rm ft_*.SAC_bit
		\rm ft_*.SAC_old
		\rm temp.ft param.dat
                rm -rf '../../'$dirday
		cd ../
                set nfile=`ls $dirday | wc -l`
                if( $nfile == 0 )rm -rf $dirday
	end #foreach day

	cd ../../
        mv $year'.'$month'/'$bp '/data/ulisse/tianye/data_locate_src/'$year'.'$month
end #foreach month

