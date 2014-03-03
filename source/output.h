#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdarg.h>

#include "d_array.h"

typedef enum { OK, WARNING, CRITICAL } StatusCode;

struct Output {
 
   DArray* status_messages;
   StatusCode status_code;
};

void initialize_output( struct Output* output );

void update_output( struct Output* output, StatusCode const status, char const* format, ...  );

char* prepare_output_for_nagios( struct Output const* output );

void free_output( struct Output* output );

#endif