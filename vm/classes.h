#ifndef CLASSES
#define CLASSES

#include "metatbl.h"

#include <stdint.h>

struct nap_vm;

/**
 * @brief The class_descriptor struct represents a class from the
 * nap script.
 */
struct class_descriptor
{
    /** the name of the class */
    const char* name;

    /** The number of variables this class has */
    uint32_t varcount;

    /** The indexes of the class variables in the metatable */
    uint32_t* mtbl_indexes;
};

/* represents an instance of a class */
struct class_instantiation
{
    /* to which class this instantiation belongs to */
    struct class_descriptor* cd;

    /* all the members of the class instantiated. We need a
     * variable_entry since we need to create a new variable
     * for every object we create */
    struct variable_entry** members_instantiation;

};

int interpret_classtable(struct nap_vm* vm,
                         uint8_t* start_location,
                         uint32_t len);


/**
 * @brief instantiate creates an instance of the given class descriptor
 * @param cd the class descriptor
 * @return the instance of the class
 */
struct class_instantiation* instantiate(struct nap_vm* vm, struct class_descriptor* cd);

#endif // CLASSES

