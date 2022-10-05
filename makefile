all: 
	gcc -pthread s-talk.c list.c -o s-talk
clean:
	rm s-talk