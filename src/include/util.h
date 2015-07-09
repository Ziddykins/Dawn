#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "colors.h"
#include <limits.h>
#include <stdlib.h>
#include <math.h>

#define STR(x) #x
#define XSTR(x) STR(x)
#define AT __FILE__ ":" XSTR(__LINE__)

#define PRINTERR(func) \
    perror(ERR AT ": " #func);
#define PRINTWARN(func) \
    perror(WARN AT ": " #func);

#define CALLEXIT(func) \
    if(!(func)) { \
        PRINTERR(func) \
        exit(1); \
    }

#define CALLRSME(func) \
    if(!(func)) { \
        PRINTERR(func) \
    }


double genNum(void);
double gaussrand(void);
double ABS(double);

#endif // UTIL_H_INCLUDED
