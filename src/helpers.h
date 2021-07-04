#ifndef HELPERS_H
#define HELPERS_H

#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#define CAST(val, type) ((type)(val))
#define MIN(v1, v2) (v1 > v2 ? v2 : v1)
#define MAX(v1, v2) (v1 > v2 ? v1 : v2)

#define streq(s1, s2, l1, l2)                                                  \
    ((l1) == (l2) ? strncmp((s1), (s2), (l1)) == 0 : false)

#endif
