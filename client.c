#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./afunix_socket"

int main(void)
{
	struct sockaddr_un remote;
	int s;
	char *buffer_w = calloc(255, sizeof(char));
	int len;

	strcpy(buffer_w, "127.0.0.1:9999/favicon.ico$127.0.0.1$404");
	
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
		perror("socket");

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCKET_PATH);

	len = strlen(remote.sun_path) + sizeof(remote.sun_family);

	if((connect(s, (struct sockaddr *)&remote, len)) !=0)
	{
		perror("connect");
		exit(1);
	}

	//for(;;) {
		/* send a message */
		if((write(s, buffer_w, strlen(buffer_w) + 1)) < 0)
			perror("write");
	//}
}