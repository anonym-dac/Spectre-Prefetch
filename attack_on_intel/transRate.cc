#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>

#include <time.h>
#include <unistd.h>

#define ATTACK_SAME_ROUNDS  10
#define TRAIN_TIMES         6
#define ROUNDS              1
#include "utils.hh"
// #include <string>
#include <random>
// #include <memory>
// #include <fstream>

#ifdef _MSC_VER
#include <intrin.h> /* for rdtsc, rdtscp, clflush */
#pragma optimize("gt",on)
#else
#include <x86intrin.h> /* for rdtsc, rdtscp, clflush */
#endif /* ifdef _MSC_VER */

/* Automatically detect if SSE2 is not available when SSE is advertized */
#ifdef _MSC_VER
/* MSC */
#if _M_IX86_FP==1
#define NOSSE2
#endif
#else
/* Not MSC */
#if defined(__SSE__) && !defined(__SSE2__)
#define NOSSE2
#endif
#endif /* ifdef _MSC_VER */

#ifdef NOSSE2
#define NORDTSCP
#define NOMFENCE
#define NOCLFLUSH
#endif

#include <sys/time.h>

#define CPU_ID 49
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))

unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// uint8_t array1[160] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
uint8_t unused2[64];
uint8_t array2[256 * 64];

char * secret = "*secREt~";

uint8_t secretBits[1000];
int results[1000]={0};
// char * secret = "s";

int cache_hit_threshold = 140;
int MissLatency = 0;
int HitLatency = 0;

uint8_t temp = 0; /* Used so compiler won’t optimize out victim_function() */

struct timespec const t_req{ .tv_sec = 0, .tv_nsec = 1000 /* 1µs */ };//秒0，纳秒1us
struct timespec t_rem;


// #define INTEL_MSR_MISC_FEATURE_CONTROL (0x1A4)
/**
 * Enum to keep the bit masks to select the prefetcher we want to control
 * via MSR_MISC_FEATURE_CONTROL register on Intel CPUs.
 */
// enum intel_prefetcher_t {
// 	INTEL_L2_HW_PREFETCHER          = 0b0001ULL,
// 	INTEL_L2_ADJACENT_CL_PREFETCHER = 0b0010ULL,
// 	INTEL_DCU_PREFETCHER            = 0b0100ULL,
// 	INTEL_DCU_IP_PREFETCHER         = 0b1000ULL,
// };

void prepare(int cpu){
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(CPU_ID, &mask);  // 设置为只在核心1上运行

  if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {//0 is pid, self
    printf("sched_setaffinity");
    return;
  }else{
    printf("Pin at Core %d.\n", CPU_ID);
  }
  uint32_t msr_reg = INTEL_MSR_MISC_FEATURE_CONTROL;
  uint64_t msr_value = rdmsr(CPU_ID, msr_reg);
  printf("Reading msr_value 0x%lx.\n", msr_value);
//   msr_value = rdmsr(CPU_ID, msr_reg);
//   printf("Now msr_value 0x%lx.\n", msr_value);

}

__attribute__((always_inline)) static inline uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}

__attribute__((always_inline)) static inline void mfence() { 
  asm volatile("mfence"); 
}

void maccess(void* p)
{
  asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
}


__attribute__((always_inline)) size_t accessLatency(uint8_t* p)
{
  size_t time = rdtsc();
  asm volatile ("movq (%0), %%rax\n"
    :
    : "c" (p)
    : "rax");
  size_t delta = rdtsc() - time;
  return delta;
}


void randomAccess(){
  for(int i=0; i < 10; i++){
    int ran_v = random() % 256;
    maccess(array2 + ran_v * 64);
  }
}

int main(){
    prepare(CPU_ID);
    size_t malicious_x = (size_t)(secret - (char * ) array1);
    MissLatency =  accessLatency(array2);//11*64
    printf("Miss = %d\t", MissLatency);

    int i;
    //initialize secret array
    for (i = 0 ; i<1000; i++){
        secretBits[i] = random() % 2;// random 
    }
    for (i = 0; i < (int)sizeof(array2); i++) {
        array2[i] = 1; /* write to array2 so in RAM not copy-on-write zero pages */
    }

    nanosleep(&t_req, &t_rem);
    HitLatency =  accessLatency(array2);//11*64
    printf("Hit = %d\n", HitLatency);
    cache_hit_threshold = (MissLatency + HitLatency)/2;
    printf("Using a cache hit threshold of %d.\n", cache_hit_threshold);


    printf("Running at Intel(R) Xeon(R) Gold 6248R CPU.\nThe secret is '*secREt~'\n");
     /* Default addresses to read is 40 (which is the length of the secret string) */

    clock_t start = clock();
    for(int pos = 0; pos < 1000; pos++){
        for (i = 0; i < 256; i++) _mm_clflush( & array2[i * 64]); /* intrinsic for clflush instruction */
        // Step1. Receiver
      {
        maccess(array2 + 64); //33d68  1*64
        maccess(array2 + 192);//33de8 3*64
        maccess(array2 + 320);// 5*64
        mfence();
      }
        // Step2. Sender
        if(secretBits[pos]){
            maccess(array2);
        }
        // Step3. Receiver
        maccess(array2 + 448);// 7*64
        nanosleep(&t_req, &t_rem);
        results[pos] += accessLatency(array2 + 576);//9*64
    }
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Execution time %f s\n", time_spent);

    for(i=0;i<1000;i++){
        printf("%d\t%d\t%d\n",i, secretBits[i], results[i]);
    }
    return 0;
}
