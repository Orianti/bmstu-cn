#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "socket.h"

static int sd;

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <message>\n", argv[0]);
		return EXIT_FAILURE;
	}

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

	const struct sockaddr_in server_sock = {
		.sin_family = AF_INET,
		.sin_port = htons(SOCKET_PORT),
		.sin_addr.s_addr = INADDR_ANY,
	};

	if (sendto(sd, argv[1], strlen(argv[1]), 0, (struct sockaddr *) &server_sock, sizeof(server_sock)) == -1) {
		perror("sendto");
		close(sd);
		return EXIT_FAILURE;
	}
	printf("sent message: %s\n", argv[1]);
	
	close(sd);
	return EXIT_SUCCESS;
}
