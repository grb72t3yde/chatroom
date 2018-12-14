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



int server_sockfd = 0,client_sockfd = 0;
ClientList *root, *now;


void catch_ctrl_c_and_exit(int sig){
	ClientList *tmp;
	while(root != NULL){
		printf("\nClose socketfd: %d\n",root->data);
		close(root->data);
		tmp = root;
		root = root->link;
		free(tmp);
	}
	printf("Bye\n");
	exit(EXIT_SUCCESS);
}

void send_to_all_clients(ClientList *np,char tmp_buffer[]){
	ClientList *tmp = root->link;
	while(tmp!=NULL){
		if(np->data!=tmp->data){
			printf("Send to sockfd %d: \"%s\" \n", tmp->data,tmp_buffer);
			send(tmp->data,tmp_buffer,LENGTH_SEND,0);
		}
		tmp = tmp->link;
	}
}


int main(){
	signal(SIGINT,catch_ctrl_c_and_exit);



	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(server_sockfd == -1){
		printf("Fail to create a socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_info,client_info;
	int s_addrlen = sizeof(server_info);
	int c_addrlen = sizeof(client_info);
	memset(&server_info,0,s_addrlen);
	memset(&client_info,0,c_addrlen);
	server_info.sin_family = PF_INET;
	server_info.sin_addr.s_addr = INADDR_ANY;
	server_info.sin_port = htons(8080);


	bind(server_sockfd,(struct sockaddr *)&server_info,s_addrlen);
	listen(server_sockfd,5);


	getsockname(server_sockfd,(struct sockaddr*)&server_info,(socklen_t *)&s_addrlen);
	printf("Start Server on: %s:%d\n", inet_ntoa(server_info.sin_addr),ntohs(client_info.sin_port));


	root=newNode(server_sockfd, inet_ntoa(server_info.sin_addr));
	now=root;


	while(1){
		cl
