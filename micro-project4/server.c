#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

/* Some constants that you can use */
#define ROT 13
#define BUFFER_SIZE 256

/* The port number you must setup the TCP socket on */
const unsigned short PORT = 2200;

/* Function prototype for the scramble string method */
static void scramble_string(char *str);

/* 
 * The main method. You must fill this out as described in the project description PDF
 */
int main()
{
	int sock;
	struct sockaddr_in server, client;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket%s\n", strerror(errno));
		exit(-1);
	}
	int tmp = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *) &server, sizeof(server)) == -1) {
		printf("Binding failed%s\n", strerror(errno));
		exit(-1);
	}

	if (listen(sock, 10) == -1) {
		printf("Listen failed%s\n", strerror(errno));
		exit(-1);
	}

	int c = sizeof(client);
	int connection = accept(sock, (struct sockaddr *) &client, (socklen_t *) &c);
	if (connection == -1) {
		printf("Connection failed\n%s\n", strerror(errno));
		exit(-1);
	}

	int strLenConverted;	// host order
	int strLen;				// network order
	int n = 0;
	while (n < sizeof(strLen)) {
		n += recv(connection, &strLen, sizeof(strLen), 0);
		if (n == -1) {
			printf("Receive failed\n%s\n", strerror(errno));
			exit(-1);
		} else {
			strLenConverted = ntohl(strLen);
			//printf("Receiving %d bytes\n", strLenConverted);
		}
	}
	char str[strLenConverted + 1];
	n = 0;
	while (n < strLenConverted) {
		n += recv(connection, str, strLenConverted, 0);
		if (n == -1) {
			printf("Receive failed\n%s\n", strerror(errno));
			exit(-1);
		}
	}
	str[strLenConverted] = '\0';

	scramble_string(str);
	//printf("%s", str);

	send(connection, str, strlen(str), 0);

	close(connection);
	close(sock);

	return 0;
}

/*
 * This function takes in a NULL terminated ASCII, C string and scrambles it
 * using the popular ROT13 cipher. MODIFY AT YOUR OWN RISK
 *
 * @param str C-style string of maximum length 2^31 - 1
 */
static void scramble_string(char *str)
{
	int i;
	char t;
	for (i = 0; str[i]; i++) {
		t = str[i];
		if (t >= 'A' && t <= 'Z') {
			if ((t + ROT) <= 'Z') {
				str[i] += ROT;
			} else {
				str[i] -= ROT;
			}
		} else if (t >= 'a' && t <= 'z') {
			if ((t + ROT) <= 'z') {
				str[i] += ROT;
			} else {
				str[i] -= ROT;
			}
		}
	}
}