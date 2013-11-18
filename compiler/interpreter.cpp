#include "interpreter.h"
#include "parametr.h"
#include "utils.h"
#include "consts.h"
#include "number.h"
#include "envelope.h"
#include "throw_error.h"
#include "bt_string.h"
#include "type.h"
#include "opr_hndl.h"
#include "notimpl.h"
#include "variable.h"
#include "method.h"
#include "sys_brkp.h"
#include "res_wrds.h"
#include "parametr.h"
#include "expression_tree.h"
#include "compiler.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

interpreter::interpreter(nap_compiler* _compiler) : mcompiler(_compiler)
{
}

std::vector<envelope*>* interpreter::listv_prepare_list(const char* src,
                                                        method* the_method,
                                                        const char* orig_expr,
                                                        call_context* cc,
                                                        int* result,
                                                        const expression_with_location* expwloc)
{
    int l = (int)strlen(src);
    std::vector<envelope*>*  lst = new std::vector<envelope*>();
    int i=0;

    // to skip the whitespace in front of the { (in case there's any)
    skip_whitespace(src, l, &i);
    if(src[i] != '{') throw_error(E0012_SYNTAXERR);
    i++;        // with this i points to the first character after the {
    while(i < l)
    {
        // can be whitespace
        skip_whitespace(src, l, &i);
        // read the next list element
        bool read_next=false;
        char* tmp = alloc_mem(char, l, cc->compiler());    // the worst case
        int j = 0;
        while(!read_next && i < l)
        {
            tmp[j++] = src[i++];
            if(src[i] == C_PAR_OP || src[i] == C_SQPAR_OP)    // read if something was in parenthesis, such as function call, etc
            {
                char x = src[i], xcl = other_par(src[i]);
                int level = 1;
                bool readt = false;
                while(!readt && i < l)                        // match the parenthesis
                {
                    tmp[j++] = src[i++];
                    if(src[i] == x) level ++;
                    if(src[i] == xcl)
                    {
                        level --;
                        if(level == 0) readt = true;
                    }
                }
                tmp[j++] = src[i++];
                if(i==l) throw_error(E0012_SYNTAXERR);
            }
            // here we have finished reading the stuff that was in parenthesis, continue till we find the ',' separator or ..
            if(src[i] == ',')
            {
                tmp[j] = 0;    // cause we added the ',' too :(
                expression_tree* new_expression = new expression_tree(expwloc);
                build_expr_tree(tmp, new_expression, the_method, orig_expr, cc, result, expwloc);
                envelope* expr_holder = new_envelope(new_expression, LIST_ELEMENT, cc->compiler());
                lst->push_back(expr_holder);

                read_next = true;
                i++;    // to skip the ','
            }
            if(src[i] == '.')
            {
                if(i+1<l)
                {
                    if(src[i+1] == '.')
                    {
                        // Implement
                    }
                }
            }
        }
    }
    return lst;
}

/*
 * Prioriy of operators (highest priority to lowest priority). Highest priority is executed first
 *  = (assignment)
 * == < > <= >= (comparison)
 * << >> (shift left/right)
 * & | ~ ^ (and, or, compl, xor)
 * * / % (multiply/divide/module
 * + - (add/substract)
 */

/**
 * This  method retrieves the first available operator regarding to the operator precedence order
 * and returns its position. The parameter foundOperator is populated with the found operator
 * and the parameter ntype gets the typeid of the operator.
 */
int interpreter::get_operator(const char* expr, const char** foundOperator, int* ntype)
{
    int zladd = level_0_add_operator(expr);                    /* the position of a +- sign on the first level */
    int zlbit = level_0_bitwise_operator(expr);                /* if any bitwise (&|~_ operators can be found on level 0 */
    int zllogic = level_0_logical_operator(expr);            /* && or || or ! on the zeroth level */
    int zlmlt=level_0_multiply_operator(expr);                /* the position of a * / sign on the first level */
    int zlev_assignment = level_0_assignment_operator(expr);/* the position of an equal operator on the first level */
    int zlev_dot = level_0_dot_operator(expr);              /* the position of a dot operator on the first level */
    int zlev_shift = level_0_shift(expr);                    /* Shift operator << >> */

    const char* found_comp_operator = NULL;                        /* the comparison operator that was found */
    int zlev_comparison = level_0_comparison_operator(expr, &found_comp_operator);

    int sgeq_type = -1;                                        /* the type of the sg_eq operator*/
    const char* found_sq_eq_operator = NULL;                        /* the found sg_eq operator*/
    int zlev_sg_eq_operator = level_0_sg_eq_operator(expr, &found_sq_eq_operator, &sgeq_type);

    int zlop = -1;

    /* this is not even an operator, it is an expression, but let's treat 
     * it as an operator, and all the other found operators will simply
     * override it */
    if(zlev_dot != -1)
    {
        *ntype = OPERATOR_DOT;
        *foundOperator = mcompiler->duplicate_string(STR_DOT);
        zlop = zlev_dot;
    }

    /* lowest priority between the numeric operators */
    if(zlev_shift != -1)
    {
        zlop = zlev_shift;
        if(expr[zlop] == C_LT)
        {
            *ntype = OPERATOR_SHIFT_LEFT;
            *foundOperator = mcompiler->duplicate_string(STR_SHLEFT);
        }
        else if(expr[zlop] == C_GT)
        {
            *ntype = OPERATOR_SHIFT_RIGHT;
            *foundOperator = mcompiler->duplicate_string(STR_SHRIGHT);
        }
    }

    if(zlbit != -1)
    {
        zlop = zlbit;
        switch(expr[zlop])
        {
        case '&':
            *ntype = OPERATOR_BITWISE_AND;
            *foundOperator = mcompiler->duplicate_string(STR_BIT_AND);
            break;
        case '|':
            *ntype = OPERATOR_BITWISE_OR;
            *foundOperator = mcompiler->duplicate_string(STR_BIT_OR);
            break;
        case '^':
            *ntype = OPERATOR_BITWISE_XOR;
            *foundOperator = mcompiler->duplicate_string(STR_BIT_XOR);
            break;
        case '~':
            *ntype = OPERATOR_BITWISE_COMP;
            *foundOperator = mcompiler->duplicate_string(STR_BIT_COMP);
            break;
        }
    }

    /* next priority */
    if(zlmlt != -1)
    {
        zlop = zlmlt;
        *foundOperator = c2str(expr[zlop], mcompiler);    /* will be "*" or "/" or "%" */
        if(C_MUL == expr[zlop])
        {
            *ntype = OPERATOR_MULTIPLY;
        }
        else if(C_DIV == expr[zlop])
        {
            *ntype = OPERATOR_DIVIDE;
        }
        else if(C_MOD == expr[zlop])
        {
            *ntype = OPERATOR_MODULO;
        }
    }

    if(zladd != -1)
    {
        zlop = zladd;
        *foundOperator = c2str(expr[zlop], mcompiler);    /*  will be "+" or "-" */
        if(C_ADD == expr[zlop])
        {
            *ntype = OPERATOR_ADD;
        }
        else if(C_SUB == expr[zlop])
        {
            *ntype = OPERATOR_MINUS;
        }
    }

    /* check the comparison operator presence */
    if(zlev_comparison != -1)
    {
        if((*ntype != OPERATOR_SHIFT_LEFT && *ntype != OPERATOR_SHIFT_RIGHT) || ((zlev_comparison != zlev_shift && zlev_comparison != zlev_shift - 1 && zlev_comparison != zlev_shift + 1)))
        {
            zlop = zlev_comparison;
            *foundOperator = mcompiler->duplicate_string(found_comp_operator);
            *ntype = get_comp_typeid(*foundOperator);
        }
    }

    if(zllogic != -1)
    {
        switch(expr[zllogic])
        {
        case '&':
            *ntype = OPERATOR_BITWISE_AND;
            *foundOperator = mcompiler->duplicate_string(STR_LOGIC_AND);
            zlop = zllogic;
            break;
        case '|':
            *ntype = OPERATOR_BITWISE_OR;
            *foundOperator = mcompiler->duplicate_string(STR_LOGIC_OR);
            zlop = zllogic;
            break;
        case '!':
            /* This is a unary operator ... see if we have found another unary operator before itm such as ~ */
            if(! (*ntype == OPERATOR_BITWISE_COMP && zlbit < zllogic))
            {
                /* now check if by mistake we didn't find a zero level logic operator != ... because that'll be shit ... */
                if(zllogic != zlev_comparison)
                {
                    *ntype = OPERATOR_NOT;
                    *foundOperator = mcompiler->duplicate_string(STR_LOGIC_NOT);
                    zlop = zllogic;
                }
            }
            break;
        }
    }

    if(zlev_assignment != -1)
    {
        /* here check if this is a valid assignment, meaning
           1. the character before it should NOT be < or > or !
           2. we already found a comparison */
        if(zlev_assignment > 0 && zlop == zlev_comparison)
        {
            //char right_before_eq = ;
            if(expr[zlev_assignment - 1] != '<' && expr[zlev_assignment - 1] != '>' && expr[zlev_assignment - 1] != '!' && expr[zlev_assignment - 1] != '=')
            {
                zlop = zlev_assignment;
                *foundOperator = mcompiler->duplicate_string("=");
                *ntype = OPERATOR_ASSIGN;
            }
        }
        else    /* if zlev_assignment is == 0 it's not valid anyway, but for this case it does not matter */
        {
            zlop = zlev_assignment;
            *foundOperator = mcompiler->duplicate_string("=");
            *ntype = OPERATOR_ASSIGN;
        }
    }

    /* evaluate the +=, -= possibilities*/
    if(zlev_sg_eq_operator != -1)
    {
        zlop = zlev_sg_eq_operator;
        *foundOperator = mcompiler->duplicate_string(found_sq_eq_operator);
        *ntype = sgeq_type;
    }

    return zlop;
}


