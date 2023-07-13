src = *.cpp

all: $(src)
	g++ -g -Wall -Wextra -o client.exe $(src)
run: all $(src)
	./client.exe $(ARGS)
test: all $(src)
	./client.exe 127.0.0.1 8080
clean:
	rm -f client.exe