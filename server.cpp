
#include<iostream>
#include <iterator> 
#include <unordered_map> 
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;

#define MAX_CLIENTS 10
#define MAX_SIZE 300
struct pass_info
{
    int hash_key;
    int hash_value;
    pass_info(int a,int b)
    {
        hash_key=a;
        hash_value=b;
    }
};

void* dostuff (void*sock); /* function prototype */
void error(char *msg)
{
    perror(msg);
    exit(1);
}
unordered_map<int,int>hash_map; // to keep track of connected threads

void print_hash()
{
    cout<<"hashmap is "<<endl;
    for(auto it=hash_map.begin();it!=hash_map.end();it++)
        cout<<it->first<<" "<<it->second<<endl;
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // used to simulate mutex

int main(int argc, char *argv[])
{
     int sockfd, portno, pid;
     socklen_t  clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,MAX_CLIENTS);
     clilen = sizeof(cli_addr);
     
     pthread_t *thread_group = (pthread_t*)malloc(sizeof(pthread_t) * MAX_CLIENTS);
     

     // initalizing with -1 represnting everything is empty.
     for(int i=1;i<=MAX_CLIENTS;i++)
        hash_map[i]=-1;

     while (1) {

         int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
         if (newsockfd < 0) 
             error("ERROR on accept");
        int count_client=1;
        for(count_client;count_client<MAX_CLIENTS;count_client++)
        {
            if (hash_map[count_client]<0)
                break;
        }

        // mutex lock
        pthread_mutex_lock(&mutex);
        hash_map[count_client]=newsockfd;  //changing hashmap for client 
        pass_info p(count_client,newsockfd); // information of new client
        printf("New Connection from client %s:%d: \n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        pthread_create(&thread_group[count_client-1], NULL, dostuff, (void *)&p); // new thread is created and assigned to client
        pthread_mutex_unlock(&mutex);
        //mutex unlock


     } /* end of while */

     return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
string get_hash_info()
{
    string s="Enter Client you want to talk to : \n";
    // s+=to_string(pthread_self())+" \n";
       
    //mutex thing
    pthread_mutex_lock(&mutex);
    for(auto it=hash_map.begin();it!=hash_map.end();it++)
    {
        if (it->second>0)
            s+=to_string(it->first)+"\n";
    }
    
    pthread_mutex_unlock(&mutex);
    // mutex thing

    return s;
}

void* dostuff (void*sock)
{

   int n,socket;
   pass_info p=*(pass_info*)sock;
   socket=p.hash_value;
   cout<<p.hash_key<<p.hash_value<<endl;
   string s="Enter 0 to get list of connected clients.\n";
   s+="Enter -1 to terminate connection.\n";
   char char_array[MAX_SIZE];
   strcpy(char_array, s.c_str());
   n = write(socket,char_array,strlen(char_array));

   while(1)
   {
       int num=-1;
       n = read(socket,&num,sizeof(int));
       printf("Client %d said:%d\n",p.hash_key,num);                 //Read No choice
       if (num<0||n<0||num>MAX_CLIENTS)
       {
            close(socket);
            cout<<"socket closed"<<endl;

            // mutex lock
            pthread_mutex_lock(&mutex);
            hash_map[p.hash_key]=-1;     // changed the hash value, -1 denotes this thread is free and use it
            pthread_mutex_unlock(&mutex);
            // mutex unlock

            return NULL;
       }
       if (num==0)
       {
        s=get_hash_info();
        bzero(char_array,MAX_SIZE);
        strcpy(char_array, s.c_str());
        n = write(socket,char_array,strlen(char_array));
        continue;
       }

       char buffer[256];
       bzero(buffer,256);
       n = read(socket,buffer,255);
       printf("Message from client is of length %d is %s\n", strlen(buffer) ,buffer);
       
       if (n < 0) 
        {
            // cout<<"WTF man"<<endl;
            error("ERROR reading from socket");
        }
       // printf("Here is the message: %s\n",buffer);
       int rcvr_id=-1;
       
       // mutex lock
       pthread_mutex_lock(&mutex);
       if (hash_map.find(num)!=hash_map.end())
        rcvr_id=hash_map[num];
        pthread_mutex_unlock(&mutex);
       // mutex unlock
        
        cout<<"reciever id is of "<<num<<" is "<<rcvr_id<<endl;
        if (rcvr_id<0)
        {
            n = write(socket,"Connection was terminated by requested client\n",strlen("Connection was terminated by requested client\n"));
        }
        else if (strlen(buffer)>0)
        {
            // hash_key said buffer
            char msg_prefix[MAX_SIZE];
            sprintf(msg_prefix,"Cleint %d: ",p.hash_key);
            strcat(msg_prefix,buffer);
           cout<<"trying to write "<< buffer<<" in "<<rcvr_id<<endl;
           n = write(rcvr_id,msg_prefix,strlen(msg_prefix));
           if (n<0)
           {
                n = write(socket,"Error occurred",strlen("Error occurred"));
                if (n < 0) error("ERROR writing to socket");
           }
           cout<<"successful"<<endl;
        }
   }
   
}