/*
 * this checks if an expression is a function or not (ex: sin(x) is s function)
 */
method* is_function_call(char *s,  call_context* cc)
{
    unsigned int i=0;
    int sc2=0;
    char *s2 = cc->compiler()->new_string(strlen(s));
    method* get_method = NULL;

    while(i<strlen(s) && s[i]!='(')
    {
        s2[sc2++]=s[i++];
    }
    s2[sc2]=0;

    s2 = trim(s2, cc->compiler());
    get_method = cc->get_method(s2);
    if(get_method) return get_method;

    return NULL;
}

/**
 * Returns the type if this expression looks like a variable definition. Return NULL if not.
 * Rule to use to define a function:
 * <return_type> <function_name,[function_parameters])
 * Where return type can be any type defined or <type func_name ( pars ) > meaning this method returns a method
 */
int interpreter::looks_like_function_def(const char* expr, int expr_len, const expression_tree* node, call_context* cc)
{
    if(node && node->father && node->father->op_type == OPERATOR_ASSIGN) return 0;    /* this is part of a templated variable definition */
    if(expr[expr_len - 1] != C_PAR_CL) return 0; /* not ending with ), return false*/
    int i=expr_len - 2; /* the first one: to skip the paranthesys, the second one is the last character inside the parantheses*/
    char* tmp = mcompiler->new_string(expr_len);    /* will hold the parameters in the first run*/
    int can_stop = 0;
    int tmpc = 0;
    int level = 1;
    while(i && !can_stop)            /* this is reading backwards */
    {
        if(expr[i] == C_PAR_CL) level ++;
        if(expr[i] == C_PAR_OP) level --;
        if(level == 0) can_stop = 1;
        if(!can_stop)tmp[tmpc++] = expr[i--];

    }
    reverse(tmp, strlen(tmp));

    if(i == 0) return 0; /* this means, we've got to the first character, there is nothing before this */

    i--;        /* skip the parantheses*/

    while(i && is_whitespace(expr[i])) i --;    /* skip the whitespaces */

    if(!i) return 0;                    /* this was something in paranthese starting with spaces */

    if(!is_identifier_char(expr[i])) return 0;    /* cannot be anything else but an identifier */

    while(i>-1 && is_identifier_char(expr[i]))    i--;    /* fetch the name */

    if(i == -1) /* meaning, either we have defined a function with no return type or this is a function call */
    {   /* we need to analyze the parameters, if they look like definition, then it's fine, give back 1 */
        std::vector<std::string> pars = string_list_create_bsep(tmp, ',', mcompiler);
        std::vector<std::string>::iterator q = pars.begin();
        while(q != pars.end())
        {
            size_t j=0;    /* will go through the parameters value and see if the first word from it is a type or not*/
            char* firstw = mcompiler->new_string(q->length());
            while(j < q->length() && (*q)[j] != ' ')
            {
                firstw[j] = (*q)[j];
                j++;
            } /* the phrase: if(int x = test() == 4) kills this ... */
            if(get_typeid(firstw) != BASIC_TYPE_DONTCARE)
            {
                /* check if the starting of the expr is not if or while or for ... */
                if(strstr(expr, STR_IF) == expr) return 0;
                if(strstr(expr, STR_WHILE) == expr) return 0;
                if(strstr(expr, STR_FOR) == expr) return 0;
                return 1;    /* function with no return type */
            }
            else
            {
                /* TODO: check if this is a constructor definition */
                if(strstr(expr, cc->get_name().c_str()) == expr)
                {
                    // this might be a constructor definition
                    const char* pfinder = expr + cc->get_name().length();
                    while(*pfinder && is_whitespace(*pfinder)) pfinder ++;
                    if(*pfinder == '(')
                    {
                        // now let's find the closing parentheses
                        while(*pfinder && *pfinder != ')') pfinder ++;
                        if(*pfinder)
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        return 0;
                    }
                }
                return 0;    /* function call*/
            }
        }

    }

    while(i && is_whitespace(expr[i])) i --;    /* skip the whitespaces between name and type */

    if(!is_identifier_char(expr[i]) && expr[i] != C_PAR_CL) return 0;    /* cannot be anything else but an identifier  */

    if(is_identifier_char(expr[i]))
    {
        char* ret_type=mcompiler->new_string(expr_len);
        int rtc = 0;
        /* fetching the return type of the function */
        while(i>-1 && is_identifier_char(expr[i]))
        {
            ret_type[rtc ++] = expr[i];
            i--;
        }
        reverse(ret_type, strlen(ret_type));
        /* now check if we still have something before this. If yes, return 0 */
        while(i > -1 && is_whitespace(expr[i])) i--; /* skip the whitespace */

        if (i == -1)
        {
            if(get_typeid(ret_type) != BASIC_TYPE_DONTCARE)
            {
                return 1;    /* valid function definition */
            }
            else
            {
                return 0;
            }
        }
        else    /* before the return type there was something else too */
        {
            /* now fetch it. it can be 'extern' or 'use' or something else */
            tmpc = 0;

            while(i > -1 && is_identifier_char(expr[i]))
            {
                tmp[tmpc++] = expr[i--];
            }
            tmp[tmpc] = 0;
            reverse(tmp, strlen(tmp));
            if(!strcmp(tmp, STR_EXTERN) || !strcmp(tmp, STR_USE))
            {
                while(i > -1 && is_whitespace(expr[i])) i--;    /* see if we have something before the extern. We may not have*/
                if(i > -1)
                {
                    return 0;    /* there is something before it*/
                }
                else
                {
                    return 1;    /* finally, valid function definition */
                }

            }
            else
            {
                return 0;    /* something which is not extern/use was found before the type*/
            }
        }
    }
    else    /* some garbage */
    {
        return 0;
    }
    /* finally, it can return 1 */
    return 0;
}

