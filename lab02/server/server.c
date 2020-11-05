#include <arpa/inet.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "itoa/itoa.h"

#define SOCKET_PORT 8888

#define BUF_SIZE 256

static int sd;

int pexit(const char *str)
{
	close(sd);
	perror(str);
	exit(EXIT_FAILURE);
}

void sigint_handler(__attribute__((unused)) int signum)
{
	close(sd);
	exit(EXIT_SUCCESS);
}

int tprintf(const char *format, ...)
{
	const time_t t = time(NULL);
	printf("[%.19s] ", ctime(&t));

	va_list ap;
	va_start(ap, format);
	const int rc = vprintf(format, ap);
	va_end(ap);

	return rc;
}

void process_client(void)
{
	struct sockaddr_in client_sock;
	socklen_t client_sock_size = sizeof(client_sock);
	char msg[BUF_SIZE];
	const ssize_t size = recvfrom(sd, msg, BUF_SIZE, 0, (struct sockaddr *) &client_sock, &client_sock_size);
	if (size == -1) {
		pexit("recvfrom");
	}
	msg[size] = '\0';
	
	tprintf("received message from %s:%d: '%s'\n", inet_ntoa(client_sock.sin_addr), ntohs(client_sock.sin_port), msg);

	char *endptr = msg;
	const int num = strtol(msg, &endptr, 10);
	if (*msg == '\n' || *endptr) {
		tprintf("  message is not a decimal number\n");
		return;
	}
	
	tprintf("  dec: %lld\n", num);

	tprintf("  bin: %s\n", itoa(num, msg, 2));
	tprintf("  oct: %s\n", itoa(num, msg, 8));
	tprintf("  hex: %s\n", itoa(num, msg, 16));
	tprintf("  pentadec: %s\n", itoa(num, msg, 15));
}

int main(void)
{
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		return EXIT_FAILURE;
	}

	const struct sockaddr_in server_sock = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(SOCKET_PORT),
	};

	if (bind(sd, (const struct sockaddr *) &server_sock, sizeof(server_sock)) == -1) {
		pexit("bind");
	}

	socklen_t server_sock_size = sizeof(server_sock);
	if (getsockname(sd, (struct sockaddr *) &server_sock, &server_sock_size) == -1) {
		pexit("getsockname");
	}
	tprintf("server is running on %s:%d\n", inet_ntoa(server_sock.sin_addr), ntohs(server_sock.sin_port));

	if (signal(SIGINT, sigint_handler) == SIG_ERR) {
		pexit("signal");
	}

	for (;;) {
		process_client();
	}
}
