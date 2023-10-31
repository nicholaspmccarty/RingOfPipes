CXX=g++-12
RM=rm -f
CPPFLAGS=--std=c++2b -Werror -Wall -Wextra -Wconversion -Wsign-conversion -Wpedantic -Wnull-dereference -Wold-style-cast -Wdouble-promotion -Wshadow

NAME=a2

SRCS=$(NAME).cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: program

program: $(OBJS)
	$(CXX) $(CPPFLAGS) -o $(NAME) $(OBJS)

# Build all C++ files
depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend

# Clean compiled C++ files
clean:
	$(RM) $(OBJS) *~ .depend $(NAME)
 

include .depend