#include "parameter.h"
/**
 * Creates a new, empty function parameter
 * @return the newly created parameter
 */
parameter::parameter(method* pthe_method, call_context* /*pcc*/)
    : simple_value(1), the_method(pthe_method)
{
}


