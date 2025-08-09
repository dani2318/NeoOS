#include "stdlib.h"
#include <stdio.h>
void iqsort(void* base, size_t num, size_t size,size_t left, size_t right, int (*compare)(const void* a, const void* b)){
    
    if(left >= right) return;

    int i = left, j = right;
    uint8_t pivot[size];
    memcpy(pivot, (uint8_t*)base + (i * size), size);
    uint8_t tmp;
    for(;;){
        while((*compare)(base + (i * size),pivot) < 0){ i++; };
        while((*compare)(pivot,base + (j * size)) < 0){ j--; };
        if (i >= j) break;
        
        uint8_t tmpBuf[size]; // temporary buffer
        memcpy(tmpBuf, (uint8_t*)base + i * size, size);
        memcpy((uint8_t*)base + i * size, (uint8_t*)base + j * size, size);
        memcpy((uint8_t*)base + j * size, tmpBuf, size);

        i++;
        j--;
   }
   iqsort(base,num,size,left,i-1, compare);
   iqsort(base,num,size,j+1,right, compare);
}

void qsort(void* base, size_t num, size_t size, int (*compare)(const void* a, const void* b)){
    iqsort(base, num, size, 0, num - 1, compare);
}