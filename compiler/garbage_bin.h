#ifndef GARBAGE_BIN_H
#define GARBAGE_BIN_H

#include <vector>
#include <map>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <typeinfo>
#include <string.h>

class garbage_bin_base;
class nap_compiler;

class garbage_bin_bin 
{
public:

    void throwIn(garbage_bin_base* rubbish) { items.push_back(rubbish); }
    void empty(const nap_compiler *_compiler);
    static void shutdown();
    static void release();
    static garbage_bin_bin& instance();
    ~garbage_bin_bin();
private:
    garbage_bin_bin();
    std::vector<garbage_bin_base*> items;
    static garbage_bin_bin* minstance;
    friend class nap_compiler;
};

class garbage_bin_base {
public:
    virtual void empty(const nap_compiler*) = 0;
    garbage_bin_base() { garbage_bin_bin::instance().throwIn(this); }
    virtual ~garbage_bin_base() {}
};

template <typename T>
class garbage_bin : public garbage_bin_base
{
private:

    struct location
    {
        T item;
    };

    /**
     * Constructor. Creates a new Garbage Bin object.
     */
    garbage_bin()
    {
    }

    // the instance
    static std::map<const nap_compiler*, garbage_bin<T>* > pinstances;

public:

    /**
     * Returns the instance of this type of garbage bin...
     */
    static garbage_bin& instance(const nap_compiler* _compiler)
    {
        if(pinstances[_compiler] == NULL)
        {
            pinstances[_compiler] = new garbage_bin();
        }

        return *(pinstances[_compiler]);
    }

    /**
     * Deletes the instance
     */
    static void deleteInstance(const nap_compiler* _compiler)
    {
        if (pinstances[_compiler])
        {
            pinstances[_compiler] = 0;
        }
    }

    /**
     * Destructor
     */
    ~garbage_bin()
    {
    }

    /**
     * Empties the garbage bin...
     */
    void empty(const nap_compiler* _compiler)
    {
        for(size_t i=0; i<items[_compiler].size(); i++)
        {
            delete [] (items[_compiler].at(i).item);
        }

        for(size_t i=0; i<singles[_compiler].size(); i++)
        {
            delete singles[_compiler][i];
        }
        items[_compiler].clear();
        singles[_compiler].clear();
        deleteInstance(_compiler);
    }

    /**
     * Adds an item to the garbage bin
     */
    void throwIn(T item, const nap_compiler* _compiler)
    {
        location a;
        a.item = item;
        items[_compiler].push_back(a);
    }

    void place(T item, const nap_compiler* _compiler)
    {
        singles[_compiler].push_back(item);
    }


private:

    // this is the vector which actually contains the pointers that will be deleted at the end
    std::map < const nap_compiler*, std::vector <location> > items;
    std::map < const nap_compiler*, std::vector<T> > singles;

};

// this is here to create the instance of the garbage bin.
template <class T> std::map<const nap_compiler*, garbage_bin<T>* >  garbage_bin<T>::pinstances;


#endif // GARBAGE_BIN_H

