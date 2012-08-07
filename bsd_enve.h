#ifndef _BSD_ENVE_H_
#define _BSD_ENVE_H_

/**
 * The struct envelope structure is holding a data tha can be worked with using the same interface by the functions.
 * It has two members:
 * 1. the type. This can denote a struct variable, a struct number, an struct indexed structure or something else
 * 2. the void* pointer, which is the actual data that needs to be interpreted
 */
struct envelope
{
	/* the type */
    int type;

	/* what is in this struct envelope */
	void* to_interpret;
};

#endif
