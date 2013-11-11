#ifndef GARBAGE_BIN_H
#define GARBAGE_BIN_H
#pragma once

#include <vector>
#include <stdlib.h>
#include <iostream>
#include <typeinfo>

#ifndef _WIN32
#include <cxxabi.h>
#endif

using namespace std;

class garbage_bin_base;

class garbage_bin_bin {
public:

    void throwIn(garbage_bin_base* rubbish) { items.push_back(rubbish); }
    void empty();
    static void shutdown();
    static garbage_bin_bin& instance();
    ~garbage_bin_bin();
private:
    garbage_bin_bin();
    vector<garbage_bin_base*> items;
    static garbage_bin_bin* minstance;
};

class garbage_bin_base {
public:
    virtual void empty() = 0;
    garbage_bin_base() { garbage_bin_bin::instance().throwIn(this); }
    virtual ~garbage_bin_base() {}
};

template <typename T>
class garbage_bin : public garbage_bin_base
{
private:

    struct location
    {
        long line;
        std::string file;
        T item;
    };

    /**
     * Constructor. Creates a new Garbage Bin object.
     */
    garbage_bin()
    {
        items = new vector<location>();
    }

    // the instance
    static garbage_bin<T>* pinstance;

public:


    /**
     * Returns the instance of this type of garbage bin...
     */
    static garbage_bin& instance()
    {
        if(pinstance == NULL)
        {
            pinstance = new garbage_bin();
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
    ~garbage_bin()
    {
        items->clear();
        delete items;
    }

    /**
     * Empties the garbage bin...
     */
    void empty()
    {
#ifndef _WINDOWS
        int status;
        char* s = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status) ;
        std::cout << "\nGBIN [" << s << "] TIN:" << items->size() << " PLC:" <<  singles.size() << std::endl;
#endif
        for(size_t i=0; i<items->size(); i++)
        {
            delete [] (items->at(i).item);
        }


        for(size_t i=0; i<singles.size(); i++)
        {
            delete singles[i];
        }
#ifndef _WINDOWS
        free(s);
#endif
    }

    /**
     * Adds an item to the garbage bin
     */
    void throwIn(T item, const char* file, long line)
    {
        location a;
        a.file = std::string(file);
        a.line = line;
        a.item = item;
        items->push_back(a);
    }

    void throwIn(T item)
    {
        location a;
        a.file = "";
        a.line = 0;
        a.item = item;
        items->push_back(a);
    }

    void place(T item)
    {
        singles.push_back(item);
    }


private:

    // this is the vector which actually contains the pointers that will be deleted at the end
    vector<location> *items;
    vector<T> singles;

};

// this is here to create the instance of the garbage bin.
template <class T> garbage_bin<T>*  garbage_bin<T>::pinstance = NULL;


#endif // GARBAGE_BIN_H
