#!/bin/bash
#
# run_gem5_alpha_spec06_benchmark.sh
# Author: Mark Gottscho Email: mgottscho@ucla.edu
# Copyright (C) 2014 Mark Gottscho
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

GEM5_DIR=../gem5-dac


fastForward=(400000000    395000000         470000000  
             20000000   120000000   165000000   0     45000000
             160000000   0   5000000    80000000    430000000 
             0          0       15000000        29000000        0
             9500000    0       4500000)

cpu17=(perlbench_r gcc_r mcf_r 
        x264_r bwaves_r cactuBSSN_r  parest_r povray_r 
        lbm_r wrf_r blender_r  cam4_r  roms_r 
        gcc_s mcf_s omnetpp_s x264_s wrf_s
        pop2_s nab_s roms_s)

# defense_type=(unsafePrefetch,ingoreSpecPrefetch,confusionPrefetch)
# fastForward=(20000000   120000000   165000000   0)

# cpu17=(x264_r bwaves_r cactuBSSN_r  parest_r )
        
# fastForward=(4500000)

# cpu17=(roms_s)

# for i in "${!cpu17[@]}"
# do
# cd $GEM5_DIR/scripts
# source ./run_spec17.sh "${cpu17[$i]}" baselineM None "${fastForward[$i]}" unsafePrefetch
# done

# choices=["unsafePrefetch", "ingoreSpecPrefetch","confusionPrefetch"]
#SDP
# for i in "${!cpu17[@]}"
# do
# cd $GEM5_DIR/scripts
# source ./run_spec17.sh "${cpu17[$i]}" baselineM StridePrefetcher "${fastForward[$i]}" unsafePrefetch
# done

# for i in "${!cpu17[@]}"
# do
# cd $GEM5_DIR/scripts
# source ./run_spec17.sh "${cpu17[$i]}" baselineM StridePrefetcher "${fastForward[$i]}" ingoreSpecPrefetch
# done

# for i in "${!cpu17[@]}"
# do
# cd $GEM5_DIR/scripts 
# source ./run_spec17.sh "${cpu17[$i]}" baselineM StridePrefetcher "${fastForward[$i]}" confusionPrefetch
# done





# # ## DCPT
# for i in "${!cpu17[@]}"
# do
# cd $GEM5_DIR/scripts
# source ./run_spec17.sh "${cpu17[$i]}" baselineM DCPTPrefetcher "${fastForward[$i]}"
# done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM DCPTPrefetcher "${fastForward[$i]}" unsafePrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM DCPTPrefetcher "${fastForward[$i]}" ingoreSpecPrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM DCPTPrefetcher "${fastForward[$i]}" confusionPrefetch
done

# # # IMP
# for i in "${!cpu17[@]}"
# do
# cd $GEM5_DIR/scripts
# source ./run_spec17.sh "${cpu17[$i]}" baselineM IndirectMemoryPrefetcher "${fastForward[$i]}"
# done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM IndirectMemoryPrefetcher "${fastForward[$i]}" unsafePrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM IndirectMemoryPrefetcher "${fastForward[$i]}" ingoreSpecPrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM IndirectMemoryPrefetcher "${fastForward[$i]}" confusionPrefetch
done

# # # AMPM
# for i in "${!cpu17[@]}"
# do
# cd $GEM5_DIR/scripts
# source ./run_spec17.sh "${cpu17[$i]}" baselineM AMPMPrefetcher "${fastForward[$i]}"
# done

#  AMPM
for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM AMPMPrefetcher "${fastForward[$i]}" unsafePrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM AMPMPrefetcher "${fastForward[$i]}" ingoreSpecPrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM AMPMPrefetcher "${fastForward[$i]}" confusionPrefetch
done

# SPP
for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM SignaturePathPrefetcher "${fastForward[$i]}" unsafePrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM SignaturePathPrefetcher "${fastForward[$i]}" ingoreSpecPrefetch
done

for i in "${!cpu17[@]}"
do
cd $GEM5_DIR/scripts
source ./run_spec17.sh "${cpu17[$i]}" baselineM SignaturePathPrefetcher "${fastForward[$i]}" confusionPrefetch
done