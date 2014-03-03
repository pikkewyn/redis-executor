#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "output.h"

struct Output output; 

static int const BASE_MESSAGE_LENGTH = 4000;

static void free_message( void* m )
{
    char* message = *( char** )( m );
    
    free( message );
}

void initialize_output( struct Output* output )
{
    output->status_messages = DArray_init( sizeof( char* ) );
    DArray_set_clear_func( output->status_messages, free_message );
    output->status_code = OK;
}

void update_output( struct Output* output, StatusCode const status, char const* format, ... )
{
    va_list vlist, vlist_bak;
    
    va_start( vlist, format );
    va_start( vlist_bak, format );
    
    int len = vsnprintf( NULL, 0, format, vlist ) + 1;
    
    char* tmp = malloc( len );
    memset( tmp, 0, len );
    vsprintf( tmp, format, vlist_bak );
    
    DArray_push_back( output->status_messages, tmp );
    
    va_end( vlist );
    va_end( vlist_bak );
    
    if( status > output->status_code )
        output->status_code = status;
}

char* prepare_output_for_nagios( struct Output const* output )
{
    char* nagios_output = malloc( BASE_MESSAGE_LENGTH );
    memset( nagios_output, 0, BASE_MESSAGE_LENGTH );
    
    int i;
    for( i = 0; i < output->status_messages->len; ++i )
    {
        char* tmp = DArray_get( output->status_messages, i, char* );
        if( strlen( nagios_output ) + strlen( tmp ) + 2 < BASE_MESSAGE_LENGTH )
        {
            strcat( nagios_output, tmp );
            strcat( nagios_output, " " );
        }
    }
    return nagios_output;
}

void free_output( struct Output* output )
{
    DArray_free( output->status_messages );
}