#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include "proto.h"
#include "string.h"

// Global variables
volatile sig_atomic_t flag = 0;
int confirm = 0 ;
int ifyes = 0 ;
int stop = 0;
int sockfd = 0;
int fileflag = 0;
char nickname[LENGTH_NAME] = {};

void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

void recv_msg_handler() {
    char receiveMessage[LENGTH_SEND] = {};
//    char question[1024] ={};
//    char readfile[1024] = {};
    char filename[1024] ={};
    while (1) {
        int receive = recv(sockfd, receiveMessage, LENGTH_SEND, 0);
        if (receive > 0) {
		if(strncmp(receiveMessage,"file:",5)==0){
			fileflag = 1;
			if(recv(sockfd,filename,LENGTH_SEND,0)>0){
				printf("U receive the file\n");
				FILE* fp;
//				int byte;
				char buf[1024]={};
//				fprintf(stderr,"%s\n",filename);
				memset(buf,'\0',1024);
				fp = fopen("new.txt","w");
				fprintf(stderr,"open file suce\n");
//				sleep(1);
				if(recv(sockfd,buf,1024,0)>0){
					fprintf(stderr,"1\n");
					fprintf(stderr,"%s\n",buf);
			//		if(strncmp(buf,"DONE",4)==0)
			//			break;

					fprintf(fp,"%s",buf);
				}
				else
					fprintf(stderr,"write error\n");
				fprintf(stderr,"end\n");
				fclose(fp);
				break;
			}
			fileflag = 0;
    
    		}else{
            		printf("\r%s\n", receiveMessage);
            		str_overwrite_stdout();
		}
		
        } else if (receive == 0) {
            	break;
        } else { 
            // -1 
        }
}
}

void send_msg_handler() {
    char message[LENGTH_MSG] = {};
    char person[1024] = {};
    char filename[1024] = {};
    char file[1024] = {};
    char *idx =NULL;
    int fd = 0;
    while (1) {
        str_overwrite_stdout();
/*	if(stop){
		fprintf(stderr,"stop = %d\nconfirm = %d\nifyes = %d\n", stop,confirm,ifyes);
		while(1){
			if(stop == 0)
				break;
		}
	}*/
	confirm = 0;
	ifyes = 0;
	if(fileflag == 1)
		continue;
        while (fgets(message, LENGTH_MSG, stdin) != NULL) {
            str_trim_lf(message, LENGTH_MSG);
            if (strlen(message) == 0) {
                str_overwrite_stdout();
            } else {
                break;
            }
        }
	if(fileflag == 1)
		continue;
	if(strncmp(message,"/sendfile",9)==0){
		send(sockfd, message, LENGTH_MSG, 0);
//		sleep(5);
		memset(person,0,1024);
		memset(filename,0,1024);

		idx = strtok(message," ");
		idx = strtok(NULL," ");
		strcpy(person,idx);
		idx = strtok(NULL,"\0");
		strcpy(filename,idx);

		memset(file,0,1024);

//		printf("%s\n",filename);

		fd = open(filename,O_RDONLY,0666);
//		fprintf(stderr,"%s\n",filename);
		if(fd != -1){
			if(sendfile(sockfd,fd,NULL,500000)>0){
//				send(sockfd,file,1024,0);
//				sendfile(sockfd,fd,NULL,500000);
	//			send(sockfd,person,1024,0);
			}
			else
				printf("Reading error\n");
		}
		else
			printf("File error\n");
	}
	else
	       	send(sockfd, message, LENGTH_MSG, 0);


        if (strcmp(message, "/exit") == 0) {
            break;
        }
	//confirm = 1;
    }
    catch_ctrl_c_and_exit(2);
}

int main()
{
    signal(SIGINT, catch_ctrl_c_and_exit);

    // Naming
    printf("Please enter your name: ");
    if (fgets(nickname, LENGTH_NAME, stdin) != NULL) {
        str_trim_lf(nickname, LENGTH_NAME);
    }
    if (strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("\nName must be more than one and less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (sockfd == -1) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }

    // Socket information
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(8888);

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(sockfd, nickname, LENGTH_NAME, 0);

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
