#include <assert.h>
#include <malloc.h>
#include <pcre.h>
#include <string.h>
#include <unistd.h>

#include "output.h"
#include "redis.h"

extern struct Output output;

static const int OVECMAX = 30;
static const int MAX_ATTEMPTS = 3;
static const int ATTEMPT_SLEEP = 3;

void create_redis( char const* host_port, struct Redis* redis )
{
    char* colon_pos = strstr( host_port, ":" );
    assert( colon_pos != NULL && "malformed redis data from sentinel" );
    
    char* host = strndup( host_port, colon_pos - host_port );
    int port = strtol( colon_pos + 1, NULL, 0 );
 
    redis->host = host;
    redis->port = port;
    redis->r_handler = NULL;
}

void clean_redis_data( void* r )
{
    if( r == NULL )
        return;
    
    struct Redis* redis = ( struct Redis* )r;
    if( redis->host )
        free( redis->host );
    if( redis->r_handler )
        redisFree( redis->r_handler );
}

static char* get_match( char const* str, char const* pattern )
{
    const char *err_msg;
    int err;
    int offsets[ OVECMAX ];
    
    pcre* re = pcre_compile( "^master\\d+:.*address=([0-9.:]+),", PCRE_MULTILINE | PCRE_NEWLINE_ANY, &err_msg, &err, NULL );
    assert( re );
    
    int rc = pcre_exec( re, NULL, str, strlen( str ), 0, 0, offsets, OVECMAX );
    
    if (rc < 0)
        update_output( &output, CRITICAL, "cant match sentinel in redis INFO output;" );
    
    int const match_len = offsets[ 2 * 1 + 1 ] - offsets[ 2 * 1 ];
    char* match = malloc( match_len + 1 );
    memset( match, '\0', match_len + 1 ); 
    strncpy( match, str + offsets[ 2 * 1 ], offsets[ 2 * 1 + 1 ] - offsets[ 2 * 1 ] );
   
    pcre_free( re );
    
    return match;
}

static bool redis_connect( struct Redis* redis )
{
    redis->r_handler = redisConnect( redis->host, redis->port );
    assert( redis->r_handler && "NULL hanlder returned from redisConnect" );
    
    if( redis->r_handler->err )
    {
        update_output( &output, CRITICAL, "connecting to %s:%d returned %s;", redis->host, redis->port, redis->r_handler->errstr );
        return false;
    }
    
    redisReply* reply = redisCommand( redis->r_handler, "PING" );
    if( reply == NULL ) 
    {
        update_output( &output, CRITICAL, "connecting to %s:%d returned dead handler;", redis->host, redis->port );
        return false;
    }
    
    freeReplyObject( reply );
    return true;
}

bool connect_to_available_master( DArray const* sentinels, struct Redis* const redis )
{
    assert( sentinels );
   
    redisContext* sentinel_handler = NULL;
    
    int i;
    for( i = 0; i < sentinels->len; ++i )
    {
        struct Redis const* sentinel = &DArray_get_const( sentinels, i, struct Redis );
        sentinel_handler = redisConnect( sentinel->host, sentinel->port );
        
        if( sentinel_handler != NULL )
        {
            if( ! sentinel_handler->err )
            {
                redisReply* reply = redisCommand( sentinel_handler, "INFO" );
    
                if( reply == NULL )
                {
                    update_output( &output, WARNING, "failed to connect to %s:%d;", sentinel->host, sentinel->port  );
                    redisFree( sentinel_handler );
                    continue;
                }
    
                if( reply->type != REDIS_REPLY_STRING )
                {
                    update_output( &output, CRITICAL, "INFO reply is not string;" );
                    freeReplyObject( reply );
                    redisFree( sentinel_handler );
                    continue;
                }
    
                char* redis_host_port = get_match( reply->str, "^(master\\d+):.*address=([0-9.:]+)," );
    
                create_redis( redis_host_port, redis );
                freeReplyObject( reply );
                redisFree( sentinel_handler );
                free( redis_host_port );
               
                if( ! redis_connect( redis ) )
                {
                    clean_redis_data( redis );
                    continue;
                }
                
                return true;
            }
            else {
                update_output( &output, CRITICAL, "%s:%d %s", sentinel->host, sentinel->port, sentinel_handler->errstr );
                redisFree( sentinel_handler );
                continue;
            }
        }
        else
            continue;
    }
    
    return false;
}

DArray* parse_sentinels( char const* sentinels_str )
{
    assert( sentinels_str );
    
    DArray* sentinels = DArray_init( sizeof( struct Redis ) );
    DArray_set_clear_func( sentinels, &clean_redis_data );
    
    char* host_port = NULL;
    char* save_ptr = NULL;
    
    
    char* const string_mutable = strdup( sentinels_str );

    host_port = strtok_r( string_mutable, ",", &save_ptr );
    assert( host_port );
    
    do
    {
        struct Redis redis;
        memset( &redis, 0, sizeof( struct Redis ) );
        
        create_redis( host_port, &redis );
        
        DArray_push_back( sentinels, redis );
    }
    while( ( host_port = strtok_r( NULL, ",", &save_ptr ) ) );

    free( string_mutable );
    
    return sentinels;
}
