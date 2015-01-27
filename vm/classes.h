#ifndef CLASSES
#define CLASSES

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
};

int interpret_classtable(struct nap_vm* vm,
                         uint8_t* start_location,
                         uint32_t len);

#endif // CLASSES

