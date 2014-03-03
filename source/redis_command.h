#ifndef REDIS_COMMAND_H
#define REDIS_COMMAND_H

#include <stdbool.h>

#include "d_array.h"
#include "redis.h"

struct Command {
    
    char const*   format;
    DArray* quoted_strings;
};

bool create_command( char const* command_str, struct Command* command );

bool parse_commands( char const* commands_string, DArray* commands_array );

bool parse_commands_from_file( char const* file, DArray* commands_array );

void free_command( void* cmd );

bool execute_command( struct Redis const* redis, struct Command* command );

#endif