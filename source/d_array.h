/*
 * one can push back pointer to object or object by value
 * when pushing pointer remember to not free it after push, it will be done later 
 */

#ifndef D_ARRAY_H
#define D_ARRAY_H

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#define DAssert( condition ) ( assert( condition ) )

typedef struct
{
    char* data;
    size_t len;
    size_t capacity;

    const size_t ItemSize;
    void (*clear_function)(void*);
	
} DArray;

DArray* DArray_init( size_t ItemSize );

DArray* DArray_copy( DArray* array );

void DArray_free( DArray* array );

size_t DArray_get_new_capacity_impl( size_t capacity );

void DArray_resize_impl(  DArray * array, size_t new_capacity );

void DArray_push_back_impl(  DArray * array, void * item );

#define DArray_push_back( array_by_ptr, item_by_value )                   \
        DAssert( ( array_by_ptr )->ItemSize == sizeof( item_by_value ) ); \
        DArray_push_back_impl( array_by_ptr, & ( item_by_value ) )       
    
void* DArray_get_impl(  DArray* array, size_t index, size_t item_size );

#define DArray_get( array_by_ptr, index, ItemType )                             \
    ( * ( ItemType * )( DArray_get_impl( array_by_ptr, index, sizeof( ItemType ) ) ) )

const void* DArray_get_const_impl( const DArray* array, size_t index, size_t item_size );

#define DArray_get_const( array_by_ptr, index, ItemType )                             \
    ( * ( const ItemType* )( DArray_get_const_impl( array_by_ptr, index, sizeof( ItemType ) ) ) )

void DArray_set_clear_func( DArray* array, void(*clear_func)(void*) );

DArray* splitString( const char* const string, const char* const delim );

void clearSplitedString( void* w );

void* DArray_find( DArray* array, void* item, bool (*cmp)(void*, void*) );

#endif

 /*! @} */ 