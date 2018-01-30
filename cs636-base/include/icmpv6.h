#include <xinu.h>
/* icmpv6.h - definintions for the Internet Control Message Protocol */

/*
 * must be preceded by an ipv6 extension header
 *
 * The structure for an ICMP message:
 *
 * The IPv6 header contains source and destination
 * information as well as information for an extension
 * header. This extension header contains the code, 58.
 * The IP header functions as the pseudo header for the
 * ICMPv6 header. The header for ICMPv6 contains the type,
 * code, and checksum. The payload then follows.
 *
 * Source: RFC 4443. Pg 3.
 * RFC 4861 pg 18-18.
 *
 * */

// check icmpsize
// make correct size
// network byte order
//
#define  IPLEN 16


#define MAX_PAYLOAD 1472
#define DESTUNREACH 1
#define PKT_BIG     2
#define TIME_EX     3
#define PVT1        100
#define PVT2        100


/*solicitation*/

#define ROUTERS     133
#define ROUTERA     134
#define NEIGHBS     135
#define NEIGHBA     136

/*informational messages */
#define ECHOREQ     128
#define ECHOREP     129
#define PVT3        200
#define PVT4        201

typedef struct{
    unsigned int type;
    unsigned int length;
    unsigned int prefixlength;
    unsigned int L:1;
    unsigned int A:1;
    unsigned int res:6;
    uint32 validlifetime;
    uint32 preferredlifetime;
    byte res2[8];
    byte prefix[16];
}

typedef struct{
    unsigned int type;
    unsigned int length;
    byte address[16];
}lladdress;

typedef struct{
    unsigned int type;
    unsigned int length;
    byte options[length];
}options;

typedef struct{
    unsigned int type;
    unsigned int code;
    unsigned short checksum;
    byte reserved[];
    byte payload[MAX_PAYLOAD];
    options opt;
}icmpv6general;

typedef struct{
    unsigned int type;
    unsigned int code;
    unsigned short checksum;
    unsigned int reserved: 32;
    options opt;
}rsolicit;

typedef struct{
    unsigned int type;
    unsigned int char code;
    unsigned short checksum;
    unsigned int  curhoplim;
    unsigned int M: 1;
    unsigned int O: 1;
    unsigned int reserved: 6;
    unsigned int routerlifetime;
    uint32 reachabletime;
    uint32 retranstimer;
    options opt;
}radvertisement;


typedef struct{
    unsigned int type;
    unsigned int code;
    unsigned short checksum;
    byte reserved[4];
    byte ipaddr[16];
    options opt;
}nsolicit;

typedef struct{
    unsigned int type;
    unsigned int code;
    unsigned short checksum;
    unsigned int R: 1;
    unsigned int S: 1;
    unsigned int O: 1;
    unsigned int res: 29;
    byte ipaddr[16];
    options opt;
}nadvert;

