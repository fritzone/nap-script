#ifndef GARBAGE_BIN_H
#define GARBAGE_BIN_H

#include <vector>
#include <map>

#include <stdlib.h>
#include <iostream>
#include <typeinfo>

#ifndef _WIN32
#include <cxxabi.h>
#endif

extern long int all_alloc;

class garbage_bin_base;
class nap_compiler;

class garbage_bin_bin {
public:

    void throwIn(garbage_bin_base* rubbish) { items.push_back(rubbish); }
    void empty(const nap_compiler *_compiler);
    static void shutdown();
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
        long line;
        std::string file;
        T item;
    };

    /**
     * Constructor. Creates a new Garbage Bin object.
     */
    garbage_bin()
    {
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
            pinstance = 0;
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
#ifndef _WINDOWS
        int status;
        char* s = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status) ;
        //std::cout << "\nGBIN [" << s << "] TIN:" << items[_compiler].size() << " PLC:" <<  singles.size() << std::endl;
#endif
        for(size_t i=0; i<items[_compiler].size(); i++)
        {
            delete [] (items[_compiler].at(i).item);
        }


        for(size_t i=0; i<singles[_compiler].size(); i++)
        {
            delete singles[_compiler][i];
        }
#ifndef _WINDOWS
        free(s);
#endif
        items.clear();
        singles.clear();
        deleteInstance(); // sets to zero only the instance, the bin_bin has the address for the delete
    }

    /**
     * Adds an item to the garbage bin
     */
    void throwIn(T item, const char* file, long line, const nap_compiler* _compiler, long count)
    {
        //std::cout << count << " @ "<< file << ":" << line << std::endl;
        all_alloc += count ;
        //std::cout<<"All memory:" << all_alloc << std::endl;

        location a;
        a.file = std::string(file);
        a.line = line;
        a.item = item;
        items[_compiler].push_back(a);
    }

    void place(T item, const nap_compiler* _compiler)
    {
        long count = sizeof(T);
        //std::cout << count << " @ " << std::endl;
        all_alloc += count ;

        //std::cout<<"All memory:" << all_alloc << std::endl;
        singles[_compiler].push_back(item);
    }


private:

    // this is the vector which actually contains the pointers that will be deleted at the end
    std::map < const nap_compiler*, std::vector <location> > items;
    std::map < const nap_compiler*, std::vector<T> > singles;

};

// this is here to create the instance of the garbage bin.
template <class T> garbage_bin<T>*  garbage_bin<T>::pinstance = NULL;


#endif // GARBAGE_BIN_H
