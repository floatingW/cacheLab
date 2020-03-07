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
#define LINE_LEN 256

void printUsage();

void get_setidx_tag(long, long *, long *, int, int);
typedef struct line {
    bool v;
    int last;
    int tag;
} line_t;

typedef struct set {
    line_t *lines;
} set_t;

typedef struct cache {
    int s;
    int e;
    int b;
    set_t *sets;
} cache_t;

cache_t *do_cache_initialization(int, int, int);
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

    /* supress error of unusing of verbose */
    if(verbose);

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
        fclose(file);
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
    line = malloc(LINE_LEN);
    while(fgets(line, LINE_LEN, file) != NULL) {
        if(*line == 'I') continue;
        line++;
        str_size = strstr(line, ",");
        if(str_size) {
            size = atoi(++str_size);
        } else continue;
        op = *line;
        line+=2;
        str_addr = strtok(line, ",");
        sscanf(str_addr, "%lx", &addr);
        /* suppress error msg */
        printf("%c %lx,%d\n", op, addr, size);
        /* cache handling */
        get_setidx_tag(addr, &set_index, &tag, s, b);
    }

    /* output summary */ 
    printSummary(hits, misses, evictions);
    fclose(file);
    return 0;
}

void printUsage() {
    // to do
    printf("Usage: brbrbr\n");
}

cache_t *do_cache_initialization(int s, int e, int b) {
    cache_t *ca;
    ca = (cache_t*)malloc(sizeof(cache_t));
    if(ca == NULL) return NULL;
    ca->sets = (set_t*)malloc((2 << (s - 1)) * sizeof(set_t));
    if(ca->sets == NULL) {
        free(ca); 
        return NULL;
    }
    ca->sets->lines = (line_t*)calloc(e, sizeof(line_t));
    if(ca->sets->lines == NULL) {
        free(ca->sets);
        free(ca);
        return NULL;
    }
    return ca;
}

void get_setidx_tag(long addr, long *set_index, long *tag, int s, int b) {
    long mask = 0x8000000000000000;
    *set_index = (addr >> b) & (~(mask >> (64 - s)));
    *tag = (addr >> (s + b)) & (~(mask >> (64 - s - b)));
}