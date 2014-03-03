// compilation: gcc -lhiredis test_quotes.c -o test-quotes
// this must be run on machine with redis running locally
// decription of problem: using quotations in issued command doesnt work as might be expected

#include <hiredis/hiredis.h>
#include <stdio.h>

int main()
{
    redisContext* redis_handler = redisConnect( "127.0.0.1", 6379 );

    if( redis_handler != NULL && redis_handler->err ) {
        printf( "Error: %s\n", redis_handler->errstr );
        return 1;
    }

    redisReply* reply = NULL;
   
    reply = redisCommand( redis_handler, "LRANGE testlist 0 -1" );//im getting current list legth to calculate then how many elements will be inserted
    int const old_list_len = reply->elements;

    
    reply = redisCommand( redis_handler, "LPUSH testlist \"Bob likes Alice\"" );
    printf( "expected number of inserted elements: 1\n" );
    printf( "real number of inserted elements:     %d\n", reply->integer - old_list_len );
     
    
    freeReplyObject( reply );
    redisFree( redis_handler );
    return 0;
}
