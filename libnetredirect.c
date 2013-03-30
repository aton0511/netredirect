/* The definition below is required to define RTLD_NEXT by including dlfcn.h */
#define _GNU_SOURCE

#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <dlfcn.h>


typedef int (*connect_function)(int, const struct sockaddr *, socklen_t);


static const char *host_variable = "NETREDIRECT_HOST";
static const char *port_variable = "NETREDIRECT_PORT";


static void redirect_host(int sockfd, const struct sockaddr *addr)
{
	char *host = getenv(host_variable);
	struct sockaddr_in *sin = (struct sockaddr_in *) addr;
	int type;
	socklen_t type_length = sizeof(type);

	if (!host) {
		goto out;
	}

	if (AF_INET != sin->sin_family) {
		goto out;
	}

	if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &type_length)) {
		goto out;
	}

	if (SOCK_STREAM != type) {
		goto out;
	}

	if (!inet_aton(host, &sin->sin_addr)) {
		goto out;
	}

out:
	return;
}

static void redirect_port(int sockfd, const struct sockaddr *addr)
{
	char *port = getenv(port_variable);
	struct sockaddr_in *sin = (struct sockaddr_in *) addr;
	int type;
	socklen_t type_length = sizeof(type);

	if (!port) {
		goto out;
	}

	if (AF_INET != sin->sin_family) {
		goto out;
	}

	if (getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &type, &type_length)) {
		goto out;
	}

	if (SOCK_STREAM != type) {
		goto out;
	}

	sin->sin_port = htons(atoi(port));

out:
	return;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	connect_function system_connect = dlsym(RTLD_NEXT, "connect");

	redirect_host(sockfd, addr);
	redirect_port(sockfd, addr);

out:
	return system_connect(sockfd, addr, addrlen);
}
