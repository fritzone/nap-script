#ifndef _ENVELOPE_H_
#define _ENVELOPE_H_

#include "bsd_enve.h"

/**
 * Creates a new struct envelope for the given type with the given data
 */
struct envelope* new_envelope(void* data, int env_type);

/**
 * Returns the type ofthe struct envelope that will be created for the given struct variable type
 */
int envelope_get_type_from_vartype(char* src);

/**
 * Checks whether the value in this envelope is false or not
 */
//int envelope_holds_false_value(envelope*);

#endif
