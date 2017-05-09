# Networks_Project
UKY Networks project, course CS-571

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
