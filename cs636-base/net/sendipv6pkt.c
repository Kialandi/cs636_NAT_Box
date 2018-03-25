#include <xinu.h>
uint8 fill_option_prefix(char* pkt);
uint8 fill_option_MTU(char* pkt) ;
uint8 fill_option_one(char* pkt) ;
void fillOptions(char* pkt, uint8* option_types, uint8 option_types_length);

void fill_dest_ip_all_routers(byte* dest) ;
void fill_dest_mac_all_router(byte* dest) ;
void    fillEthernet(struct netpacket *, byte *);
void    fillIPdatagram(struct base_header *, byte *, byte *);
void 	fillICMP(void *pkt, byte type, uint8* option_types, uint8 option_types_length);
bpid32  ipv6bufpool; //pool of buffers for IPV6

void makePseudoHdr(struct pseudoHdr * pseudo, struct rsolicit * pkt) {
    memset(pseudo, NULLCH, PSEUDOLEN);

    pseudo->dest[0] = 0xff;
    pseudo->dest[1] = 0x02;
    pseudo->dest[15] = 0x02;
    pseudo->len = htonl(ICMPSIZE);
    pseudo->next_header = IPV6_ICMP;

    memcpy(pseudo->icmppayload, pkt, ICMPSIZE);
}
/*
void    fillICMP(struct rsolicit * pkt) {
    pkt->type = ROUTERS;  
    //assumed code, checksum, and reserved were all set to 0 before coming in
    void * pseudo = (void *) getmem(PSEUDOLEN);
    makePseudoHdr((struct pseudoHdr *) pseudo, pkt);

    uint16 sum = checksumv6(pseudo, PSEUDOLEN);
    //kprintf("checksum: %d\n", sum);
    pkt->checksum = htons(sum);
    freemem(pseudo, PSEUDOLEN);
}
*/
void    fillICMP(void * pkt_icmp, byte type, uint8* option_types, uint8 option_types_length) {

    if (type == ROUTERS){ 

            struct rsolicit * pkt = pkt_icmp;
    	    pkt->type = type;
	    //assumed code, checksum, and reserved were all set to 0 before coming in
	    void * pseudo = (void *) getmem(PSEUDOLEN);
	    makePseudoHdr((struct pseudoHdr *) pseudo, pkt);

	    uint16 sum = checksumv6(pseudo, PSEUDOLEN);
	    //kprintf("checksum: %d\n", sum);
	    pkt->checksum = htons(sum);
	    freemem(pseudo, PSEUDOLEN);

    } else if (type == ROUTERA) {
	struct radvert * pkt = pkt_icmp;
	memcpy(pkt, &radvert_from_router, sizeof(struct radvert));
	pkt->type = type; 
	fillOptions((char*) pkt + 16,  option_types, option_types_length);
        	
	//assumed code, checksum, and reserved were all set to 0 before coming in
	void * pseudo = (void *) getmem(PSEUDOLEN);
	makePseudoHdr((struct pseudoHdr *) pseudo, pkt);

	uint16 sum = checksumv6(pseudo, PSEUDOLEN);
	//kprintf("checksum: %d\n", sum);
	pkt->checksum = htons(sum);
	freemem(pseudo, PSEUDOLEN);

   }

}

status  sendipv6pkt(byte type, byte * dest) {//byte[] destination, uint16 message) {
    struct netpacket * packet;
    byte dest_mac[ETH_ADDR_LEN];
    char dest_ip[IPV6_HDR_LEN];
    switch (type) {
        case ROUTERS:
            kprintf("Sending Router Solicitation...\n");

            uint32 len = ETH_HDR_LEN + IPV6_HDR_LEN + ICMPSIZE;
            packet = (struct netpacket *) getbuf(ipv6bufpool);
            memset((char *) packet, NULLCH, len);

	    fill_dest_mac_all_routers(dest_mac);

            fillEthernet(packet, dest_mac);
 
            char src_ip[IPV6_HDR_LEN];
	    memset(src_ip, NULLCH, IPV6_HDR_LEN);
            
	    fill_dest_ip_all_routers(dest_ip);
	   //fillIPdatagram((struct base_header *) ((char *) packet + ETH_HDR_LEN));
            fillIPdatagram((struct base_header *) ((char *) packet + ETH_HDR_LEN), src_ip, dest_ip);

            fillICMP((struct rsolicit *) ((char *) packet + ETH_HDR_LEN + IPV6_HDR_LEN), ROUTERS, NULL, 0);


            if( write(ETHER0, (char *) packet, len) == SYSERR) {
                kprintf("THE WORLD IS BURNING\n");
                kill(getpid());
            }

            //TODO: potential race condition if uncommented, ask Comer
            //freebuf((char *) packet);
            return 1;

        case ROUTERA:
	    kprintf("\nSending Router Advertisement...\n");
            //normal packet sending
            packet = (struct netpacket *) getbuf(ipv6bufpool);
            memset((char*) packet, NULLCH, PACKLEN);

	    fill_dest_mac_all_nodes(dest_mac);
	    fillEthernet(packet, dest_mac);
	    kprintf("======Ethernet Header=========\n");            
     	    payload_hexdump(packet, ETH_HDR_LEN);
	    fill_dest_ip_all_nodes(dest_ip);
           // kprintf("test dest_ip\n");
	   // print_ipv6_addr(dest_ip); 
      
           fillIPdatagram((struct base_header *) ((char *) packet + ETH_HDR_LEN), link_local, dest_ip);   		 
 	   kprintf("=======IP header ==============\n");
   	   payload_hexdump((char*)packet + ETH_HDR_LEN, sizeof(struct base_header));
	   uint8 option_types[3] = {1, 5, 3};
           fillICMP((struct radvert *) ((char *) packet + ETH_HDR_LEN + IPV6_HDR_LEN), ROUTERA, option_types, 3);
            if( write(ETHER0, (char *) packet, len) == SYSERR) {
                kprintf("THE WORLD IS BURNING\n");
                kill(getpid());
            }
            //printPacket(packet);
	    return 1;
    }
    //TODO: figure out the correct status to send back, reaching here is bad
    return 0;
}

