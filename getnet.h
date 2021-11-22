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

#if __has_include(<netpacket/packet.h>)
	#include <netpacket/packet.h>
#endif

#if __has_include(<net/ethernet.h>)
	#include <net/ethernet.h>
#endif

#if __has_include(<net/if_dl.h>)
	#include <net/if_dl.h>
#endif

/* Network adapter configuration information */
struct netif {
	char *name;         // System/Device name of network adapter
	char *ip4addr;      // adapter's IPv4 address, NULL if none found
	char *ip6addr;      // adapter's IPv6 address, NULL if none found
	char *macaddr;      // adapter's MAC address, NULL if none found
	struct netif *next; // Pointer to next adapter, or NULL at end of list
};

/* Defines to replace memory management functions, if needed */

#ifndef NET_MAC_ADDRSTRLEN
#define NET_MAC_ADDRSTRLEN 18
#endif

#ifndef GETNET_MALLOC
#define GETNET_MALLOC malloc
#endif

#ifndef GETNET_FREE
#define GETNET_FREE free
#endif

#ifndef GETNET_STRDUP
#define GETNET_STRDUP strdup
#endif

/* Return a pointer to the head of a netif struct list enumerating all avaliable network interfaces. */
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
				// Create a new entry for this interface
				current = GETNET_MALLOC(sizeof(struct netif));
				current->name = GETNET_STRDUP(ifa->ifa_name);
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
				current->ip4addr = GETNET_MALLOC(sizeof(char) * INET_ADDRSTRLEN + 1);
				void *addrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
				inet_ntop(ifa->ifa_addr->sa_family, addrPtr, current->ip4addr, INET_ADDRSTRLEN);
			}

			if (ifa->ifa_addr->sa_family == AF_INET6) {
				current->ip6addr = GETNET_MALLOC(sizeof(char) * INET6_ADDRSTRLEN + 1);
				void *addrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
				inet_ntop(ifa->ifa_addr->sa_family, addrPtr, current->ip6addr, INET6_ADDRSTRLEN);
			}

			#ifdef AF_PACKET
			if (ifa->ifa_addr->sa_family == AF_PACKET) {
				current->macaddr = GETNET_MALLOC(sizeof(char) * NET_MAC_ADDRSTRLEN + 1); 
				struct sockaddr_ll *s = (struct sockaddr_ll*)(ifa->ifa_addr);
				int i;
				int len = 0;
				for (i = 0; i < 6; i++)
					len += sprintf(current->macaddr+len, "%02x%s", s->sll_addr[i], i < 5 ? ":":"");
			}
			#endif

			#ifdef AF_LINK
			if (ifa->ifa_addr->sa_family == AF_LINK) {
				current->macaddr = GETNET_MALLOC(sizeof(char) * NET_MAC_ADDRSTRLEN + 1);
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

/* Free all resources used by a struct netif list */
void freenetifs(struct netif *start)
{
	struct netif *current;
	for (current = start; current != NULL; ) {
		GETNET_FREE(current->name);
		GETNET_FREE(current->ip4addr);
		GETNET_FREE(current->ip6addr);
		GETNET_FREE(current->macaddr);
		struct netif *next = current->next;
		GETNET_FREE(current);
		current = next;
	}
}
