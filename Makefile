COMPILECPP 	= g++ -std=gnu++17 -g -O0 -Wall -Wextra
FLEX 			= flex --outfile=${CLGEN}

BISON			= bison --defines=${HYGEN} \
				  --output=${CYGEN}

EXECBIN		= oc
SOURCES		= astree.cpp lyutils.cpp \
				  string_set.cpp auxlib.cpp \
				  symtable.cpp attr_bitset.cpp \
				  oiler.cpp main.cpp
ALLSOURCES  = ${SOURCES} ${CLGEN} ${CYGEN}
HEADERS     = astree.h lyutils.h string_set.h auxlib.h \
				  symtable.h attr_bitset.h oiler.h
LSOURCES		= scanner.l
CLGEN			= yylex.cpp
YSOURCES  	= parser.y
HYGEN 		= yyparse.h
CYGEN 		= yyparse.cpp

OBJECTS		= ${ALLSOURCES:.cpp=.o}
	MAKEFILE 	= Makefile

all : ${EXECBIN}


${EXECBIN} : ${CYGEN} ${HYGEN} ${OBJECTS}
	${COMPILECPP} -o ${EXECBIN} ${OBJECTS}

%.o : %.cpp
	#checksource ${SOURCES}
	${COMPILECPP} -c $<

yylex.o : yylex.cpp
	g++ -std=gnu++14 -g -O0 -c $<

${CLGEN}	: ${LSOURCES}
	${FLEX} ${LSOURCES}

${CYGEN} ${HYGEN} : ${YSOURCES}
	${BISON} ${YSOURCES}

clean		:
	rm -f ${CLGEN} ${CYGEN} ${HYGEN} ${DEPFILE} \
		*.output *.o *.out *.err *.str *.tok *.ast *.sym \
		*.oil

spotless    :
	${MAKE} clean; rm -f ${EXECBIN}

ci	:

