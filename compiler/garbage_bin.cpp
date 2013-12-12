#include "garbage_bin.h"

garbage_bin_bin* garbage_bin_bin::minstance = NULL;

#ifdef MEMORY_DEBUG
long int all_alloc = 0;
#endif

garbage_bin_bin::garbage_bin_bin()
{
    minstance = this;
}

void garbage_bin_bin::empty(const nap_compiler* _compiler)
{
    for(size_t i=0; i<items.size(); i++)
    {
        items[i]->empty(_compiler);
    }
}

garbage_bin_bin& garbage_bin_bin::instance()
{
    if(minstance == 0)
    {
        minstance = new garbage_bin_bin();
    }
    return *minstance;
}

garbage_bin_bin::~garbage_bin_bin()
{
}

void garbage_bin_bin::release()
{
    for(size_t i=0; i<minstance->items.size(); i++)
    {
        delete minstance->items[i];
    }
    minstance->items.clear();
}

void garbage_bin_bin::shutdown()
{
    if(minstance == 0)
    {
        return;
    }

    for(size_t i=0; i<minstance->items.size(); i++)
    {
        delete minstance->items[i];
    }
    minstance->items.clear();

    delete minstance;
    minstance = 0;
}
