#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "cachesim.h"

void runTrace(FILE* fin, uint64_t c, uint64_t b, uint64_t s, uint64_t currSize, double *minAAT, uint64_t *bestC, uint64_t *bestB, uint64_t *bestS, uint64_t *totalSize);

int main(int argc, char* argv[]) {
    FILE* fin;

    double minAAT = DBL_MAX;
    uint64_t bestC = 0;
    uint64_t bestB = 0;
    uint64_t bestS = 0;
    uint64_t totalSize = 0;
    //for (uint64_t c = 15; c >= 0; c--) {
    uint64_t c = 0;
    uint64_t currSize = 0;
    do {
        //if (c > 15)
            //c = 0;
        for (uint64_t b = 0; b <= c && b <= 6; b++) {
            for (uint64_t s = 0; s <= c - b; s++) {
                currSize = (66 - c + s) * ((1 << c) / (1 << b));
                if (currSize <= (1 << 15)) {
                    fin  = fopen("traces/astar.trace", "r");
                    runTrace(fin, c, b, s, currSize, &minAAT, &bestC, &bestB, &bestS, &totalSize);
                }
            }
        }
        //if (c == 0)
            //break;
        c++;
    } while(c < 32);

    printf("minAAT = %f\n", minAAT);
    printf("bestC = %" PRIu64 "\n", bestC);
    printf("bestB = %" PRIu64 "\n", bestB);
    printf("bestS = %" PRIu64 "\n", bestS);
    printf("total cache size = %" PRIu64 "\n", totalSize);
    
    return 0;
}

void runTrace(FILE* fin, uint64_t c, uint64_t b, uint64_t s, uint64_t currSize, double *minAAT, uint64_t *bestC, uint64_t *bestB, uint64_t *bestS, uint64_t *totalSize) {
    printf("c = %" PRIu64 "\n", c);
    printf("b = %" PRIu64 "\n", b);
    printf("s = %" PRIu64 "\n", s);

    /* Setup the cache */
    cache_init(c, s, b);

    /* Setup statistics */
    struct cache_stats_t stats;
    memset(&stats, 0, sizeof(struct cache_stats_t));
    stats.miss_penalty = 100;
    stats.access_time = 2;

    /* Begin reading the file */ 
    char rw;
    uint64_t address;
    while (!feof(fin)) { 
        int ret = fscanf(fin, "%c %" PRIx64 "\n", &rw, &address); 
        if(ret == 2) {
            cache_access(rw, address, &stats);
        }
    }

    cache_cleanup(&stats);
    
    if (stats.avg_access_time <= *minAAT) {
        *minAAT = stats.avg_access_time;
        *bestC = c;
        *bestB = b;
        *bestS = s;
        *totalSize = currSize;
    }

    fclose(fin);
}
