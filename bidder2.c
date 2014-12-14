/*
 ** BIDDER2.c --
 In this file the bidder#2 is sending authentication data and various items and prices which he/she wants to sell.
 Flow:
 
 1) connServer_phase1 -- Connect To Server PHASE 1 , send the login information and decides whether bidder is accepted or rejected. If rejected , will be idle for next phases otherwise send data in the next phase.
 
 2) listenserver  -- This is the start for phase-3 where the bidder is listening to the server for item broadcast on UDP connection
 
 3) finishphase3 - this phase is will open a tcp connection and start listening for any incoming connection on specific port.
 
 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>



///////////////////////////////////////////////////////////
///////////////  GLOBAL VARIABLES /////////////////////////
//////////////////////////////////////////////////////////



/*#define bidding1 "/Users/lavishsingal/desktop/socket_programming/inputfiles/test/bidding2.txt"
#define filepath2 "/Users/lavishsingal/desktop/socket_programming/inputfiles/test/bidderPass2.txt"
*/


/// FOR NUNKI/////
#define filepath2 "bidderPass2.txt"
#define bidding1 "bidding2.txt"


#define PORT "1253" // the PORT client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define MAXITEMS 10
#define IPSTR "nunki.usc.edu"


//////////////// BIDDER PORT - PHASE3 /////////////////
#define BIDTPORT1 "4353"  // tcp ports where bidder is listening for final data
#define BIDUPORT1 "3353"


char buf[MAXDATASIZE];
char broadcastItems[MAXDATASIZE*10]="";

char ip_add_2[10]="",port_2[5]="";
char name[10] , pwd[10] , bankno[10];

//user info variables
char name[10] , pwd[10] , bankno[10], type[2];

// for bidders
char biditems[MAXDATASIZE]="";
char displaybiditems[MAXDATASIZE] ="";

struct item_info{
    char i_name[15];
    char i_price[10];
    
};
struct item_info i_info[MAXITEMS];


///////////////////////////////////////////////////////////
///////////////  GET SOCKET ADDRESS ///////////////////////
//////////////////////////////////////////////////////////

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


///////////////////////////////////////////////////////////
///////////////  FILLINFO ////////////////////////////////
//////////////////////////////////////////////////////////

char *fillInfo(){
    //char login[50] = "Login#";
    
    char * login = (char *) malloc (50* sizeof(char));
    
    FILE *fp;
    fp = fopen(filepath2, "r");
    
    
    if(fp == NULL){
        puts("cannot open File bidderPass2.txt");
        exit(EXIT_FAILURE);
    }
    
    fscanf(fp,"%s %s %s %s",type ,name , pwd , bankno);
    
    //  printf("type: %s",type);
    
    strcat(login,"Login#");
    strcat(login,type);
    strcat(login," ");
    strcat(login,name);
    strcat(login," ");
    strcat(login,pwd);
    strcat(login, " ");
    strcat(login,bankno);
    
    
    fclose(fp);
    return login;
    
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Display the final result of items and prices sent by the auction server////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void displayresult(){
    
    char *final;
    int inc =0;
    char itm[10] ="",prc[10]="";
    
    final = strtok(buf," \n");
    while(strtok){
        if( inc == 0)
            strcpy(itm,final);
        if(inc == 1){
            strcpy(prc,final);
            printf("\nPhase 3: Item %s was sold at price %s.\n",itm,prc);
            inc = 0;
            final = strtok (NULL, " \n");
            continue;
        }
        inc++;
        final = strtok (NULL, " \n");
        
    }
    printf("\nEnd of Phase 3 for <Bidder#2>.\n");
    
    
}



///////////////////////////////////////////////////////////
///////////////  ACCEPTED  FOR Bidder /////////////////////
//////////////////////////////////////////////////////////

