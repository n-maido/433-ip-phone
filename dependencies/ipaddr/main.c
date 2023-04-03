/*
	
*/

#include <stdio.h>
#include "ipaddr.h"

int main(void){
	char test[30] = "";
	IP_get_eth0_ip(test);
	printf("test = %s\n", test);
	// IP_test();

	return 0;
}