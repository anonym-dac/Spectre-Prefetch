/*********************************************************************
*
* Spectre PoC
*
* This source code originates from the example code provided in the 
* "Spectre Attacks: Exploiting Speculative Execution" paper found at
* https://spectreattack.com/spectre.pdf
*
* Minor modifications have been made to fix compilation errors and
* improve documentation where possible.
*
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include "cacheutils.hh"
#include "utils.hh"
#include "mapping.hh"
#include <random>
#include <sys/mman.h>
#include <x86intrin.h>
#include "time.h"

void maccess(void* p)
{
  asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
}

// TIME STAMP END
size_t accessLatency(uint8_t* addr)
{
  // time1 = __rdtscp( & junk); /* READ TIMER */
  // junk = * addr; /* MEMORY ACCESS TO TIME */
  // time2 = __rdtscp( & junk) - time1; 
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  return delta;
}

#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define CPU_ID 0
#define LINE_SIZE 64
#define Items 2560

size_t prefetch_histogram[1000] = {0};
size_t noPrefetch_histogram[1000] = {0};

unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[16] = {0};
uint8_t unused2[64];
uint8_t array2[Items * LINE_SIZE];
size_t res[2000] = {0};

void randomAccess(){
  for(int i=0; i < 100; i++){
    int ran_v = random() % Items;
    maccess(array2 + ran_v * LINE_SIZE);
  }
}

void PinCore(int cpu){
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(CPU_ID, &mask);  // 设置为只在核心1上运行

  if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {//0 is pid, self
    printf("sched_setaffinity");
    return;
  }else{
    printf("Pin at Core %d.\n", CPU_ID);
  }
}

void EnableStride(int cpu){
  uint32_t msr_reg = INTEL_MSR_MISC_FEATURE_CONTROL;
  uint64_t msr_value = rdmsr(CPU_ID, msr_reg);
  printf("Enable Stride....Reading msr_value 0x%lx.\n", msr_value);
  set_intel_prefetcher(cpu, INTEL_L2_HW_PREFETCHER, false);
  set_intel_prefetcher(cpu, INTEL_L2_ADJACENT_CL_PREFETCHER, false);
  set_intel_prefetcher(cpu, INTEL_DCU_PREFETCHER, false);
  set_intel_prefetcher(cpu, INTEL_DCU_IP_PREFETCHER, true);
  msr_value = rdmsr(CPU_ID, msr_reg);
  printf("Now msr_value 0x%lx.\n", msr_value);
}

void DisableStride(int cpu){
  uint32_t msr_reg = INTEL_MSR_MISC_FEATURE_CONTROL;
  uint64_t msr_value = rdmsr(CPU_ID, msr_reg);
  printf("Disable Stride....Reading msr_value 0x%lx.\n", msr_value);
  set_intel_prefetcher(cpu, INTEL_L2_HW_PREFETCHER, false);
  set_intel_prefetcher(cpu, INTEL_L2_ADJACENT_CL_PREFETCHER, false);
  set_intel_prefetcher(cpu, INTEL_DCU_PREFETCHER, false);
  set_intel_prefetcher(cpu, INTEL_DCU_IP_PREFETCHER, false);
  msr_value = rdmsr(CPU_ID, msr_reg);
  printf("Now msr_value 0x%lx.\n", msr_value);
}

int main(){
  register uint64_t time1, time2;
  volatile uint8_t * addr;
  unsigned int junk = 0;

  PinCore(CPU_ID);
  struct timespec const t_req{ .tv_sec = 0, .tv_nsec = 15000 /* 1µs */ };//秒0，纳秒1us
  struct timespec t_rem;

  EnableStride(CPU_ID);
  int i;    
  for (i = 0; i < (int)sizeof(array2); i++) {
    maccess(&array2[i]);
  }
  for (int i = 0; i < Items; i++){
    flush(&array2[i * LINE_SIZE]); /* intrinsic for clflush instruction */
  }

  int stride = 5;

  int cnt = 0;
  int rounds = 1000;
  EnableStride(CPU_ID);
  for(int train_step = 5; train_step < 10; train_step++){
    for(int times = 0; times <200; times++){//
      for(int i=0; i < 256; i++){res[i] = 0;}
      // for (volatile int z = 0; z < 100; z++) {}
      for(int t = 0; t< rounds; t++){
            randomAccess();
            for (int i = 0; i < Items; i++){
                flush(&array2[i * LINE_SIZE]); /* intrinsic for clflush instruction */
            }
            // mfence();
            for(int step = 0; step < train_step; step++){
                maccess(&array2[step * stride * LINE_SIZE]);
                mfence();
                // sched_yield();
            }
            nanosleep(&t_req, &t_rem);
            // mfence();
            // res[0] += accessLatency(&array2[0* stride* LINE_SIZE]);
            addr = &array2[train_step * stride* LINE_SIZE];
            time1 = __rdtscp( & junk); /* READ TIMER */
            junk = * addr; /* MEMORY ACCESS TO TIME */
            time2 = __rdtscp( & junk) - time1; 
            res[cnt] += time2;
            
      }
      // printf("%d\t%d\n",cnt, res[cnt]/rounds);
      prefetch_histogram[MIN(599,res[cnt]/rounds)]++;
      // prefetch_histogram[cnt]  = res[cnt]/rounds;
      cnt++; 
      
    }
  }

  DisableStride(CPU_ID);
  cnt = 0;
  // for(int train_step = 5; train_step < ; train_step++){
    int train_step = 8;
    for(int times = 0; times <1000; times++){//
      for(int i=0; i < 256; i++){res[i] = 0;}
      // for (volatile int z = 0; z < 100; z++) {}
      for(int t = 0; t< rounds; t++){
            randomAccess();
            for (int i = 0; i < Items; i++){
                flush(&array2[i * LINE_SIZE]); /* intrinsic for clflush instruction */
            }
            // mfence();
            for(int step = 0; step < train_step; step++){
                maccess(&array2[step * stride * LINE_SIZE]);
                mfence();
                // sched_yield();
            }
            nanosleep(&t_req, &t_rem);
            // mfence();
            // res[0] += accessLatency(&array2[0* stride* LINE_SIZE]);
            addr = &array2[train_step * stride* LINE_SIZE];
            time1 = __rdtscp( & junk); /* READ TIMER */
            junk = * addr; /* MEMORY ACCESS TO TIME */
            time2 = __rdtscp( & junk) - time1; 
            res[cnt] += time2;
      }
      // printf("cnt %d, Miss Latency %d\n", cnt, res[cnt]/rounds );
      // noPrefetch_histogram[cnt]  = res[cnt]/rounds;
      noPrefetch_histogram[MIN(599, res[cnt]/rounds)]++;
      cnt++; 
    }

    // printf("Hit Latency = %d, Miss Latency %d\n", res[0]/rounds, res[1]/rounds);
  for(int i=0;i<600;i++){
    printf("%d\t%d\n", prefetch_histogram[i], noPrefetch_histogram[i]);
  }
  return 0;
}