bool interpreter::is_list_value(const char* what)
{
    int i=0;
    const char *what2 = what;
    skip_whitespace(what, strlen(what2), &i);
    if(what2[i] == '{') return true;
    return false;
}

/**
 * Returns the type if this expression looks like a variable definition. Return NULL if not.
 * Also checks if the variable defined is a class instance. (ie: TestClass a;)
 */
char* interpreter::looks_like_var_def(const call_context* cc, char* expr, int expr_len)
{
    char* first_word = mcompiler->new_string(expr_len);
    int flc = 0;
    int tc = 0;
    /* try to determine whether this is a variable definition or not */
    while(tc < expr_len && is_identifier_char(expr[tc]))
    {
        first_word[flc++] = expr[tc++];
    }
    first_word = trim(first_word, cc->compiler());

    if(get_typeid(first_word) != BASIC_TYPE_DONTCARE) /* starts with a type. maybe user defined type*/
    {
        /* now see if it continues with something like: first_word something( which means it's a method definition */
        while ( tc < expr_len && expr[tc] != C_PAR_OP && expr[tc] != C_EQ ) tc ++;
        if(tc == expr_len || expr[tc] == C_EQ)
        {
            return first_word;
        }
        if(expr[tc] == C_PAR_OP)
        {
            int tclev = 1;
            int can_stop = 0;
            tc ++;    /* to skip the parenthesys */
            while(tc < expr_len && !can_stop)
            {
                if(expr[tc] == C_PAR_OP) tclev ++;
                if(expr[tc] == C_PAR_CL) tclev --;
                if(tclev == 0) can_stop = 1;
                if(!can_stop) tc ++;
            }
            /* now tc points to the closing ')' on the zeroth level */
            tc ++;
            while(tc < expr_len && is_whitespace(expr[tc])) tc ++;
            if(tc < expr_len && expr[tc] == '=')
            {
                return first_word;    /* this is a templated variable definition */
            }
        }
    }

    if(cc->get_class_declaration(first_word))
    {
        return first_word;
    }
    return NULL;
}

/**
 * Defines a method
 */
method* interpreter::define_method(const char* expr, int expr_len, expression_tree* node, call_context* cc, const expression_with_location* expwloc)
{
    int i=expr_len - 2; /* the first one: to skip the paranthesys, the second one is the last character inside the parantheses*/
    int par_counter = 0;
    int name_counter = 0;
    int type_counter = 0;
    method* created_method = NULL;
    char* parameters = mcompiler->new_string(expr_len);    /* will hold the parameters */
    char* func_name = mcompiler->new_string(expr_len);        /* will hold the name of the function .. maybe optimize it a bit*/
    char* ret_type = mcompiler->new_string(expr_len);        /* will hold the return type definition */
    int can_stop = 0;
    int level = 1;
    while(i && !can_stop)                    /* this reads backwards */
    {
        if(expr[i] == C_PAR_CL) level ++;
        if(expr[i] == C_PAR_OP) level --;
        if(level == 0) can_stop = 1;
        if(!can_stop) parameters[par_counter++] = expr[i--];
    }
    if(i == 0)
    {
        throw_error(E0010_INVFNCDF, expr, NULL);
    }
    reverse(parameters, par_counter);
    i --;
    while(is_whitespace(expr[i])) i--;
    while(i>-1 && is_identifier_char(expr[i]))
    {
        func_name[name_counter++] = expr[i--];
    }
    reverse(func_name, name_counter);
    i--;
    while(i > -1)
    {
        ret_type[type_counter++] = expr[i--];
    }
    reverse(ret_type, type_counter);
    //printf("Defining function ret_type:[%s] name:[%s] pars:[%s]\n", ret_type, func_name, parameters);
    func_name = trim(func_name, cc->compiler());
    ret_type = trim(ret_type, cc->compiler());
    if(strlen(func_name) == 0)
    {
        throw_error(E0010_INVFNCDF, expr, NULL);
    }
    created_method = new method(mcompiler, func_name, ret_type, cc);
    created_method->feed_parameter_list(trim(parameters, cc->compiler()), expwloc);

    if(!strcmp(ret_type, "int")) created_method->ret_type = BASIC_TYPE_INT;
    // TODO: the others too

    cc->add_method(created_method);

    node->op_type = FUNCTION_DEFINITION;
    node->reference = new_envelope(created_method, FUNCTION_DEFINITION, cc->compiler());
    return created_method;
}

/**
 * Returns true if this variable declaration is followed by a definition
 * The following is the logic:
 * <NAME>[white space]['(' or '=' or '[']{if ( then skip till closed}[nothing or '=']
 * @return the position of the valid = sign for the definition
 */
int interpreter::var_declaration_followed_by_initialization(const std::string& pexpr)
{
    const char* expr = pexpr.c_str();
    int expr_len = pexpr.length();
    int i = 0;
    // here we can get [10] c = a; so let's skip them if any
    skip_whitespace(expr, expr_len, &i);    // skip the leading whitespace if any
    if(expr[i] == C_SQPAR_OP)    // square parenthesis?
    {
        skip_sq_pars(expr, expr_len, &i);
    }
    skip_whitespace(expr, expr_len, &i);
    /* now i points to the first non blank character after the square parenthesis */

    while(i < expr_len && is_identifier_char(expr[i])) i++;    /* skip the name */
    skip_whitespace(expr, expr_len, &i);
    // actually there can be parentheses even after this ...
    if(expr[i] == C_PAR_OP || expr[i] == C_SQPAR_OP)
    {
        if(expr[i] == C_PAR_OP) skip_pars(expr, expr_len, &i); /* now i points to the character after parenthesis */
        else skip_sq_pars(expr, expr_len, &i); /* now i points to the character after square parenthesis */
    }
    skip_whitespace(expr, expr_len, &i);
    if(expr[i] == C_EQ) return i;
    return 0;
}

/**
 * Checks if the parameter is a variable name that will be accepted as a variable name:
 * - it should not start with a number
 * - it should not be a keyword
 */
int interpreter::accepted_variable_name(const std::string& name)
{
    if(name.length() < 1) return 0;
    if(isdigit(name[0])) return 0;
    for(unsigned int i=0; i<sizeof(keywords) / sizeof(keywords[0]); i++)
    {
        if(name == keywords[i]) return 0;
    }
    return 1;
}

/**
 * Defines the variables that can be found below ...
 */
