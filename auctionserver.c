/*
 ** AUCTIONSERVER.c --
 
 This file is acting as an auction server which handles various requests from the sellers and bidders.
 Flow of this file:
 
 1) s_initSocket - This function will intialize the phase-1 and verify which users are accepted and which are rejected
 
 2) s_initSocketP2 - this function will start listening to a port on which the sellers will send their items and prices
 
 3) sendItemtoBidder1 --This function will send the items ,prices and seller name for bidding to bidder1
 
 4)  sendItemtoBidder2 --  This function will send the items ,prices and seller name for bidding to bidder2
 
 5) sendfinalbid - This function will send the packet containig items and prices to well known ports of clients
 
 6) startnextphases - start phase 2 (for sellers )and phase 3 (for bidders)
 
 7) notifyall -  This function will notify all the users after bidding is complete
 
 */



////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////GLOBAL VARIABLES////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

/* #define FILENAME "/Users/lavishsingal/Desktop/Socket_Programming/InputFiles/test/Registration.txt"
 #define BROADCAST "/Users/lavishsingal/Desktop/Socket_Programming/InputFiles/test/broadcast.txt"
 #define BIDDERFILE "/Users/lavishsingal/Desktop/Socket_Programming/InputFiles/test/bidderall.txt"
 #define AUTHSELLERS "/Users/lavishsingal/Desktop/Socket_Programming/InputFiles/test/authsellers.txt"
 #define AUTHBIDDERS "/Users/lavishsingal/Desktop/Socket_Programming/InputFiles/test/authbidders.txt"
 #define TEMP "/Users/lavishsingal/Desktop/Socket_Programming/InputFiles/test/temp.txt"
 */
#define FILENAME "Registration.txt"
#define BROADCAST "broadcast.txt"
#define BIDDERFILE "bidderall.txt"
#define AUTHSELLERS "authsellers.txt"
#define AUTHBIDDERS "authbidders.txt"
#define TEMP "temp.txt"



#define PORT1 "1253"  // the port users will be connecting to for phase1
#define PORT2 "1353"  // port for phase 2
#define PORTBID1 "3253" // port where the server will send the data to the bidder#1
#define PORTBID2 "3353" // port where the server will send the data to the bidder#2

#define PORTP3BID1 "4253" // port where the bidder#1 is listening for incomming connection from server (tcp)
#define PORTP3BID2 "4353" // port where the bidder#2 is listening for incomming connection from server (tcp)
#define PORTP3SEL1 "2253"// port where the seller#1 is listening for incomming connection from server (tcp)
#define PORTP3SEL2 "2353" // port where the seller#2 is listening for incomming connection from server (tcp)

#define BACKLOG 10	 // how many pending connections queue will hold

#define IPADDR "nunki.usc.edu"
#define MAXDATASIZE 100
#define MAXITEMS 10
#define NO_OF_USERS 20

char buf[MAXDATASIZE];
char broadcastItems[MAXDATASIZE*10] = "";
char bidderdata[MAXDATASIZE*10] = "";

int no_of_users =0,count=0;
int total_sellers = 0;
int total_bidders = 0;
int total_items = 0;
char command[6], u_info[10], pwd[10], bankno[10] , type[2];

struct soldinfo{
    char b_name[15];
    char s_name[15];
    char item[10];
    char price[10];
};

struct soldinfo i_sold[MAXITEMS];

struct s_items{
    char name[10];
    char iname[10];
    char iprice[10];
};

struct s_items o_items[NO_OF_USERS];


struct s_info{
    char name[10];
    char pwd[10];
    char bankno[10];
};

struct s_info o_info[MAXITEMS];


char bs_name[15]="",ss_name[15]="",is_name[10]="",is_price[5]="";
char finalbidding[MAXDATASIZE*10] ="";




///////////////////////////////////////////////////////////
/////////////// REAPING THE ZOMBIE PROCESSES//////////////////
//////////////////////////////////////////////////////////

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

///////////////////////////////////////////////////////////
///////////////  CHECK USER AUTHENTICITY /////////////////////
//////////////////////////////////////////////////////////


