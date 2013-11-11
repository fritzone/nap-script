#include "garbage_bin.h"

garbage_bin_bin* garbage_bin_bin::minstance = NULL;

garbage_bin_bin::garbage_bin_bin()
{
    minstance = this;
}

void garbage_bin_bin::empty()
{
    for(size_t i=0; i<items.size(); i++)
    {
        items[i]->empty();
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
}


void garbage_bin_bin::shutdown()
{
    delete minstance;
}