void fillOptions(char* pkt, uint8* option_types, uint8 option_types_length) {
	uint8 i = 0;
	uint8 length = 0;
	while (i < option_types_length) {
		switch(option_types[i]) {
			case 1:
				length = fill_option_one(pkt);
				kprintf("option_one:\n");
				payload_hexdump(pkt, length);
				break;
			case 5:
				length = fill_option_MTU(pkt);
				kprintf("option_MTU:\n");
			  	payload_hexdump(pkt, length);
 				break;
			case 3:
				length = fill_option_prefix(pkt);
				kprintf("option_prefix:\n");
			  	payload_hexdump(pkt, length);
				break;
			default:
				break;

 		}
		pkt = pkt + length;
		i++;
       }

}

uint8 fill_option_prefix(char* pkt) {
	memcpy(pkt, &option_prefix_default, sizeof(option_prefix));
        return sizeof(struct option_prefix);
}

uint8 fill_option_MTU(char* pkt) {
	struct option_MTU * temp = pkt;
	memset(temp, NULLCH, sizeof(struct option_MTU));
	temp->type = 0x05;
	temp->length = 0x01;
	temp->payload = htons(MTU);
	return sizeof(struct option_MTU);
}

uint8 fill_option_one(char* pkt) {
	struct option_one* temp = pkt;
	temp->type = 0x01;
	temp->length = 0x01;
	memcpy((char*)temp + 2, if_tab[ifprime].if_macucast, ETH_ADDR_LEN);
	return sizeof(struct option_one); // equal to length(1) * 8 bytes
}
void fill_dest_ip_all_nodes(byte* dest_ip) {
	memset(dest_ip, NULLCH, IPV6_HDR_LEN);
	dest_ip[0] = 0xff;
	dest_ip[1] = 0x02;
	dest_ip[15] = 0x01;

}
void fill_dest_ip_all_routers(byte* dest) {
	memset(dest, NULLCH, IPV6_HDR_LEN);
	dest[0] = 0xff;
	dest[1] = 0x02;
	dest[15] = 0x02;

}
void fill_dest_mac_all_nodes(byte* dest) {
	
	dest[0] = 0x33;
	dest[1] = 0x33;
	dest[2] = 0x00;
	dest[3] = 0x00;
	dest[4] = 0x00;
	dest[5] = 0x01;
}
void fill_dest_mac_all_routers(byte* dest) {
	dest[0] = 0x33;
	dest[1] = 0x33;
	dest[2] = 0x00;
	dest[3] = 0x00;
	dest[4] = 0x00;
	dest[5] = 0x02;
}

void    fillEthernet(struct netpacket * pkt, byte* dest) {

    memcpy(&pkt->net_dst, dest, ETH_ADDR_LEN);
    kprintf("if_prime:%d\n", ifprime);
    memcpy(&pkt->net_src, if_tab[ifprime].if_macucast, ETH_ADDR_LEN);
    pkt->net_type = htons(ETH_IPv6);
}

void    fillIPdatagram(struct base_header * pkt, byte* src_ip, byte* dest_ip) {
    pkt->info[0] = 0x60;
    pkt->info[1] = 0x00;
    pkt->info[2] = 0x00;
    pkt->info[3] = 0x00;

    pkt->payload_len = htons(ICMPSIZE);
    pkt->next_header = IPV6_ICMP;
    pkt->hop_limit = 255;

    memcpy(&(pkt->src), src_ip, IPV6_HDR_LEN);
    memcpy(&(pkt->dest), dest_ip, IPV6_HDR_LEN); 
    //kprintf("pkt src:\n");
    //print_ipv6_addr(&(pkt->src));
    //kprintf("pkt dest:\n");
    //print_ipv6_addr(&(pkt->dest));
    /*
    //set IP dest addr, assumes src is non-specified (all 0s)
    pkt->dest[0] = 0xff;
    pkt->dest[1] = 0x02;
    pkt->dest[15] = 0x02;
    */
}
