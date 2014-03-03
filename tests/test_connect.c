// compilation: gcc -lhiredis test.c 

// it seems that redisConnect returns silently broken handler when any open port will
// be provided as connection parameter

#include <assert.h>
#include <hiredis/hiredis.h>
#include <stdio.h>
#include <string.h>

int main()
{
    redisContext* sentinel_handler = NULL;

    sentinel_handler = redisConnect( "127.0.0.1", 22 );
    if( sentinel_handler == NULL )
    {
        printf( "null\n" );
        return 1;
    }
    else
        if ( sentinel_handler->err ) {
            printf("Error: %s\n", sentinel_handler->errstr);
            redisFree( sentinel_handler );
            return 1;
        }
        else{
            printf( "should be error" );
        }

    redisReply* reply = redisCommand( sentinel_handler, "PING" );
    assert( reply );
    
    if( reply->type != REDIS_REPLY_STRING || strcpy( reply->str, "PONG" ) != 0 )
        printf( "error\n" );
    
    return 0;
}
