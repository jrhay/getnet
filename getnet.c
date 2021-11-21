
#include <stdio.h>
#include "getnet.h"

#define OUTFORMAT "%s\t%17s %15s %s\n"

int main (int argc, const char * argv[]) {
	struct netif *netifs = getnetifs();

	printf(OUTFORMAT, "name", "MAC", "IP4", "IP6");
	for (struct netif *current = netifs; current != NULL; current = current->next) {
		printf(OUTFORMAT,
			current->name, 
			(current->macaddr != NULL ? current->macaddr : ""), 
			(current->ip4addr != NULL ? current->ip4addr : ""),
			(current->ip6addr != NULL ? current->ip6addr : ""));
    	}

	freenetifs(netifs);

    	return 0;
}