int c_accepted(){
    char * info;
    int inc = 0;
    char command[15]="";
    
    //  printf("c_accepted called");
    info = strtok (buf," #");
    while (info != NULL)
    {
        if( inc == 0)
            strcpy(command,info);
        if( inc == 1)
            strcpy(ip_add_2,info);
        if(inc == 2)
            strcpy(port_2, info);
        inc++;
        info = strtok (NULL, " #");
    }
    
    if(strcmp("ACCEPTED",command) == 0){
        
        return 1;
        
    }
    return 0;
    
}

///////////////////////////////////////////////////////////
///////////////  BIDDING BY BIDDER ////////////////////////
//////////////////////////////////////////////////////////

int bidforItems(){
    
    FILE *bid1;
    bid1 = fopen(bidding1, "r");
    char s_name[15]="",bi_name[15]="",bi_price[15]="";
    
    if(bid1 == NULL ){
        puts("cannot open File bidderPass1.txt");
        exit(EXIT_FAILURE);
    }
    
    while(fscanf(bid1,"%s %s %s",s_name,bi_name,bi_price) != EOF){
        // name is temporary added
        strcat(biditems,name);
        strcat(biditems," ");
        strcat(biditems,s_name);
        strcat(displaybiditems,s_name);
        strcat(biditems," ");
        strcat(displaybiditems," ");
        strcat(displaybiditems,bi_name);
        strcat(biditems,bi_name);
        strcat(displaybiditems," ");
        strcat(biditems," ");
        strcat(displaybiditems,bi_price);
        strcat(biditems,bi_price);
        strcat(biditems,"\n");
        strcat(displaybiditems,"\n");
        
    }
    
    return 1;
    
}


///////////////////////////////////////////////////////////
///////////////  Connect To Server PHASE 1/////////////////
//////////////////////////////////////////////////////////

int connServer_phase1(int argc , const char * argv[] ,char* login){
    int sockfd = -1, numbytes=0;
    struct addrinfo hints, *servinfo, *p;
    int rv,addrlen,getsock_check;
    struct sockaddr_in sa;
    char s[INET6_ADDRSTRLEN];
    char ipstr[INET_ADDRSTRLEN];
    char *command ;
    
    
    command = (char*) malloc(sizeof(char)*11); // ACCEPETED#
    
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(IPSTR, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue; }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    
    
    //Retrieve the locally-bound name of the specified socket and store it in the sockaddr
    addrlen = sizeof(sa);
    getsock_check=getsockname(sockfd,(struct sockaddr *)&sa, (socklen_t *)&addrlen) ;
    //Error checking
    if (getsock_check== -1) {
        perror("getsockname");
        exit(1);
    }
    
    //convert the IP address to string format
    inet_ntop(sa.sin_family, &(sa.sin_addr), ipstr, INET_ADDRSTRLEN);
    
    printf("\nPhase 1: <Bidder#2>  has TCP port: %hu and IP address: %s\n",sa.sin_port,ipstr);
    
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    
    //  printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    
    printf("\nPhase 1: Login request. User: %s password: %s Bank account: %s \n",name,pwd,bankno);
    
    if (send(sockfd, login, strlen(login)+1, 0) == -1){
        perror("send");
        exit(1);
    }
    
    
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    
    else{
        buf[numbytes] = '\0';
        if(c_accepted()){
            printf("\nPhase 1: Login request reply: ACCEPTED.\n");
            printf("\nEnd of Phase 1 for <Bidder#2>.\n");
            close(sockfd);
			// start listening to the server for incoming broadcast info
			listenserver();
            return 0;
        }
        else{
            printf("\nPhase 1: Login request reply: REJECTED.\n");
            printf("\nEnd of Phase 1 for <Bidder#2>.\n");
            exit(1);
        }
    }
    
    close(sockfd);
    return 0;
}


///////////////////////////////////////////////////////////
///////////////  LISTEN TO THE SERVER ////////////////////
//////////////////////////////////////////////////////////