int authenticate_user(char *s){
    char * info;
    int inc = 0,j=0;
    FILE *s_user,*b_user;
    
    if ( ( s_user = fopen(AUTHSELLERS,"a") ) == NULL)
    {
        printf("Sorry, cannot open the file %s","authsellers.txt");
        return -1;
    }
    
    if ( ( b_user = fopen(AUTHBIDDERS,"a") ) == NULL)
    {
        printf("Sorry, cannot open the file %s","authbidders.txt");
        return -1;
    }
    
    
    info = strtok (buf," #");
    while (info != NULL)
    {
        if( inc == 0)
            strcpy(command,info);
        if( inc == 2)
            strcpy(u_info,info);
        if(inc == 3)
            strcpy(pwd, info);
        if(inc == 4)
            strcpy(bankno,info);
        if(inc == 1)
            strcpy(type,info);
        inc++;
        info = strtok (NULL, " #");
    }
    
    if(strcmp(type, "2") == 0){ //seller
        fprintf(s_user,"%s %s\n",u_info,s);
        total_sellers++;
        
    }
    else{ //bidder
        fprintf(b_user,"%s %s\n",u_info,s);
        total_bidders++;
    }
    
    //  printf("%s %s %s %s %s------ from auth",command,type,u_info,pwd,bankno);
    if(strcmp("Login",command) == 0){
        for(j=0 ;j<NO_OF_USERS;j++){
            if((strcmp(u_info,o_info[j].name)==0) && (strcmp(pwd,o_info[j].pwd) == 0) && (strcmp(bankno,o_info[j].bankno) == 0)){
                return j;
                
            }
        }
    }
    fclose(s_user);
    fclose(b_user);
    
    return -1;
    
}

//////////////////////////////////////////////////////////////////////////////////////
///////////////  add items to the file which will be broadcasted later /////////////////////
/////////////////////////////////////////////////////////////////////////////////////

int addItemToFile(){
    FILE *stream;
    
    if ( ( stream = fopen(BROADCAST,"a") ) == NULL)
    {
        printf("Sorry, cannot open the file %s","broadcast.txt");
        return -1;
    }
    else
    {
        fprintf(stream, "%s", buf);
    }
    
    fclose(stream);
    return 1;
    
    
}

///////////////////////////////////////////////////////////////////////////////////
///////////////  it will add the items received from the bidder  /////////////////////
//////////////////////////////////////////////////////////////////////////////////

int addBidderItem(){
    FILE *stream;
    
    
    if ( ( stream = fopen(BIDDERFILE,"a") ) == NULL)
    {
        printf("Sorry, cannot open the file %s",BIDDERFILE);
        return -1;
    }
    else
    {
        fprintf(stream, "%s", bidderdata);
    }
    
    fclose(stream);
    
    
    return 1;
    
    
}


///////////////////////////////////////////////////////////
///////////////  READ ITEMS FROM FILE BROADCAST ///////////
//////////////////////////////////////////////////////////

int readTheItems(){
    
    FILE *item;
    int i=0;
    
    item = fopen(BROADCAST, "r");
    
    
    if(item == NULL){
        puts("Sellers has not yet ");
        exit(1);
    }
    
    
    while(fscanf(item," %s %s %s",o_items[i].name,o_items[i].iname,o_items[i].iprice)!= EOF){
        strcat(broadcastItems, o_items[i].name);
        strcat(broadcastItems, " ");
        strcat(broadcastItems, o_items[i].iname);
        strcat(broadcastItems, " ");
        strcat(broadcastItems, o_items[i].iprice);
        strcat(broadcastItems,"\n");
        i++;
    }
    total_items=i;
    
    fclose(item);
    
    return 1;
    
    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////  It will fill the required info in the structure for valid users from registration.txt////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void s_fillInfo(){
    
    FILE *fp;
    
    
    fp = fopen(FILENAME, "r");
    
    if(fp == NULL){
        puts("cannot open File registration.txt");
        exit(1);
    }
    
    while(fscanf(fp," %s %s %s",o_info[no_of_users].name,o_info[no_of_users].pwd,o_info[no_of_users].bankno)!= EOF){
        no_of_users++;
    }
    
    fclose(fp);
    
}


