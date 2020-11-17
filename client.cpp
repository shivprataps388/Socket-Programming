#include<iostream>
using namespace std;
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <pthread.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

//The error() function is identical to that in the server, as are the variables sockfd, portno, and n. The variable serv_addr will contain the address of the server to which we want to connect. It is of type struct sockaddr_in.
void *read_server(void *vargp) 
{ 
    int sockfd=*(int *)vargp;
    char buffer[256];
    int n;
    while(1)
    {
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
        {
             error("ERROR reading from socket");
             return NULL;
        }
        if (strlen(buffer)>0)
        printf("%s",buffer);
    }
} 

void *write_server(void *vargp) 
{ 
    int sockfd=*(int *)vargp;
    char buffer[256];
    int n;
    while(1)
    {
        int num;
        cin>>num;
        // scanf("%d\n" ,&num);
        // getchar(); 
        n = write(sockfd,&num,sizeof(int));
        getchar();
        // Implement logic for connection termination
        if (num<0)
        {
            close(sockfd);
            return NULL;
        }
        if (num==0)
        {
            // cout<<"i am inside the continue block"<<endl;
            continue;
        }
        cout<<"You to Client"<<num<<": ";
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        if (strlen(buffer)>0)
        n = write(sockfd,buffer,255);
        // printf("You: %s",buffer);
        if (n < 0)
        {
            error("ERROR writing to socket");
            break;
        } 
    }
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;
    //The variable server is a pointer to a structure of type hostent. This structure is defined in the header file netdb.h:

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        error("ERROR opening socket");

    //All of this code is the same as that in the server.
  
  server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    //argv[1] contains the name of a host on the Internet. The function: struct hostent *gethostbyname(char *name) Takes such a name as an argument and returns a pointer to a hostent containing information about that host. The field char *h_addr contains the IP address. If this structure is NULL, the system could not locate a host with this name.
    // The mechanism by which this function works is complex, often involves querying large databases all around the country.


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);
    //This code sets the fields in serv_addr. Much of it is the same as in the server. However, because the field server->h_addr is a character string, we use the function: void bcopy(char *s1, char *s2, int length) which copies length bytes from s1 to s2.
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    //The connect function is called by the client to establish a connection to the server. It takes three arguments, the socket file descriptor, the address of the host to which it wants to connect (including the port number), and the size of this address. This function returns 0 on success and -1 if it fails.

    //Notice that the client needs to know the port number of the server, but it does not need to know its own port number. This is typically assigned by the system when connect is called.
    pthread_t write_thread,read_thread;
    pthread_create(&write_thread, NULL, write_server, (void *)&sockfd); // this thread is used for continuous writing to server
    pthread_create(&read_thread, NULL, read_server, (void *)&sockfd); // this thread is used for continuous reading from server
    pthread_join(read_thread, NULL);  
    return 0;
    // The remaining code should be fairly clear. It prompts the user to enter a message, uses fgets to read the message from stdin, writes the message to the socket, reads the reply from the socket, and displays this reply on the screen.

}
