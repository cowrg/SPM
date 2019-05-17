OBJECTS = main.cpp
CFLAGS = -pthread -std=c++17 -O3 
CC = g++
PROGRAM_NAME = main 

$(PROGRAM_NAME):$(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(PROGRAM_NAME)
	@echo "----> Ex. "$(PROGRAM_NAME)"4 10000" 

clean: 
	rm -f $(PROGRAM_NAME)
	rm -f $(TESTNAME)
	@echo "Clean done"
