#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "argument.h"

void printHelp()
{
    printf( "Available flags:\n"
		"    -c --commands-file [commands file]\n"
		"        path to file containing redis commands one per line\n\n"
		
		"    -s --sentinels [sentinels]\n"
		"        list of comma sperated sentinels in form host:port,host:port\n\n"
		
		"    -h --help\n"
		"        print this help\n\n"
        "   after flags should come list of semicolon separated redis commands,\n"
        "   if none are specified default PING command will be used\n\n"
        "Sample usage:\n"
        "redis_executor -s \"127.0.0.1:26379\" \"LPUSH lista888 \"Here is some text\";LPUSH lista888 \"Here is other text\"\n\n"
	);
}

void cleanArgument( struct Argument const* argument )
{
    free( argument->commands_str );
	free( argument->commands_file );
    free( argument->sentinels_str );
}


struct Argument setArgument( int argc, char* argv[] )
{
    static struct option options[] = 
    {
		{"sentinels",			required_argument,  0,	's'},
        {"commands-file",       required_argument, 	0,	'c'},
        {"help",                no_argument,		0,	'h'},
        { NULL,                 0,                  NULL, 0 } 
    };

    int c;
    int option_index = 0;
    
    struct Argument a = { 
				.sentinels_str  			= NULL,
                .commands_str               = NULL,
                .commands_file              = NULL
    };
	
    putenv("POSIXLY_CORRECT=y");
    
	while( ( c = getopt_long( argc, argv, "c:s:h", options, &option_index ) ) != -1 )
	{    
        switch ( c )
		{
            case 'c':
                a.commands_file = strdup( optarg );
                break; 
                
            case 's':
                a.sentinels_str = strdup( optarg );
                break;
				
            case 'h':
                printHelp();
                exit( 1 );
                break;
				
            default:
				printHelp();
                exit( 1 );
        }
    }
   
    if( optind < argc )
    {
        a.commands_str = strdup( "" );
        size_t len = 0;
        
        while ( optind < argc)
        {
            char const* tmp = argv[ optind++ ]; 
            size_t const tmp_len = strlen( tmp );
            len += ( tmp_len + 2 );
            char* new_block = realloc( a.commands_str, len + 1 );
            assert( new_block );
            a.commands_str = new_block;
                    
            strncat( a.commands_str, tmp, tmp_len );
            
            if( optind + 1 <= argc )
                strcat ( a.commands_str, " " );
        }
    }
    
    return a;    
}


