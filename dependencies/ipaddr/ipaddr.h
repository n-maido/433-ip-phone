/*
	A module for getting (a specific) IP address. Used for getting the eth0 address here.
	Written by Ryan Tio. 
*/

#ifndef rtioipaddr
#define rtioipaddr

//returns the eth0 ip via the str parameter. Assumes that it has been allocated properly.
void IP_get_eth0_ip(char * str);

#endif