std::vector<variable_definition*>* interpreter::define_variables(char* var_def_type,
                                                                 char* expr_trim,
                                                                 expression_tree* node,
                                                                 method* the_method,
                                                                 call_context* cc,
                                                                 const char* orig_expr,
                                                                 int* result,
                                                                 const expression_with_location* expwloc)
{
    char * copied = mcompiler->duplicate_string(expr_trim + strlen(var_def_type));

    std::vector<std::string> var_names = string_list_create_bsep(copied, ',', mcompiler);
    std::vector<std::string>::iterator q = var_names.begin();
    std::vector<variable_definition*>* var_def_list = new std::vector<variable_definition*>(); /* will contain the variable definitions if any*/

    garbage_bin<std::vector<variable_definition*>*>::instance().place(var_def_list, cc->compiler());

    var_def_type = trim(var_def_type, cc->compiler());
    while(q != var_names.end())
    {
        multi_dimension_def* mdd = NULL, *qm;    /* will contain the dimension definitions if any */
        std::string name = *q;

        if(!accepted_variable_name(name))
        {
            throw_error(E0037_INV_IDENTF, name);
        }
        variable* added_var = NULL;    /* will be used if we'll need to implement the definition */
        const char* idx_def_start = strchr(name.c_str(), C_SQPAR_OP);
        const char* const pos_eq = strchr(name.c_str(), C_EQ);
        if(pos_eq && idx_def_start > pos_eq)
        {
            idx_def_start = NULL; /* no index definition if the first index is after an equation sign*/
        }

        variable_definition* var_def = NULL; /* the variable definition for this variable. might contain
                                                multi-dimension index defintion and/or value initialization
                                                or neither of these two */
        if(idx_def_start) /* index defined? */
        {
            int can_stop = 0;
            std::string index;

            if(!strchr(name.c_str(), C_SQPAR_CL)) /* definitely an error */
            {
                throw_error(E0011_IDDEFERR, name.c_str(), NULL);
            }
            /* now read the index definition */
            idx_def_start ++;
            int level = 0;
            while(*idx_def_start && !can_stop)
            {
                if(*idx_def_start == C_SQPAR_OP) level ++;
                if(*idx_def_start == C_SQPAR_CL && --level == -1) can_stop = 1;
                if(!can_stop) index += *idx_def_start ++;
            }
            std::vector<std::string> dimensions = string_list_create_bsep(index.c_str(), C_COMMA, mcompiler);
            std::vector<std::string>::iterator qDimensionStrings = dimensions.begin();    /* to walk through the dimensions */
            int countedDimensions = 0;
            mdd = alloc_mem(multi_dimension_def,1, cc->compiler());
            qm = mdd;
            while(qDimensionStrings != dimensions.end())
            {
                expression_tree* dim_def_node = new expression_tree(expwloc);
                if(qDimensionStrings->length() > 0)
                {
                    build_expr_tree((*qDimensionStrings).c_str(), dim_def_node, the_method, orig_expr, cc, result, expwloc);
                }
                else
                {
                    /* check if there are multiple dimensions for this variable. If yes disallow the dynamic dimensions for now */
                    if(countedDimensions > 0)    /* awkward but correct */
                    {
                        throw_error(E0038_DYNDIMNALL, expr_trim);
                    }
                    qm->dynamic = 1;
                }
                qm->dimension = -1;
                qm->expr_def = dim_def_node;
                qm->next = alloc_mem(multi_dimension_def,1, cc->compiler());
                qm = qm->next;
                countedDimensions ++;
                qDimensionStrings ++;
            }
        }

        /* check whether we have direct initialization */
        int eqp = var_declaration_followed_by_initialization(name);
        char* deflist = NULL;                /* the definition list for this variable */
        if(eqp)
        {
            char* name_val = mcompiler->duplicate_string(name.c_str());
            char* pos_eq = name_val + eqp;
            *pos_eq = 0; // TODO: This is ugly, fix it!
            name = trim(name_val, cc->compiler());
            deflist = mcompiler->duplicate_string(pos_eq + 1);
            deflist = trim(deflist, cc->compiler());
        }

        if(idx_def_start)
        {
            const char* tmpname = strrchr(name.c_str(), C_SQPAR_CL) + 1;
            /* TODO: ez meghal: int x=a[1]; re */
            char* tmp1name = trim(tmpname, cc->compiler());
            if(strlen(tmp1name) == 0)    /* in this case the index definition was after the name: int name[12];*/
            {
                std::string temp;
                size_t tc = 0;
                while(tc < name.length() && (name[tc] != C_SQPAR_OP && name[tc] != C_SPACE))
                {
                    temp += name[tc];
                    tc ++;
                }

                name = temp;
            }
            else    /* if the index definition was before the name: int[12] name*/
            {
                name = mcompiler->duplicate_string(tmp1name);
            }
        }

        if(cc)
        {
            added_var = cc->add_variable(name.c_str(), var_def_type, 1, expwloc);
        }

        if(!added_var)
        {
            throw_error("Internal: a variable cannot be defined", NULL);
        }

        /* create the variable definition/declaration structure for both of them */
        var_def = alloc_mem(variable_definition,1, cc->compiler());
        var_def->the_variable = added_var;
        var_def->md_def = mdd;

        if(deflist)    /* whether we have a definition for this variable. if yes, we need to populate a definition_list */
        {
            expression_tree* var_def_node = new expression_tree(expwloc);
            garbage_bin<expression_tree*>::instance().place(var_def_node, cc->compiler());

            build_expr_tree(deflist, var_def_node, the_method, orig_expr, cc, result, expwloc);
            var_def->the_value = var_def_node;
        }

        var_def_list->push_back(var_def);

        q ++;
    }

    node->op_type = NT_VARIABLE_DEF_LST;
    node->info = expr_trim;
    node->reference = new_envelope(var_def_list, NT_VARIABLE_DEF_LST, cc->compiler());
    return var_def_list;
}

/**
 * Handles a function call.
 * @param expr_trim - the expression as text
 * @param expr_len - the length of the expression
 * @param node - the current position in the tree
 * @param func_call - is the function that is being called
 * @param the_method - we are inside this method
 * @param orig_expr - this is the expression in which method call was found
 * @param cc - the call cotnext we are in
 * @param result - will hold the result value. Used in subsequent calls to build_expr_tree
 * @param type_of_call - whether this is a normal function call (0) a constructor call (1) or 
 *                       a method call from an object (2) or a static methid of a class (3)
 */
call_frame_entry* interpreter::handle_function_call(char *expr_trim, int expr_len, expression_tree* node,
        method* func_call, method* the_method,
        const char* orig_expr, call_context* cc, int* result,
        const expression_with_location* expwloc, int type_of_call)
{
    char *params_body = mcompiler->new_string(expr_len);
    std::vector<std::string> parameters;
    std::vector<parameter*> pars_list;
    call_frame_entry* cfe = NULL;
    strcpy(params_body, expr_trim + strlen(func_call->method_name) + 1); /* here this is supposed to copy the body of the parameters without the open/close paranthesis */
    while(is_whitespace(*params_body)) params_body ++;
    if(*params_body == C_PAR_OP) params_body ++;        /* now we've skipped the opening paranthesis */

    int pb_end = strlen(params_body);
    while(is_whitespace(params_body[pb_end]))
    {
        pb_end --;
        params_body[pb_end - 1] = 0;
    }    /* this removed the trailing spaces */

    if(params_body[pb_end - 1] == C_PAR_CL)
    {
        params_body[pb_end - 1]=0;
    }    /* this removed the closing paranthesis */


    params_body = trim(params_body, cc->compiler());
    //printf("Checking function call:[%s]\n", params_body);
    /* Now: To build the parameter list, and create a call_frame_entry element to insert into the tree */
    parameters = string_list_create_bsep(params_body, ',', mcompiler);
    std::vector<std::string>::iterator q = parameters.begin();
    while(q != parameters.end())
    {
        expression_tree* cur_par_expr = NULL;
        if(q->length() > 0)
        {
            cur_par_expr = new expression_tree(expwloc);
            garbage_bin<expression_tree*>::instance().place(cur_par_expr, cc->compiler());
            build_expr_tree((*q).c_str(), cur_par_expr, the_method, orig_expr, cc, result, expwloc);
            
            parameter* cur_par_obj = new_parameter(the_method);
            cur_par_obj->expr = cur_par_expr;

            pars_list.push_back(cur_par_obj);
        }

        q ++;
    }
    cfe = alloc_mem(call_frame_entry, 1, cc->compiler());
    cfe->the_method = func_call;
    cfe->parameters = pars_list;
    cfe->previous_cf = func_call->cur_cf;

    node->info = mcompiler->duplicate_string(func_call->method_name);
    node->op_type = FUNCTION_CALL + type_of_call; // TODO: This is tricky and ugly
    node->reference = new_envelope(cfe, FUNCTION_CALL + type_of_call, cc->compiler());
    cfe->the_method = cc->get_method(func_call->method_name);
    return cfe;
}

