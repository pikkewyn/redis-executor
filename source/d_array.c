#include <stdio.h>
#include <string.h>

#include "d_array.h"


DArray* DArray_init( size_t ItemSize )
{
    DAssert( ItemSize > 0 );
	
	DArray init = { .ItemSize = ItemSize };
    
	DArray* array = malloc( sizeof( DArray ) );
	DAssert( array != NULL );
	
	memcpy( array, &init, sizeof( DArray ) );

    array->data = NULL;
    array->len = 0;
    array->capacity = 0;

    //array->ItemSize = ItemSize;
    array->clear_function = NULL;

    return array;
}

/*DArray* DArray_copy( DArray* array )
{
	DArray* array_copy = malloc( sizeof( DArray ) );
	
	array_copy->data = malloc( array->capacity );
	array_copy->data = strdup( array->data );
	array_copy->len = array->len;
	array_copy->ItemSize = array->ItemSize;
	array_copy->clear_function = array->clear_function;
	
	return array_copy;
	
}*/


void DArray_free( DArray* array )
{	
    if( array == NULL ) //by analogy to free
        return;
    
    if( array != NULL && array->data != NULL && array->clear_function != NULL )
	{
        int i = 0;
        for( i=0; i < array->len; ++i )
		{
            (*array->clear_function)( ( array->data + i * array->ItemSize ) );
        }
    }
    
    free( array->data );
    array->data = NULL;

    free( array );	
}

size_t DArray_get_new_capacity_impl( size_t capacity )
{	
    if( capacity == 0 )
	{
        return 2;
    }

    return 2 * capacity;
}

void DArray_resize_impl(  DArray* array, size_t new_capacity )
{
	DAssert( array != NULL );
    DAssert( new_capacity > array->capacity );

    void * raw_memory = malloc( new_capacity * array->ItemSize );
    DAssert( raw_memory != NULL );

    memcpy( raw_memory, array->data, array->len * array->ItemSize );
    free( array->data );

    array->data = (char*)raw_memory;
    array->capacity = new_capacity;
}

void DArray_push_back_impl(  DArray* array, void * item )
{	
    DAssert( array != NULL );
    DAssert( item != NULL );

    if( array->len == array->capacity )
    {
        DArray_resize_impl( array, DArray_get_new_capacity_impl( array->capacity ) );
    }

    DAssert( array->data != NULL );

    void * raw_memory = ( void * )( array->data + array->len * array->ItemSize );

    memcpy( raw_memory, item, array->ItemSize );

    array->len += 1;
}

void * DArray_get_impl( DArray* array, size_t index, size_t item_size )
{
    DAssert( array != NULL );
    DAssert( array->data != NULL );
    DAssert( index < array->len );
    DAssert( item_size == array->ItemSize );

    return array->data + index * array->ItemSize;
}

const void* DArray_get_const_impl(  const DArray * array, size_t index, size_t item_size )
{	
	return ( const void* )( DArray_get_impl( (void*)array, index, item_size ) );
}

void DArray_set_clear_func( DArray* array, void(*clear_func)(void*) )
{
    DAssert( array != NULL );
    DAssert( clear_func != NULL );

    array->clear_function = clear_func;
}

void* DArray_find( DArray* array, void* item, bool (*cmp)(void*, void*) )
{
	int i = 0;

	for( ; i < array->len; ++i)
	{
		void* const current = DArray_get_impl( array, i, sizeof( char* ) );
		assert( current != NULL );
		
		if( cmp( current, item ) ){
			return current;
		}
	}
	
	return NULL;
}

DArray* splitString( const char* const string_const, const char* const delim )
{
    DAssert( string_const );
    DAssert( delim );

    DArray* array = DArray_init( sizeof( char* ) );
    DArray_set_clear_func( array, &clearSplitedString );
    
    char* token = NULL;
    char* savePtr = NULL;
    
    char* const string_mutable = strdup( string_const );

    if( ( token = strtok_r( string_mutable, delim, &savePtr ) ) )
    {
        do
        {
            token = strdup( token );
            DArray_push_back( array, token );
        }
        while( ( token = strtok_r( NULL, delim, &savePtr ) ) );
    }

    free( string_mutable );

    return array;
}

void clearSplitedString( void* w )
{
    char* word = *( char** )( w );

    free( word );
}
