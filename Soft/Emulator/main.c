#include <stdio.h>
#include <stdlib.h>

#include "maluch.h"

// font: http://xyzzy.freeshell.org/cp437/

int main() {
    Maluch maluch = { 0 };
    FILE *f = fopen("add_test", "rb");
    if(f == NULL) {
        fprintf(stderr, "file not found.\n");
        return EXIT_FAILURE;
    }
    {
        unsigned short words[30];
        fread(words, 1, 60, f);
        fclose(f);
        for(int i = 0; i < 30; i++) {
            maluch.rom[i] = __builtin_bswap16(words[i]);
        }
    }

    for(int i = 0; i < 25000000/60; i++) {
        step(&maluch);
    }
}
