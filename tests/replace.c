#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
char* str_replace ( const char *string, const char *replacement )
{
    char* substr_pos = NULL;
    char* substr_end = NULL;
    char* newstr = NULL;
    char* oldstr = NULL;
  
    if ( replacement == NULL ) 
        return strdup( string );
 
    newstr = strdup( string );
  
    while( 1 ) 
    {
        if( ( substr_pos = strstr( newstr, "\"" ) ) == NULL )
            break;
        
        if( substr_pos == newstr + strlen( newstr ) )
            break;

        if( ( substr_end = strstr( substr_pos + 1, "\"" ) ) == NULL )
            break;

        int const substr_len = substr_end - substr_pos;
        char* substr = malloc( substr_len );
        memset( substr, '\0', substr_len );
        strncpy( substr, substr_pos + 1,  substr_len - 1 ); 
 
        oldstr = newstr;
        newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
                
        memset( newstr, '\0', strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );

        memcpy( newstr, oldstr, substr_pos - oldstr );
        memcpy( newstr + ( substr_pos - oldstr ), replacement, strlen ( replacement ) );
        memcpy( newstr + ( substr_pos - oldstr ) + strlen( replacement ), substr_end + 1, strlen ( oldstr ) - ( substr_end - oldstr) - 1 );
         
        free( substr );
        free (oldstr);
    }
  return newstr;
}
 
int main( int argc, char **argv ){
  char *ns = NULL;
  if( argc != 2 ) {
    return 1;
  }
  ns = str_replace( argv[1], " %s " );
  fprintf( stdout, "Old string: %s\nNew string: %s\n", argv[ 1 ], ns );
  free(ns);
  return 0;
}