///////////////////////////////////////////////////////////////////////////
/////// COMAPARE THE ITEMS to maximize the profit AND BROADCAST THEM ////////////
//////////////////////////////////////////////////////////////////////////////

void compareItemsAndBroadcast(){
    
    FILE *bidderall,*item , *temp;
    int select =0,ret=-1;
    int bidderPrice = 0 , sellerPrice = 0;
    char s_name[15]="",s_item[15]="",s_price[15]="";
    
    
    item = fopen(BROADCAST, "r");
    
    
    if(item == NULL){
        puts("cannot open the file broadcast.txt ");
        exit(1);
    }
    
    
    
    while(fscanf(item," %s %s %s",s_name,s_item,s_price)!= EOF){
        
		sellerPrice = atoi(s_price);
        select =0;
      	
		bidderall = fopen(BIDDERFILE, "r");
	    temp = fopen(TEMP,"a") ;
		
		if(bidderall == NULL){
			puts("cannot open File bidderall.txt");
			exit(1);
		}
        
		if(temp == NULL){
			puts("cannot open File temp.txt");
			exit(1);
		}
        rewind(bidderall);
        
		while(fscanf(bidderall," %s %s %s %s",bs_name,ss_name,is_name,is_price)!= EOF){
            bidderPrice = atoi(is_price);
            if(sellerPrice <= bidderPrice && strcmp(is_name,s_item) == 0 && strcmp(ss_name,s_name) == 0){
                if(select < bidderPrice){
                    select = bidderPrice;
                    strcat(finalbidding,bs_name);
                    strcat(finalbidding," ");
                    strcat(finalbidding,ss_name);
                    strcat(finalbidding," ");
                    strcat(finalbidding,is_name);
                    strcat(finalbidding," ");
                    strcat(finalbidding,is_price);
                    strcat(finalbidding,"\n");
                    
                }
            }
			else{
				fprintf(temp,"%s %s %s %s\n",bs_name,ss_name,is_name,is_price);
			}
            
        }
		fclose(temp);
		fclose(bidderall);
		remove(BIDDERFILE);
		ret = rename(TEMP,BIDDERFILE);
		if(ret != 0) printf("unsuccessful renaming of file");
		
    }
    //  fclose(bidderall);
    //printf("%s",finalbidding);
    fclose(item);
	//fclose(temp);
    
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// This function will parse the bidder name, seller price , items and price which are finalized after bidding is complete//////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char *parseitems(char *port){
    
    char * items = (char *) malloc (50* sizeof(char));
    char * info,selname[15]="",selIP[15]="",bidname[15]="",bidIP[15]="";
    int inc = 0,save =0,i=0,count =0,j;
    FILE *sel,*bid;
    char * sellers [NO_OF_USERS];//(char *) malloc (50* sizeof(char));
    char * bidders[NO_OF_USERS] ;//(char *) malloc (50* sizeof(char));
    
    
    for (j=0; j<NO_OF_USERS; j++) {
        sellers[j] = (char*) malloc(50*sizeof(char));
        bidders[j] = (char*) malloc(50*sizeof(char));
    }
    
    sel = fopen(AUTHSELLERS, "r");
    bid = fopen(AUTHBIDDERS, "r");
    
    if(sel == NULL || bid == NULL){
        puts("cannot open File");
        exit(1);
    }
    
    
    //  char seller1[MAXITEMS],seller2[MAXITEMS],bidder1[MAXITEMS],bidder2[MAXITEMS];
    
    info = strtok (finalbidding," \n");
    while (info != NULL)
    {
        if( inc == 0)
            strcpy(i_sold[save].b_name,info);
        if( inc == 1)
            strcpy(i_sold[save].s_name,info);
        if(inc == 2)
            strcpy(i_sold[save].item, info);
        if(inc == 3){
            strcpy(i_sold[save].price,info);
            printf("\nPhase 3: Item %s was sold at price %s.\n",i_sold[save].item,i_sold[save].price);
            save++;
            inc = 0;
            info = strtok (NULL, " \n");
            
            continue;
            
        }
        inc++;
        info = strtok (NULL, " \n");
    }
    
    if(strcmp(port, PORTP3SEL2)==0 || (strcmp(port, PORTP3SEL1)==0)){
        while(fscanf(sel, "%s %s",selname,selIP)!=EOF){
            while(strcmp(i_sold[i].s_name,"")!=0){
                if(strcmp(selname, i_sold[i].s_name)==0){
                    strcat(sellers[count], i_sold[i].item);
                    strcat(sellers[count], " ");
                    strcat(sellers[count], i_sold[i].price);
                    strcat(sellers[count], "\n");
                    
                }i++;
            }count++; i=0;
        }
        if(strcmp(port, PORTP3SEL1)==0){
            items = sellers[0];
        }
        else{
            items = sellers[1];
        }
    }
    
    else // bidder if(strcmp(port, PORTP3SEL2)==0 || (strcmp(port, PORTP3SEL1)==0))
    {
        while(fscanf(bid, "%s %s",bidname,bidIP)!=EOF){
            while(strcmp(i_sold[i].b_name,"")!=0){
                if(strcmp(bidname, i_sold[i].b_name)==0){
                    strcat(bidders[count], i_sold[i].item);
                    strcat(bidders[count], " ");
                    strcat(bidders[count], i_sold[i].price);
                    strcat(bidders[count], "\n");
                    
                }i++;
            }count++; i=0;
        }
        if(strcmp(port, PORTP3BID1)==0){
            items = bidders[0];
        }
        else{
            items = bidders[1];
        }
    }
    
    return items;
    
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////  This function will intialize the phase-1 and verify which users are accepted and which are rejected/////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int s_initSocket(void)
{
    int sockfd = -1, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1 , numbytes =0 ,addrlen=0,getsock_check=0;
    char s[INET6_ADDRSTRLEN];
    int rv, auth =0;
    pid_t pid =-1;
    char *string_accepted;
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in sa_ip;
    
    
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(IPADDR, PORT1, &hints, &servinfo)) != 0) {
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
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
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
    
    printf("Phase 1: Auction server has TCP port number %hu and IP address %s",sa_ip.sin_port,ipstr);
    
    
    printf("\nAuction server is waiting for connections...\n");
    
    
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue; }
        else count++;
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        
        //   printf("server: got connection from %s\n", s);
        
        pid = fork();
        
        if (!pid) { // this is the child process
            close(sockfd); // child doesn't need the listener
            
            if((numbytes = recv(new_fd,buf,MAXDATASIZE,0)) == -1)
                perror("recv");
            else{
                buf[numbytes] = '\0';
            }
            
            auth = authenticate_user(s);
            
            if(auth>=0){
                printf("\nPhase 1: Authentication request. User#: Username %s Password: %s Bank Account: %s User IP Addr: %s . Authorized: YES\n",u_info,pwd,bankno,s);
                
                //                printf("enter auth");
                string_accepted = (char*)malloc(sizeof(char)*30);
                
                strcat(string_accepted,"ACCEPTED#");
                strcat(string_accepted,ipstr);
                strcat(string_accepted," ");
                strcat(string_accepted,PORT2);
                
                //lavish missed some info
                if (send(new_fd, string_accepted, MAXDATASIZE-1, 0) == -1){
                    perror("send");
                }
                else{
                    //save the IP address and bind to the username of the accepted user ---lavish
                    printf("\nPhase 1: Auction Server IP Address: %s PreAuction Port Number: %s sent to the <Seller#%d>\n",ipstr,PORT2,count);
                    
                }
            }
            else{
                printf("\nPhase 1: Authentication request. User#: Username :%s Password: %s Bank Account: %s User IP Addr: %s . Authorized: NO\n",u_info,pwd,bankno,s);
                
                if(send(new_fd, "REJECTED#", MAXDATASIZE-1, 0) == -1)
                    perror("send");
            }
            
            close(new_fd);
            exit(0);
        }
        close(new_fd);
        if(count == 4)
        {
            sleep(2);
            count=0;
            break;
            
        }
        
    }
    
    printf("\n\nEnd of Phase 1 for Auction Server\n\n");
    
    
    return 0;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////this function will start listening to a port on which the sellers will send their items and prices ////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int s_initSocketP2(void){
    
    int sockfd = -1, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1 , numbytes =0 ,addrlen=0,getsock_check=-1;
    char s[INET6_ADDRSTRLEN];
    int rv=-1,u=0,count2=0;;
    pid_t pid =-1;
    struct sockaddr_in *ipv4 ;
    void *addr;
    char ipstr[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE; // use my IP
    
    if ((rv = getaddrinfo(IPADDR, PORT2, &hints, &servinfo)) != 0) {
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
    
    ipv4 = (struct sockaddr_in*) p->ai_addr;
    addr = &(ipv4->sin_addr);
    inet_ntop(p->ai_family,addr,ipstr,sizeof(ipstr));
    
    
    
    
    /*addrlen = sizeof(sa);
     getsock_check=getsockname(sockfd,p->ai_addr, (socklen_t *)&addrlen) ;
     //Error checking
     if (getsock_check== -1) {
     perror("getsockname");
     exit(1);
     }
     
     inet_ntop(sa_ip.sin_family, &(sa_ip.sin_addr), ipstr, INET_ADDRSTRLEN);*/
    
    
    //lavish
    printf("\nPhase 2: Auction Server IP Address: %s PreAuction TCP Port Number: %s .\n",ipstr,PORT2);
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    //    printf("\nserver: waiting for connections...\n");
    
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue; }
        else count2++;
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        
        // printf("server: got connection from %s\n", s);
        
        pid = fork();
        
        if (!pid) { // this is the child process
            //    close(sockfd); // child doesn't need the listener
            
            if((numbytes = recv(new_fd,buf,MAXDATASIZE,0)) == -1)
                perror("recv");
            else{
                u++;
                buf[numbytes] = '\0';
                // lavish problem
                printf("\nPhase 2: <Seller#>%d send item lists.\n",count2);
                printf("\nPhase 2: (Received Item list display here)\n%s\n",buf);
                addItemToFile();
                
            }
            
            close(new_fd);
            exit(0);
        }
        close(new_fd);
        if(count2 == 2)
        {
            sleep(2);
            count2=0;
            break;
            
        }
        
        // printf("closing the new_fd :%d",new_fd);
        close(new_fd);  // parent doesn't need this
    }
    printf("\n\nEnd of Phase 2 for Auction Server\n\n");
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
////  This function will send the items ,prices and seller name for bidding to bidder1////////////////
/////////////////////////////////////////////////////////////////////////////////////////

