# Compiler and flags
CXX = g++
CXXFLAGS = -Wextra -O3 -march=native -std=c++23 -I include

# Targets
TARGETS = bbo_parser_v2 microstructure

# Source files
COMMON_SRC = order_book_manager.cpp order_book.cpp
PARSER_SRC = bbo_printer_singlethread.cpp
PARSER_V2_SRC = bbo_printer_verbose.cpp
PARSER_V3_SRC = bbo_theo_signal.cpp
ARB_FINDER_SRC = arbitrage_finder.cpp

# Object files
PARSER_OBJ = $(PARSER_SRC:.cpp=.o) $(COMMON_SRC:.cpp=.o)
PARSER_V2_OBJ = $(PARSER_V2_SRC:.cpp=.o) $(COMMON_SRC:.cpp=.o)
PARSER_V3_OBJ = $(PARSER_V3_SRC:.cpp=.o) $(COMMON_SRC:.cpp=.o)
ARB_FINDER_OBJ = $(ARB_FINDER_SRC:.cpp=.o) $(COMMON_SRC:.cpp=.o)

# Libraries
LIBS = -lpcap

# Build all targets
all: $(TARGETS)

# Rules to build each target
bbo_parser: $(PARSER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(PARSER_OBJ) $(LIBS)

bbo_parser_v2: $(PARSER_V2_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(PARSER_V2_OBJ) $(LIBS)

advanced_signals: $(PARSER_V2_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(PARSER_V3_OBJ) $(LIBS)

arbitrage_finder: $(ARB_FINDER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(ARB_FINDER_OBJ) $(LIBS)


# Rule to build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
.PHONY: clean
clean:
	rm -f *.o $(TARGETS)