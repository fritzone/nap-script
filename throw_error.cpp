#include "throw_error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const expression_with_location* g_location  = NULL;

static void prepare_location(char* location)
{
    sprintf(location, "expr:[%s] file:[%s] around line:[%d]", g_location->expression, g_location->location->file_name, g_location->location->start_line_number);
}

void throw_error(const char* error, void* res)
{
    char location[1024];
    prepare_location(location);
    char finalError[2048];
    sprintf(finalError, "%s, %s", error, location);
    fputs(finalError, stderr);
    exit(1);
}

void throw_error(const char* error, const char* par1, const char* par2, void* res)
{
    char location[1024];
    prepare_location(location);
    char s[2048];
    sprintf(s, "%s [%s] and [%s], %s\n", error, par1, par2, location);
    fprintf(stderr, "%s", s);
    exit(1);
}

void throw_error(const char* error, const char* par, void* res)
{
    char location[1024];
    prepare_location(location);
    char s[2048];
    sprintf(s, "%s %s, %s\n", error, par, location);
    fprintf(stderr, "%s", s);
    exit(1);
}

void throw_error(const char* error, int id, const char* par, void* res)
{
    char location[1024];
    prepare_location(location);
    char s[1024];
    char s2[2048];

    sprintf(s, "%s %s, %s\n", error, par, location);
    sprintf(s2, s, id);
    fprintf(stderr, "%s", s2);
    exit(1);
}

void throw_index_out_of_range(const char* variable_name, int maximum_allowed, int got, void* res)
{
    char location[1024];
    prepare_location(location);
    char s[2048];
    sprintf(s,"[Exception.IndexOutOfRange] Index [%d] out of range for [%s]. Max allowed [%d], %s\n", got, variable_name ,maximum_allowed, location);
    fprintf(stderr, "%s", s);
    exit(1);
}

void set_location(const expression_with_location* loc)
{
    g_location = loc;
}

