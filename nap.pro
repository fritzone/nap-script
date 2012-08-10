TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
bt_string.cpp    \
evaluate.cpp     \
method.cpp     \
parametr.cpp    \
type.cpp\
call_ctx.cpp     \
hash.cpp        \
notimpl.cpp    \
parser.cpp      \
utils.cpp\
code_output.cpp \
indexed.cpp    \
number.cpp    \
preverify.cpp \
variable.cpp\
compiler.cpp   \
interpreter.cpp \
operations.cpp \
res_wrds.cpp\
consts.cpp     \
is.cpp       \
opr_func.cpp \
throw_error.cpp\
envelope.cpp   \
listv.cpp      \
opr_hndl.cpp  \
tree.cpp

HEADERS += bsd_enve.h     com_strings.h  is.h          opr_hndl.h     tree.h \
bsd_extr.h     consts.h       listv.h       parametr.h     type.h \
bsd_indx.h     envelope.h     method.h      parser.h       utils.h \
bsd_numb.h     evaluate.h     notimpl.h     preverify.h    variable.h \
bt_string.h    hash.h         number.h      res_wrds.h \
call_ctx.h     indexed.h      operations.h  sys_brkp.h \
code_output.h  interpreter.h  opr_func.h    throw_error.h
