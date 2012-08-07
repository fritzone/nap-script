CC=g++
CFLAGS=-c -g3 -Wall -I../gc/include
LDFLAGS= -L. 
SOURCES= bt_string.cpp call_ctx.cpp comparison.cpp compiler.cpp consts.cpp envelope.cpp \
        hash.cpp indexed.cpp interpreter.cpp is.cpp method.cpp number.cpp operations.cpp \
        opr_func.cpp opr_hndl.cpp parser.cpp preverify.cpp throw_error.cpp notimpl.cpp \
        tree.cpp type.cpp utils.cpp variable.cpp parametr.cpp evaluate.cpp res_wrds.cpp code_output.cpp
SOURCE_DIRECTORY=nap_src
OBJECTS=$(SOURCES:.cpp=.o) 
EXECUTABLE=nap-script

all: $(SOURCES) $(EXECUTABLE)

srcpack:
	mkdir $(SOURCE_DIRECTORY)
	cp $(SOURCES) $(SOURCE_DIRECTORY)/
	cp *.h $(SOURCE_DIRECTORY)/
	cp Makefile $(SOURCE_DIRECTOR)/
	tar cvf $(SOURCE_DIRECTORY)/nap_src.tar $(SOURCE_DIRECTORY)/*
	gzip $(SOURCE_DIRECTORY)/nap_src.tar

clean:
	rm $(OBJECTS)
	rm nap.exe
	
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


