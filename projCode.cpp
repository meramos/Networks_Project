/* 
Written by:     Maria E. Ramos Morales
For the course: CS-571 computer networks
Date published: December 2, 2014

Program description:

This program maintains two tables. One is the neighbour table, recording the information read from the
configuration file. The other is the routing table. Each entry in the table includes destination, distance and next hop.
The neighbour table is used for updating the routing table after a distance vector is received from a neighbour.
These two tables have different formats, and both of them are different from the distance vector.

After reading the configuration file, the algorithm establishes its initial routing table. It then sends out its initial
distance vector to all of its neighbours. After that, it waits for receiving distance vectors sent from its neighbours. 
Whenever a distance vector is received, it updates its routing table. If the table has been changed, it sends a distance 
vector to all of its neighbours. In addition, it also sends out its distance vector to all of its neighbours periodically. 
This is done by using the alarm() function.

The algorithm also prints out the initial routing table. Whenever it receives a distance vector from a neighbour, it
prints out what has been received and its routing table after update. It also prints out a message when it periodically 
sends out the distance vector. 
*/


#include<iostream>
#include<fstream>
#include <sstream>
#include<iomanip>

#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and alarm() */
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */

#define TIMEOUT_SECS    5       /* Seconds between retransmits. Retransmit every 5 seconds. */
/*#define MAXTRIES        5        Tries before giving up */
#define MAXNODES       	7       /* Maximum ammount of nodes for this program */

using namespace std;

int tries=0;    /*Count of times sent - GLOBAL for signal-handler access */

void DieWithError(string errorMessage);   /* Error handling function */
void CatchAlarm(int ignored);            /* Handler for SIGALRM */

/*define structures*/

/*distance vector*/

struct element_DV {
char dest;
int dist;
};

struct distance_vector_ {
char sender;
int num_of_dests;
struct element_DV contentDV[MAXNODES];
};

/*routing table*/

struct element_RT {
char dest;
int dist;
char nexthop;
};

struct routing_table_ {
struct element_RT contentRT[MAXNODES];
};

/*neighbor table*/

struct element_NT{
char dest;
int dist;
char IP[10]; /*9 IP chars and then a null terminator*/
};

struct neighbor_table_ {
char node;
int portnum;
struct element_NT contentNT[MAXNODES];
};

