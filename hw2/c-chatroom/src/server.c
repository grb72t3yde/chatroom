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
#include <sys/stat.h>
#include <fcntl.h>
#include "proto.h"
#include "server.h"

// Global variables
int server_sockfd = 0, client_sockfd = 0;
ClientList *root, *now;

void catch_ctrl_c_and_exit(int sig) {
    ClientList *tmp;
    while (root != NULL) {
        printf("\nClose socketfd: %d\n", root->data);
        close(root->data); // close all socket include server_sockfd
        tmp = root;
        root = root->link;
        free(tmp);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np, char tmp_buffer[]) {
    ClientList *tmp = root->link;
    while (tmp != NULL) {
        if (np->data != tmp->data) { // all clients except itself.
//            printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
            send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
        }
        tmp = tmp->link;
    }
}

void send_to_one_client(ClientList *np, char tmp_buffer[], char target[]) {
	ClientList *tmp = root->link;
	while(tmp != NULL){
		if(strcmp(tmp->name,target)==0) {
	//		printf("Send to sockfd %d: \"%s\" \n", tmp->data, tmp_buffer);
			send(tmp->data, tmp_buffer, LENGTH_SEND, 0);
			break;
		}
		tmp = tmp->link;
	}
}

void send_file_to_one(ClientList *np,char file[],char target[],char filename[]){
	ClientList *tmp = root->link;
	int fd;
//	FILE* fp;
//	char content[1024] ={};
//	char response[1024] ={};
	fprintf(stderr,"in\n");
	while(tmp != NULL){
		if(strcmp(tmp->name,target)==0){
			fprintf(stderr,"find\n");
//			send(tmp->data,"Get a file,accept?(yes/no)\n",50,0);
	/*		memset(response,0,1024);
			if(recv(tmp->data,response,1024,0)>0){
				if(recv(tmp->data,response,1024,0)>0)
					fprintf(stderr,"response = %s\n",response);
			}*/
//			memset(response,0,1024);
//			if(recv(tmp->data,response,1024,0)>0){
//				if(recv(tmp->data,response,1024,0)>0)
//					fprintf(stderr,"%s\n",response);
			
		//		printf("%s",file);
		//	printf("456\n");
			send(tmp->data,filename,strlen(filename),0);
			fd = open("123.txt",O_RDWR,0666);
			if(write(fd,file,strlen(file))){
				printf("write\n");
//			fprintf(fp,"%s",file);
//			fprintf(stderr,"%s",file);
//			if(fgets(content,1024,fp)!=NULL){
//				fprintf(stderr,"%s\n",content);
				if(sendfile(tmp->data,fd,NULL,strlen(file))){
					printf("send\n");
					send(tmp->data,"DONE",4,0);
				}
				else
					fprintf(stderr,"send error\n");
			}
//			
			else
				fprintf(stderr,"error\n");

		}
		
		tmp = tmp->link;
		}
	}


void client_handler(void *p_client) {
    int leave_flag = 0;
    char nickname[LENGTH_NAME] = {};
    char recv_buffer[LENGTH_MSG] = {};
    char send_buffer[LENGTH_SEND] = {};

    char person[1024] = {};
    char message[1024] = {};
//    char file[500000] = {};
    char filename[1024] = {};
    char file[1024] = {};
    char *word;
    ClientList *np = (ClientList *)p_client;
    ClientList *idx = np;

    // Naming
    if (recv(np->data, nickname, LENGTH_NAME, 0) <= 0 || strlen(nickname) < 2 || strlen(nickname) >= LENGTH_NAME-1) {
        printf("%s didn't input name.\n", np->ip);
        leave_flag = 1;
    } else {
        strncpy(np->name, nickname, LENGTH_NAME);
        printf("%s(%s)(%d) join the chatroom.\n", np->name, np->ip, np->data);
        sprintf(send_buffer, "%s(%s) join the chatroom.", np->name, np->ip);
        send_to_all_clients(np, send_buffer);
    }

    // Conversation
    while (1) {
        if (leave_flag) {
            break;
        }
	memset(recv_buffer,0,LENGTH_MSG);
        int receive = recv(np->data, recv_buffer, LENGTH_MSG, 0);
//	fprintf(stderr,"%s\n",recv_buffer);
	memset(person,0,1024);
	memset(message,0,1024);

    
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
	    else if(strcmp(recv_buffer,"/show")==0){
		    memset(send_buffer,0,sizeof(send_buffer));
		    idx = root->link;
		    strcat(send_buffer,"\nOnline chatter:\n");
		    while(idx != NULL){
			    strcat(send_buffer,idx->name);
			    strcat(send_buffer,"\n");
//			    printf("%s\n",idx->name);
			    idx = idx->link;
		    }
		    send(np->data,send_buffer,LENGTH_SEND,0);
		    continue;
	    }
	    else if(strncmp(recv_buffer,"/to",3)==0){
		    memset(person,0,1024);
		    memset(message,0,1024);

		    word = strtok(recv_buffer," ");
		    word = strtok(NULL," ");
		    strcpy(person,word);
		    word = strtok(NULL,"\0");
		    strcpy(message,word);

		    sprintf(send_buffer,"%s:%s from %s", np->name, message, np->ip);

		    send_to_one_client(np,send_buffer,person);
		    continue;
	    }
	    else if(strncmp(recv_buffer,"/broadcast",10)==0){
		    fprintf(stderr,"in\n");
		    memset(message,0,1024);

		    word = strtok(recv_buffer," ");
		    word = strtok(NULL,"\0");
		    strcpy(message,word);
		    sprintf(send_buffer, "%s:%s from %s", np->name, message, np->ip);
	    }
	    else if(strncmp(recv_buffer,"/sendfile",9) == 0){
//		    fprintf(stderr,"123\n");
//		    recv(np->data,person,1024,0);
//		    printf("%s\n",person);

		    memset(person,0,1024);
		    memset(filename,0,1024);

		    word = strtok(recv_buffer," ");
		    word = strtok(NULL," ");
		    strcpy(person,word);
		    word = strtok(NULL,"\0");
		    strcpy(filename,word);

		    sprintf(send_buffer,"file:%s:%s from %s",np->name, filename, np->ip);
		    
		    send_to_one_client(np,send_buffer,person);
//		    sleep(5);
		    
		    memset(recv_buffer,0,LENGTH_MSG);
		    receive = recv(np->data,recv_buffer,LENGTH_MSG,0);
		    printf("%s\n",recv_buffer);
//		    memset(recv_buffer,0,LENGTH_MSG);
//		    receive = recv(np->data,recv_buffer,LENGTH_MSG,0);


//		    receive = recv(np->data,filename,LENGTH_MSG,0);
//		    fprintf(stderr,"%s\n",filename);
	//	    fprintf(stderr,"%s\n",recv_buffer);

//		    printf("%ld\n",read(np->data,recv_buffer,LENGTH_MSG));

//		    if(read(np->data,recv_buffer,LENGTH_MSG)){
//			fprintf(stderr,"inif");
//		    	printf("%s\n",recv_buffer);
	            strcpy(file,filename);
		    send_file_to_one(np,recv_buffer,person,file);
		    
		//    else
		//	    printf("Read error\n");

		    continue;
	    }
//	   else
//		sprintf(send_buffer, "%s：%s from %s", np->name, recv_buffer, np->ip); 
	} else if (receive == 0 || strcmp(recv_buffer, "/exit") == 0) {
            printf("%s(%s)(%d) leave the chatroom.\n", np->name, np->ip, np->data);
            sprintf(send_buffer, "%s(%s) leave the chatroom.", np->name, np->ip);
            leave_flag = 1;
        } 
	  else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        send_to_all_clients(np, send_buffer);
    }

    // Remove Node
    close(np->data);
    if (np == now) { // remove an edge node
        now = np->prev;
        now->link = NULL;
    } else { // remove a middle node
        np->prev->link = np->link;
        np->link->prev = np->prev;
    }
    free(np);
}

int main()
{
    signal(SIGINT, catch_ctrl_c_and_exit);

    // Create socket
    server_sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if (server_sockfd == -1) {
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
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(8888);

    // Bind and Listen
    bind(server_sockfd, (struct sockaddr *)&server_info, s_addrlen);
    listen(server_sockfd, 5);

    // Print Server IP
    getsockname(server_sockfd, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    printf("Start Server on: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

    // Initial linked list for clients
    root = newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
    now = root;

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);

        // Print Client IP
        getpeername(client_sockfd, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
        printf("Client %s:%d come in.\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

        // Append linked list for clients
        ClientList *c = newNode(client_sockfd, inet_ntoa(client_info.sin_addr));
        c->prev = now;
        now->link = c;
        now = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)client_handler, (void *)c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
