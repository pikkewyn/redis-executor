#include <assert.h>
#include <errno.h>
#include <hiredis/hiredis.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "output.h"
#include "redis_command.h"

extern struct Output output;

static int const MAX_SUBSTRINGS = 2;

static void free_quoted_string( void* s )
{
    char* word = *( char** )( s );
    
    free( word );
}

static int count_quote_marks( char const* str )
{
    int i = 0;
    while( str[i] )
    {
        if( str[ i ] == '\"' )
            i+=1;
        else
            str++;
    }
    return i;
}

bool create_command( char const* command_string, struct Command* command )
{
    DArray* quoted_strings = DArray_init( sizeof( char* ) );
    DArray_set_clear_func( quoted_strings, &free_quoted_string );
    
    bool command_creation = true;
    
    
    if( count_quote_marks( command_string ) % 2 != 0 )
        command_creation = false;
    
    char const* replacement = "%s";
    char* substr_pos = NULL;
    char* substr_end = NULL;
    char* newformat = strdup( command_string );
    char* oldformat = NULL;
    
    while( command_creation )
    {
        if( ( substr_pos = strstr( newformat, "\"" ) ) == NULL )
            break;
        
        substr_end = strstr( substr_pos + 1, "\"" );
        
        
        int const substr_len = substr_end - substr_pos;
        char* substr = malloc( substr_len );
        memset( substr, '\0', substr_len );
        strncpy( substr, substr_pos + 1,  substr_len - 1 );
       
        DArray_push_back( quoted_strings, substr );
        if( quoted_strings->len > MAX_SUBSTRINGS )
        {
            update_output( &output, WARNING, "max allowed substrings: %d", MAX_SUBSTRINGS );
            command_creation = false;
        }
        
        oldformat = newformat;
        newformat = malloc ( strlen ( oldformat ) - strlen ( substr ) + strlen ( replacement ) + 1 );
        
        memset( newformat, '\0', strlen ( oldformat ) - strlen ( substr ) + strlen ( replacement ) + 1 );
        
        memcpy( newformat, oldformat, substr_pos - oldformat );
        memcpy( newformat + ( substr_pos - oldformat ), replacement, strlen ( replacement ) );
        memcpy( newformat + ( substr_pos - oldformat ) + strlen( replacement ), substr_end + 1, strlen ( oldformat ) - ( substr_end - oldformat) - 1 );
        
        free( oldformat );
    }
    
    
    if( ! command_creation )
    {
        DArray_free( quoted_strings );
        free( newformat );
    }
    else{
        command->quoted_strings = quoted_strings;
        command->format = newformat;
    }
    
    return command_creation;
}

bool parse_commands( char const* commands_string, DArray* commands_array )
{
    char* token = NULL;
    char* savePtr = NULL;
   
    assert( commands_string );
    char* const string_mutable = strdup( commands_string );
    
    token = strtok_r( string_mutable, ";", &savePtr );
   
    do
    {
        struct Command command;
        if( ! create_command( token, &command ) )
        {
            update_output( &output, CRITICAL, "malformed command" );
            free( string_mutable );
            return false;
        }
        
        DArray_push_back( commands_array, command );
    }
    while( ( token = strtok_r( NULL, ";", &savePtr ) ) );
    
    free( string_mutable );
    
    return true;
}
   
bool parse_commands_from_file( char const* file, DArray* commands_array )
{
    FILE* fd = fopen ( file, "r" );
    
    if( fd == NULL )
    {
        update_output( &output, CRITICAL,  "%s %s", file, strerror( errno ) );
        return false;
    }
    
    size_t n = 0;
    char* line_buffer = NULL;
  
    bool create_command_success = true;
    
    while( getline( &line_buffer, &n, fd ) > 0 )
    {    
        if( line_buffer[ 0 ] == '#' )
            continue;
        
        struct Command command;
       
        line_buffer[ strlen( line_buffer ) - 1 ] = '\0';
        create_command_success = create_command( line_buffer, &command );
        
        if( create_command_success )
        {
            DArray_push_back( commands_array, command );
        }
        else
        {
            update_output( &output, CRITICAL, "malformed command: %s;", line_buffer );
            break;
        }
    }
    free( line_buffer );
    fclose( fd );
    return create_command_success;
}

bool parse_reply( redisReply const* reply, struct Redis const* redis, char const* format, int const duration, bool reply_array )
{
    if( reply == NULL )
    {
        update_output( &output, CRITICAL, "redis: %s:%d command %s execution failed, duration: %d ms;", redis->host, redis->port, format, duration );
        return false;
    }
   
    switch( reply->type )
    {
        case REDIS_REPLY_ERROR:
            if( reply_array )
                update_output( &output, CRITICAL, " %s", reply->str );
            else
                update_output( &output, CRITICAL, "redis: %s:%d command: %s reply: %s, duration: %d ms;", redis->host, redis->port, format, reply->str, duration );
            break;
        
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
            if( reply_array )
                update_output( &output, OK, " %s", reply->str );
            else
                update_output( &output, OK, "redis: %s:%d command: %s reply: %s, duration: %d ms;", redis->host, redis->port, format, reply->str, duration );
            break;
        
        case REDIS_REPLY_INTEGER:
            if( reply_array )
                update_output( &output, OK, " %d", reply->integer );
            else
                update_output( &output, OK, "redis: %s:%d command: %s, reply: %d, duration: %d ms;", redis->host, redis->port, format, reply->integer, duration );
            break;
        
        case REDIS_REPLY_NIL:
            if( reply_array )
                update_output( &output, OK, "NULL" );
            else
            update_output( &output, OK, "redis: %s:%d command: %s, reply: NULL duration: %d ms;", redis->host, redis->port, format, duration );
            break;
        
        case REDIS_REPLY_ARRAY:
            if( ! reply_array )
            {
                update_output( &output, OK, "redis: %s:%d command: %s, duration: %d ms reply: ", redis->host, redis->port, format, duration );
                reply_array = true;
            }
            
            int i;
            for( i = 0; i < reply->elements; ++i )
                parse_reply( reply->element[ i ], NULL, NULL, 0, reply_array );
            break; 
        
        default:
            update_output( &output, CRITICAL, "unsupported reply type" );
    }
    
    return true;
}

bool execute_command( struct Redis const* redis, struct Command* command )
{
    assert( redis );
    assert( redis->r_handler );
    assert( command );
    assert( command->format );
    
    
    char* t[ 10 ];
    memset( t, 0, 10 );
    
    int i;
    for( i = 0; i < command->quoted_strings->len; ++i )
    {
        t[ i ] = DArray_get( command->quoted_strings, i, char* );
    }
  
    struct timespec t_start;
    struct timespec t_end;
    
    clock_gettime( CLOCK_MONOTONIC, &t_start );
    
    char const* format = command->format;
    
    redisReply* reply = redisCommand( redis->r_handler, format, t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], t[8], t[9] );
   
    clock_gettime( CLOCK_MONOTONIC, &t_end );
    int const duration = ( t_end.tv_sec - t_start.tv_sec ) * 1000 + ( t_end.tv_nsec - t_start.tv_nsec) / 100000;
    
    bool const status = parse_reply( reply, redis, format, duration, false );
    
    freeReplyObject( reply );
    return status;
}

void free_command( void* cmd )
{
    struct Command* command = ( struct Command* )cmd;
    DArray_free( command->quoted_strings );
    free( ( char* ) command->format );
}