int listenserver()
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes,addrlen,getsock_check;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in sa;
    
    
    
    
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    if((rv = getaddrinfo(NULL,BIDUPORT1,&hints,&servinfo)) !=0){
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
        return 1;
    }
    
    
    for(p = servinfo ; p != NULL ; p = p->ai_next){
        if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
            perror("listener: socket");
            continue;
        }
        
        
        if(bind(sockfd, p->ai_addr,p->ai_addrlen) == -1){
            close(sockfd);
            perror("Bidder Listerner : socket" );
            continue;
        }
        
        break;
    }
    
    //Retrieve the locally-bound name of the specified socket and store it in the sockaddr
    addrlen = sizeof(sa);
    getsock_check=getsockname(sockfd,(struct sockaddr *)&sa, (socklen_t *)&addrlen) ;
    //Error checking
    if (getsock_check== -1) {
        perror("getsockname");
        exit(1);
    }
    
    //convert the IP address to string format
    inet_ntop(sa.sin_family, &(sa.sin_addr), ipstr, INET_ADDRSTRLEN);
    
    printf("\nPhase 3: <Bidder#2>  has UDP port %s and IP address: %s \n",BIDUPORT1,ipstr);
    
    
    if(p == NULL){
        fprintf(stderr,"listener: failed to bind socket");
        return 2;
        
    }
    
    
    freeaddrinfo(servinfo);
    
    
    addr_len = sizeof their_addr;
    if((numbytes = recvfrom(sockfd,broadcastItems,MAXDATASIZE*10-1,0,(struct sockaddr*)&their_addr,&addr_len)) == -1){
        perror("recvfrom");
        exit(1);
    }
    else{
        printf("\nPhase 3: (Item list displayed here)\n%s\n", broadcastItems);
        
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        
        //save the ip address of the server
        broadcastItems[numbytes] = '\0';
    }
    
    //bidder will send the items which he wants to bid
    bidforItems();
    
    //send the bidding info to the server
    if((numbytes = sendto(sockfd,biditems,strlen(biditems),0,(struct sockaddr *)&their_addr,addr_len)) == -1){
        perror("sendto");
        close(sockfd);
        exit(1);
    }
    else{
        printf("\nPhase 3: <Bidder#2> (Bidding information displayed here)\n%s\n",displaybiditems);
    }
    
    close(sockfd);
	// start listening for the final decision
	finishphase3();
    
    return 0;
    
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////  This function has created a socket and start listening to the server to receive the final decision /////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int finishphase3(){
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1 , numbytes =0 ,addrlen=0,getsock_check=0;
    char s[INET6_ADDRSTRLEN];
    int rv;
    pid_t pid =-1;
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in sa_ip;
    
    
    
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(IPSTR, BIDTPORT1, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        
        break;
    }
    
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        exit(1);
    }
    
    addrlen = sizeof(sa_ip);
    getsock_check=getsockname(sockfd,(struct sockaddr *)&sa_ip, (socklen_t *)&addrlen) ;
    //Error checking
    if (getsock_check== -1) {
        perror("getsockname");
        exit(1);
    }
    
    inet_ntop(sa_ip.sin_family, &(sa_ip.sin_addr), ipstr, INET_ADDRSTRLEN);
    
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue; }
        
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        
        
        pid = fork();
        
        if (!pid) { // this is the child process
            close(sockfd); // child doesn't need the listener
            
            if((numbytes = recv(new_fd,buf,MAXDATASIZE,0)) == -1)
                perror("recv");
            else{
                buf[numbytes] = '\0';
                displayresult();
            }
            
            
            close(new_fd);
            exit(0);
        }
        close(new_fd);
        break;
        
    }
    
    
    return 0;
    
}


///////////////////////////////////////////////////////////
///////////////  MAIN ////////////////////////////////
//////////////////////////////////////////////////////////

int main(int argc, const char * argv[])
{
    char * login;
    int value = -1;
    login = fillInfo();
    value = connServer_phase1(argc , argv ,login);
    
    return 0;
}

