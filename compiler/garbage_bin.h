#ifndef GARBAGE_BIN_H
#define GARBAGE_BIN_H
#pragma once

#include <vector>
#include <stdlib.h>
using namespace std;

class garbage_bin_base;

class garbage_bin_bin {
public:

    void throwIn(garbage_bin_base* rubbish) { items.push_back(rubbish); }
    void empty();
    static void shutdown() { delete minstance;}
    static garbage_bin_bin& instance();
private:
    garbage_bin_bin();
    vector<garbage_bin_base*> items;
    static garbage_bin_bin* minstance;
};

class garbage_bin_base {
public:
    virtual void empty() = 0;
    garbage_bin_base() { garbage_bin_bin::instance().throwIn(this); }
};

template <typename T>
class garbage_bin : public garbage_bin_base
{
private:

    /**
     * Constructor. Creates a new Garbage Bin object.
     */
    garbage_bin<T>(void)
    {
        items.clear();
    }

    // the instance
    static garbage_bin<T>* pinstance;

public:


    /**
     * Returns the instance of this type of garbage bin...
     */
    static garbage_bin<T>& instance()
    {
        if(pinstance == NULL)
        {
            pinstance = new garbage_bin<T>();
        }

        return *pinstance;
    }

    /**
     * Deletes the instance
     */
    static void deleteInstance()
    {
        if (pinstance)
        {
            delete pinstance;
        }
    }

    /**
     * Destructor
     */
    ~garbage_bin<T>(void)
    {
    }

    /**
     * Empties the garbage bin...
     */
    void empty(void)
    {
        for(size_t i=0; i<items.size(); i++)
        {
            free (items[i]);
        }
    }

    /**
     * Adds an item to the garbage bin
     */
    void throwIn(T item)
    {
        items.push_back(item);
    }

private:

    // this is the vector which actually contains the pointers that will be deleted at the end
    vector<T> items;
};

// this is here to create the instance of the garbage bin.
template <class T> garbage_bin<T>*  garbage_bin<T>::pinstance = NULL;


#endif // GARBAGE_BIN_H
