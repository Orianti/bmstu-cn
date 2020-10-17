#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PORT 8888

#define BUF_SIZE 256

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
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(SOCKET_PORT),
	};

	size_t len = strlen(argv[1]);
	ssize_t bytes = sendto(sd, argv[1], len, 0, (struct sockaddr *) &server_sock, sizeof(server_sock));
	if (bytes == -1) {
		perror("sendto");
		close(sd);
		return EXIT_FAILURE;
	}
	printf("%ld out of %lu bytes were sent\n", bytes, len);
	
	close(sd);
	return EXIT_SUCCESS;
}
