src = *.cpp

all: $(src)
	g++ -g -Wall -Wextra -o client.exe $(src)
run: all $(src)
	./client.exe $(ARGS)
clean:
	rm -f client