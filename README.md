# getnet
Portable C code for to get network interface names, IPv4, IPv6, and MAC addresses

## Overview

getnet is designed to be a simple, portable, straight-C header file that can be easily included in any
project to obtain a list of system network interfaces and their associated addresses.  This hides all
underlying implementation and returns network interface information in the following structure:

```
/* Network adapter configuration information */
struct netif {
        char *name;         // System/Device name of network adapter
        char *ip4addr;      // adapter's IPv4 address, NULL if none found
        char *ip6addr;      // adapter's IPv6 address, NULL if none found
        char *macaddr;      // adapter's MAC address, NULL if none found
        struct netif *next; // Pointer to next adapter, or NULL at end of list
};
```

## Interface

All functionality is provided in the `getnet.h` header file.

```
struct netif *getnetifs()
```
Returns a newly-allocated pointer to the head of a list of `struct netif` structures detailing every network interface found on the system.  All data is allocated by getnet, and should be freed with `freenetifs()` when no longer needed.

```
void freenetifs(struct netif *head)
```
Free an entire list of `struct netif` structures allocated by `getnetifs()`.  No action taken if given a NULL pointer.

## Memory Allocator

By default, getnet uses the standard C memory allocator.  However, the macros `GETNET_MALLOC`, `GETNET_FREE`, and `GETNET_STRDUP` may be defined before including `getnet.h` to use an alternative allocator.

## Example/Tester

Included `getnet.c` provdes a simple example of using `getnetifs()` to print a table of network interfaces to stdout.  It may be compiled on any system simply via `gcc getnet.c`.

## Portability

getnet has been tested and is known to work on:

*Compilers and Libraries*
- GCC 6.2 and later
- Apple Clang 13 and later

*OSes*
- Ubuntu 18 and later
- CentOS 6 and later
- Raspbian 8.3 and later
 
