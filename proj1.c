#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int get = 0;
    int newfd; // client's fd
    struct sockaddr_in servAddr;
    struct sockaddr_in cliAddr;
    char buf[2056]= {0}; // buf is for information received from client
    int sockfd;
    int portNum = 9090;

    if (argc > 2)
    {
        fprintf(stderr, "Usage: ./server <port number>  (default port number is 9090). \n");
        exit(1);
    }
    if (argc == 2)
    {
        portNum = atoi(argv[1]);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        fprintf(stderr, "Creating socket failed\n");
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(portNum);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(servAddr.sin_zero, '\0', sizeof(servAddr.sin_zero));
    if(bind(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        fprintf(stderr, "Bind failed. Port %d is already in use. Usage: ./server <port number>. Default port number is 9090. \n", portNum);
        exit(1);
    }
    if ( listen(sockfd, 100) < 0 )
    {
        fprintf(stderr, "Listen failed\n");
        exit(1);
    }

    //printf("listening...\n");

    socklen_t clientLength = sizeof(cliAddr);
    newfd = accept(sockfd, (struct sockaddr *)&cliAddr, &clientLength);
    if ( newfd < 0)
    {
        fprintf(stderr, "Accept failed\n");
        //continue;
    }
    //printf("Connected...\n");

    while(1)
    {
        if(read(newfd, buf, sizeof(buf)) < 0)
        {
           printf("Reading from client syscall error\n");
           exit(1);
        }
        // printing what client sent
        // printf("%s", buf);
        // checking for GET
        if (buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T')
        {
            get = 1;
        }
        else
        {
            fprintf(stderr, "Not a GET command. \n");
            exit(1);
        }

        // parsing for filename
        char filename[1024] = {0};
        for (int i = 5; i<2056; i++)
        {
            if (buf[i] == ' ')
                break;
            strncat(filename, &buf[i], 1);
        }
        // printf("filename: %s\n", filename);

        // grabbing requested file's contents
        char content[1002000]={0};
        FILE *fp = fopen(filename, "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Unable to open requested file\n");
            exit(1);
        }
        int contentLen = fread(content, sizeof(char), 1002000, fp);

        // stringifying content length
        char strContentLen[12];
        sprintf(strContentLen, "%d", contentLen);
        //printf("content length: %s\n", strContentLen);

        // parsing file type
        char contentType[30] = {0};
        char * token = strtok(filename, ".");
        token = strtok(NULL, ".");
        // token contains the file type found after .
        if (token == NULL)
            sprintf(contentType, "application/octet-stream");
        else if (strcmp("html", token) == 0)
            sprintf(contentType, "text/html");
        else if (strcmp("txt", token) == 0)
            sprintf(contentType, "text/plain; charset=utf-8");
        else if (strcmp("png", token) == 0)
            sprintf(contentType, "image/png");
        else if (strcmp("jpg", token) == 0)
            sprintf(contentType, "image/jpg");
        else
            sprintf(contentType, "application/octet-stream");

        // entire thing to send
        char buffer[1002000]={0};

        // raw html file
        // int hi = sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: Closed\r\n\r\n<html><head></head><body>Hello World!</body></html>\r\n");

        // my html file
        sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Length: %s\r\nContent-Type: %s\r\nConnection: Closed\r\n\r\n", strContentLen, contentType);
        int contentTypeLen = strlen(contentType);
        int strContentLenLen = strlen(strContentLen);
        int totSize = 76 + strContentLenLen + contentTypeLen;
        write(newfd, &buffer, totSize-3);
        write(newfd, &content, contentLen);
        write(newfd, "\r\n", 2);
        // printf("%s", buffer);
        // printf("da content: \n");
        // printf("%s", content);
        close(newfd);
        exit(0);
    }

    return 0;
}
