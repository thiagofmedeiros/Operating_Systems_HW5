/* This is the client part for sending one file through network stream socket 
 and waiting for its return.  The server must be running on osnode16 in
this version.  This writer can run anywhere. 
To compile under the solaris version of cc.
cc writern.c -lsocket -lxnet
gcc writern.c -lsocket -lxnet
explicit library calls are required for some machines. */


#include <sys/types.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#define IP_ADDR "10.247.53.97"   /* osnode16.cse.usf.edu IP address, where server 
                                    must run */
/* For test #define PORT_NUM 1150 */
/* #define PORT_NUM 1050 */
#define PORT_NUM 1044
#define BUFFLEN 100


int  main(int argc, char *argv[])
{
	int len;
	int sock;
	struct sockaddr_in  server,remote;
	int remotelength;
	int fd;
	char buf[BUFFLEN];
	if ( argc != 2 ) /* argc should be 2 for correct execution */
	  {
	    /* We print argv[0] assuming it is the program name */
	    printf( "usage: %s filename", argv[0] );
            exit(0);
	  }
	/* socket() call generates one socket for ipc */
	if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Opening Stream Socket");
		exit(1);
	}

	/* address naming for the internet domain socket */
	server.sin_family = AF_INET;
      	server.sin_port = htons(PORT_NUM);
      	server.sin_addr.s_addr = inet_addr(IP_ADDR);

	/* use the socket and server address to initiate connection */
	if( connect(sock, (struct sockaddr*)&server, sizeof(server))<0)	{
		perror("connecting stream socket");
		exit(1);
	}


	/* using open() system call to be consistent with write() and send(),etc. */
	if ( (fd = open("infile", O_RDONLY)) == 0)
		{
		perror("file open");
		exit(1);
		}

	while ( (len = read(fd, buf, BUFFLEN)) != 0)
		{
		/* send the message throught communication socket */

                write(0, buf, len);   
        printf("Connected and sending out a message of len %d\n", len);
        fflush(stdout);
		if (send(sock, buf, len, 0) == -1)
			{
			perror("Write to Socket Channel");
			exit(1);
			}
		}

	close(fd);
        len = recv(sock, buf, BUFFLEN, 0);
        printf("Got back: \n");
        fflush(stdout);
        write(0, buf, len);
        printf("End msg*** \n");
	/* close the communication socket and listenning socket */
	close(sock);

}

