#define _GNU_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argument.h"
#include "output.h"
#include "redis.h"
#include "redis_command.h"
#include "d_array.h"

extern struct Output output;

static const int ATTEMPT_SLEEP;
static const int MAX_ATTEMPTS;

int main( int argc,char** argv)
{
	struct Argument const argument = setArgument( argc, argv );
    
    initialize_output( &output );
    
    DArray* sentinels           = NULL;
    DArray* commands_array      = NULL;
    struct Redis redis_master;
    memset( &redis_master, 0, sizeof( struct Redis ) );
   
    {//setting up sentinels
        if( argument.sentinels_str == NULL )
        {
            update_output( &output, CRITICAL, "empty sentinels list" );
            goto cleanup;
        }
        
        if( ( sentinels = parse_sentinels( argument.sentinels_str ) ) == NULL )
            goto cleanup;
    }
    
    {//setting up commands
        commands_array = DArray_init( sizeof( struct Command ) );
        DArray_set_clear_func( commands_array, &free_command );
    
        if( argument.commands_file )
            if( ! parse_commands_from_file( argument.commands_file, commands_array ) )
                goto cleanup;
        
        bool parse_commands_suceeded = false;
        if( argument.commands_str == NULL )
        {
            char* tmp_command = strdup( "PING" );
            parse_commands_suceeded = parse_commands( tmp_command, commands_array );
            free( tmp_command );
        }
        else{
            parse_commands_suceeded = parse_commands( argument.commands_str, commands_array );
        }
        
        if( ! parse_commands_suceeded )
            goto cleanup;
    }
   
    {//executing commands
    if( ! connect_to_available_master( sentinels, &redis_master ) )
            goto cleanup;
    
        int i = 0;
        bool tried = false;
    
        while( i < commands_array->len )
        {
            struct Command* command = &DArray_get( commands_array, i, struct Command );
            
            if( ! execute_command( &redis_master, command ) && ! tried )
            {//give second chance as replication could happen
                tried = true;
                continue;
            }
            else {
                tried = false;
                i += 1;
            }
            
            clean_redis_data( &redis_master );
            
            if( ! connect_to_available_master( sentinels, &redis_master ) )
                break;
        }
    }
    
    cleanup:
        clean_redis_data( &redis_master );
        cleanArgument( &argument );
        DArray_free( sentinels );
        DArray_free( commands_array );
       
    char* nagios_output = prepare_output_for_nagios( &output );
    printf( "%s", nagios_output );
    
    free( nagios_output );
    free_output( &output );
    
    return output.status_code;
}