int sendItemtoBidder1(){
    
    int sockfd = -1;
    struct addrinfo hints,*servinfo,*p;
    int rv;
    int numbytes,addrlen,getsock_check;
    struct sockaddr_in sa;
    char ipstr[INET_ADDRSTRLEN];
    
    sleep(5);
    
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    
    if((rv = getaddrinfo(IPADDR,PORTBID1,&hints,&servinfo)) !=0){
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
        return 1;
    }
    
    for(p = servinfo ; p != NULL ; p = p->ai_next){
        if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
            perror("listener: socket");
            continue;
        }
        
        break;
    }
    
    addrlen = sizeof(sa);
    
    getsock_check=getsockname(sockfd,(struct sockaddr *)&sa, (socklen_t *)&addrlen) ;
    //Error checking
    if (getsock_check== -1) {
        perror("getsockname");
        exit(1);
    }
    
    inet_ntop(sa.sin_family, &(sa.sin_addr), ipstr, INET_ADDRSTRLEN);
    
    // error printing thr port number etc
    
    printf("\nPhase 3: Auction Server IP Address: %s Auction UDP Port Number: %hu .\n",ipstr,sa.sin_port);
    
    
    if(p == NULL){
        fprintf(stderr,"listener: failed to bind socket");
        return 2;
        
    }
    //extract the data from file broadcast and send to the bidders
    
    
    readTheItems();
    
    if((numbytes = sendto(sockfd,broadcastItems,strlen(broadcastItems),0,p->ai_addr,p->ai_addrlen)) == -1){
        perror("sendto");
        exit(1);
    }
    else{
        printf("\nPhase 3: (Item list displayed here)\n%s\n",broadcastItems);
    }
    
    if((numbytes = recvfrom(sockfd,bidderdata,MAXDATASIZE*10-1,0,p->ai_addr,&p->ai_addrlen)) == -1){
        perror("recvfrom");
        exit(1);
    }
    else{
        bidderdata[numbytes] = '\0';
        printf("\nPhase 3: Auction Server received a bidding from <Bidder#>1 \nPhase 3: (Bidding information displayed here)\n%s\n",bidderdata);
        addBidderItem();
    }
    
    
    close(sockfd);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
