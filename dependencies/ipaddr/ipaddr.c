#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <errno.h>

static unsigned int max_hosts = 1025;

void IP_get_eth0_ip(char * str){
	struct ifaddrs *ifaddr_list_head = NULL, *ifa_node = NULL;
    const char *s = NULL;
    char host[max_hosts];

    if (getifaddrs(&ifaddr_list_head) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }
	//using a for loop because while loop broke :(
	for (ifa_node = ifaddr_list_head; ifa_node != NULL; ifa_node = ifa_node->ifa_next) {
        if (ifa_node->ifa_addr == NULL) {
            continue;
        }

		//limit to ipv4 addrs only
        if (ifa_node->ifa_addr->sa_family == AF_INET) {
            s = inet_ntop(ifa_node->ifa_addr->sa_family, &(((struct sockaddr_in*)ifa_node->ifa_addr)->sin_addr), host, max_hosts);
            if (s == NULL) {
                printf("inet_ntop() failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

			if(!strncmp(ifa_node->ifa_name, "eth0", strlen("eth0"))){
				//"255.255.255.255" is 15 + 1 characters long.
				strncpy(str, host, 16);
				break;
			}

            // //test when eth0 not connnected
			// if(!strncmp(ifa_node->ifa_name, "lo", strlen("lo"))){
			// 	//"255.255.255.255" is 15 + 1 characters long.
			// 	strncpy(str, host, 16);
			// 	break;
			// }
        }
    }

    freeifaddrs(ifaddr_list_head);
}

