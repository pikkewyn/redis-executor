/*! \file
 * \details This file is declares argument struct holding together all arguments specified by
    user before runtime. 
 */

#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <getopt.h>
#include <stdbool.h>

#include "d_array.h"

struct Argument {
   
    char*   sentinels_str;
    char*   commands_str;
    char*   commands_file;
	
};

void printHelp();

struct Argument setArgument( int argc, char** argv );

void cleanArgument( struct Argument const* argument ); 

#endif