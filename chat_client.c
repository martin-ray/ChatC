#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<string.h>
#include<pthread.h>
#include<sys/types.h>
#include<signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define NAME_LEN 32

volatile sig_atomic_t flag = 0;

int sockfd = 0;
int i;
char name[NAME_LEN];

void str_overwrite_stdout(){
    printf("%s",">");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length){
    printf("%s,length = %d",arr,strlen(arr));
    for(i=0;i<length;i++){
        if(arr[i] == '\n');
        arr[i] = '\0';
        
        break;
    }
}

void catch_ctrl_c_and_exit(){
    flag = 1;
}

void recv_msg_handler(){
    char message[BUFFER_SZ];
    while(1){
        int receive = recv(sockfd,message,BUFFER_SZ,0);
        if(receive >0){
            printf("%s",message);
            str_overwrite_stdout();
        }
        else if(receive ==0){
            break;
        }
        bzero(message,BUFFER_SZ);
    }
}

void send_msg_handler(){
    char buffer[BUFFER_SZ];
    char message[BUFFER_SZ + NAME_LEN] = {};

    while(1){
        str_overwrite_stdout();
        scanf("%s",buffer);
        //str_trim_lf(buffer,BUFFER_SZ);

        if(strcmp(buffer,"exit") ==0 ){
            break;
        } else {
            sprintf(message,"%s:%s\n",name,buffer);
            send(sockfd,message,strlen(message),0);
        }

        bzero(buffer,BUFFER_SZ);
        bzero(message,BUFFER_SZ + NAME_LEN);

    }

    flag = 1;

}

int main(int argc,char *argv[]){
    if(argc != 3){
        printf("usage:%s <ip> <port>\n",argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = "127.0.0.1";
    ip = argv[1];
    int port = atoi(argv[2]);

    signal(SIGINT,catch_ctrl_c_and_exit);

    printf("enter your name:");
    scanf("%s",name);
    //str_trim_lf(name,strlen(name));

    if(strlen(name) > NAME_LEN -1 || strlen(name) < 2){
        printf("enter name correctly\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in serv_addr;
    //socket settings
    sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    //connet to the server
    int err = connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(err == -1){
        printf("error:connect\n");
        return EXIT_FAILURE;
    }

    send(sockfd,name,NAME_LEN,0);

    printf("=== WELCOME TO THE CHAT ===\n");

    pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread,NULL,(void*)send_msg_handler,NULL) != 0){
        printf("error:pthread msg send\n");
        return EXIT_FAILURE;
    }
    
    pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread,NULL,(void*)recv_msg_handler,NULL) != 0){
        printf("error:pthread msg rcv\n");
        return EXIT_FAILURE;
    }

    while(1){
        if(flag){
            printf("\nBye\n");
            break;
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}