/**
 * This method check whether the expression passed in is some sort of indexed
 * access. In case it is, returns the element that is indexed. The value of the
 * index is populated with the index
 */
char* interpreter::is_indexed(const char* expr_trim, int expr_len, char** index)
{
    const char* p = expr_trim;
    char* the_indexed_part = mcompiler->new_string(expr_len);
    char* q = the_indexed_part;
    int level = 0;
    while(*p)
    {
        if(*p == C_PAR_OP)
        {
            level ++;
            p ++;
            int can_stop = 0;
            while(*p && !can_stop)
            {
                if(*p == C_PAR_OP) level ++;
                if(*p == C_PAR_CL && --level == 0) can_stop = 1;
                if(!can_stop) *q ++ = *p ++;
            }
            *q ++ = *p ++;    /* fetched the closing parantheses */
        } /* here p points to the first character after the closing ')'*/
        else if(*p =='"')
        {
            p++;
            int can_stop = 0;
            while(*p && !can_stop) p++;
            if(*p == C_QUOTE && *(p-1) != C_BACKSLASH) can_stop = 1;
        }
        else if(*p == C_SQPAR_OP) /*now try to copy the index*/
        {
            p++;
            char* idx = *index;
            int can_stop = 0;
            int level_idx = 1;
            while(!can_stop)
            {
                if(*p == C_SQPAR_OP) level_idx ++;
                if(*p == C_SQPAR_CL && --level_idx == 0) can_stop = 1;
                if(!can_stop) *idx ++ = *p ++;
            }
            p++;
            if(*p)
            {
                while(is_whitespace(*p)) p++;
                if(!*p) return the_indexed_part;    /* we're at the end, found something before the indexes that can be returned*/
                if(*p == C_SQPAR_OP)                /* double index: string vector's xth. element yth character*/
                {
                    strcat(the_indexed_part, "[");
                    strcat(the_indexed_part, *index);    /* update the first part*/
                    strcat(the_indexed_part, "]");
                    int canstop2 = 0;
                    int level2 = 1;
                    p++;                            /* skip the second opening square bracket*/
                    memset(*index, 0, strlen(*index));    /* reset the index*/
                    idx = *index;
                    while(!canstop2)
                    {
                        if(*p == C_SQPAR_OP) level2 ++;
                        if(*p == C_SQPAR_CL && --level2 == 0) canstop2 = 1;
                        if(!canstop2) *idx ++ = *p ++;
                    }
                    p++;    /* now idx contains the new index and p points to the first character after the second index*/
                    if(*p)
                    {
                        while (is_whitespace(*p)) p++;
                        if(!*p) return the_indexed_part;
                        return NULL;
                    }
                    else
                    {
                        return the_indexed_part;
                    }
                }
                else
                {
                    return NULL;    /* here: check if we have a vector of funtions and trying to call one of them */
                }
            }
            else
            {
                return the_indexed_part;
            }
        }
        else
        {
            *q ++ = *p ++;
        }
    }
    return NULL;
}

/**
 * Checks if the expression passed in some statement that starts with a reserved word.
 * The return value is the part after the keywords
 */
char* interpreter::is_some_statement(const char* expr_trim, const char* keyword)
{
    if(strstr(expr_trim, keyword) == expr_trim)
    {
        char* retv = trim(mcompiler->duplicate_string(expr_trim + strlen(keyword)), mcompiler);
        return retv;
    }
    return NULL;
}

/**
 * Returns the necessary structure for the break/continue statements
 */
void* interpreter::deal_with_one_word_keyword( call_context* cc, expression_tree* node, int* &result, const char* keyw, int statement )
{
    if(cc->get_type() != CALL_CONTEXT_TYPE_WHILE && cc->get_type() != CALL_CONTEXT_TYPE_FOR)
    {
        int in_iterative_cc = 0;
        call_context* tmpcc = cc->get_father();
        while(tmpcc)
        {
            if(tmpcc->get_type() == CALL_CONTEXT_TYPE_WHILE || tmpcc->get_type() == CALL_CONTEXT_TYPE_FOR)
            {
                in_iterative_cc = 1;
                break;
            }
            tmpcc = tmpcc->get_father();
        }
        if(!in_iterative_cc)
        {
            throw_error(E0036_NOBREAK);
        }
    }
    node->info = mcompiler->duplicate_string(keyw);
    node->reference = new_envelope(cc, ENV_TYPE_CC, cc->compiler());
    node->op_type = statement;
    *result = statement;
    return node->reference;
}

/**
 * This method deals with preparing structures for the keywords if/while since these are handled quite similarly this phase
 */
void* interpreter::deal_with_conditional_keywords(char* keyword_if,
                                                  char* keyword_while,
                                                  expression_tree* node,
                                                  const expression_with_location* expwloc,
                                                  char* expr_trim,
                                                  int expr_len,
                                                  method* the_method,
                                                  const char* orig_expr,
                                                  call_context* cc,
                                                  int* &result )
{
    int one_line_stmt = -1;
    int stmt = -1;
    const char* keyw = NULL;
    /* Check if this is an IF or a WHILE statement, since these two are handled the same way mostly */
    if(keyword_if || keyword_while)
    {
        if(keyword_if)
        {
            one_line_stmt = STATEMENT_IF_1L;
            stmt = STATEMENT_IF;
            keyw = STR_IF;
        }
        else if(keyword_while)
        {
            one_line_stmt = STATEMENT_WHILE_1L;
            stmt = STATEMENT_WHILE;
            keyw = STR_WHILE;
        }
    }

    if(keyw)
    {
        node->info = mcompiler->duplicate_string(keyw);
        expression_tree* expt = new expression_tree(expwloc);
        garbage_bin<expression_tree*>::instance().place(expt, cc->compiler());

        /* here fetch the part which is in the parentheses after the keyword and build the tree based on that*/
        char *condition = mcompiler->duplicate_string(expr_trim + strlen(keyw));
        while(is_whitespace(*condition)) condition ++;    /* skip the whitespace */
        if(*condition != C_PAR_OP)                        /* next char should be '(' */
        {
            throw_error(E0012_SYNTAXERR, NULL);
        }
        char* p = ++condition;                                /* skip the parenthesis */
        char* m_cond = mcompiler->new_string(expr_len);
        p = trim(extract_next_enclosed_phrase(p, C_PAR_OP, C_PAR_CL, m_cond), mcompiler);
        build_expr_tree(m_cond, expt, the_method, orig_expr, cc, result, expwloc);
        if(strlen(p) > 1)    /* means: there is a one lined statement after the condition*/
        {
            node->info = p;    /* somehow we must tell the external world what's the next expression */
            node->reference = new_envelope(expt, one_line_stmt, cc->compiler());
            *result = one_line_stmt;
            node->op_type = one_line_stmt;
        }
        else
        {
            node->reference = new_envelope(expt, stmt, cc->compiler());
            *result = stmt;
            node->op_type = stmt;
        }
        return node->reference;
    }

    return NULL;
}

