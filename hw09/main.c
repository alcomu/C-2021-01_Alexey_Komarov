#include "crc32_calc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int f;
    unsigned long crc32;
    char *mmptr;
    struct stat statbuf;
    int err = 0;

    if (argc == 1) {
        printf("Usage: ./crv32_calc filename\n");
        exit(0);
    } else if (argc != 2) {
        fprintf(stderr, "Error arguments count!!!\n");
        exit(1);
    }

    f = open(argv[1], O_RDONLY);
    if (!f) {
        fprintf(stderr, "Error read input file %s!!!\n", argv[1]);
        exit(1);
    }

    err = fstat(f, &statbuf);
    if (err < 0) {
        printf("\n\"%s \" could not open\n", argv[1]);
        exit(1);
    }

    mmptr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, f, 0);
    if (mmptr == MAP_FAILED) {
        fprintf(stderr, "Mapping Failed\n");
        exit(1);
    }
    close(f);

    if (crc32_comp_mmap(mmptr, statbuf.st_size, &crc32) == 0)
        printf("%s: %ld\n", argv[1], crc32);
    else
        printf("Calc crc32 for %s error!!!\n", argv[1]);

    err = munmap(mmptr, statbuf.st_size);
    if (err != 0) {
        fprintf(stderr, "UnMapping Failed\n");
        exit(1);
    }

    return 0;
}