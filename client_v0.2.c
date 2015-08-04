#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT "8888"
#define IP "192.168.42.1"
#define FILE_NAME "RACK001201508031011.vid" //23 byte

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

// Send binary data to socket
// Usage: send_binary_data(filename, socket_fd);
int send_binary_data(char* filename, int sockfd)
{
    /* Open the file that we wish to transfer */
    FILE *fp = fopen(filename,"rb");
    if(fp==NULL)
    {
        printf("File opern error");
        return 1;
    }

    char filesize_char[100];
    sprintf(filesize_char, "%d", filesize(fp));
    printf("Filesize: %s\n", filesize_char);
    write(sockfd, filesize_char, strlen(filesize_char));
   
    //Wait for Server I/O
    usleep(500);

    // Send file name
    printf("FIle Name: %s\n", filename);
    write(sockfd, filename, strlen(filename));
    
    /* Wait for server I/O */
    usleep(500);

    /* Read data from file and send it */
    while(1)
    {
        /* First read file in chunks of 256 bytes */
        unsigned char buff[256]= {0};
        int nread = fread(buff,1,256,fp);

        /* If read was success, send data. */
        if(nread > 0)
        {
            //printf("Sending. \n");
            write(sockfd, buff, nread);
        }

        /* There is something tricky going on with read ..
         * Either there was error, or we reached end of file. */
        if (nread < 256)
        {
            if (feof(fp))
                printf("End of file. Transmission is over.\n");
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
    return 0;
}


int main(int argc, char *argv[])
{

    int portnum = atoi(PORT);
    char *IP_address = IP;
    struct in_addr temp;
    int sockfd = 0;
    
    struct sockaddr_in serv_addr;

    /* Create a socket first */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    /* Initialize sockaddr_in data structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portnum); // port
    serv_addr.sin_addr.s_addr = inet_addr(IP_address);

    /* Attempt a connection */
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    printf("Connect success\n");


    send_binary_data(FILE_NAME,sockfd);

    return 0;
}