int main(int argc, char *argv[])
{
		//*************
		//Define variables.
		//*************

        int sock_rcv;                        /* Socket descriptor for receiving*/
		int sock_snd;                        /* Socket descriptor for sending*/
        struct sigaction myAction;       	 /* For setting signal handler */
        struct sockaddr_in echoServAddr_rcv; /* Echo server address for receiving*/
		struct sockaddr_in echoServAddr_snd; /* Echo server address for sending*/
        const char *servIP;                  /*IP address of server*/
        unsigned short echoServPort;     	 /*echo server port*/
		struct sockaddr_in echoClntAddr; 	 /* Address of node that our node has received something from.*/
		unsigned int cliAddrLen;         	 /* Length of incoming message */

        ifstream infile;
        string configFile;
        struct distance_vector_ DistVec;    /*distance vector*/
		struct distance_vector_ RecDistVec; /*the received distance vector*/
        struct routing_table_ RoutTab;      /*routing table*/
        struct neighbor_table_ NeighTab;    /*neighbor table*/
		
        int respStringLen;
        //unsigned int fromSize;
        //struct sockaddr_in fromAddr;
        int new_cost;
		int destination;

		//*************
		//Get console input.
		//*************
		
        if (argc != 2)    /* Test for correct number of arguments   need 1 argument*/
        {
                fprintf(stderr,"Usage: %s <Configuration File>\n", argv[0]);
                return 0;
        }

		//*************
        //Store read arguments.
		//*************

        configFile = argv[1];       /* First and only arg:  configuration file name */

        infile.open(configFile.c_str());

        int i = 0;         //index to keep track in which line of the file we are (1st, 2nd, other)
        int index = 0;	   //index for the neighbour table
        int num_neighbors; //for keeping track of the number of neighbours
        string line;	   //where read line will be stored

        //The following are to temporarily keep dest-dist-IP info.
        char a;   //temp. destination
        int b;    //temp. distance
        char c[10] = {'\0'}; //temp. IP address. Initialized.

        while (getline(infile, line))
        {
                stringstream ss(line);

                if(i==0)
                {
                        ss >> NeighTab.node;
						i++;
                }
                else
                {       if(i==1)
                        {
                                ss >> NeighTab.portnum;
								i++;
                        }
                        else
                        {
                                if(ss >> a >> b >> c)
                                { 
                                        NeighTab.contentNT[index].dest = a;
										
                                        NeighTab.contentNT[index].dist = b;
										
										for(int a = 0; a < 9; a++)
										{
											NeighTab.contentNT[index].IP[a] = c[a];
										}
										
                                        NeighTab.contentNT[index].IP[9] = '\0'; //null terminate the string

                                        index++;
										
										memset(c, 0x0, 10); //reset the temp char array, c.
                                }
                                else
                                {
                                        break;
                                }
                        }
                }
        }

        num_neighbors = index;

        infile.close(); /*close opened file*/
		
        //*************
        //Establish the initial routing table. Fill unknown entries with "$" (for dest and nexthop) and -1 (for dist)
        //*************
		
        for(int j = 0; j < num_neighbors; j++) //known entries
        {
                RoutTab.contentRT[j].dest = NeighTab.contentNT[j].dest;
                RoutTab.contentRT[j].dist = NeighTab.contentNT[j].dist;
                RoutTab.contentRT[j].nexthop = NeighTab.contentNT[j].dest;
        }
		
		for(int j = num_neighbors; j < MAXNODES; j++) //unknown entries
		{
				RoutTab.contentRT[j].dest = '$';
				RoutTab.contentRT[j].dist = -1;
                RoutTab.contentRT[j].nexthop = '$';
		}

        //*************
        //Print the initial routing table.
        //*************

        cout<<"\nInitial routing table:\n";
        cout<<"dest"<<setw(10)<<"cost"<<setw(10)<<"nexthop"<<endl;

        for(int j = 0; j < MAXNODES; j++)
        {
                cout<<RoutTab.contentRT[j].dest<<setw(10)<<RoutTab.contentRT[j].dist<<setw(10)<<RoutTab.contentRT[j].nexthop<<endl;
        }

        //************
        //Create distance vector.
        //************

        DistVec.sender = NeighTab.node;
        DistVec.num_of_dests = num_neighbors;

        for(int j = 0; j < num_neighbors; j++)
        {
                DistVec.contentDV[j].dest = NeighTab.contentNT[j].dest;
                DistVec.contentDV[j].dist = NeighTab.contentNT[j].dist;
        }
		

		//************
        //Set up socket, signal handler for alarm signal, and server address structure for communication.
        //************
		
        /* Create a best-effort datagram socket using UDP (for receiving) */
		
		sock_rcv = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		
		if (sock_rcv < 0)
		{
			DieWithError("socket() failed");
		}
		
		/* Create a best-effort datagram socket using UDP (for sending) */
		
		sock_snd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		
		if (sock_snd < 0)
		{
			DieWithError("socket() failed");
		}
		
		/* Set signal handler for alarm signal */
    
		myAction.sa_handler = CatchAlarm; /*sa_handler is a pointer to a signal-catching function*/
	
		//sigfillset() initializes and fills a signal set
	
		if (sigfillset(&myAction.sa_mask) < 0) /* block everything in handler. sa_mask is a set of signals to be blocked during execution of the signal handling function */
        {
			DieWithError("sigfillset() failed");
		}
    
		myAction.sa_flags = 0;

		if (sigaction(SIGALRM, &myAction, 0) < 0)
        {
			DieWithError("sigaction() failed for SIGALRM");
		}
		
		
		/* Construct the server address structure for receiving */
		
		echoServPort = NeighTab.portnum;
		memset(&echoServAddr_rcv, 0, sizeof(echoServAddr_rcv));    /* Zero out structure */
		echoServAddr_rcv.sin_family = AF_INET;
		echoServAddr_rcv.sin_addr.s_addr = htonl(INADDR_ANY);    /* Any incoming interface */
		echoServAddr_rcv.sin_port = htons(echoServPort);       /* Server port */
		
		/* Construct the server address structure for sending */		
		
		memset(&echoServAddr_snd, 0, sizeof(echoServAddr_snd));      /* Zero out structure */
		echoServAddr_snd.sin_family = AF_INET;
		/* servIP will be changed to the IP of a neighbor */
		echoServAddr_snd.sin_port = htons(echoServPort);         /* Local port */
		
		/* Bind to the local address that is trying to receive */
		
		if (bind(sock_rcv, (struct sockaddr *) &echoServAddr_rcv, sizeof(echoServAddr_rcv)) < 0)
        {
			DieWithError("bind() failed");
		}

		//************
        //Send out initial distance vector to all neighbours.
        //************
		
		for(int j = 0; j < num_neighbors; j++)
		{
			servIP = NeighTab.contentNT[j].IP; 
			echoServAddr_snd.sin_addr.s_addr = inet_addr(servIP);
			
			/*sendto(sock_snd, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr_snd, sizeof(echoServAddr_snd)) != echoStringLen*/
		
			if(sendto(sock_snd, &DistVec, sizeof(DistVec), 0, (struct sockaddr *) &echoServAddr_snd, sizeof(echoServAddr_snd)) != sizeof(DistVec))
			{
				DieWithError("sendto() sent a different number of bytes than expected");
			}
		}

		//************
        // Loop for the continued sending and receiving of distance vectors to and from neighbouring nodes.
        //************
		
		for (;;) /* Run forever */
		{
		
			//************
			// Wait to receive distance vectors from neighbouring nodes.
			//************
		
			/*Periodic DV sending*/
			
			alarm(TIMEOUT_SECS);        /* Set the timeout. Wait 5sec to resend DV if nothing is received in that time frame. */
										/* So, program will periodically send its DV every 5s. */
		
			/* Set the size of the in-out parameter */
			cliAddrLen = sizeof(echoClntAddr);
			
			//While nothing is received ...
			while ((respStringLen = recvfrom(sock_rcv, &RecDistVec, sizeof(RecDistVec), 0,(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0) /*have to re-read if nothing received. but also must resend.*/
			{	
				//5 seconds passed
				//If recvfrom returns -1, then there was an error.
			
				if (errno == EINTR)     /* Not a fatal error ... Alarm went off  */
				{
					//Nothing was received. 
					
					for(int j = 0; j < num_neighbors; j++)
					{
						servIP = NeighTab.contentNT[j].IP; 
						echoServAddr_snd.sin_addr.s_addr = inet_addr(servIP);
					
						if (sendto(sock_snd, &DistVec, sizeof(DistVec), 0, (struct sockaddr *) &echoServAddr_snd, sizeof(echoServAddr_snd)) != sizeof(DistVec))
						{
							DieWithError("sendto() sent a different number of bytes than expected");
						}
					}
					
					alarm(TIMEOUT_SECS); /*reset timeout back to 5sec. So, if nothing received again, resend the DV within 5 sec.*/
				} 
				else
				{
					DieWithError("recvfrom() failed");
				}
			}

			/* recvfrom() got something --  cancel the timeout. Because soon enough the DV will be updated and resent. */
			alarm(0);	
			
			/*Now, deal with actually getting info!!!*/
			
			/*Print received distance vector*/
						
			cout<<"\nReceived distance Vector from node "<<RecDistVec.sender<<":"<<endl;
			cout<<"dest"<<setw(10)<<"dist"<<endl;
			
			for(int n=0; n < RecDistVec.num_of_dests; n++)
			{
				cout<<RecDistVec.contentDV[n].dest<<setw(10)<<RecDistVec.contentDV[n].dist<<endl;
			}
			
			/*Update the routing table*/
			
			//first, look for index of the entry in routing table that corresponds to accessing the sender of the DV
			//for example, if program is ran by node A, and the received DV is from B, then find entry where dest is B.
			//	this is done so that later we can add the distance it takes to get from A to B to the distance it takes
			//	B to get to some other node, say, G, so that we get the distance from A to G with nexthop = B.
			
			int o = 0;
			bool found = false; 
			int indexInRout;
			
			while((RoutTab.contentRT[o].dest != '$') && (o < MAXNODES) && (found != true))
			{
				if(RoutTab.contentRT[o].dest == RecDistVec.sender)
				{
					indexInRout = o;
					found = true;
				}
				else
				{
					o++;
				}
			}
			
			//Go through the received DV nodes to see what extra stuff to add to the routing table and the DV of the node we are on.
			
			int y; //initialize y for later use ...
			int x; //initialize x for later use ...
			bool update = false; //boolean variable to keep track of if we update our own routing table. True if updated, false if not updated.
			char aDVnode; //initialize aDVnode for later use ...
			
			for(int m = 0; m < RecDistVec.num_of_dests; m++)
			{
				aDVnode = RecDistVec.contentDV[m].dest; //node we are on within the received DV
				
				if(aDVnode == NeighTab.node)
				{
					break;
				}
				
				y = 0; //index counter for within our routing table, corresponding to dest = aDVnode
				found = false; //boolean variable to store if aDVnode has been found within our routing table or not.
				
				//check to see if aDVnode (the dest element from the received DV that we are on) is in the routing table of the node running the program
				
				while((RoutTab.contentRT[y].dest != '$') && (found != true))
				{
					if(RoutTab.contentRT[y].dest == aDVnode)
					{
						found = true; //found the aDVnode within the routing table
					}
					else
					{
						y++;
					}
				}
				
				//if aDVnode not found in the routing table, then the received DV has a new entry that our node (routing table) can learn from
				
				if(found != true) //if not found in routing table ...
				{
					//add aDVnode into the routing table of the node we are on (for example, A)
					
					//if aDVnode hadn't been found initially, this means that the routing table entries are not completely full (not up to MAXNODES),
					//	so this means that there are still entries with dest = '$'. So, we change the value of the entry after the last entry we visited,
					//	which is the index y, and we changes its dest, dist and nexthop information in the routing table.
					
					RoutTab.contentRT[y].dest = aDVnode; //adding new node into our node's routing table.
					RoutTab.contentRT[y].dist = RecDistVec.contentDV[m].dist + RoutTab.contentRT[indexInRout].dist; //add dist from sender to aDVnode and dist from our node to the sender.
					RoutTab.contentRT[y].nexthop = RecDistVec.sender; //the next hop in our routing table entry will be the node that sent the received distance vector.
					
					//ALSO add aDVnode into the distance vector of the node we are on.
					
					DistVec.contentDV[DistVec.num_of_dests].dest = aDVnode; //add onto the 'end' of the distance vector
					DistVec.contentDV[DistVec.num_of_dests].dist = RoutTab.contentRT[y].dist;
					DistVec.num_of_dests = DistVec.num_of_dests + 1; //increment the number of destinations in our DV
					
					update = true; //routing table (and DV) has been updated! NOTE: One update in for loop is enough to keep update = true so to use after loop is done. 
									//	So should NOT reinitialize update to false within the loop.
				}
				else //if aDVnode found in our routing table ...
				{
					//aDVnode is within our routing table. Now we must check if the distance from our node to aDVnode is greater than
					//	the distance from our node to the sender of the DV plus the distance from the sender of the DV to aDVnode.
					
					//If our stored distance to aDVnode is greater than the distance from our node to the sender node to aDVnode,
					//	then we must update the routing table with the smaller distance!
					
					if((RecDistVec.contentDV[m].dist + RoutTab.contentRT[indexInRout].dist) < RoutTab.contentRT[y].dist)
					{
						//update the routing table!
						
						RoutTab.contentRT[y].dist = RecDistVec.contentDV[m].dist + RoutTab.contentRT[indexInRout].dist;
						RoutTab.contentRT[y].nexthop = RecDistVec.sender;
						
						//ALSO update our distance vector!
						
						//but first, must look for location of the entry with node = aDVnode
						
						x = 0; //index counter for within our DV, corresponding to dest = aDVnode. our index will be kept in x.
						found = false; //reset found to false.
						
						//find index for aDVnode in the distance vector of the node running the program
						
						while((x < DistVec.num_of_dests) && (found != true))
						{
							if(DistVec.contentDV[x].dest == aDVnode)
							{
								//the aDVnode, so set found to true.
								
								found = true;
							}
							else
							{
								x++;
							}
						}
						
						//okay, now we can update the distance vector!
						
						DistVec.contentDV[x].dist = RoutTab.contentRT[y].dist;
						
						update = true; //routing table (and DV) has been updated!
						
					}
					//if the possible new dist is not less than the dist we already have, then no change is made to the routing table.
					//	so 'update' stays false.
				}
			}
			
			//If routing table was updated, then print it and send its distance vector (which was also updated) to its neighbors.
		
			if(update == true)
			{
				//print routing table
				
				cout<<"\nUpdated routing table:\n";
				cout<<"dest"<<setw(10)<<"cost"<<setw(10)<<"nexthop"<<endl;

				for(int j = 0; j < MAXNODES; j++)
				{
						cout<<RoutTab.contentRT[j].dest<<setw(10)<<RoutTab.contentRT[j].dist<<setw(10)<<RoutTab.contentRT[j].nexthop<<endl;
				}
				
				//send own distance vector to all its neighbours
				
				for(int j = 0; j < num_neighbors; j++)
				{
					servIP = NeighTab.contentNT[j].IP; 
					echoServAddr_snd.sin_addr.s_addr = inet_addr(servIP);
					
					/*sendto(sock_snd, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr_snd, sizeof(echoServAddr_snd)) != echoStringLen*/
				
					if(sendto(sock_snd, &DistVec, sizeof(DistVec), 0, (struct sockaddr *) &echoServAddr_snd, sizeof(echoServAddr_snd)) != sizeof(DistVec))
					{
						DieWithError("sendto() sent a different number of bytes than expected");
					}
				}
			}
			
		}
		
		close(sock_rcv);
		close(sock_snd);


        return 0;
}

void CatchAlarm(int ignored)     /* Handler for SIGALRM */
{
    tries += 1;
}

void DieWithError(string errorMessage)
{
    cout<<"\n"<<errorMessage<<endl;
	cerr <<strerror(errno)<<endl;
    exit(1);
}
                                                                                                                           