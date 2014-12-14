READ ME
******************PLEASE OPEN THE FILE IN NOTEPAD++ if the INDENTATION IS NOT CORRECT*****************
******************************************************************************************************


________


a)Name: Lavish Kumar Singal

b)Student ID: 7593-8451-53

c)In the assignment I have completed all the phases as mentioned in the given pdf with their proper screen messages. The file names are used in the same way as mentioned in the guide.

d)
I have created total of 5 files namely:
1) auctionserver.c - used for running the auction server
2) seller1.c - for seller1
3) seller2.c - for seller2
4) bidder1.c - for bidder1
5) bidder2.c - for bidder2

Each file requires separate instance opened in the terminal/unix environment.

I had used fork() command for handling various instance concurrently in the auction server.c file.

e) in order to run the software follow below steps:

1) First open five terminals.
2) go the the specific directory where all the files are placed.
3) execute these commands to make unix executable file.

i) for auctionserver.c file — gcc auctionserver.c -o auctionserver -lnsl -lsocket
ii) for seller1.c file — gcc seller1.c -o seller1 -lnsl -lsocket
iii) for seller2.c file - gcc seller2.c -o seller2 -lnsl -lsocket
iv) for bidder1.c file - gcc bidder1.c -o bidder1 -lnsl -lsocket
v) for bidder2.c file - gcc bidder2.c -o bidder2 -lnsl -lsocket

Once they are executed run following commands in 5 instances:

1) ./auctionserver
2) ./seller1
3) ./seller2
4) ./bidder1
5) ./bidder2


NOTE: Since when sellers are authenticated they will initiate the phase2 but at that time the server is not listening to specific port so there is a DELAY of 30sec (I know it is long time but just has given enough time so that one can complete typing ).

After they are executed properly it will display all the required information as mentioned in the pdf.

f) The format of messages are the same as mentioned.

g) Since I have assumed that there are 2 sellers and 2 bidders ,all of which are authentic users, my program will run if we provide all 4 users which will be ACCEPTED. but if any users who is rejected will make the server keep waiting for the information he is expecting from all of his users. Although I have handled properly the REJECTED case also. In case to run the rejected case, change the file path and run the <users>.c file.

h) I have used the code from BEEJ GUIDE for implementing the sockets.





