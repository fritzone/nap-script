#ifndef _ENVELOPE_H_
#define _ENVELOPE_H_

#include "bsd_enve.h"

#define ENV_HOLDS_NUMBER(env) (BASIC_TYPE_INT == (env)->type || BASIC_TYPE_REAL == (env)->type)

#define ENV_HOLDS_VARIABLE(env) (BASIC_TYPE_VARIABLE == (env)->type)

#define ENV_HOLDS_INDEXED(env) (BASIC_TYPE_INDEXED == (env)->type)

#define ENV_HOLDS_STRING(env) (BASIC_TYPE_STRING == (env)->type)

#define ENV_GET_NUMBER(env) ((number*)(env)->to_interpret)

#define ENV_GET_VARIABLE(env) ((variable*)(env)->to_interpret)

#define ENV_GET_INDEXED(env) ((indexed*)(env)->to_interpret)

#define ENV_GET_STRING(env) ((bt_string*)(env)->to_interpret)

#define ENV_TYPE(env) (env->type)

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
