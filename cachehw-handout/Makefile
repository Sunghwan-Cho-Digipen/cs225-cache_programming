#
# Student makefile for Cache Lab
# Note: requires a 64-bit x86-64 system 
#
CC = g++
CFLAGS = -O2 -Wall -W -Werror -std=c++17 -m64

csim: csim.cpp cachelab.cpp cachelab.h
	$(CC) $(CFLAGS) -o csim csim.cpp cachelab.cpp -lm 


#
# Clean the src dirctory
#
clean:
	rm -rf *.o
	rm -f csim
	rm -f .csim_results .marker
