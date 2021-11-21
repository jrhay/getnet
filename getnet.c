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

		if (ifa->ifa_addr->sa_family == AF_INET) {
			void *addrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			inet_ntop(ifa->ifa_addr->sa_family, addrPtr, ip4addr, INET_ADDRSTRLEN);
			found++;
		}

		if (ifa->ifa_addr->sa_family == AF_INET6) {
			void *addrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			inet_ntop(ifa->ifa_addr->sa_family, addrPtr, ip6addr, INET6_ADDRSTRLEN);
			found++;
		}

		#ifdef AF_PACKET
		if (ifa->ifa_addr->sa_family == AF_PACKET) {
			struct sockaddr_ll *s = (struct sockaddr_ll*)(ifa->ifa_addr);
			int i;
			int len = 0;
			for (i = 0; i < 6; i++)
				len += sprintf(macaddr+len, "%02x%s", s->sll_addr[i], i < 5 ? ":":"");
			found++;
		}
		#endif

		#ifdef AF_LINK
		if (ifa->ifa_addr->sa_family == AF_LINK) {
                	unsigned char *ptr = (unsigned char *)LLADDR((struct sockaddr_dl *)(ifa->ifa_addr));
                	sprintf(macaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
                        	            *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
			found++;
		}
		#endif

	}

	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return found;
}

struct strlist {
	char *name;
	struct strlist *next;
};

// Get a list of all known interface names, skipping duplicate names
struct strlist *getifnames()
{
    	struct ifaddrs * ifAddrStruct=NULL;
    	struct ifaddrs * ifa=NULL;
	struct strlist * ifnames = NULL;
	struct strlist * start = NULL;

    	getifaddrs(&ifAddrStruct);
    	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        	if (ifa->ifa_addr != NULL) {
			int found = 0;
			for (struct strlist *ifn = start; ifn != NULL && found == 0; ifn = ifn->next)
				found = (strcmp(ifn->name, ifa->ifa_name) == 0);

			if (!found) {
				struct strlist *next = malloc(sizeof(struct strlist));
				next->name = strdup(ifa->ifa_name);
				next->next = NULL;
				if (ifnames == NULL) {
					ifnames = next;
					start = ifnames;
				} else {
					ifnames->next = next;
					ifnames = next;
				}
			}
		}
    	}

	if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
	return start;
}

int main (int argc, const char * argv[]) {
	char ip4addr[INET_ADDRSTRLEN], ip6addr[INET6_ADDRSTRLEN], macaddr[18];

	struct strlist *next = NULL;
	struct strlist *ifnames = getifnames();
	for (struct strlist *ifn = ifnames; ifn != NULL; ifn = next) {
		if (getaddrs(ifn->name, ip4addr, ip6addr, macaddr) > 0)
			printf("%s\t%17s %15s %s\n", ifn->name, macaddr, ip4addr, ip6addr);

		free(ifn->name);
		next = ifn->next;
		free(ifn);
    	}

    	return 0;
}
