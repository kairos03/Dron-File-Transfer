/* server.c
 * version 0.4
 * file send error fixed
 */

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
#include <dirent.h>

#define PORT "8888"
#define BUFFSIZE 256

void dir_check()
{
    // if media folder are not exist make it    
    DIR *dp;
    struct dirent *dir;
    int check=0;
    if( (dp = opendir(".")) == NULL )
        printf("dir opend error\n");
    else
    {
        while( (dir = readdir(dp)) != NULL )
        {
            if(dir->d_ino == 0) continue;
            else if( strcmp(dir->d_name,  "media") == 0)
            {
                check = 1;
            }
        }
    }
    if( check == 0 )
    {
        printf("media folder not found\n");
        system("mkdir media");
        printf("make media done\n");
    }
    else
        printf("media folder founded\n");
    sleep(1);

    return;
}

// read file size
int filesize(FILE *fp)
{
    int fsize;
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    return fsize;
}


// Print percentage and time for every 5%
// Usage: log(count, sum, total);
int log_display(int count, int sum, int total)
{
    time_t now;
    time(&now);

    if(100*(sum/total) >= count*10)
    {
        printf("%3d%% ", count*10);
        printf("%s", ctime(&now));
        count+=1;
    }
    return count;
}


// Receive binary data from socket
// Usage: receive_binary_data( socket_fd);
int receive_binary_data( int sockfd )
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
    memset(recvBuff, '\0', sizeof(recvBuff));

//read file name
    char fname[32];
    memset(fname, '\0', sizeof(fname));
    read(sockfd, fname, 31);
    printf("File Name:             %s\n", fname);

//add filename to location
//    char file_location[32] = "./media/";
//    strcat(file_location, fname);

// file open
    fp = fopen(fname, "wb");
    if(NULL == fp)
    {
        printf("Error opening file");
	return 1;
    }
 
     /* Receive data in chunks of BUFFSIZE bytes */
    while((bytesReceived = read(sockfd, recvBuff, BUFFSIZE)) > 0)
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
  
    return 0;
}

int main(int argc, char *argv[])
{

    dir_check();

    int portnum = atoi(PORT);

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

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);

        receive_binary_data( connfd );
        close(connfd);
    }

    close(listenfd);
    return 0;
}
