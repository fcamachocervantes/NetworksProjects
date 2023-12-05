#include <cstdio>
#include <unistd.h>
#include <strings.h>
#include <limits>
#include <iostream>
#include <list>
#include <cstring>
#include <algorithm>
#include <math.h>


inline int LOG_LEVEL = 3;
#define TRACE   if (LOG_LEVEL > 5) { std::cout << "TRACE: "
#define DEBUG   if (LOG_LEVEL > 4) { std::cout << "DEBUG: "
#define INFO    if (LOG_LEVEL > 3) { std::cout << "INFO: "
#define WARNING if (LOG_LEVEL > 2) { std::cout << "WARNING: "
#define ERROR   if (LOG_LEVEL > 1) { std::cout << "ERROR: "
#define FATAL   if (LOG_LEVEL > 0) { std::cout << "FATAL: "
// #define ENDL  " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; }
#define ENDL "" << std::endl; }



#include "simulator.h"
#include "main.h"
#include "GoBackN.h"
