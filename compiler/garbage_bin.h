#ifndef GARBAGE_BIN_H
#define GARBAGE_BIN_H

#include <vector>
#include <map>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <typeinfo>
#include <string.h>

#ifndef _WINDOWS
	#include <cxxabi.h>
	#include <execinfo.h>
#else
	#ifdef NAP_CPL_BUILT_AS_SHARED
		#include "nap_cpl_exp.h"
	#elif defined NAP_CPL_BUILT_AS_STATIC
		#include "nap_cpl_s_exp.h"
	#else
		#define NAP_LIB_API
	#endif
#endif



// #define MEMORY_DEBUG

#ifdef MEMORY_DEBUG
extern long int all_alloc;
#endif

class garbage_bin_base;
class nap_compiler;

class NAP_LIB_API garbage_bin_bin 
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

class NAP_LIB_API garbage_bin_base {
public:
    virtual void empty(const nap_compiler*) = 0;
    garbage_bin_base() { garbage_bin_bin::instance().throwIn(this); }
    virtual ~garbage_bin_base() {}
};

template <typename T>
class NAP_LIB_API garbage_bin : public garbage_bin_base
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
#ifdef MEMORY_DEBUG
#ifndef _WINDOWS
        int status;
        char* s = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status) ;
        std::cout << "\nEM GBIN [" << s << "] TIN:"
                  << items[_compiler].size() << " PLC:" <<  singles.size()
                  << " COMPILER:" << _compiler
                  << std::endl;
#endif
#endif
        for(size_t i=0; i<items[_compiler].size(); i++)
        {
#ifdef MEMORY_DEBUG
            std::cerr << "DEL ITEM: "  << (void*)(items[_compiler].at(i).item )<< std::endl;
#endif
            delete [] (items[_compiler].at(i).item);
        }

#ifdef MEMORY_DEBUG
        std::cerr << " -- " << std::endl;
#endif

        for(size_t i=0; i<singles[_compiler].size(); i++)
        {
#ifdef MEMORY_DEBUG
            std::cerr << " ITEM: "  << (void*)(singles[_compiler][i]) << std::endl;
#endif
            delete singles[_compiler][i];
        }
#ifdef MEMORY_DEBUG
#ifndef _WINDOWS
        free(s);
#endif
#endif
        items[_compiler].clear();
        singles[_compiler].clear();
        deleteInstance(_compiler);
    }

#ifdef MEMORY_DEBUG
    void print_trace(FILE *out, const char *file, int line)
    {
        const size_t max_depth = 100;
        size_t stack_depth;
        void *stack_addrs[max_depth];
        char **stack_strings;

        stack_depth = backtrace(stack_addrs, max_depth);
        stack_strings = backtrace_symbols(stack_addrs, stack_depth);

        fprintf(out, "Call stack from %s:%d:\n", file, line);

        for (size_t i = 1; i < stack_depth; i++) {
            int status;
            char*t = strchr(stack_strings[i], '(');
            if(t)
            {
                stack_strings[i] = t+1;
            }
            char* t1= strchr(stack_strings[i], '+');
            if(t1)
            {
                *t1 = 0;
            }

            char* s = abi::__cxa_demangle(stack_strings[i], 0, 0, &status) ;
            if(s == NULL)
            {
                fprintf(out, "    %s\n",stack_strings[i]);
            }
            else
            {
                fprintf(out, "    %s\n", s);
                free(s);
            }

        }
        free(stack_strings); // malloc()ed by backtrace_symbols
        fflush(out);
    }
#endif

    /**
     * Adds an item to the garbage bin
     */
    void throwIn(T item, const char* file, long line, const nap_compiler* _compiler, long
             #ifdef MEMORY_DEBUG
                 count
             #endif
                 )
    {
#ifdef MEMORY_DEBUG
#ifndef _WINDOWS
int status;
char* s = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status) ;
std::cout << "\nGBIN [" << s << "] TIN:" << items[_compiler].size() << " PLC:" << singles.size()
          << " COMPILER:" << _compiler
          << std::endl;
std::cout << "ITM:" << (void*)item << std::endl;
#endif
        std::cerr << "xxxxxxxxxxxxxxxxx " << count << " @ "<< file << ":" << line << std::endl;
        //print_trace(stderr, file, line);
        std::cerr << std::endl;
        all_alloc += count ;
        std::cerr<<"All memory:" << all_alloc << std::endl;
#endif
        location a;
        a.file = std::string(file);
        a.line = line;
        a.item = item;
        items[_compiler].push_back(a);

#ifdef MEMORY_DEBUG
#ifndef _WINDOWS
free(s);
#endif
#endif
    }

    void place(T item, const nap_compiler* _compiler)
    {

#ifdef MEMORY_DEBUG
        long count = sizeof(T);
        std::cout << count << " @ " << std::endl;
        all_alloc += count ;
        std::cout<<"All memory:" << all_alloc << std::endl;
#endif

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

