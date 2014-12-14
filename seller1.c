/*
 ** SELLER1.c --
 In this file the seller#1 is sending authentication data and various items and prices which he/she wants to sell.
 Flow:
 
 1) connServer_phase1 -- Connect To Server PHASE 1 , send the login information and decides whether seller is accepted or rejected. If rejected , will be idle for next phases otherwise send data in the next phase.
 
 2) connServer_phase2  -- if seller is accepted this phase will be executed and the seller will send the item and price information
 
 3) finishphase3 - this phase is will open a tcp connection and start listening for any incoming connection on specific port.
 
 NOTE: After Phase1 is completed there is a delay of 30sec so that the server completes phase1 during that time span.
 
 
 
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

int connServer_phase2();
int finishphase3();

/*#define filepath2 "/Users/lavishsingal/desktop/socket_programming/inputfiles/test/sellerPass1.txt"
#define itemlist1 "/Users/lavishsingal/desktop/socket_programming/inputfiles/test/itemList1.txt"
*/

/// FOR NUNKI/////
#define filepath2 "sellerPass1.txt"
#define itemlist1 "itemList2.txt"


#define PORT "1253" // the PORT client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define MAXITEMS 10
#define IPSTR "nunki.usc.edu"

//////////////// SELLER PORT - PHASE3 /////////////////
#define SELLPORT1 "2253"


char buf[MAXDATASIZE];
char broadcastItems[MAXDATASIZE*10]="";

char ip_add_2[10]="",port_2[5]="";
char name[10] , pwd[10] , bankno[10];

//user info variables
char name[10] , pwd[10] , bankno[10], type[2];

// for bidders
char biditems[MAXDATASIZE]="";

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
        puts("cannot open File sellerPass1.txt");
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
    printf("\nEnd of Phase 3 for <Seller#1>.\n");
    
    
}



///////////////////////////////////////////////////////////
///////////////  ACCEPTED  FOR SELLER /////////////////////
//////////////////////////////////////////////////////////

int c_accepted(){
    char * info;
    int inc = 0, item =0;
    char command[15]="", c_name[10]="" ;
    FILE *ipath;
    
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
        //   printf ("%s\n",info);
        inc++;
        info = strtok (NULL, " #");
    }
    
    
    //   printf("%s %s %s------ from accepted()",command,ip_add_2,port_2);
    if(strcmp("ACCEPTED",command) == 0){
        //load the file itemlist1.txt
        ipath = fopen(itemlist1, "r");
        fscanf(ipath,"%s",c_name);
        while(fscanf(ipath,"%s %s",i_info[item].i_name,i_info[item].i_price)!=EOF){
            //         printf("%s %s \n",i_info[item].i_name,i_info[item].i_price);
            item++;
        }
        return 1;
        
    }
    return 0;
    
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
    
    // printf("seller2 : port number %hu \n", sa.sin_port);
    //convert the IP address to string format
    inet_ntop(sa.sin_family, &(sa.sin_addr), ipstr, INET_ADDRSTRLEN);
    
    // printf("seller2 : IP address of Client %s \n", ipstr);
    //lavish
    printf("\nPhase 1: <Seller#1> has TCP port %hu and IP address: %s\n",sa.sin_port,ipstr);
    
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    //  printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    
    //lavish
    printf("\nPhase 1: Login request. User: %s password: %s Bank account: %s \n",name,pwd,bankno);
    
    if (send(sockfd, login, strlen(login)+1, 0) == -1){
        perror("send");
        exit(1);
    }
    else{
        //  printf("\nsent info\n");
    }
    
    
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    
    else{
        buf[numbytes] = '\0';
        //    printf("client: received '%s'\n",buf);
        // parse it and start the phase 2 --lavish
        if(c_accepted()){
            //send the data to the server
            //lavish
            printf("\nPhase 1: Login request reply: ACCEPTED.\n");
            printf("\nPhase 1: Auction Server has IP Address %s and PreAuction TCP Port Number %s\n", ip_add_2,port_2);
            printf("\nEnd of Phase 1 for <Seller#1>.\n");
            close(sockfd);
            connServer_phase2();
            return 0;
        }
        else{
            // lavish
            printf("\nPhase 1: Login request reply: REJECTED.\n");
            printf("\nEnd of Phase 1 for <Seller#1>.\n");
            close(sockfd);
        }
    }
    //printf("%s",login);
    
    close(sockfd);
    return 0;
}


///////////////////////////////////////////////////////////
///////////////  Connect To Server PHASE2 /////////////////
//////////////////////////////////////////////////////////

int connServer_phase2(){
    int sockfd = -1;
    struct addrinfo hints, *servinfo, *p;
    int rv,addrlen,getsock_check,snd=0;
    struct sockaddr_in sa;
    char s[INET6_ADDRSTRLEN];
    char ipstr[INET_ADDRSTRLEN];
    char items[MAXDATASIZE]="";
    
    sleep(30);
    
    printf("\nPhase 2: Auction Server IP Address: %s PreAuction Port Number: %s .\n",ip_add_2,port_2);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(ip_add_2, port_2, &hints, &servinfo)) != 0) {
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
    
    // printf("seller2 : port number %hu \n", sa.sin_port);
    //convert the IP address to string format
    inet_ntop(sa.sin_family, &(sa.sin_addr), ipstr, INET_ADDRSTRLEN);
    
    //   printf("seller2 : IP address of Client %s \n", ipstr);
    
    
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    //  printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    
    while(strcmp(i_info[snd].i_name,"")){
        strcat(items, name);
        strcat(items, " ");
        strcat(items,i_info[snd].i_name);
        strcat(items," ");
        strcat(items,i_info[snd].i_price);
        strcat(items,"\n");
        //  printf("%s\n",items);
        snd++;
        
    }
    
    printf("\nPhase 2: <Seller#1> send item lists.\nPhase 2: (Item list displayed here)\n%s\n",items);
    
    //    printf("%s /n",items);
    if(send(sockfd,items,strlen(items)+1,0 )== -1)
        perror("send");
    
    
    close(sockfd);
    printf("\nEnd of Phase 2 for <Seller#1>.\n");
    
    /* start listening to get the last responce from the server */
    
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
    
    if ((rv = getaddrinfo(IPSTR, SELLPORT1, &hints, &servinfo)) != 0) {
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
            
            if((numbytes = recv(new_fd,buf,MAXDATASIZE*10,0)) == -1)
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
    
    printf("\nEnd of Phase 3 for <Seller#1>.\n");
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