/**
 * This builds the tree of the expression.
 * @param expr - is the expression as a string, initially the full expression and at a later stage parts of it
 *                 which were identified by the procedure and used in a recursive call
 * @param node - is the current node we are working on. at the first step it's the root of the tree, and later stages
 *                 is the current part of the expression
 * @param the_method - is the  method in which this expression is used. This parameter can be used to identify the
 *                        variables that might have been used in the expression
 * @param orig_expr - is the original expression, not changed by the  method. Used for error reporting.
 * The  method builds the tree of expressions. At its first call the whole expression is sent in, after this based on
 * parantheses and operators, the expression is broken up into pieces and recursively the  method is called to continue
 * the building of the tree for various parts of the expression.
 * The parameter node should be a valid pointer at the entry, this  method is only manipulating its context.
 * The operator precedence dealed is the following, from highest priority to lowest one:
 * 1. assignment operator (=)
 * 2. comparison operators (==, !=, <, >, <=, >=)
 * 3. multiplication (*, /, %)
 * 4. addition (+, -)
 */
void* interpreter::build_expr_tree(const char *expr, expression_tree* node, method* the_method, const char* orig_expr, call_context* cc, int* result, const expression_with_location* expwloc)
{
    set_location(expwloc);

    /* some variables that will be used at a later stage too */
    char* expr_trim = trim(expr, mcompiler);
    int expr_len = strlen(expr_trim);
    const char* foundOperator;    /* the operator which will be identified*/
    int zlop;                    /* the index of the identified zero level operation */
    char* var_def_type = looks_like_var_def(cc, expr_trim, expr_len);
    int func_def = looks_like_function_def(expr_trim, expr_len, node, cc);
    method* func_call = NULL; /* if this entry is a function call or or not ... */
    char* index = mcompiler->new_string(expr_len);
    char* indexed_elem = strchr(expr_trim,C_SQPAR_CL) && strchr(expr_trim, C_SQPAR_OP) ? is_indexed(expr_trim, expr_len, &index): NULL;

    char* keyword_return = is_some_statement(expr_trim, STR_RETURN);
    char* keyword_if = is_some_statement(expr_trim, STR_IF);
    char* keyword_while = is_some_statement(expr_trim, STR_WHILE);
    char* keyword_for = is_some_statement(expr_trim, STR_FOR);
    char* keyword_new = is_some_statement(expr_trim, STR_NEW);

    if(!strcmp(expr, STR_CLOSE_BLOCK)) /* destroy the call context*/
    {
        node->op_type = STATEMENT_CLOSE_CC;
        return NULL;
    }

    if(!strcmp(expr, STR_OPEN_BLOCK))
    {
        node->op_type = STATEMENT_NEW_CC;
        return NULL;
    }

    if(is_list_value(expr))
    {
        std::vector<envelope*> *thlist = listv_prepare_list(expr, the_method, orig_expr, cc, result, expwloc);
        node->op_type = LIST_VALUE;
        node->reference = new_envelope(thlist, LIST_VALUE, cc->compiler());
        return 0;
    }


    /* first check: See if this expr is a string like expression, meaning in quotes. */
    if(bt_string::is_string(expr_trim, expr_len))
    {
        expr_trim[expr_len-1] = 0;    // removing the double quotes
        expr_trim++;
        bt_string* the_str = new bt_string(expr_trim);
        node->reference = new_envelope(the_str, BASIC_TYPE_STRING, cc->compiler());
        node->info = mcompiler->duplicate_string(the_str->the_string());
        *result = RESULT_STRING;
        node->op_type = BASIC_TYPE_STRING;
        return expr_trim;
    }

    /* this is a 'statement' string, a string which is executed and the result is interpreted by the interpreter backticks: `` */
    if(bt_string::is_statement_string(expr_trim, expr_len))
    {
        expr_trim[expr_len-1] = 0;    /* removing the double quotes */
        expr_trim++;
        node->reference = new_envelope(new bt_string(expr_trim), BASIC_TYPE_STRING, cc->compiler());
        node->info = mcompiler->duplicate_string(expr_trim);
        *result = BACKQUOTE_STRING;
        return expr_trim;
    }

    /* check if this is a function definition */
    if(func_def)
    {
        method* mth = define_method(expr_trim, expr_len, node, cc, expwloc);
        *result = FUNCTION_DEFINITION;
        return mth;
    }

    /* second check: variable definition? */
    if(var_def_type && strcmp(var_def_type, "new"))
    {
        std::vector<variable_definition*>* vdl = define_variables(var_def_type, expr_trim,
                                                         node, the_method, cc,
                                                         orig_expr, result,
                                                         expwloc);
        *result = NT_VARIABLE_DEF_LST;
        return vdl;
    }

    /* now check if any keyword is used here, and deal with that */

    if(keyword_new)
    {
        node->op_type = STATEMENT_NEW;
        char* constructor_name = mcompiler->duplicate_string(keyword_new);
        if(strchr(constructor_name, '('))
        {
            *strchr(constructor_name, '(') = 0;
        }
        class_declaration* cd = cc->get_class_declaration(constructor_name);
        if(!cd)
        {
            throw_error("Invalid constructor: ", constructor_name);
        }
        method* called_constructor = cd->get_method(constructor_name);
        constructor_call* tmp = (constructor_call*)realloc(called_constructor, sizeof(constructor_call));
        if(tmp == NULL)
        {
            throw_error("Not enough memory");
        }
        called_constructor = tmp;
        constructor_call* ccf = (constructor_call*)called_constructor;
        ccf->the_class = cd;
        call_frame_entry* cfe = handle_function_call(keyword_new, expr_len,
                                                     node, ccf, the_method,
                                                     orig_expr, cd, result,
                                                     expwloc,
                                                     METHOD_CALL_CONSTRUCTOR);
        *result = STATEMENT_NEW;
        return cfe;
    }

    /* check if this is a 'return' statement */
    if(keyword_return)
    {
        node->info = mcompiler->duplicate_string(STR_RETURN);
        expression_tree* expt = new expression_tree(expwloc);
        garbage_bin<expression_tree*>::instance().place(expt, cc->compiler());
        build_expr_tree(keyword_return, expt, the_method, orig_expr, cc, result, expwloc);
        node->reference = new_envelope(expt, RETURN_STATEMENT, cc->compiler());
        *result = RETURN_STATEMENT;
        node->op_type = RETURN_STATEMENT;
        return node->reference;
    }
    if(!strcmp(expr_trim, STR_BREAK))    /* the break keyword */
    {
        return deal_with_one_word_keyword(cc, node, result, STR_BREAK, STATEMENT_BREAK);
    }

    if(!strcmp(expr_trim, STR_CONTINUE))    /* the continue keyword */
    {
        return deal_with_one_word_keyword(cc, node, result, STR_CONTINUE, STATEMENT_CONTINUE);
    }

    /* if or while? */
    if(keyword_if || keyword_while)
    {
        return deal_with_conditional_keywords(keyword_if, keyword_while,
                                              node, expwloc, expr_trim,
                                              expr_len, the_method,
                                              orig_expr, cc, result);
    }

    /* the for keyword? */
    if(keyword_for)
    {
        char* fors_trm = trim(keyword_for, mcompiler);
        int fors_len = strlen(fors_trm);
        if(fors_trm[0] != C_PAR_OP && fors_trm[fors_len - 1] != C_PAR_CL)
        {
            throw_error(E0012_SYNTAXERR);
        }
        char* for_par = mcompiler->new_string(fors_len);
        int i = 0, j = 1, level = 1;
        int done = 0;
        while(!done && j < fors_len)
        {
            if(fors_trm[j] == C_PAR_OP) level ++;
            if(fors_trm[j] == C_PAR_CL) level --;
            if(level == 0) done = 1;
            if(!done)for_par[i++] = fors_trm[j++];
        }
        for_par = trim(for_par, mcompiler);
        if(strlen(for_par) == 0)
        {
            throw_error(E0012_SYNTAXERR);
        }
        std::vector<std::string> content = string_list_create_bsep(for_par, C_SEMC, mcompiler);
        if(content.empty())
        {
            throw_error(E0012_SYNTAXERR, for_par);
        }
        char* init_stmt = mcompiler->duplicate_string(content[0].c_str());                /* the init statement */
        char* cond_stmt = content.size() > 1?mcompiler->duplicate_string(content[1].c_str()):0;                    /* the condition */
        char* expr_stmt = content.size() == 3?mcompiler->duplicate_string(content[2].c_str()):0;
        if(content.size() > 3)
        {
            throw_error(E0012_SYNTAXERR, for_par);
        }
        resw_for* rswfor = alloc_mem(resw_for,1, cc->compiler());

        rswfor->unique_hash = mcompiler->duplicate_string(generate_unique_hash().c_str());
        rswfor->init_stmt = init_stmt;
        rswfor->tree_init_stmt = new expression_tree(expwloc);
        build_expr_tree(init_stmt, rswfor->tree_init_stmt, the_method, orig_expr, cc, result, expwloc);

        rswfor->condition = cond_stmt;
        rswfor->tree_condition = new expression_tree(expwloc);
        build_expr_tree(cond_stmt, rswfor->tree_condition, the_method, orig_expr, cc, result, expwloc);

        rswfor->expr = expr_stmt;
        rswfor->tree_expr = new expression_tree(expwloc);
        build_expr_tree(expr_stmt, rswfor->tree_expr, the_method, orig_expr, cc, result, expwloc);

        node->info = mcompiler->duplicate_string(STR_FOR);

        char *condition = mcompiler->duplicate_string(expr_trim + strlen(STR_FOR));    /* not actually condition, but the for's three statements: init, cond, expr*/
        while(is_whitespace(*condition)) condition ++;    /* skip the whitespace */
        if(*condition != C_PAR_OP)                        /* next char should be '(' */
        {
            throw_error(E0012_SYNTAXERR, NULL);
        }
        char* p = ++condition;                                /* skip the parenthesis */
        char* m_cond = mcompiler->new_string(expr_len);
        p = trim(extract_next_enclosed_phrase(p, C_PAR_OP, C_PAR_CL, m_cond), mcompiler);
        if(strlen(p) > 1)    /* means: there is a one lined statement after the condition*/
        {
            node->info = p;    /* somehow we must tell the external world what's the next expression */
            node->reference = new_envelope(rswfor, STATEMENT_FOR_1L, cc->compiler());
            *result = STATEMENT_FOR_1L;
            node->op_type = STATEMENT_FOR_1L;
        }
        else
        {
            node->reference = new_envelope(rswfor, STATEMENT_FOR, cc->compiler());
            *result = STATEMENT_FOR;
            node->op_type = STATEMENT_FOR;
        }
        return node->reference;
    }

    /* now: find the operators and just play with them */
    foundOperator = NULL;
    int ntype = NO_OPERATOR;                    /* the type number of the node, firstly let's assume the node contains nothing like an operator */
    zlop = get_operator(expr_trim, &foundOperator, &ntype);    /* zlop will contain the index of the zeroth level operator */
    /* ok, here start checking what we have gathered till now */

    if(zlop!=-1)    /* we have found an operator on the zero.th level */
    {
        char *beforer = trim(before(zlop, expr_trim, mcompiler), mcompiler);
        if(strlen(beforer) == 0)
        {
            /* now we should check for the 'unary' +- operators */
            if(ntype == OPERATOR_ADD || ntype == OPERATOR_MINUS || ntype == OPERATOR_BITWISE_COMP || ntype == OPERATOR_NOT)
            {
                /* check if this is a number or not ?*/
                char* afterer = trim(mcompiler->duplicate_string(expr_trim + 1), mcompiler);
                if(!isnumber(afterer))
                {
                    node->info = mcompiler->duplicate_string(foundOperator);
                    if(ntype == OPERATOR_ADD)
                    {
                        ntype = OPERATOR_UNARY_PLUS;
                    }
                    else if(ntype == OPERATOR_MINUS)
                    {
                        ntype = OPERATOR_UNARY_MINUS;
                    }
                    node->op_type = ntype;
                    node->left = new expression_tree(node, expwloc);
                    build_expr_tree(trim(mcompiler->duplicate_string(expr_trim + 1), mcompiler),
                                    node->left, the_method, orig_expr, cc, result, expwloc);
                    return NULL;
                }
                else
                {
                    zlop = -1;
                }
            }
            else    /* right now we are not dealing with more unary operators */
            {
                throw_error(E0012_SYNTAXERR, expr, NULL);
            }
        }
        else
        {
            /* finding the part which is after the operator */

            char* afterer = trim(after(zlop + (foundOperator ? strlen(foundOperator) -1 : 0), expr_trim, mcompiler), mcompiler);
            if(strlen(afterer)==0)
            {
                throw_error(E0012_SYNTAXERR, expr, NULL);
            }
            node->info = mcompiler->duplicate_string(foundOperator);
            node->op_type = ntype;
            node->left=new expression_tree(node, expwloc);
            node->right=new expression_tree(node, expwloc);
            garbage_bin<expression_tree*>::instance().place(node->left, cc->compiler());
            garbage_bin<expression_tree*>::instance().place(node->right, cc->compiler());

            /* the order here is important for the "." operator ... it needs to know the parent in order to identify the object to find its call_context*/
            build_expr_tree(beforer, node->left, the_method, orig_expr, cc, result, expwloc);
            build_expr_tree(afterer, node->right, the_method, orig_expr, cc, result, expwloc);
        }

        if(zlop != -1)
        {
            return NULL;
        }
    }

    // no operator on the zeroth level

    /* Is this is a function call?*/
    if((func_call = is_function_call(expr_trim, cc)))
    {
        call_frame_entry* cfe = handle_function_call(expr_trim, expr_len, node, func_call, the_method, orig_expr, cc, result, expwloc, METHOD_CALL_NORMAL);
        *result = FUNCTION_CALL;
        return cfe;
    }

    /* Check if this is a function call of an object*/
    if(node->father && node->father->op_type == OPERATOR_DOT)
    {
        /* let's search for the type in the left of the father.. */
        if(node->father->left && node->father->left != node)
        {
            if(node->father->left->op_type == BASIC_TYPE_VARIABLE)  // to solve: a.func()
            {
                variable* v = (variable*)(node->father->left->reference->to_interpret);
                class_declaration* cd = v->cc->get_class_declaration(v->c_type);
                if(!cd)
                {
                    // see if this is a string or not
                    if(v->i_type == BASIC_TYPE_STRING)
                    {
                        if(!strcmp(expr_trim, "len")) // the length of the string
                        {
                            *result = FUNCTION_STRING_LEN;
                            node->op_type = *result;
                            return 0;
                        }
                    }
                    else
                    {
                        throw_error("Only class type variables can call methods on themselves", v->name);
                    }
                }
                if((func_call = is_function_call(expr_trim, cd)))
                {
                    call_frame_entry* cfe = handle_function_call(expr_trim, expr_len, node, func_call, the_method, orig_expr, cd, result, expwloc, METHOD_CALL_OF_OBJECT);
                    *result = FUNCTION_CALL;
                    return cfe;
                }

                // now see if this is a class variable: a.b = 4
                std::vector<variable*>::const_iterator it = cd->variable_list_has_variable(expr_trim, cd->get_variables());
                if(it != cd->get_variables().end())
                {
                    variable* var = *it;
                    *result = MEMBER_ACCESS_OF_OBJECT;
                    envelope* envl = new_envelope(var, BASIC_TYPE_VARIABLE, cc->compiler());
                    node->op_type = MEMBER_ACCESS_OF_OBJECT;
                    node->reference = envl;
                    return envl;
                }
            }
            else
                if(node->father->left->op_type == FUNCTION_CALL)    // to solve func().anotherOne()
                {
                    notimpl("func().anotherOne()");
                }
        }
    }

    /* check: pre-increment */
    if(expr_len > 2 && expr_trim[0] == expr_trim[1] && (expr_trim[0] == C_SUB || expr_trim[0] == C_ADD) )
    {
        const char* p = STR_PLUSPLUS;
        ntype = OPERATOR_PREINC;
        if(expr_trim[0] == C_SUB)
        {
            p = STR_MINMIN;
            ntype = OPERATOR_PREDEC;
        }
        node->info = mcompiler->duplicate_string(p);
        node->op_type = ntype;
        node->left = new expression_tree(node, expwloc);
        build_expr_tree(expr_trim + 2, node->left, the_method, orig_expr, cc, result, expwloc);
    }
    else
        /* check if it's post increment/decrement */
        if(expr_len > 2 && expr_trim[expr_len - 1] == expr_trim[expr_len - 2] && (expr_trim[expr_len - 1] == C_SUB || expr_trim[expr_len - 1] == C_ADD ) )
        {
            const char* p = STR_PLUSPLUS;
            ntype = OPERATOR_POSTINC;
            if(C_SUB == expr_trim[expr_len - 1])
            {
                p = STR_MINMIN;
                ntype = OPERATOR_POSTDEC;
            }
            node->info = mcompiler->duplicate_string(p);
            node->op_type = ntype;

            /* now add the  variable to the tree... */
            node->left = new expression_tree(node, expwloc);
            expr_trim[expr_len - 2] = 0;    /* this is to cut down the two ++ or -- signs ... */
            build_expr_tree(expr_trim, node->left, the_method, orig_expr, cc, result, expwloc);
        }
        else if( C_PAR_OP == expr_trim[0] ) /* if this is enclosed in a paranthesis */
        {
            if(expr_len > 1 && C_PAR_CL == expr_trim[expr_len - 1])
            {
                expr_trim[expr_len-1]=0;        /* here we have removed the trailing parantheses */
                expr_trim ++;
                expr_trim = trim(expr_trim, mcompiler);
                if(strlen(expr_trim)==0)
                {
                    throw_error(E0012_SYNTAXERR, expr, NULL);
                }
                else
                {
                    build_expr_tree(expr_trim, node, the_method, orig_expr, cc, result, expwloc);
                }
            }
            else
            {
                throw_error(E0009_PARAMISM, expr_trim, NULL);
            }
        }
        else if(indexed_elem)    /* if this is something indexed */
        {   /* here we should grab from the end the first set of square parantheses and pass the stuff before it to the indexed, the stuff in it to the index...*/
            std::vector<std::string> entries = string_list_create_bsep(index, C_COMMA, mcompiler);
            std::vector<std::string>::iterator q = entries.begin();
            std::vector<expression_tree*>* index_list = new std::vector<expression_tree*>();
            multi_dimension_index* dim = new_multi_dimension_index(expr_trim, mcompiler);

            node->info = mcompiler->duplicate_string(STR_IDXID);
            node->left = new expression_tree(node, expwloc);
            node->right = new expression_tree(node, expwloc);
            build_expr_tree(indexed_elem, node->left, the_method, orig_expr, cc, result, expwloc);    /* to discover the indexed element */

            /* and now identify the indexes and work on them*/

            int indx_cnt = 0;
            while(q != entries.end())
            {
                expression_tree* cur_indx = new expression_tree(expwloc);
                build_expr_tree(q->c_str(), cur_indx, the_method, orig_expr, cc, result, expwloc);
                index_list->push_back(cur_indx);
                q ++;
                indx_cnt ++;
            }
            dim->dimension_values = index_list;
            node->right->reference = new_envelope(dim, MULTI_DIM_INDEX, cc->compiler()); /*((variable*)node->father->left->reference->to_interpret)->mult_dim_def*/
            node->right->info = mcompiler->duplicate_string(expr_trim);
            node->op_type = MULTI_DIM_INDEX;
        }
        else
        {
            /* here determine what can be this
                here we are supposed to add only  variables/attributes, or the post increment stuff */
            envelope* envl = NULL;
            char* t = mcompiler->duplicate_string(expr_trim);
            int tlen = strlen(t);
            int templated = 0;
            int env_var = 0;

            variable* var = 0;
            if(the_method)
            {
                /* is t a variable from the method we are running in?
                   (or from the call context ofthe method if any )*/
                var = the_method->has_variable(cc, t, &templated, &env_var);
            }

            if(env_var)
            {
                node->op_type = ENVIRONMENT_VARIABLE;
                envl = new_envelope(t, ENVIRONMENT_VARIABLE, cc->compiler());
            }

            if(!var)
            {
                /* is this a variable from the current call context? */
                std::vector<variable*>::const_iterator varit = cc->variable_list_has_variable(t, cc->get_variables());
                if(varit != cc->get_variables().end())
                {
                    var = *varit;
                }
            }

            if(var)    /* if this is a variable */
            {
                // TODO: Check if this is a class variable
                envl = new_envelope(var, BASIC_TYPE_VARIABLE, cc->compiler());
                node->op_type = BASIC_TYPE_VARIABLE;
            }



            if(node->info == expr)
            {
                {
                    throw_error(E0012_SYNTAXERR, expr, NULL);
                }
            }
            node->info=mcompiler->duplicate_string(expr_trim);
            while(tlen > 0 && !isalnum( t[tlen - 1]) && t[tlen -1] != '\"' && t[tlen - 1] != '(' && t[tlen - 1] != ')' )
            {
                t[tlen - 1] = 0 ;
                t = trim(t, mcompiler);
                tlen = strlen(t);
            }

            if(strlen(t) == 0)
            {
                throw_error(E0012_SYNTAXERR, orig_expr, NULL);
            }

            if(isnumber(t))
            {
                number* nr = new number(t, cc->compiler());
                garbage_bin<number*>::instance().place(nr, cc->compiler());
                envl = new_envelope(nr, nr->type(), cc->compiler());
                node->op_type = nr->type();
            }
            else
            if(!strcmp(t, "true"))
            {
                envl = new_envelope(0, KEYWORD_TRUE, cc->compiler());
                node->op_type = KEYWORD_TRUE;
                *result = KEYWORD_TRUE;
            }
            else
            if(!strcmp(t, "false"))
            {
                envl = new_envelope(0, KEYWORD_FALSE, cc->compiler());
                node->op_type = KEYWORD_FALSE;
                *result = KEYWORD_FALSE;
            }
            else
            {
                /* here maybe we should check for cases like: a[10]++ */
            }

            if(strstr(t, "class") == t) /* class definition */
            {
                char* cname = t + 5;
                while(isspace(*cname)) cname ++;
                char* tcname = mcompiler->duplicate_string(cname);
                char* the_class_name = tcname;
                while(is_identifier_char(*cname))
                {
                    tcname ++;
                    cname ++;
                }
                *tcname = 0;
                class_declaration* cd = new class_declaration(mcompiler, the_class_name, cc);

                envl = new_envelope(cd, CLASS_DECLARATION, cc->compiler());
                node->op_type = CLASS_DECLARATION;
                *result = CLASS_DECLARATION;
            }

            if(!strcmp(expr, "asm"))
            {
                *result = ASM_BLOCK;
                envl = new_envelope(0, ASM_BLOCK, cc->compiler());
                node->op_type = ASM_BLOCK;
            }

            if(!envl)
            {
                build_expr_tree(t, node, the_method, orig_expr, cc, result, expwloc);
            }
            node->reference = envl;
        }

    return NULL;
}
