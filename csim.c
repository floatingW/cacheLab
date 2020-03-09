/*
 * Wei Fu
 * 961028
 *
 * compile without warnings
 * ignore all instruction cache accesses
 * must call printSummary at the end of main function
 * ignore the request sizes in traces
 * cache uses the LRU replacement policy
 * about 200-300 lines
 */

#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#define LINE_LEN 16 

/* function cache return value */
#define HITS 1
#define MISSES 2
#define EVICTIONS 3

/* for LRU */
static int cycle = 0;

void printUsage();
void printHits(int);
void printInstruction(char, long, int);
void countHits(int, int *, int *, int*);
void get_idx_tag(long, long *, long *, int, int);

typedef struct line {
    bool v;
    int last;
    int tag;
} line_t;

typedef struct set {
    int lru;
    line_t *lines;
} set_t;

typedef struct cache {
    int s;
    int e;
    int b;
    set_t *sets;
} cache_t;

void myfree(cache_t *);
cache_t *do_cache_initialization(int, int, int);
int cache(cache_t *, long, long);
int main(int argc, char *argv[]) {

    extern char *optarg;
    extern int optind, opterr, optopt;

    int verbose = 0;
    int error = 0;
    int s = -1;
    int e = -1;
    int b = -1;
    FILE *file;
    char opt;

    /* reading arg */
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(opt) {
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                e = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                file = fopen(optarg, "read");
                break;
            case 'h':
            default:
                error = 1;
        }
    }

    /* error check */
    if(error == 1 || s < 1 || e < 1 || b < 1 || file == NULL) {
        printUsage();
        exit(1);
    }

    /* do cache struct initialization */
    cache_t *ca;
    ca = do_cache_initialization(s, e, b);
    if(ca == NULL) {
        exit(1);
    } 

    /* do cache processing */
    char *line, *str_addr, *str_size;

    char op;
    int size;
    unsigned long addr;

    int hits = 0;
    int misses = 0;
    int evictions = 0;

    long set_index = 0;
    long tag = 0;

    int cache_hits = -1;
    line = calloc(LINE_LEN, sizeof(char)); 
    char *p;
    while(fgets(line, LINE_LEN, file) != NULL) {
        p = line;
        if(*p== 'I') continue;
        p++;
        str_size = strstr(p, ",");
        if(str_size) {
            size = atoi(++str_size);
        } else continue;
        op = *p;
        p+=2;
        str_addr = strtok(p, ",");
        sscanf(str_addr, "%lx", &addr);
        /* cache handling */
        get_idx_tag(addr, &set_index, &tag, s, b);
        //printf("%ld, %ld\n", set_index, tag);
        switch(op) {
            case 'L':
            case 'S':
                cache_hits = cache(ca, set_index, tag);
                countHits(cache_hits, &hits, &misses, &evictions);
                if(verbose) {
                    printInstruction(op, addr, size);
                    printHits(cache_hits);
                    printf("\n");
                }
                break;
            case 'M':
                cache_hits = cache(ca, set_index, tag);
                if(verbose) {
                    countHits(cache_hits, &hits, &misses, &evictions);
                    printInstruction(op, addr, size);
                    printHits(cache_hits);
                }
                cache_hits = cache(ca, set_index, tag);
                if(verbose) {
                    countHits(cache_hits, &hits, &misses, &evictions);
                    printHits(cache_hits);
                    printf("\n");
                }
                break;
        }
    }

    /* output summary */ 
    fclose(file);
    printSummary(hits, misses, evictions);
    return 0;
}

void printUsage() {
    // to do
    printf("Usage: brbrbr\n");
}

void printInstruction(char op, long addr, int size) {
    printf("%c %lx,%d", op, addr, size);
}

void printHits(int hits) {
    if(hits == HITS) printf(" hit");
    if(hits == MISSES) printf(" miss");
    if(hits == EVICTIONS) printf(" miss eviction");
}

void countHits(int hit, int *hits, int *misses, int *evictions) {
    if(hit == HITS) ++*hits;
    if(hit == MISSES) ++*misses;
    if(hit == EVICTIONS) {
        ++*misses;
        ++*evictions;
    }
}

void myfree(cache_t * ca) {
    set_t *sets;
    line_t *lines;
    if(ca) {
        if(ca->sets) {
            sets = ca->sets;
            for(int i = 0; i < (2 << (ca->s - 1)); ++i) {
                if((lines = (sets + i)->lines)) {
                    free(lines);
                }
            }
            free(sets);
        }
        free(ca);
    }
}

cache_t *do_cache_initialization(int s, int e, int b) {
    cache_t *ca;
    ca = (cache_t*)malloc(sizeof(cache_t));
    ca->s = s;
    ca->e = e;
    ca->b = b;
    if(ca == NULL) return NULL;
    ca->sets = (set_t*)calloc(2 << (s - 1), sizeof(set_t));
    if(ca->sets == NULL) {
        myfree(ca);
        return NULL;
    }
    set_t *sets = ca->sets;
    for(int i = 0; i < (2<<(s-1)); ++i) {
        sets->lines = (line_t*)calloc(e, sizeof(line_t));
        if(ca->sets->lines == NULL) {
            myfree(ca);
            return NULL;
        }
        ++sets;
    }
    return ca;
}

void get_idx_tag(long addr, long *idx, long *tag, int s, int b) {
    long mask = 0x8000000000000000;
    *idx = (addr >> b) & (~(mask >> (63 - s)));
    *tag = (addr >> (s + b)) & (~(mask >> (s + b - 1)));
}

int cache(cache_t *ca, long idx, long tag) {
    set_t *set = ca->sets + idx;
    line_t *line = set->lines;
    int result;
    bool found = false;
    ++cycle;
    for(int i = 0; i < ca->e; ++i) {
        if(line->v) {
            if(line->tag == tag) {
                found = true;
                result = HITS;
                line->last = cycle;
                break;
            } else {
                continue;
            }
        } else {
            found = true;
            line->tag = tag;
            line->v = true;
            result = MISSES;
            line->last = cycle;
        }
    }
    if(!found) {
        line = set->lines + set->lru;
        line->tag = tag;
        result = EVICTIONS;
        line->last = cycle;
    }
    /* maintain LRU value */
    int min = 0x7FFFFFFF;
    int tmp;
    int lru;
    for(int i = 0; i < ca->e; ++i) {
        if((tmp = (line->last + i)) < min) {
            lru = i;
        }
    }
    set->lru = lru;
    return result;
}