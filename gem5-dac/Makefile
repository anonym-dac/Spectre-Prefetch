GEM5_RISCV_SIMULATOR   =  ./build/RISCV/gem5.opt
GEM5_X86_SIMULATOR   =  ./build/X86/gem5.opt

DEBUGFLAGS 		+= 	--debug-flags=HWPrefetch \
			 		--debug-file=trace.out \

RISCV_CPU += --cpu-type=RiscvO3CPU
X86_CPU += --cpu-type=DerivO3CPU
# --cpu-type=RiscvO3CPU \

BOOMFLAGS     += configs/example/my_se.py \
          --bp-type=BiModeBP \
          --caches \
          --l2cache \
          --cacheline=64 \
          --num-l2cache=1 \
          --l1i_size=64kB \
          --l1i_assoc=4 \
          --l1d_size=16kB \
          --l1d_assoc=4 \
          --l2_size=256kB \
          --l2_assoc=4 \
          --mem-size=8192MB \
          --l1d-hwp-type=StridePrefetcher \
          --defense-type=unsafePrefetch

          # --l1d-hwp-type=StridePrefetcher\
# unsafePrefetch \choices=["unsafePrefetch", "ingoreSpecPrefetch","confusionPrefetch"],

ATTACk_HOME_RISCV=../attack_on_gem5/attack_on_riscv/bin
ATTACk_HOME_X86=../attack_on_gem5/attack_on_x86/bin

help:
	$(GEM5_RISCV_SIMULATOR) configs/example/se.py --help

debug-smix:
	$(GEM5_RISCV_SIMULATOR) $(DEBUGFLAGS) $(BOOMFLAGS) $(RISCV_CPU) --cmd=$(ATTACk_HOME_RISCV)/smix.riscv

smix:
	$(GEM5_RISCV_SIMULATOR) $(BOOMFLAGS) $(RISCV_CPU) --cmd=$(ATTACk_HOME_RISCV)/smix.riscv

s1:
	$(GEM5_RISCV_SIMULATOR) $(BOOMFLAGS) $(RISCV_CPU) --cmd=$(ATTACk_HOME_RISCV)/spectre-v1.riscv

spAttack1:
	$(GEM5_RISCV_SIMULATOR) $(DEBUGFLAGS) $(BOOMFLAGS) $(RISCV_CPU) --cmd=$(ATTACk_HOME_RISCV)/SpectrePrefetchV1.riscv

spAttack2:
	$(GEM5_RISCV_SIMULATOR) $(BOOMFLAGS) $(RISCV_CPU) --cmd=$(ATTACk_HOME_RISCV)/SpectrePrefetchV2.riscv

spAttack1-x86:
	$(GEM5_X86_SIMULATOR) $(DEBUGFLAGS) $(BOOMFLAGS) $(X86_CPU) --cmd=$(ATTACk_HOME_X86)/SpectrePrefetchV1.x86

spAttack2-x86:
	$(GEM5_X86_SIMULATOR) $(BOOMFLAGS) $(X86_CPU) --cmd=$(ATTACk_HOME_X86)/SpectrePrefetchV2.x86


build:
	scons $(GEM5_X86_SIMULATOR) -j17
