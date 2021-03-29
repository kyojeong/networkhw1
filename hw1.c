#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>



void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void tcpserver(char *ip,int port)
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[1024];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = port;
     serv_addr.sin_family = AF_INET;
     //serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_addr.s_addr = inet_addr(ip);//connect ip
     serv_addr.sin_port = htons(portno);//connect port
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");

     
     listen(sockfd,5);//wait client
     clilen = sizeof(cli_addr);
    
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);
     if (newsockfd < 0)
          error("ERROR on accept");
     bzero(buffer,1024);

     //begin receive message
     while(n=read(newsockfd,buffer,1023)>0){
     	//n = read(newsockfd,buffer,255);
     	if (n < 0) error("ERROR reading from socket");
     	//printf("Here is the message: %s\n",buffer);
     	n = write(newsockfd,"I got your message",18);//return a message to client
     	if (n < 0) error("ERROR writing to socket");
	bzero(buffer,1024);
     }
     close(newsockfd);
     close(sockfd);
     

}

void tcpclient(char *ip,int port,char *file)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[1024];
    portno = port;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //bcopy((char *)server->h_addr,
     //    (char *)&serv_addr.sin_addr.s_addr,
     //    server->h_length);
    serv_addr.sin_addr.s_addr=inet_addr(ip);//connect ip
    serv_addr.sin_port = htons(portno);//connect port
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    FILE *fp=fopen(file,"r");//open test file
    struct stat sb;

    //check file size
    if(stat(file,&sb)==-1){
	    perror("stat");
	    exit(EXIT_FAILURE);
    }
    

    time_t now= time(0);//catch time data
    int start=now;
    char *dt=ctime(&now);
    printf("0%% %s",dt);//print begin time

    int filesize=sb.st_size;
    int nowsize=0;
    int check1=0,check2=0,check3=0;
    bzero(buffer,1024);

    //start to send message to server
    while(fgets(buffer,1023,fp)!=NULL){
	nowsize=nowsize+strlen(buffer);//record the process of sending
    	//printf("Please enter the message: %s",buffer);
    	n = write(sockfd,buffer,strlen(buffer));

	//check process of sending
	//25%
	if(nowsize>=filesize/4 && check1==0){
                now=time(0);
                dt=ctime(&now);
                printf("25%% %s",dt);
                check1=1;
        }

	//50%
        else if(nowsize>=filesize/2 && check2==0){
                now=time(0);
                dt=ctime(&now);
                printf("50%% %s",dt);
                check2=1;
        }

	//75%
        else if(nowsize>=filesize/4*3 && check3==0){
                now=time(0);
                dt=ctime(&now);
                printf("75%% %s",dt);
                check3=1;
        }

    	if (n < 0)
        	error("ERROR writing to socket");
    	bzero(buffer,1024);
    	n = read(sockfd,buffer,1023);
    	if (n < 0)
        	error("ERROR reading from socket");
    //	printf("%s\n",buffer);
    }
    now=time(0);
    int end= now;//get end time
    dt=ctime(&now);
    printf("100%% %s",dt);
    printf("Total trans time: %d s\n",end-start);
    printf("file size: %lu MB\n",sb.st_size/1000000);
    close(sockfd);


}

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void echo_ser(int sock)
{
    char recvbuf[1024] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n;

    while (1)
    {

        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));

	//receive message from client
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,
                     (struct sockaddr *)&peeraddr, &peerlen);
        if (n == -1)
        {

            if (errno == EINTR)
                continue;

            ERR_EXIT("recvfrom error");
        }
        else if(n > 0)
        {

            //fputs(recvbuf, stdout);
	    //return massage to client
            sendto(sock, recvbuf, n, 0,
                   (struct sockaddr *)&peeraddr, peerlen);
        }
    }
    close(sock);
}

void udpserver(char *ip,int port)
{
    int sock;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket error");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    //servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");

    echo_ser(sock);

}

void udpclient(char *ip,int port,char *file)
{
    int sock;
    time_t now= time(0);
    int start=now;
    char *dt=ctime(&now);

    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);//connect port
    servaddr.sin_addr.s_addr = inet_addr(ip);//connect ip

    int ret;
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};
    
    FILE *fp=fopen(file,"r");//open test file
    struct stat sb;
    if(stat(file,&sb)==-1){
            perror("stat");
            exit(EXIT_FAILURE);
    }

    int filesize=sb.st_size;//get file size
    int nowsize=0,recvsize=0;
    int check1=0,check2=0,check3=0;
    printf("0%% %s",dt);
    
    //start to send message
    while (fgets(sendbuf, sizeof(sendbuf), fp) != NULL)
    {
	nowsize=nowsize+strlen(sendbuf);//record process of sending
	//fgets(sendbuf, sizeof(sendbuf), stdin); 
        sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	
	//25%
	if(nowsize>=filesize/4 && check1==0){
                now=time(0);
                dt=ctime(&now);
                printf("25%% %s",dt);
                check1=1;
        }

	//50%
        else if(nowsize>=filesize/2 && check2==0){
                now=time(0);
                dt=ctime(&now);
                printf("50%% %s",dt);
                check2=1;
        }

	//75%
        else if(nowsize>=filesize/4*3 && check3==0){
                now=time(0);
                dt=ctime(&now);
                printf("75%% %s",dt);
                check3=1;
        }

        ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        if (ret == -1)
        {
            if (errno == EINTR)
               // continue;
            ERR_EXIT("recvfrom");
        }
	//printf("receive: ");
        //fputs(recvbuf, stdout);
	recvsize=recvsize+strlen(recvbuf);//record the success of sending
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    now=time(0);
    int end= now;//get end time
    dt=ctime(&now);
    printf("100%% %s",dt);
    printf("Total trans time: %d s\n",end-start);
    printf("file size: %lu MB\n",sb.st_size/1000000);

    float loss=1-recvsize/nowsize;//get loss of sending
    printf("packet loss: %.0f %%\n",loss*100);

    close(sock);

}

int main(int argc, char *argv[])
{
	if(strcmp(argv[1],"tcp")==0){
		if(strcmp(argv[2],"send")==0){
			tcpclient(argv[3],atoi(argv[4]),argv[5]);
		}
		else if(strcmp(argv[2],"recv")==0){
			tcpserver(argv[3],atoi(argv[4]));
		}
	}
	else if(strcmp(argv[1],"udp")==0){
		if(strcmp(argv[2],"send")==0){
                        udpclient(argv[3],atoi(argv[4]),argv[5]);
                }
                else if(strcmp(argv[2],"recv")==0){
                        udpserver(argv[3],atoi(argv[4]));
                }

	}
}
