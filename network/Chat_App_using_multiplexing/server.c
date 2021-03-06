#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100
void error_handling(char *buf);

int main(int argc, char *argv[])
{
    //variable setup
	int serv_sock, clnt_sock;       //socket variable
	struct sockaddr_in serv_adr, clnt_adr;  //socket structure
    struct timeval timeout;        //timeout structure
	fd_set fds, cpy_fds;            //fd set
	socklen_t adr_sz;
	int fd_max, str_len, fd_num, i;
	char buf[BUF_SIZE];
    
	if(argc != 2) {             // if input format does not match print error message
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET, SOCK_STREAM, 0);          //create server socket
    
	memset(&serv_adr, 0, sizeof(serv_adr));             //initialize server address
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)   //assign server's address information to socket
		error_handling("bind() error");
    
	if(listen(serv_sock, 5) == -1)      //listen
		error_handling("listen() error");

	FD_ZERO(&fds);      //reset all the bits in fds to zero
	FD_SET(serv_sock, &fds);    //register socket descriptor to fds
	fd_max = serv_sock;         //save socket number

	while(1)
	{
		cpy_fds = fds;          //copy fds
        
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;     //timeout setup

		if((fd_num = select(fd_max + 1, &cpy_fds, 0, 0, &timeout)) == -1){  //check whether received data is exist in every file descriptor
			break;
		}
		
		if(fd_num == 0) {           //if Timeout occur
			continue;
		}
		
		for(i = 0; i < fd_max + 1; i++)     //if there is a change
		{
			if(FD_ISSET(i, &cpy_fds))
			{
				if(i == serv_sock) // if connection request!
				{
					
					adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);    // accept connection request
					FD_SET(clnt_sock, &fds);        //to keep watching the connection
					if(fd_max < clnt_sock)          //if clnt_sock increase
						fd_max = clnt_sock;         //update fd_max
					printf("connected client: %d \n", clnt_sock);
				}
				else // read message!
				{
					str_len = read(i, buf, BUF_SIZE);
					if(str_len == 0) // close request!
					{
						FD_CLR(i, &fds);
						close(i);
						printf("closed client: %d \n", i);
					}
                    else {// echo!
                        for(int j=4; j<fd_max+1; j++){
                            write(j, buf, str_len);     //send message back to all client
                        }
                    }
				}
			}
		}
	}
	close(serv_sock);
	return 0;
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}
