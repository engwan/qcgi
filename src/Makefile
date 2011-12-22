CC=g++
CFLAGS=
LDFLAGS=-lsqlite3 -lmysqlclient -lfcgi -lrudecgi
SOURCES=qcgi+.cpp parser.cpp interpreter.cpp db.cpp utils.cpp sessions.cpp
#OBJECTS=$(SOURCES:.cpp=.o)
OBJECTS=$(SOURCES:.cpp=.o) /usr/lib/libboost_regex-mt.so
EXECUTABLE=qcgi+

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	mv $(EXECUTABLE) ../$(EXECUTABLE)

clean:
	rm *.o
