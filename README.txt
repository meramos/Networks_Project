
Creator:  Maria E. Ramos Morales
Purpose:  For CS-571, computer networks
Publish:  December 2, 2014
Language: C++

%%%General description:

This program is meant to be ran on more than one server/node. It takes in a configuration file with the
name of the node corresponding to the server, a port number that all nodes will use to communicate,
and lastly the neighbours of the node (their name, the cost to that neighbour and the neighbour's IP
address). The program creates a neighbour table with the information of that configuration file.
Then, the program uses sockets to connect to the node's neighbours. It connects to the neighbours so
to send its own distance vector to them and to receive the distance vectors of the neighbours. All the 
servers/nodes running the program will communicate with each other (given that the configuration tables 
are written accordingly depending on the nodes being used), sending distance vectors to each other, with
the purpose of building a routing table of their network. This program is meant to run indefinitely,
constantly sending and receiving distance vectors, and updating the node's routing table when new
information is introduced.

%%%Instructions for running:

Open 7 terminals. Login to different protogeni nodes in each terminal (for example, one terminal is 
logged into node A, another terminal is logged into node B, another terminal is logged into node C,
etc.) To login to my protogeni nodes, write the following, one per node (corresponding node is written
at the left of the colon):

A: > ssh pc2.instageni.illinois.edu -p 32058 -l mariaera

B: > ssh pc2.instageni.illinois.edu -p 32059 -l mariaera

C: > ssh pc2.instageni.illinois.edu -p 32060 -l mariaera

D: > ssh pc2.instageni.illinois.edu -p 32061 -l mariaera

E: > ssh pc2.instageni.illinois.edu -p 32062 -l mariaera

F: > ssh pc2.instageni.illinois.edu -p 32063 -l mariaera

G: > ssh pc2.instageni.illinois.edu -p 32064 -l mariaera

Store the program in all the servers/nodes. Store the configuration files within their respective nodes
(configA.txt in node A, configB.txt in node B, configC.txt in node C, etc.) Compile the program in each
node by writting the following in the terminal:

> make

It compiles the program using the file: makefile
Next, write the following in each node to run the program (the program is projCode.cpp and the executable
is projCode):

> ./projCode configA.txt

configA.txt should be changed to configB.txt for node B, configC.txt for node C, and so on.

Now let the programs run concurrently and see how the routing table gets updated in time. To close the programs,
press Ctrl + C in every terminal.

%%%Limitations of the implementation:

The implementation of the distance vector algorithm does not take into account the possibility of a node failing.
The distance vector algorithm is not working to perfection. 

























