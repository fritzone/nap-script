#include "garbage_bin.h"

garbage_bin_bin* garbage_bin_bin::minstance = NULL;

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
    for(size_t i=0; i<items.size(); i++)
    {
        delete items[i];
    }
    items.clear();
}


void garbage_bin_bin::shutdown()
{
    delete minstance;
    minstance = 0;
}
