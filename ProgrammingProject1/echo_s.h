// ********************************************************
// * A common set of system include files needed for socket() programming
// ********************************************************
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>


// ********************************************************
// * These don't really provide any improved functionality,
// * but IMHO they make the code more readable.
// *
// * There is a much better logging facility included
// * in the BOOST library package. I recommend that for
// * real world applications.
// ********************************************************
extern bool VERBOSE;
#define DEBUG if (VERBOSE) { std::cout
#define FATAL if (true) { std::cout
#define ENDL  " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; }

