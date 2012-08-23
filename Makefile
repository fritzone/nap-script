CC=g++
CFLAGS=-c -ggdb -g3 -Wall -I../gc/include
LDFLAGS= -L. 
SOURCES= bt_string.cpp call_ctx.cpp compiler.cpp consts.cpp envelope.cpp \
        hash.cpp indexed.cpp interpreter.cpp is.cpp method.cpp number.cpp operations.cpp \
        opr_func.cpp opr_hndl.cpp parser.cpp preverify.cpp throw_error.cpp notimpl.cpp \
        tree.cpp type.cpp utils.cpp variable.cpp parametr.cpp evaluate.cpp res_wrds.cpp code_output.cpp \
        code_stream.cpp
        
SOURCE_DIRECTORY=nap_src
OBJECTS=$(SOURCES:.cpp=.o) 
EXECUTABLE=nap-script

all: $(SOURCES) $(EXECUTABLE)

clean:
	rm $(OBJECTS)
	rm $(EXECUTABLE)
	
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


