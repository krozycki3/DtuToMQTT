SHELL = /bin/bash
all:
	gcc -o DtuToMQTT DtuToMQTT.c -lpaho-mqtt3c
clean:
