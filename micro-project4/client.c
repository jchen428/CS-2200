#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <arpa/inet.h>

/* The port number you must create a TCP socket at */
const unsigned short PORT = 2200;

/*
 * The main function where you will be performing the tasks described under the
 * client section of the project description PDF
 */
int main(int argc, char **argv)
{
	/* Making sure that the correct number of command line areguments are being passed through */
	if (argc < 3) {
		fprintf(stderr, "client usage: ./client <server IP address> <string to send>\n");
		exit(-1);
	}

	int sock;
	char *ip = argv[1];
	char *sendStr = argv[2];
	int strLen = strlen(sendStr);
	char recvStr[strLen + 1];
	struct sockaddr_in address;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("Could not create socket%s\n", strerror(errno));
		exit(-1);
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	inet_aton(ip, (struct in_addr *) &address.sin_addr.s_addr);

	//printf("sock = %d\nip = %s\nsendStr = %s\n", sock, ip, sendStr);

	if (connect(sock, (struct sockaddr *) &address, sizeof(address)) == -1) {
		printf("Connection failed%s\n", strerror(errno));
		exit(-1);
	}

	int strLenConverted = htonl(strLen);
	int n = send(sock, &strLenConverted, sizeof(strLenConverted), 0);
	if (n == -1) {
		printf("Sending failed%s\n", strerror(errno));
		exit(-1);
	}
	n = send(sock, sendStr, strLen, 0);
	if (n == -1) {
		printf("Sending failed%s\n", strerror(errno));
		exit(-1);
	}

	bzero(recvStr, strLen + 1);
	n = 0;
	while (n < strLen) {
		n += recv(sock, recvStr, strLen, 0);
		if (n == -1) {
			printf("Receive failed\n%s\n", strerror(errno));
			exit(-1);
		}
	}
	recvStr[strLen] = '\0';

	close(sock);
	printf("%s\n", recvStr);

	return 0;
}
