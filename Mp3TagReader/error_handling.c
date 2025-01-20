#include <stdio.h>
#include "error_handling.h"

void display_error(const char *message, ...)
{
    fprintf(stderr, "Error: %s\n", message);
}