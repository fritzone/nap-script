#include "interpreter.h"
#include "parameter.h"
#include "utils.h"
#include "consts.h"
#include "number.h"
#include "envelope.h"
#include "bt_string.h"
#include "type.h"
#include "opr_hndl.h"
#include "variable.h"
#include "method.h"
#include "sys_brkp.h"
#include "res_wrds.h"
#include "expression_tree.h"
#include "compiler.h"

// from VM
#include "nbci_impl.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

interpreter::interpreter(nap_compiler* _compiler) : mcompiler(_compiler)
{
}

interpreter::~interpreter()
{
}

std::vector<envelope*>* interpreter::listv_prepare_list(const std::string& src,
                                                        method* the_method,
                                                        const char* orig_expr,
                                                        call_context* cc,
                                                        int* result,
                                                        expression_with_location* expwloc, bool& psuccess)
{
    psuccess = true;
    int l = src.length();
    std::vector<envelope*>*  lst = new std::vector<envelope*>();
    int i=0;

    // to skip the whitespace in front of the { (in case there's any)
    skip_whitespace(src, l, &i);
    if(src[i] != '{')
    {
        mcompiler->throw_error(E0012_SYNTAXERR, "String declaration not starting wiht '{'");
        psuccess = false;
        return 0;
    }

    i++;        // with this i points to the first character after the {
    while(i < l)
    {
        // can be whitespace
        skip_whitespace(src, l, &i);
        // read the next list element
        bool read_next=false;
        std::string tmp;
        while(!read_next && i < l)
        {
            tmp += src[i++];
            if(src[i] == C_PAR_OP || src[i] == C_SQPAR_OP)    // read if something was in parenthesis, such as function call, etc
            {
                char x = src[i], xcl = other_par(src[i]);
                int level = 1;
                bool readt = false;
                while(!readt && i < l)                        // match the parenthesis
                {
                    tmp += src[i++];
                    if(src[i] == x) level ++;
                    if(src[i] == xcl)
                    {
                        level --;
                        if(level == 0) readt = true;
                    }
                }
                tmp += src[i++];
                if(i==l)
                {
                    mcompiler->throw_error(E0012_SYNTAXERR);
                    psuccess = false;
                    return 0;
                }
            }
            // here we have finished reading the stuff that was in parenthesis, continue till we find the ',' separator or ..
            if(src[i] == ',')
            {
                tmp = tmp.substr(0, tmp.length() - 1); // cause we added the ',' too :(
                expression_tree* new_expression = new expression_tree(expwloc);
                build_expr_tree(tmp, new_expression, the_method, orig_expr, cc, result, expwloc, psuccess);
                SUCCES_OR_RETURN 0;

                envelope* expr_holder = new_envelope(new_expression, LIST_ELEMENT, cc->compiler);
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
                        psuccess = false;
                        cc->compiler->throw_error("Dot followed by dot?");
                        return 0;
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
int interpreter::get_operator(const std::string& expr, const char** foundOperator, int* ntype, bool& psuccess)
{
    int zladd = level_0_add_operator(mcompiler, expr, psuccess);                    /* the position of a +- sign on the first level */
    SUCCES_OR_RETURN -1;

    int zlbit = level_0_bitwise_operator(mcompiler, expr, psuccess);                /* if any bitwise (&|~_ operators can be found on level 0 */
    SUCCES_OR_RETURN -1;

    int zllogic = level_0_logical_operator(mcompiler, expr, psuccess);            /* && or || or ! on the zeroth level */
    SUCCES_OR_RETURN -1;

    int zlmlt=level_0_multiply_operator(mcompiler, expr, psuccess);                /* the position of a * / sign on the first level */
    SUCCES_OR_RETURN -1;

    int zlev_assignment = level_0_assignment_operator(mcompiler, expr, psuccess);/* the position of an equal operator on the first level */
    SUCCES_OR_RETURN -1;

    int zlev_dot = level_0_dot_operator(mcompiler, expr, psuccess);              /* the position of a dot operator on the first level */
    SUCCES_OR_RETURN -1;

    int zlev_shift = level_0_shift(mcompiler, expr, psuccess);                    /* Shift operator << >> */
    SUCCES_OR_RETURN -1;

    const char* found_comp_operator = NULL;                        /* the comparison operator that was found */
    int zlev_comparison = level_0_comparison_operator(mcompiler, expr, &found_comp_operator, psuccess);
    SUCCES_OR_RETURN -1;

    int sgeq_type = -1;                                        /* the type of the sg_eq operator*/
    const char* found_sq_eq_operator = NULL;                        /* the found sg_eq operator*/
    int zlev_sg_eq_operator = level_0_sg_eq_operator(mcompiler, expr, &found_sq_eq_operator, &sgeq_type, psuccess);
    SUCCES_OR_RETURN -1;

    int zlop = -1;

    /* this is not even an operator, it is an expression, but let's treat 
     * it as an operator, and all the other found operators will simply
     * override it */
    if(zlev_dot != -1)
    {
        *ntype = OPERATOR_DOT;
        *foundOperator = STR_DOT;
        zlop = zlev_dot;
    }

    /* lowest priority between the numeric operators */
    if(zlev_shift != -1)
    {
        zlop = zlev_shift;
        if(expr[zlop] == C_LT)
        {
            *ntype = OPERATOR_SHIFT_LEFT;
            *foundOperator = STR_SHLEFT;
        }
        else if(expr[zlop] == C_GT)
        {
            *ntype = OPERATOR_SHIFT_RIGHT;
            *foundOperator = STR_SHRIGHT;
        }
    }

    if(zlbit != -1)
    {
        zlop = zlbit;
        switch(expr[zlop])
        {
        case '&':
            *ntype = OPERATOR_BITWISE_AND;
            *foundOperator = STR_BIT_AND;
            break;
        case '|':
            *ntype = OPERATOR_BITWISE_OR;
            *foundOperator = STR_BIT_OR;
            break;
        case '^':
            *ntype = OPERATOR_BITWISE_XOR;
            *foundOperator = STR_BIT_XOR;
            break;
        case '~':
            *ntype = OPERATOR_BITWISE_COMP;
            *foundOperator = STR_BIT_COMP;
            break;
        }
    }

    /* next priority */
    if(zlmlt != -1)
    {
        zlop = zlmlt;

        if(C_MUL == expr[zlop])
        {
            *ntype = OPERATOR_MULTIPLY;
            *foundOperator = STR_MUL;
        }
        else if(C_DIV == expr[zlop])
        {
            *ntype = OPERATOR_DIVIDE;
            *foundOperator = STR_DIV;
        }
        else if(C_MOD == expr[zlop])
        {
            *ntype = OPERATOR_MODULO;
            *foundOperator = STR_MOD;
        }
    }

    if(zladd != -1)
    {
        zlop = zladd;
        if(C_ADD == expr[zlop])
        {
            *ntype = OPERATOR_ADD;
            *foundOperator = STR_PLUS;
        }
        else if(C_SUB == expr[zlop])
        {
            *ntype = OPERATOR_MINUS;
            *foundOperator = STR_MINUS;
        }
    }

    /* check the comparison operator presence */
    if(zlev_comparison != -1)
    {
        if((*ntype != OPERATOR_SHIFT_LEFT && *ntype != OPERATOR_SHIFT_RIGHT)
                || ((zlev_comparison != zlev_shift && zlev_comparison != zlev_shift - 1 && zlev_comparison != zlev_shift + 1)))
        {
            zlop = zlev_comparison;
            *foundOperator = found_comp_operator;
            *ntype = get_comp_typeid(*foundOperator);
        }
    }

    if(zllogic != -1)
    {
        switch(expr[zllogic])
        {
        case '&':
            *ntype = OPERATOR_BITWISE_AND;
            *foundOperator = STR_LOGIC_AND;
            zlop = zllogic;
            break;
        case '|':
            *ntype = OPERATOR_BITWISE_OR;
            *foundOperator = STR_LOGIC_OR;
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
                    *foundOperator = STR_LOGIC_NOT;
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
                *foundOperator = STR_EQUAL;
                *ntype = OPERATOR_ASSIGN;
            }
        }
        else    /* if zlev_assignment is == 0 it's not valid anyway, but for this case it does not matter */
        {
            zlop = zlev_assignment;
            *foundOperator = STR_EQUAL;
            *ntype = OPERATOR_ASSIGN;
        }
    }

    /* evaluate the +=, -= possibilities*/
    if(zlev_sg_eq_operator != -1)
    {
        zlop = zlev_sg_eq_operator;
        *foundOperator = found_sq_eq_operator;
        *ntype = sgeq_type;
    }

    return zlop;
}

/*
 * this checks if an expression is a function or not (ex: sin(x) is s function)
 */
method* interpreter::is_function_call(const std::string& s,  call_context* cc, int* special)
{
    unsigned int i=0;
    *special = 0;
    std::string method_name;
    while( i<s.length() && s[i] != C_PAR_OP && !is_whitespace(s[i]))
    {
        method_name += s[i++];
    }

    // now if whitespace, find the opening parenthesis
    while(i<s.length() && is_whitespace(s[i]) && s[i] != C_PAR_OP )
    {
        i++;
    }

    if(s[i] != C_PAR_OP) // named (and lot of whitespace) not followed by parenthesis
    {
        return 0;
    }

    if(method_name == "nap_execute")
    {
#if RUNTIME_COMPILATION
        *special = METHOD_CALL_SPECIAL_EXECUTE;
#else
        cc->compiler->throw_error("Cannot compile this code since runtime compilation was disabled");
#endif
        return 0;
    }
    return cc->get_method(method_name);
}

/**
 * Returns the type if this expression looks like a variable definition. Return NULL if not.
 * Rule to use to define a function:
 * <return_type> <function_name,[function_parameters])
 * Where return type can be any type defined or <type func_name ( pars ) > meaning this method returns a method
 */
int interpreter::looks_like_function_def(const std::string& expr, int expr_len, const expression_tree* node, call_context* cc, bool& psuccess)
{
    if(node && node->father && node->father->op_type == OPERATOR_ASSIGN)
    {
        return 0;    /* this is part of a templated variable definition */
    }

    if(expr[expr_len - 1] != C_PAR_CL)
    {
        return 0; /* not ending with ), return false*/
    }

    int i=expr_len - 2; /* the first one: to skip the paranthesys, the second one is the last character inside the parantheses*/
    std::string tmp;    /* will hold the parameters in the first run*/
    int can_stop = 0;
    int level = 1;
    while(i && !can_stop)            /* this is reading backwards */
    {
        if(expr[i] == C_PAR_CL) level ++;
        if(expr[i] == C_PAR_OP) level --;
        if(level == 0) can_stop = 1;
        if(!can_stop)
        {
            tmp += expr[i--];
        }
    }
    std::reverse(tmp.begin(), tmp.end());

    if(i == 0) return 0; /* this means, we've got to the first character, there is nothing before this */

    i--;        /* skip the parantheses*/

    while(i && is_whitespace(expr[i])) i --;    /* skip the whitespaces */

    if(!i) return 0;                    /* this was something in paranthese starting with spaces */

    if(!is_identifier_char(expr[i])) return 0;    /* cannot be anything else but an identifier */

    while(i>-1 && is_identifier_char(expr[i]))    i--;    /* fetch the name */

    if(i == -1) /* meaning, either we have defined a function with no return type or this is a function call */
    {   /* we need to analyze the parameters, if they look like definition, then it's fine, give back 1 */
        std::vector<std::string> pars = string_list_create_bsep(tmp, ',', mcompiler, psuccess);
        SUCCES_OR_RETURN -1;

        std::vector<std::string>::iterator q = pars.begin();
        while(q != pars.end())
        {
            size_t j=0;    /* will go through the parameters value and see if the first word from it is a type or not*/
            std::string firstw;
            while(j < q->length() && (*q)[j] != ' ')
            {
                firstw += (*q)[j++];
            } /* the phrase: if(int x = test() == 4) kills this ... */

            if(get_typeid(firstw) != BASIC_TYPE_DONTCARE)
            {
                /* check if the starting of the expr is not if or while or for ... */
                if(strstr(expr.c_str(), STR_IF) == expr.c_str()) return 0;
                if(strstr(expr.c_str(), STR_WHILE) == expr.c_str()) return 0;
                if(strstr(expr.c_str(), STR_FOR) == expr.c_str()) return 0;
                return 1;    /* function with no return type */
            }
            else
            {
                /* TODO: check if this is a constructor definition */
                if(strstr(expr.c_str(), cc->name.c_str()) == expr.c_str())
                {
                    // this might be a constructor definition
                    const char* pfinder = expr.c_str() + cc->name.length();
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
        std::string ret_type = "";
        /* fetching the return type of the function */
        while(i>-1 && is_identifier_char(expr[i]))
        {
            ret_type += expr[i];
            i--;
        }
        std::reverse(ret_type.begin(), ret_type.end());
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

            tmp = "";
            while(i > -1 && is_identifier_char(expr[i]))
            {
                tmp += expr[i--];
            }

            std::reverse(tmp.begin(), tmp.end());
            if(tmp == STR_EXTERN || tmp == STR_USE)
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

}

bool interpreter::is_list_value(const std::string& what)
{
    int i=0;
    skip_whitespace(what, what.length(), &i);
    if(what[i] ==  C_BRACKET_OP)
    {
        return true;
    }
    return false;
}

/**
 * Returns the type if this expression looks like a variable definition. Return NULL if not.
 * Also checks if the variable defined is a class instance. (ie: TestClass a;)
 */
std::string interpreter::looks_like_var_def(const call_context* cc, const std::string& expr, int expr_len)
{
    std::string first_word;

    int tc = 0;
    /* try to determine whether this is a variable definition or not */
    while(tc < expr_len && is_identifier_char(expr[tc]))
    {
        first_word += expr[tc++];
    }
    strim(first_word);

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
    return "";
}

/**
 * Defines a method
 */
method* interpreter::define_method(const std::string& expr, int expr_len,
                                   expression_tree* node, call_context* cc,
                                   expression_with_location* expwloc, bool& psuccess)
{
    int i=expr_len - 2; /* the first one: to skip the paranthesys, the second one is the last character inside the parantheses*/
    method* created_method = NULL;
    std::string parameters;    /* will hold the parameters */
    std::string func_name;        /* will hold the name of the function .. maybe optimize it a bit*/
    std::string ret_type;        /* will hold the return type definition */
    int can_stop = 0;
    int level = 1;
    while(i && !can_stop)                    /* this reads backwards */
    {
        if(expr[i] == C_PAR_CL) level ++;
        if(expr[i] == C_PAR_OP) level --;
        if(level == 0) can_stop = 1;
        if(!can_stop) parameters += expr[i--];
    }
    if(i == 0)
    {
        mcompiler->throw_error(E0010_INVFNCDF, expr, NULL);
        psuccess = false;
        return 0;
    }
    std::reverse(parameters.begin(), parameters.end());
    i --;
    while(is_whitespace(expr[i])) i--;
    while(i>-1 && is_identifier_char(expr[i]))
    {
        func_name += expr[i--];
    }
    std::reverse(func_name.begin(), func_name.end());
    i--;
    while(i > -1)
    {
        ret_type += expr[i--];
    }
    std::reverse(ret_type.begin(), ret_type.end());
    //printf("Defining function ret_type:[%s] name:[%s] pars:[%s]\n", ret_type, func_name, parameters);
    strim(func_name);
    strim(ret_type);
    strim(parameters);
    if(func_name.empty())
    {
        mcompiler->throw_error(E0010_INVFNCDF, expr, NULL);
        psuccess = false;
        return 0;
    }
    created_method = new method(mcompiler, func_name, ret_type, cc);
    created_method->feed_parameter_list(parameters.c_str(), expwloc, psuccess);
    SUCCES_OR_RETURN 0;

    created_method->ret_type = (uint8_t)get_typeid(created_method->return_type);
    cc->add_method(created_method);

    node->op_type = FUNCTION_DEFINITION;
    node->reference = new_envelope(created_method, FUNCTION_DEFINITION, cc->compiler);
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
    return is_valid_variable_name(name.c_str());
}

/**
 * Defines the variables that can be found below ...
 */
std::vector<variable_definition*>* interpreter::define_variables(const std::string& var_def_type,
                                                                 const std::string& expr_trim,
                                                                 expression_tree* node,
                                                                 method* the_method,
                                                                 call_context* cc,
                                                                 const char* orig_expr,
                                                                 int* result,
                                                                 expression_with_location* expwloc, bool& psuccess)
{
    psuccess = true;
    std::string copied = expr_trim.substr(var_def_type.length());
    std::vector<std::string> var_names = string_list_create_bsep(copied, ',', mcompiler, psuccess);
    SUCCES_OR_RETURN 0;

    std::vector<std::string>::iterator q = var_names.begin();
    std::vector<variable_definition*>* var_def_list = new std::vector<variable_definition*>(); /* will contain the variable definitions if any*/

    garbage_bin<std::vector<variable_definition*>*>::instance(mcompiler).place(var_def_list, cc->compiler);

    while(q != var_names.end())
    {
        multi_dimension_def* mdd = NULL, *qm;    /* will contain the dimension definitions if any */
        std::string name = *q;
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
                mcompiler->throw_error(E0011_IDDEFERR, name.c_str(), NULL);
                psuccess = false;
                return 0;
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
            std::vector<std::string> dimensions = string_list_create_bsep(index, C_COMMA, mcompiler, psuccess);
            SUCCES_OR_RETURN 0;

            std::vector<std::string>::iterator qDimensionStrings = dimensions.begin();    /* to walk through the dimensions */
            int countedDimensions = 0;
            mdd = alloc_mem(multi_dimension_def,1, cc->compiler);
            qm = mdd;
            while(qDimensionStrings != dimensions.end())
            {
                expression_tree* dim_def_node = expwloc->new_expression();

                if(qDimensionStrings->length() > 0)
                {
                    build_expr_tree((*qDimensionStrings).c_str(), dim_def_node, the_method, orig_expr, cc, result, expwloc, psuccess);
                    SUCCES_OR_RETURN 0;
                }
                else
                {
                    /* check if there are multiple dimensions for this variable. If yes disallow the dynamic dimensions for now */
                    if(countedDimensions > 0)    /* awkward but correct */
                    {
                        mcompiler->throw_error(E0038_DYNDIMNALL, expr_trim);
                        psuccess = false;
                        return 0;
                    }
                    qm->dynamic = true;
                }
                qm->dimension = -1;
                qm->expr_def = dim_def_node;
                qm->next = alloc_mem(multi_dimension_def,1, cc->compiler);
                qm = qm->next;
                countedDimensions ++;
                qDimensionStrings ++;
            }
        }

        /* check whether we have direct initialization */
        int eqp = var_declaration_followed_by_initialization(name);
        std::string deflist = "";                /* the definition list for this variable */
        if(eqp)
        {
            deflist = name.substr(eqp + 1);
            name = name.substr(0, eqp);
            strim(name);
            strim(deflist);
        }

        if(idx_def_start)
        {
            std::string tmpname = strrchr(name.c_str(), C_SQPAR_CL) + 1;
            /* TODO: ez meghal: int x=a[1]; re */
            strim(tmpname);
            if(tmpname.empty())    /* in this case the index definition was after the name: int name[12];*/
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
                name = tmpname;
            }
        }

        if(!accepted_variable_name(name))
        {
            mcompiler->throw_error(E0037_INV_IDENTF, name);
            psuccess = false;
            return 0;
        }

        if(cc)
        {
            added_var = cc->add_variable(name, var_def_type, 1, psuccess);
            SUCCES_OR_RETURN 0;
        }

        if(!added_var)
        {
            mcompiler->throw_error("Internal: a variable cannot be defined", NULL);
            psuccess = false;
            return 0;
        }

        /* create the variable definition/declaration structure for both of them */
        var_def = alloc_mem(variable_definition,1, cc->compiler);
        var_def->the_variable = added_var;
        var_def->md_def = mdd;

        if(!deflist.empty())    /* whether we have a definition for this variable. if yes, we need to populate a definition_list */
        {
            expression_tree* var_def_node = expwloc->new_expression();
            build_expr_tree(deflist.c_str(), var_def_node, the_method, orig_expr, cc, result, expwloc, psuccess);
            SUCCES_OR_RETURN 0;

            var_def->the_value = var_def_node;
        }

        var_def_list->push_back(var_def);

        q ++;
    }

    node->op_type = NT_VARIABLE_DEF_LST;
    node->info = expr_trim;
    node->reference = new_envelope(var_def_list, NT_VARIABLE_DEF_LST, cc->compiler);
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
call_frame_entry* interpreter::handle_function_call(const std::string& expr_trim, expression_tree* node,
        method* func_call, method* the_method,
        const char* orig_expr, call_context* cc, int* result,
        expression_with_location* expwloc, int type_of_call, bool& psuccess)
{
    std::string params_body (expr_trim.substr(func_call->method_name.length() + 1));
    std::vector<std::string> parameters;
    std::vector<parameter*> pars_list;
    call_frame_entry* cfe = NULL;

    strim(params_body);
    if(params_body[0] == C_PAR_OP)
    {
        params_body = params_body.substr(1);        /* now we've skipped the opening paranthesis */
        strim(params_body);
    }

    int pb_end = params_body.length();
    while(is_whitespace(params_body[pb_end]))
    {
        pb_end --;
    }

    if(params_body[pb_end - 1] == C_PAR_CL)
    {
        params_body = params_body.substr(0, pb_end - 1);
        strim(params_body);
    }    /* this removed the closing paranthesis */


    //printf("Checking function call:[%s]\n", params_body);
    /* Now: To build the parameter list, and create a call_frame_entry element to insert into the tree */
    parameters = string_list_create_bsep(params_body, ',', mcompiler, psuccess);
    SUCCES_OR_RETURN 0;

    std::vector<std::string>::iterator q = parameters.begin();
    while(q != parameters.end())
    {
        expression_tree* cur_par_expr = NULL;
        if(q->length() > 0)
        {
            cur_par_expr = expwloc->new_expression();

            build_expr_tree((*q).c_str(), cur_par_expr, the_method, orig_expr, cc, result, expwloc, psuccess);
            SUCCES_OR_RETURN 0;

            // this parameter will be delete in the call_frame_entry's destructor
            parameter* cur_par_obj = new parameter(the_method, "", -1);

            cur_par_obj->expr = cur_par_expr;

            pars_list.push_back(cur_par_obj);
        }

        q ++;
    }
    cfe = new call_frame_entry();
    garbage_bin<call_frame_entry*>::instance(mcompiler).place(cfe, mcompiler);

    cfe->the_method = func_call;
    cfe->parameters = pars_list;

    node->info = func_call->method_name;
    node->op_type = FUNCTION_CALL + type_of_call; // TODO: This is tricky and ugly
    node->reference = new_envelope(cfe, FUNCTION_CALL + type_of_call, cc->compiler);
    cfe->the_method = func_call; //TODO: why was this: cc->get_method(func_call->method_name);
    return cfe;
}

/**
 * This method check whether the expression passed in is some sort of indexed
 * access. In case it is, returns the element that is indexed. The value of the
 * index is populated with the index
 */
std::string interpreter::is_indexed(const std::string& expr_trim, int expr_len, std::string& index)
{
    const char* p = expr_trim.c_str();
    char* the_indexed_part = (char*)calloc(expr_len, 1);
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
            int can_stop = 0;
            int level_idx = 1;
            while(!can_stop)
            {
                if(*p == C_SQPAR_OP) level_idx ++;
                if(*p == C_SQPAR_CL && --level_idx == 0) can_stop = 1;
                if(!can_stop) index += *p ++;
            }
            p++;
            if(*p)
            {
                while(is_whitespace(*p)) p++;
                if(!*p)
                {
                    std::string r = the_indexed_part;
                    free(the_indexed_part);
                    return r;    /* we're at the end, found something before the indexes that can be returned*/
                }
                if(*p == C_SQPAR_OP)                /* double index: string vector's xth. element yth character*/
                {
                    strcat(the_indexed_part, "[");
                    strcat(the_indexed_part, index.c_str());    /* update the first part*/
                    strcat(the_indexed_part, "]");
                    int canstop2 = 0;
                    int level2 = 1;
                    p++;                            /* skip the second opening square bracket*/
                    index = "";


                    while(!canstop2)
                    {
                        if(*p == C_SQPAR_OP) level2 ++;
                        if(*p == C_SQPAR_CL && --level2 == 0) canstop2 = 1;
                        if(!canstop2) index += *p ++;
                    }
                    p++;    /* now idx contains the new index and p points to the first character after the second index*/
                    if(*p)
                    {
                        while (is_whitespace(*p)) p++;
                        if(!*p)
                        {
                            std::string r = the_indexed_part;
                            free(the_indexed_part);
                            return r;
                        }
                        free(the_indexed_part);
                        return "";
                    }
                    else
                    {
                        std::string r = the_indexed_part;
                        free(the_indexed_part);
                        return r;
                    }
                }
                else
                {
                    free(the_indexed_part);
                    return "";    /* here: check if we have a vector of funtions and trying to call one of them */
                }
            }
            else
            {
                std::string r = the_indexed_part;
                free(the_indexed_part);
                return r;
            }
        }
        else
        {
            *q ++ = *p ++;
        }
    }
    free(the_indexed_part);
    return "";
}

/**
 * Checks if the expression passed in some statement that starts with a reserved word.
 * The return value is the part after the keywords
 */
std::string interpreter::is_some_statement(const std::string& expr_trim, const std::string& keyword)
{
    if( starts_with(expr_trim, keyword) )
    {
        std::string retv = expr_trim.substr(keyword.length());
        strim(retv);
        return retv;
    }
    return "";
}

/**
 * Returns the necessary structure for the break/continue statements
 */
void* interpreter::deal_with_one_word_keyword( call_context* cc, expression_tree* node, int* &result, const char* keyw, int statement, bool& psuccess )
{
    if(cc->type != call_context::CC_WHILE && cc->type != call_context::CC_FOR)
    {
        int in_iterative_cc = 0;
        call_context* tmpcc = cc->father;
        while(tmpcc)
        {
            if(tmpcc->type == call_context::CC_WHILE || tmpcc->type == call_context::CC_FOR)
            {
                in_iterative_cc = 1;
                break;
            }
            tmpcc = tmpcc->father;
        }
        if(!in_iterative_cc)
        {
            mcompiler->throw_error(E0036_NOBREAK);
            psuccess = false;
            return 0;
        }
    }
    node->info =keyw;
    node->reference = new_envelope(cc, ENV_TYPE_CC, cc->compiler);
    node->op_type = statement;
    *result = statement;
    return node->reference;
}

/**
 * This method deals with preparing structures for the keywords if/while since these are handled quite similarly this phase
 */
void* interpreter::deal_with_conditional_keywords(const std::string& keyword_if,
                                                  const std::string& keyword_while,
                                                  expression_tree* node,
                                                  expression_with_location* expwloc,
                                                  const std::string& expr_trim,
                                                  method* the_method,
                                                  const char* orig_expr,
                                                  call_context* cc,
                                                  int* &result,
                                                  bool& psuccess)
{
    int one_line_stmt = -1;
    int stmt = -1;
    const char* keyw = NULL;
    /* Check if this is an IF or a WHILE statement, since these two are handled the same way mostly */
    if(!keyword_if.empty() || !keyword_while.empty())
    {
        if(!keyword_if.empty())
        {
            one_line_stmt = STATEMENT_IF_1L;
            stmt = STATEMENT_IF;
            keyw = STR_IF;
        }
        else
        if(!keyword_while.empty())
        {
            one_line_stmt = STATEMENT_WHILE_1L;
            stmt = STATEMENT_WHILE;
            keyw = STR_WHILE;
        }
    }

    if(keyw)
    {
        node->info = keyw;
        expression_tree* expt = expwloc->new_expression();

        /* here fetch the part which is in the parentheses after the keyword and build the tree based on that*/
        std::string condition =  expr_trim.substr(strlen(keyw));
        strim(condition);

        if(condition[0] != C_PAR_OP)                        /* next char should be '(' */
        {
            mcompiler->throw_error(E0012_SYNTAXERR, NULL);
            psuccess = false;
            return 0;
        }
        const char* p = condition.c_str();
        p++;  /* skip the parenthesis */

        std::string m_cond = extract_next_enclosed_phrase(p, C_PAR_OP, C_PAR_CL);
        // I will remove the m_cond from condition plus two parentheses to see
        // if there is anything after
        condition = condition.substr(1); // the opening parentheses
        condition = condition.substr(m_cond.length()); // the condition itself
        condition = condition.substr(1); // the closing parentheses

        strim(m_cond);
        strim(condition); // if there was a one line if this has the command in

        build_expr_tree(m_cond, expt, the_method, orig_expr, cc, result, expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        if(condition.length() > 1)    /* means: there is a one lined statement after the condition*/
        {
            node->info = condition;    /* somehow we must tell the external world what's the next expression */
            node->reference = new_envelope(expt, one_line_stmt, cc->compiler);
            *result = one_line_stmt;
            node->op_type = one_line_stmt;
        }
        else
        {
            node->reference = new_envelope(expt, stmt, cc->compiler);
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
void* interpreter::build_expr_tree(const std::string& expr, expression_tree* node,
                                   method* the_method, const char* orig_expr,
                                   call_context* cc, int* result,
                                   expression_with_location* expwloc,
                                   bool& psuccess)
{
    psuccess = true;
    mcompiler->set_location(expwloc);

    /* some variables that will be used at a later stage too */
    std::string expr_trim(expr);
    strim(expr_trim);
    int expr_len = expr_trim.length();

    const char* foundOperator;    /* the operator which will be identified. first byte: operator, second byte: 0*/
    int zlop;                    /* the index of the identified zero level operation */
    std::string var_def_type = looks_like_var_def(cc, expr_trim, expr_len);
    int func_def = looks_like_function_def(expr_trim, expr_len, node, cc, psuccess);
    SUCCES_OR_RETURN 0;

    method* func_call = NULL; /* if this entry is a function call or or not ... */
    std::string index;
    std::string indexed_elem = strchr(expr_trim.c_str(), C_SQPAR_CL)
            && strchr(expr_trim.c_str(), C_SQPAR_OP) ? is_indexed(expr_trim, expr_len, index): "";

    std::string keyword_return = is_some_statement(expr_trim, STR_RETURN);
    std::string  keyword_if = is_some_statement(expr_trim, STR_IF);
    std::string  keyword_while = is_some_statement(expr_trim, STR_WHILE);
    std::string  keyword_for = is_some_statement(expr_trim, STR_FOR);
    std::string  keyword_new = is_some_statement(expr_trim, STR_NEW);

    if(expr == STR_CLOSE_BLOCK) /* destroy the call context*/
    {
        node->op_type = STATEMENT_CLOSE_CC;
        return NULL;
    }

    if(expr == STR_OPEN_BLOCK)
    {
        node->op_type = STATEMENT_NEW_CC;
        return NULL;
    }

    if(is_list_value(expr))
    {
        std::vector<envelope*> *thlist = listv_prepare_list(expr, the_method, orig_expr, cc, result, expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        node->op_type = LIST_VALUE;
        node->reference = new_envelope(thlist, LIST_VALUE, cc->compiler);
        return 0;
    }


    /* first check: See if this expr is a string like expression, meaning in quotes. */
    if(is_string(expr_trim, expr_len))
    {
        expr_trim[expr_len-1] = 0;    // removing the double quotes
        expr_trim = expr_trim.substr(1);
        bt_string* the_str = new bt_string(expr_trim.c_str());
        garbage_bin<bt_string*>::instance(cc->compiler).place(the_str, cc->compiler);
        node->reference = new_envelope(the_str, BASIC_TYPE_STRING, cc->compiler);
        node->info = the_str->m_the_string;
        *result = RESULT_STRING;
        node->op_type = BASIC_TYPE_STRING;
        return the_str->m_the_string;
    }

    /* this is a 'statement' string, a string which is executed and the result is interpreted by the interpreter backticks: `` */
    if(is_statement_string(expr_trim, expr_len))
    {
        expr_trim[expr_len-1] = 0;    /* removing the double quotes */
        expr_trim = expr_trim.substr(1);
        bt_string* the_str = new bt_string(expr_trim.c_str());
        node->reference = new_envelope(the_str, BASIC_TYPE_STRING, cc->compiler);
        node->info = expr_trim;
        *result = BACKQUOTE_STRING;
        return the_str->m_the_string;
    }

    /* check if this is a function definition */
    if(func_def)
    {
        method* mth = define_method(expr_trim, expr_len, node, cc, expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        *result = FUNCTION_DEFINITION;
        return mth;
    }

    /* second check: variable definition? */
    if(!var_def_type.empty() && var_def_type != "new")
    {
        std::vector<variable_definition*>* vdl = define_variables(var_def_type, expr_trim,
                                                         node, the_method, cc,
                                                         orig_expr, result,
                                                         expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        *result = NT_VARIABLE_DEF_LST;
        return vdl;
    }

    /* now check if any keyword is used here, and deal with that */

    if(!keyword_new.empty())
    {
        node->op_type = STATEMENT_NEW;
        std::string constructor_name = keyword_new;
        if(constructor_name.find('(') != std::string::npos)
        {
            constructor_name = constructor_name.substr(0, constructor_name.find('(') );
        }
        class_declaration* cd = cc->get_class_declaration(constructor_name);
        if(!cd)
        {
            mcompiler->throw_error("Invalid constructor: ", constructor_name);
            psuccess = false;
            return 0;
        }
        method* called_constructor = cd->get_method(constructor_name);
        constructor_call* tmp = (constructor_call*)realloc(called_constructor, sizeof(constructor_call));
        if(tmp == NULL)
        {
            mcompiler->throw_error("Not enough memory");
            psuccess = false;
            return 0;
        }
        called_constructor = tmp;
        constructor_call* ccf = (constructor_call*)called_constructor;
        ccf->the_class = cd;
        call_frame_entry* cfe = handle_function_call(keyword_new,
                                                     node, ccf, the_method,
                                                     orig_expr, cd, result,
                                                     expwloc,
                                                     METHOD_CALL_CONSTRUCTOR,
                                                     psuccess);
        SUCCES_OR_RETURN 0;

        *result = STATEMENT_NEW;
        return cfe;
    }

    /* check if this is a 'return' statement */
    if(!keyword_return.empty())
    {
        node->info = STR_RETURN;
        expression_tree* expt = expwloc->new_expression();
        build_expr_tree(keyword_return, expt, the_method, orig_expr, cc, result, expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        node->reference = new_envelope(expt, RETURN_STATEMENT, cc->compiler);
        *result = RETURN_STATEMENT;
        node->op_type = RETURN_STATEMENT;
        return node->reference;
    }
    if(expr_trim == STR_BREAK)    /* the break keyword */
    {
        void *v = deal_with_one_word_keyword(cc, node, result, STR_BREAK, STATEMENT_BREAK, psuccess);
        SUCCES_OR_RETURN 0;

        return v;
    }

    if(expr_trim == STR_CONTINUE)    /* the continue keyword */
    {
        void* v = deal_with_one_word_keyword(cc, node, result, STR_CONTINUE, STATEMENT_CONTINUE, psuccess);
        SUCCES_OR_RETURN 0;

        return v;
    }

    /* if or while? */
    if(!keyword_if.empty() || !keyword_while.empty())
    {
        void* t = deal_with_conditional_keywords(keyword_if, keyword_while,
                                              node, expwloc, expr_trim,
                                              the_method, orig_expr, cc, result, psuccess);
        SUCCES_OR_RETURN 0;
        return t;
    }

    /* the for keyword? */
    if(!keyword_for.empty())
    {
        strim(keyword_for);
        int fors_len = keyword_for.length();
        if(keyword_for[0] != C_PAR_OP && keyword_for[fors_len - 1] != C_PAR_CL)
        {
            mcompiler->throw_error(E0012_SYNTAXERR);
            psuccess = false;
            return 0;
        }
        std::string for_par = "";
        int j = 1, level = 1;
        int done = 0;
        while(!done && j < fors_len)
        {
            if(keyword_for[j] == C_PAR_OP) level ++;
            if(keyword_for[j] == C_PAR_CL) level --;
            if(level == 0) done = 1;
            if(!done)
            {
                for_par += keyword_for[j++];
            }
        }
        strim(for_par);

        if(for_par.empty())
        {
            mcompiler->throw_error(E0012_SYNTAXERR);
            psuccess = false;
            return 0;
        }
        std::vector<std::string> content = string_list_create_bsep(for_par, C_SEMC, mcompiler, psuccess);
        SUCCES_OR_RETURN 0;

        // this might not be the case .... or
        if(content.empty())
        {
            mcompiler->throw_error(E0012_SYNTAXERR, for_par);
            psuccess = false;
            return 0;
        }
        std::string init_stmt = content[0];                /* the init statement */
        std::string cond_stmt = content.size() > 1?content[1]:"";                    /* the condition */
        std::string expr_stmt = content.size() == 3?content[2]:"";
        if(content.size() > 3)
        {
            mcompiler->throw_error(E0012_SYNTAXERR, for_par);
            psuccess = false;
            return 0;
        }
        resw_for* rswfor = new resw_for;
        garbage_bin<resw_for*>::instance(cc->compiler).place(rswfor, cc->compiler);

        rswfor->unique_hash = generate_unique_hash();
        rswfor->tree_init_stmt = expwloc->new_expression();

        build_expr_tree(init_stmt, rswfor->tree_init_stmt, the_method, orig_expr, cc, result, expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        rswfor->tree_condition = expwloc->new_expression();

        build_expr_tree(cond_stmt, rswfor->tree_condition, the_method, orig_expr, cc, result, expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        rswfor->tree_expr = expwloc->new_expression();

        build_expr_tree(expr_stmt, rswfor->tree_expr, the_method, orig_expr, cc, result, expwloc, psuccess);
        SUCCES_OR_RETURN 0;

        node->info = STR_FOR;

        std::string condition = expr_trim.substr(strlen(STR_FOR));    /* not actually condition, but the for's three statements: init, cond, expr*/
        strim(condition);

        if(condition[0] != C_PAR_OP)                        /* next char should be '(' */
        {
            mcompiler->throw_error(E0012_SYNTAXERR, NULL);
            psuccess = false;
            return 0;
        }
        const char* p = condition.c_str();                                /* skip the parenthesis */
        p++;

        std::string m_cond = extract_next_enclosed_phrase(p, C_PAR_OP, C_PAR_CL);
        strim(m_cond);

        // I will remove the m_cond from condition plus two parentheses to see
        // if there is anything after
        condition = condition.substr(1); // the opening parentheses
        condition = condition.substr(m_cond.length()); // the condition itself
        condition = condition.substr(1); // the closing parentheses
        strim(condition);

        if(condition.length() > 1)    /* means: there is a one lined statement after the condition*/
        {
            node->info = condition;    /* somehow we must tell the external world what's the next expression */ // this will go away?
            node->reference = new_envelope(rswfor, STATEMENT_FOR_1L, cc->compiler);
            *result = STATEMENT_FOR_1L;
            node->op_type = STATEMENT_FOR_1L;
        }
        else
        {
            node->reference = new_envelope(rswfor, STATEMENT_FOR, cc->compiler);
            *result = STATEMENT_FOR;
            node->op_type = STATEMENT_FOR;
        }
        return node->reference;
    }

    /* now: find the operators and just play with them */
    foundOperator = NULL;
    int ntype = NO_OPERATOR;                    /* the type number of the node, firstly let's assume the node contains nothing like an operator */
    zlop = get_operator(expr_trim, &foundOperator, &ntype, psuccess);    /* zlop will contain the index of the zeroth level operator */
    SUCCES_OR_RETURN 0;

    /* ok, here start checking what we have gathered till now */
    int isNumber = isnumber(expr_trim);

    // 1. if we have found an operator on the zero.th level,
    if(zlop != -1)
    {
        // 2. but see that it is not the "." opwrator
        // 3. and this expression is not a number */
        if( !(foundOperator[0] == '.' && isNumber) )
        {
            std::string beforer = std::string(expr_trim).substr(0, zlop);
            if(beforer.empty())
            {
                /* now we should check for the 'unary' +- operators */
                if(ntype == OPERATOR_ADD || ntype == OPERATOR_MINUS || ntype == OPERATOR_BITWISE_COMP || ntype == OPERATOR_NOT)
                {
                    /* check if this is a number or not ?*/
                    std::string afterer = expr_trim.substr(1);
                    afterer = strim(afterer);
                    if(!isnumber(afterer.c_str()))
                    {
                        node->info = foundOperator;
                        if(ntype == OPERATOR_ADD)
                        {
                            ntype = OPERATOR_UNARY_PLUS;
                        }
                        else if(ntype == OPERATOR_MINUS)
                        {
                            ntype = OPERATOR_UNARY_MINUS;
                        }
                        node->op_type = ntype;
                        node->left = expwloc->new_expression(node);

                        build_expr_tree(afterer.c_str(), node->left, the_method, orig_expr, cc, result, expwloc, psuccess);

                        return NULL;
                    }
                    else
                    {
                        zlop = -1;
                    }
                }
                else    /* right now we are not dealing with more unary operators */
                {
                    mcompiler->throw_error(E0012_SYNTAXERR, expr);
                    psuccess = false;
                    return 0;
                }
            }
            else
            {
                /* finding the part which is after the operator */
                int l = strlen(foundOperator) ;
                std::string afterer =  expr_trim.substr(zlop + (foundOperator ?  l : 0));
                strim(afterer);
                if(afterer.length() == 0)
                {
                    mcompiler->throw_error(E0012_SYNTAXERR, expr);
                    psuccess = false;
                    return 0;
                }
                node->info = foundOperator;
                node->op_type = ntype;
                node->left = expwloc->new_expression(node);
                node->right = expwloc->new_expression(node);

                /* the order here is important for the "." operator ... it needs to know the parent in order to identify the object to find its call_context*/
                build_expr_tree(beforer.c_str(), node->left, the_method, orig_expr, cc, result, expwloc, psuccess);
                SUCCES_OR_RETURN 0;

                build_expr_tree(afterer.c_str(), node->right, the_method, orig_expr, cc, result, expwloc, psuccess);
                SUCCES_OR_RETURN 0;

            }

            if(zlop != -1)
            {
                return NULL;
            }
        }
    }

    // no operator on the zeroth level

    /* Is this is a function call?*/
    int special = 0;
    func_call = is_function_call(expr_trim, cc, &special);
    if(func_call || special != 0)
    {
        if(func_call)
        {
            call_frame_entry* cfe = handle_function_call(expr_trim,
                                                         node,
                                                         func_call,
                                                         the_method,
                                                         orig_expr,
                                                         cc, result,
                                                         expwloc,
                                                         METHOD_CALL_NORMAL,
                                                         psuccess);
            SUCCES_OR_RETURN 0;

            *result = FUNCTION_CALL;
            return cfe;
        }
        else
        {
            if(special == METHOD_CALL_SPECIAL_EXECUTE)
            {
                //#if RUNTIME_COMPILATION
#if 1
                *result = FUNCTION_CALL_NAP_EXEC;

                node->op_type = FUNCTION_CALL_NAP_EXEC;

                // now parse out the string
                std::string temp =expr_trim.substr( strlen("nap_execute") );
                strim(temp);
                std::string temp2 = extract_next_enclosed_phrase(temp.c_str(), C_PAR_OP, C_PAR_CL);
                // and now remove the parentheses
                if(temp2.length() > 1)
                {
                    temp2 = temp2.substr(1);
                }

                temp2 = temp2.substr(0, temp2.length() - 1);
                node->info = temp2;
                node->left = expwloc->new_expression(node);

                build_expr_tree(temp2, node->left, the_method, orig_expr, cc, result, expwloc, psuccess);
                SUCCES_OR_RETURN 0;

                node->reference = new_envelope(0, FUNCTION_CALL_NAP_EXEC, cc->compiler);
#else
                cc->compiler->throw_error("Cannot compile this code since runtime compilation was disabled");
#endif
                return 0;
            }
        }
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
                        if(expr_trim == "len") // the length of the string
                        {
                            *result = FUNCTION_STRING_LEN;
                            node->op_type = *result;
                            return 0;
                        }
                    }
                    else
                    {
                        mcompiler->throw_error("Only class type variables can call methods on themselves", v->name);
                        psuccess = false;
                        return 0;
                    }
                }
                int type = 0;
                func_call = is_function_call(expr_trim, cd, &type);
                if(func_call || type != 0)
                {
                    if(func_call)
                    {
                        call_frame_entry* cfe = handle_function_call(expr_trim,
                                                                     node,
                                                                     func_call,
                                                                     the_method,
                                                                     orig_expr, cd,
                                                                     result,
                                                                     expwloc,
                                                                     METHOD_CALL_OF_OBJECT,
                                                                     psuccess);
                        SUCCES_OR_RETURN 0;

                        *result = FUNCTION_CALL;
                        return cfe;
                    }
                    else // we shouldn't get in here ... ever,
                    {

                    }
                }

                // now see if this is a class variable: a.b = 4
                std::vector<variable*>::const_iterator it = cd->has_variable(expr_trim);
                if(it != cd->variables.end())
                {
                    variable* var = *it;
                    *result = MEMBER_ACCESS_OF_OBJECT;
                    envelope* envl = new_envelope(var, BASIC_TYPE_VARIABLE, cc->compiler);
                    node->op_type = MEMBER_ACCESS_OF_OBJECT;
                    node->reference = envl;
                    return envl;
                }
            }
            else
                if(node->father->left->op_type == FUNCTION_CALL)    // to solve func().anotherOne()
                {
                    mcompiler->throw_error("func().anotherOne()");
                    psuccess = false;
                    return 0;
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
        node->info = p;
        node->op_type = ntype;
        node->left = expwloc->new_expression(node);

        std::string ep2(expr_trim.substr(2));
        build_expr_tree(ep2.c_str(), node->left, the_method, orig_expr, cc, result, expwloc, psuccess);
        SUCCES_OR_RETURN 0;
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
            node->info = p;
            node->op_type = ntype;

            /* now add the  variable to the tree... */
            node->left = expwloc->new_expression(node);

            //expr_trim[expr_len - 2] = 0;    /* this is to cut down the two ++ or -- signs ... */
            std::string ep2(expr_trim.substr(0, expr_len- 2));
            build_expr_tree(ep2.c_str(), node->left, the_method, orig_expr, cc, result, expwloc, psuccess);
            SUCCES_OR_RETURN 0;

        }
        else
        if( C_PAR_OP == expr_trim[0] ) /* if this is enclosed in a paranthesis */
        {
            if(expr_len > 1 && C_PAR_CL == expr_trim[expr_len - 1])
            {
                /*expr_trim[expr_len-1]=0;        // here we have removed the trailing parantheses
                expr_trim ++;
                expr_trim = trim(expr_trim, mcompiler);
                */
                expr_trim = expr_trim.substr(1, expr_len - 2);
                strim(expr_trim);

                if(expr_trim.empty())
                {
                    mcompiler->throw_error(E0012_SYNTAXERR, expr, NULL);
                    psuccess = false;
                    return 0;
                }
                else
                {
                    build_expr_tree(expr_trim.c_str(), node, the_method, orig_expr, cc, result, expwloc, psuccess);
                    SUCCES_OR_RETURN 0;
                }
            }
            else
            {
                mcompiler->throw_error(E0009_PARAMISM, expr_trim, NULL);
                psuccess = false;
                return 0;
            }
        }
        else
        if(!indexed_elem.empty())    /* if this is something indexed */
        {   /* here we should grab from the end the first set of square parantheses and pass the stuff before it to the indexed, the stuff in it to the index...*/
            std::vector<std::string> entries = string_list_create_bsep(index, C_COMMA, mcompiler, psuccess);
            SUCCES_OR_RETURN 0;

            std::vector<std::string>::iterator q = entries.begin();
            std::vector<expression_tree*>* index_list = new std::vector<expression_tree*>();
            garbage_bin< std::vector<expression_tree*>* >::instance(mcompiler).place(index_list, mcompiler);

            node->info = STR_IDXID;
            node->left = expwloc->new_expression(node);
            node->right = expwloc->new_expression(node);

            build_expr_tree(indexed_elem, node->left, the_method, orig_expr, cc, result, expwloc, psuccess);    /* to discover the indexed element */
            SUCCES_OR_RETURN 0;

            /* and now identify the indexes and work on them*/

            int indx_cnt = 0;
            while(q != entries.end())
            {
                expression_tree* cur_indx = expwloc->new_expression();

                build_expr_tree(q->c_str(), cur_indx, the_method, orig_expr, cc, result, expwloc, psuccess);
                SUCCES_OR_RETURN 0;

                index_list->push_back(cur_indx);
                q ++;
                indx_cnt ++;
            }

            multi_dimension_index* dim = alloc_mem(multi_dimension_index, 1, mcompiler);

            dim->dimension_values = index_list;
            node->right->reference = new_envelope(dim, MULTI_DIM_INDEX, cc->compiler); /*((variable*)node->father->left->reference->to_interpret)->mult_dim_def*/
            node->right->info = expr_trim;
            node->op_type = MULTI_DIM_INDEX;
        }
        else
        {
            /* here determine what can be this
                here we are supposed to add only  variables/attributes, or the post increment stuff */
            envelope* envl = NULL;
            std::string t = expr_trim;
            int tlen = t.length();
            int env_var = 0;

            variable* var = 0;
            if(the_method)
            {
                /* is t a variable from the method we are running in?
                   (or from the call context ofthe method if any )*/
                var = the_method->has_variable(cc, t, &env_var);
            }

            // TODO: env_var is not used

            if(!var)
            {
                /* is this a variable from the current call context? */
                std::vector<variable*>::const_iterator varit = cc->has_variable(t);
                if(varit != cc->variables.end())
                {
                    var = *varit;
                }
                else
                {
                    // is this a variable in a call context above the current one?
                    call_context* cc1 = cc->father;
                    while(cc1)
                    {
                        varit = cc1->has_variable(t);
                        if(varit != cc1->variables.end())
                        {
                            var = *varit;
                            break;
                        }
                        else
                        {
                            cc1 = cc1->father;
                        }
                    }
                }
            }

            // still not found a variable ... See if it comes from father VMs
            if(!var)
            {
                int tt;
                if(cc->compiler->vm_chain())
                {
                    if(nap_vmi_has_variable(cc->compiler->vm_chain(), t.c_str(), &tt))
                    {
                        // it is a variable in the father VM
                        var = new variable(1, tt, t, "extern", cc);
                        garbage_bin<variable*>::instance(cc->compiler).place(var, cc->compiler);
                        envl = new_envelope(var, BASIC_TYPE_EXTERN_VARIABLE, cc->compiler);
                        node->op_type = BASIC_TYPE_EXTERN_VARIABLE;
                    }
                }
            }
            else
            if(var)    /* if this is a variable */
            {
                // TODO: Check if this is a class variable
                envl = new_envelope(var, BASIC_TYPE_VARIABLE, cc->compiler);
                node->op_type = BASIC_TYPE_VARIABLE;
            }

            if(node->info == expr)
            {
                mcompiler->throw_error(E0012_SYNTAXERR, expr, "");
                psuccess = false;
                return 0;
            }
            node->info = expr_trim;
            while(tlen > 0 && !isalnum( t[tlen - 1]) && t[tlen -1] != '\"'
                  && t[tlen -1] != '\'' && t[tlen - 1] != '(' && t[tlen - 1] != ')' )
            {
                t = t.substr(0, tlen - 1);
                strim(t);
                tlen = t.length();
            }

            if(tlen == 0)
            {
                mcompiler->throw_error(E0012_SYNTAXERR, orig_expr, "");
                psuccess = false;
                return 0;
            }

            if(isnumber(t))
            {
                number* nr = new number(t);
                garbage_bin<number*>::instance(cc->compiler).place(nr, cc->compiler);
                envl = new_envelope(nr, nr->type(), cc->compiler);
                node->op_type = nr->type();
            }
            else
            if(is_immediate_byte(expr_trim))
            {
                uint8_t temp = (uint8_t)(t[1]);
                char s[12];
                sprintf(s, "%d", temp);
                number* nr = new number(s);
                garbage_bin<number*>::instance(cc->compiler).place(nr, cc->compiler);
                envl = new_envelope(nr, nr->type(), cc->compiler);
                node->op_type = nr->type();
            }
            else
            if(t == "true")
            {
                envl = new_envelope(0, KEYWORD_TRUE, cc->compiler);
                node->op_type = KEYWORD_TRUE;
                *result = KEYWORD_TRUE;
            }
            else
            if(t == "false")
            {
                envl = new_envelope(0, KEYWORD_FALSE, cc->compiler);
                node->op_type = KEYWORD_FALSE;
                *result = KEYWORD_FALSE;
            }
            else
            {
                /* here maybe we should check for cases like: a[10]++ */
            }

            if(starts_with(t, "class")) /* class definition */
            {
                std::string cname = t.substr(5); // skip the class
                int begin = 0, i = 0, end = 0;
                while(isspace(cname[i]))
                {
                    i ++;
                }
                begin = i;
                while(is_identifier_char(cname[i]))
                {
                    i ++;
                }
                end = i;
                std::string tcname = cname.substr(begin, end - begin);
                class_declaration* cd = new class_declaration(mcompiler, tcname, cc);

                envl = new_envelope(cd, CLASS_DECLARATION, cc->compiler);
                node->op_type = CLASS_DECLARATION;
                *result = CLASS_DECLARATION;
            }

            if(expr == "asm")
            {
                *result = ASM_BLOCK;
                envl = new_envelope(0, ASM_BLOCK, cc->compiler);
                node->op_type = ASM_BLOCK;
            }

            if(!envl)
            {
                build_expr_tree(t, node, the_method, orig_expr, cc, result, expwloc, psuccess);
                SUCCES_OR_RETURN 0;

            }
            node->reference = envl;
        }

    return NULL;
}
