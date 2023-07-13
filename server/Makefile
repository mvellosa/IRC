src = *.cpp

all: $(src)
	g++ -g -Wall -Wextra -o server.exe $(src)
run: all $(src)
	./server.exe $(ARGS)
clean:
	rm -f server.exe