////  This function will send the items ,prices and seller name for bidding to bidder2////////////////
/////////////////////////////////////////////////////////////////////////////////////////

int sendItemtoBidder2(){
    
    int sockfd = -1;
    struct addrinfo hints,*servinfo,*p;
    int rv;
    int numbytes;
    
    
    sleep(5);
    
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    
    
    if((rv = getaddrinfo(IPADDR,PORTBID2,&hints,&servinfo)) !=0){
        fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
        return 1;
    }
    
    for(p = servinfo ; p != NULL ; p = p->ai_next){
        if((sockfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
            perror("listener: socket");
            continue;
        }
        
        break;
    }
    if(p == NULL){
        fprintf(stderr,"listener: failed to bind socket");
        return 2;
        
    }
    //extract the data from file broadcast and send to the bidders
    
    //readTheItems();
    
    if((numbytes = sendto(sockfd,broadcastItems,strlen(broadcastItems),0,p->ai_addr,p->ai_addrlen)) == -1){
        perror("sendto");
        close(sockfd);
        exit(1);
    }
    else{
        //printf("send buf ::: %s",buf);
    }
    
    if((numbytes = recvfrom(sockfd,bidderdata,MAXDATASIZE*10-1,0,p->ai_addr,&p->ai_addrlen)) == -1){
        perror("recvfrom");
        exit(1);
    }
    else{
        bidderdata[numbytes] = '\0';
        printf("\nPhase 3: Auction Server received a bidding from <Bidder#>2 \nPhase 3: (Bidding information displayed here)\n%s\n",bidderdata);
        addBidderItem();
    }
    
    return 1;
    
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  This function will send the packet containig items and prices to well known ports of clients  ////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

int sendfinalbid(char *port)
{
    int sockfd = -1;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char *itemsold="";
    
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(IPADDR, port, &hints, &servinfo)) != 0) {
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
    
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    //  printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    
    //    printf("%s",finalbidding);
    itemsold = parseitems(port);
    
    
    if (send(sockfd,itemsold , strlen(itemsold)+1, 0) == -1){
        perror("send");
        exit(1);
    }
    
    close(sockfd);
    return 0;
}


////////////////////////////////////////////////////////////////////////
///  start phase 2 (for sellers )and phase 3 (for bidders)  ////////////
//////////////////////////////////////////////////////////////////////////

void startnextphases(){
    
    //phase2
    s_initSocketP2();
    //phase 3
    sendItemtoBidder1();
    sendItemtoBidder2();
}
/////////////////////////////////////////////////////////////////////////
//  This function will delete all the temp files created during phase  //
/////////////////////////////////////////////////////////////////////////

deletealltempfiles(){
    //delete the broadcast.txt file if exists
    remove(BROADCAST);
    remove(BIDDERFILE);
    remove(AUTHSELLERS);
    remove(AUTHBIDDERS);
}
/////////////////////////////////////////////////////////////////////////
//  This function will notify all the users after bidding is complete  //
/////////////////////////////////////////////////////////////////////////

void notifyall(){
    
    sendfinalbid(PORTP3SEL1);
    sendfinalbid(PORTP3SEL2);
    sendfinalbid(PORTP3BID1);
    sendfinalbid(PORTP3BID2);
    printf("\nEnd of Phase 3 for Auction Server.\n");
	deletealltempfiles();
    
    
}

///////////////////////////////////////////////////////////
///////////////  MAIN     ////////////////////////////////
//////////////////////////////////////////////////////////


int main(int argc,  char * argv[])
{
    int sock = -1;
    s_fillInfo();
    sock = s_initSocket();
    startnextphases();
    compareItemsAndBroadcast();
    notifyall();
    
    
    return 0;
}













