#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define PORT "8888"
#define BUFFSIZE 256

int filesize(FILE *fp)
{
    int sz;
    fseek(fp, 0L, SEEK_END);
    sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return sz;
}


// Print percentage and time for every 5%
// Usage: log(count, sum, total);
int log_display(int count, int sum, int total)
{
    time_t now;
    time(&now);

    if(100*sum/total >= count*5)
    {
        printf("%3d%% ", count*5);
        printf("%s", ctime(&now));
        count+=1;
    }
    return count;
}


// Receive binary data from socket
// Usage: receive_binary_data(filename, socket_fd);
int receive_binary_data(char* filename, int sockfd)
{
    /* Create file where data will be stored */
    FILE *fp;
    int bytesReceived = 0;
    char recvBuff[BUFFSIZE];
    memset(recvBuff, 0, sizeof(recvBuff));

    int sum = 0;
    int count = 0;

//read file byte
    read(sockfd, recvBuff, BUFFSIZE);
    int filesize = atoi(recvBuff);
    printf("Receive filesize:      %d\n", filesize);
    memset(recvBuff, 0, sizeof(recvBuff));

//read file name
    read(sockfd, recvBuff, BUFFSIZE);
    char name[50];
    strcpy(name, recvBuff);
    printf("File Name:             %s\n", name);
    memset(recvBuff, 0, sizeof(recvBuff));   

//add filename to location
    char file_location[100] = "./media/";
    strcat(file_location, name);

    fp = fopen(file_location, "wb");
    if(NULL == fp)
    {
        printf("Error opening file");
	return 1;
    }
 
    while(1)
    {
    
        /* Receive data in chunks of BUFFSIZE bytes */
        while((bytesReceived = read(sockfd, recvBuff, BUFFSIZE-1)) > 0)
        {
            /* Print the log-message */
            sum += bytesReceived;
            count = log_display(count, sum, filesize);
            fwrite(recvBuff, 1, bytesReceived, fp);
        }

        count+=1;
        log_display(count, sum, filesize);

        if(bytesReceived < 0)
        {
            printf("\n Read Error \n");
        }

        if(filesize != sum)
        {
    	    printf("file save ERROR\n");	
            printf("Retry...");
            sum = 0;
            count = 0;
            char commend[100] = "rm -f ./media/";
            strcat(commend, name);
            system(commend);
            sleep(1);
        }
        else
        {
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{

    int portnum = atoi(PORT);

/*
    if (argc < 2) {
        fprintf(stderr,"usage %s <Hostname/IP-address> \n", argv[0]);
        exit(0);
    }
*/

    int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    printf("Socket retrieve success\n");

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portnum);

    bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

    if(listen(listenfd, 10) == -1)
    {
        printf("Failed to listen\n");
        return -1;
    }
    connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);


    receive_binary_data("output_data.txt", connfd);


    close(connfd);
    return 0;
}
