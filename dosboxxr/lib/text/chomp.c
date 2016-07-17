
#include <string.h>

#include "dosboxxr/lib/text/chomp.h"

// C/C++ implementation of Perl's chomp function
void chomp(char *s) {
    char *t = s + strlen(s) - 1;
    while (t >= s && (*t == '\n' || *t == '\r')) *t-- = 0;
}

