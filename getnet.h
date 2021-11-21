/*
    Output a list of all found network interfaces, their IP address and MAC address
    Uses the getifaddrs() API to determine address information
    Created with the help of several answers on StackOverflow.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef __linux__
	#include <netpacket/packet.h>
	#include <net/ethernet.h>
#else
	#include <net/if_dl.h>
#endif

struct netif {
	char *name;
	char *ip4addr;
	char *ip6addr;
	char *macaddr;
	struct netif *next;
};

#ifndef NET_MAC_ADDRSTRLEN
#define NET_MAC_ADDRSTRLEN 18
#endif

// Return a pointer to the head of a netif struct list enumerating
// all avaliable network interfaces.
struct netif *getnetifs()
{
    	struct ifaddrs * ifAddrStruct=NULL;
    	struct ifaddrs * ifa=NULL;
	struct netif * end = NULL;
	struct netif * start = NULL;
	struct netif * current = NULL;

    	getifaddrs(&ifAddrStruct);
    	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        	if (ifa->ifa_addr != NULL) {
			current = NULL;
			for (struct netif *curif = start; curif != NULL && current == NULL; curif = curif->next)
			{
				if (strcmp(curif->name, ifa->ifa_name) == 0)
					current = curif;
			}

			if (current == NULL) {
				current = malloc(sizeof(struct netif));
				current->name = strdup(ifa->ifa_name);
				current->ip4addr = NULL;
				current->ip6addr = NULL;
				current->macaddr = NULL;
				current->next = NULL;
				if (end == NULL) {
					end = current;
					start = end;
				} else {
					end->next = current;
					end = current;
				}
			}

			// We have the entry for this interface; fill in what we know from this
			// entry in the ifaddrs array

			if (ifa->ifa_addr->sa_family == AF_INET) {
				current->ip4addr = malloc(sizeof(char) * INET_ADDRSTRLEN + 1);
				void *addrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
				inet_ntop(ifa->ifa_addr->sa_family, addrPtr, current->ip4addr, INET_ADDRSTRLEN);
			}

			if (ifa->ifa_addr->sa_family == AF_INET6) {
				current->ip6addr = malloc(sizeof(char) * INET6_ADDRSTRLEN + 1);
				void *addrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
				inet_ntop(ifa->ifa_addr->sa_family, addrPtr, current->ip6addr, INET6_ADDRSTRLEN);
			}

			#ifdef AF_PACKET
			if (ifa->ifa_addr->sa_family == AF_PACKET) {
				current->macaddr = malloc(sizeof(char) * NET_MAC_ADDRSTRLEN + 1); 
				struct sockaddr_ll *s = (struct sockaddr_ll*)(ifa->ifa_addr);
				int i;
				int len = 0;
				for (i = 0; i < 6; i++)
					len += sprintf(macaddr+len, "%02x%s", s->sll_addr[i], i < 5 ? ":":"");
			}
			#endif

			#ifdef AF_LINK
			if (ifa->ifa_addr->sa_family == AF_LINK) {
				current->macaddr = malloc(sizeof(char) * NET_MAC_ADDRSTRLEN + 1);
                		unsigned char *ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifa->ifa_addr));
                		sprintf(current->macaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
                        		            *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
			}
			#endif

		}
    	}

	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return start;	
}

// Given an interface name, look up IP4, IP6, and MAC addresses for that interface
int getaddrs(char *name, char *ip4addr, char *ip6addr, char *macaddr)
{
    	struct ifaddrs * ifAddrStruct=NULL;
    	struct ifaddrs * ifa=NULL;
	int found = 0;

	ip4addr[0] = '\0';
	ip6addr[0] = '\0';
	macaddr[0] = '\0';

    	getifaddrs(&ifAddrStruct);
    	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(ifa->ifa_name, name))
			continue;
	}

	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return found;
}

// Free all resources used by a struct netif list
void freenetifs(struct netif *start)
{
	struct netif *current;
	for (current = start; current != NULL; ) {
		free(current->name);
		free(current->ip4addr);
		free(current->ip6addr);
		free(current->macaddr);
		struct netif *next = current->next;
		free(current);
		current = next;
	}
}
