#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>

#include "d_array.h"

struct Redis {

    char* host;
    int port;
    redisContext* r_handler;
};

void create_redis( char const* host_port, struct Redis* const redis );

void clean_redis_data( void* redis );

bool connect_to_available_master( DArray const* sentinels, struct Redis* const redis );

DArray* parse_sentinels( char const* sentinels );

#endif