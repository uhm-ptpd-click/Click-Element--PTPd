/**
 * @file   net.c
 * @date   Tue Jul 20 16:17:49 2010
 * 
 * @brief  Functions to interact with the network sockets and NIC driver.
 * 
 * 
 */

#include "ptpd.hh"
#include <netdb.h>

/* shut down the UDP stuff */
bool 
netShutdown(NetPath * netPath)
{
	struct ip_mreq imr;

	/* Close General Multicast */
	imr.imr_multiaddr.s_addr = netPath->multicastAddr;
	imr.imr_interface.s_addr = htonl(INADDR_ANY);

	setsockopt(netPath->eventSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, 
		   &imr, sizeof(struct ip_mreq));
	setsockopt(netPath->generalSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, 
		   &imr, sizeof(struct ip_mreq));

	/* Close Peer Multicast */
	imr.imr_multiaddr.s_addr = netPath->peerMulticastAddr;
	imr.imr_interface.s_addr = htonl(INADDR_ANY);

	setsockopt(netPath->eventSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, 
		   &imr, sizeof(struct ip_mreq));
	setsockopt(netPath->generalSock, IPPROTO_IP, IP_DROP_MEMBERSHIP, 
		   &imr, sizeof(struct ip_mreq));


	netPath->multicastAddr = 0;
	netPath->unicastAddr = 0;
	netPath->peerMulticastAddr = 0;

	/* Close sockets */
	if (netPath->eventSock > 0)
		close(netPath->eventSock);
	netPath->eventSock = -1;

	if (netPath->generalSock > 0)
		close(netPath->generalSock);
	netPath->generalSock = -1;

	return TRUE;
}

/*Test if network layer is OK for PTP*/
UInteger8 
lookupCommunicationTechnology(UInteger8 communicationTechnology)
{
#if defined(linux)

	switch (communicationTechnology) {
	case ARPHRD_ETHER:
	case ARPHRD_EETHER:
	case ARPHRD_IEEE802:
		return PTP_ETHER;

	default:
		break;
	}

#elif defined(BSD_INTERFACE_FUNCTIONS)

#endif

	return PTP_DEFAULT;
}


 /* Find the local network interface */
UInteger32 
findIface(Octet * ifaceName, UInteger8 * communicationTechnology,
    Octet * uuid, NetPath * netPath)
{
#if defined(linux)

	/* depends on linux specific ioctls (see 'netdevice' man page) */
	unsigned int i, flags;
	struct ifconf data;
	struct ifreq device[IFCONF_LENGTH];

	data.ifc_len = sizeof(device);
	data.ifc_req = device;

	memset(data.ifc_buf, 0, data.ifc_len);

	flags = IFF_UP | IFF_RUNNING | IFF_MULTICAST;

	/* look for an interface if none specified */
	if (ifaceName[0] != '\0') {
		i = 0;
		memcpy(device[i].ifr_name, ifaceName, IFACE_NAME_LENGTH);

		if (ioctl(netPath->eventSock, SIOCGIFHWADDR, &device[i]) < 0)
			DBGV("failed to get hardware address\n");
		else if ((*communicationTechnology = 
			  lookupCommunicationTechnology(
				  device[i].ifr_hwaddr.sa_family)) 
			 == PTP_DEFAULT)
			DBGV("unsupported communication technology (%d)\n", 
			     *communicationTechnology);
		else
			memcpy(uuid, device[i].ifr_hwaddr.sa_data, 
			       PTP_UUID_LENGTH);
	} else {
		/* no iface specified */
		/* get list of network interfaces */
		if (ioctl(netPath->eventSock, SIOCGIFCONF, &data) < 0) {
			PERROR("failed query network interfaces");
			return 0;
		}
		if ((unsigned int)data.ifc_len >= sizeof(device))
			DBG("device list may exceed allocated space\n");

		/* search through interfaces */
		for (i = 0; i < (unsigned int)data.ifc_len / sizeof(device[0]); ++i) {
			DBGV("%d %s %s\n", i, device[i].ifr_name, 
			     inet_ntoa(((struct sockaddr_in *)
					&device[i].ifr_addr)->sin_addr));

			if (ioctl(netPath->eventSock, SIOCGIFFLAGS, 
				  &device[i]) < 0)
				DBGV("failed to get device flags\n");
			else if ((device[i].ifr_flags & flags) != flags)
				DBGV("does not meet requirements"
				     "(%08x, %08x)\n", device[i].ifr_flags, 
				     flags);
			else if (ioctl(netPath->eventSock, SIOCGIFHWADDR, 
				       &device[i]) < 0)
				DBGV("failed to get hardware address\n");
			else if ((*communicationTechnology = 
				  lookupCommunicationTechnology(
					  device[i].ifr_hwaddr.sa_family)) 
				 == PTP_DEFAULT)
				DBGV("unsupported communication technology"
				     "(%d)\n", *communicationTechnology);
			else {
				DBGV("found interface (%s)\n", 
				     device[i].ifr_name);
				memcpy(uuid, device[i].ifr_hwaddr.sa_data, 
				       PTP_UUID_LENGTH);
				memcpy(ifaceName, device[i].ifr_name, 
				       IFACE_NAME_LENGTH);
				break;
			}
		}
	}

	if (ifaceName[0] == '\0') {
		ERROR("failed to find a usable interface\n");
		return 0;
	}
	if (ioctl(netPath->eventSock, SIOCGIFADDR, &device[i]) < 0) {
		PERROR("failed to get ip address");
		return 0;
	}
	return ((struct sockaddr_in *)&device[i].ifr_addr)->sin_addr.s_addr;

#elif defined(BSD_INTERFACE_FUNCTIONS)

	struct ifaddrs *if_list, *ifv4, *ifh;

	if (getifaddrs(&if_list) < 0) {
		PERROR("getifaddrs() failed");
		return FALSE;
	}
	/* find an IPv4, multicast, UP interface, right name(if supplied) */
	for (ifv4 = if_list; ifv4 != NULL; ifv4 = ifv4->ifa_next) {
		if ((ifv4->ifa_flags & IFF_UP) == 0)
			continue;
		if ((ifv4->ifa_flags & IFF_RUNNING) == 0)
			continue;
		if ((ifv4->ifa_flags & IFF_LOOPBACK))
			continue;
		if ((ifv4->ifa_flags & IFF_MULTICAST) == 0)
			continue;
                /* must have IPv4 address */
		if (ifv4->ifa_addr->sa_family != AF_INET)
			continue;
		if (ifaceName[0] && strncmp(ifv4->ifa_name, ifaceName, 
					    IF_NAMESIZE) != 0)
			continue;
		break;
	}

	if (ifv4 == NULL) {
		if (ifaceName[0]) {
			ERROR("interface \"%s\" does not exist,"
			      "or is not appropriate\n", ifaceName);
			return FALSE;
		}
		ERROR("no suitable interfaces found!");
		return FALSE;
	}
	/* find the AF_LINK info associated with the chosen interface */
	for (ifh = if_list; ifh != NULL; ifh = ifh->ifa_next) {
		if (ifh->ifa_addr->sa_family != AF_LINK)
			continue;
		if (strncmp(ifv4->ifa_name, ifh->ifa_name, IF_NAMESIZE) == 0)
			break;
	}

	if (ifh == NULL) {
		ERROR("could not get hardware address for interface \"%s\"\n", 
		      ifv4->ifa_name);
		return FALSE;
	}
	/* check that the interface TYPE is OK */
	if (((struct sockaddr_dl *)ifh->ifa_addr)->sdl_type != IFT_ETHER) {
		ERROR("\"%s\" is not an ethernet interface!\n", ifh->ifa_name);
		return FALSE;
	}
	DBG("==> %s %s %s\n", ifv4->ifa_name,
	    inet_ntoa(((struct sockaddr_in *)ifv4->ifa_addr)->sin_addr),
	    ether_ntoa((struct ether_addr *)
		       LLADDR((struct sockaddr_dl *)ifh->ifa_addr))
	    );

	*communicationTechnology = PTP_ETHER;
	memcpy(ifaceName, ifh->ifa_name, IFACE_NAME_LENGTH);
	memcpy(uuid, LLADDR((struct sockaddr_dl *)ifh->ifa_addr), 
	       PTP_UUID_LENGTH);

	return ((struct sockaddr_in *)ifv4->ifa_addr)->sin_addr.s_addr;

#endif
}


/** 
 * start all of the UDP stuff 
 * must specify 'subdomainName', and optionally 'ifaceName', 
 * if not then pass ifaceName == "" 
 * on socket options, see the 'socket(7)' and 'ip' man pages 
 *
 * @param netPath 
 * @param rtOpts 
 * @param ptpClock 
 * 
 * @return TRUE if successful
 */
bool 
netInit(NetPath * netPath, RunTimeOpts * rtOpts, PtpClock * ptpClock)
{
	int temp;
	struct in_addr interfaceAddr, netAddr;
	struct sockaddr_in addr;
	struct ip_mreq imr;
	char addrStr[NET_ADDRESS_LENGTH];

	DBG("netInit\n");

	/* open sockets */
	if ((netPath->eventSock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0
	    || (netPath->generalSock = socket(PF_INET, SOCK_DGRAM, 
					      IPPROTO_UDP)) < 0) {
		PERROR("failed to initalize sockets");
		return FALSE;
	}
	/* find a network interface */
	if (!(interfaceAddr.s_addr = 
	      findIface(rtOpts->ifaceName, 
			&ptpClock->port_communication_technology,
			ptpClock->port_uuid_field, netPath)))
		return FALSE;

	DBG("Local IP address used : %s \n", inet_ntoa(interfaceAddr));

	temp = 1;			/* allow address reuse */
	if (setsockopt(netPath->eventSock, SOL_SOCKET, SO_REUSEADDR, 
		       &temp, sizeof(int)) < 0
	    || setsockopt(netPath->generalSock, SOL_SOCKET, SO_REUSEADDR, 
			  &temp, sizeof(int)) < 0) {
		DBG("failed to set socket reuse\n");
	}
	/* bind sockets */
	/*
	 * need INADDR_ANY to allow receipt of multi-cast and uni-cast
	 * messages
	 */
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PTP_EVENT_PORT);
	if (bind(netPath->eventSock, (struct sockaddr *)&addr, 
		 sizeof(struct sockaddr_in)) < 0) {
		PERROR("failed to bind event socket");
		return FALSE;
	}
	addr.sin_port = htons(PTP_GENERAL_PORT);
	if (bind(netPath->generalSock, (struct sockaddr *)&addr, 
		 sizeof(struct sockaddr_in)) < 0) {
		PERROR("failed to bind general socket");
		return FALSE;
	}
	/* send a uni-cast address if specified (useful for testing) */
	if (rtOpts->unicastAddress[0]) {
		/* Attempt a DNS lookup first. */
		struct hostent *host;
		host = gethostbyname2(rtOpts->unicastAddress, AF_INET);
                if (host != NULL) {
			if (host->h_length != 4) {
				PERROR("unicast host resolved to non ipv4"
				       "address");
				return FALSE;
			}
			netPath->unicastAddr = 
				*(uint32_t *)host->h_addr_list[0];
		} else {
			/* Maybe it's a dotted quad. */
			if (!inet_aton(rtOpts->unicastAddress, &netAddr)) {
				ERROR("failed to encode uni-cast address: %s\n",
				      rtOpts->unicastAddress);
				return FALSE;
				netPath->unicastAddr = netAddr.s_addr;
			}
                }
        } else
                netPath->unicastAddr = 0;
 
	/* Init General multicast IP address */
	memcpy(addrStr, DEFAULT_PTP_DOMAIN_ADDRESS, NET_ADDRESS_LENGTH);

	if (!inet_aton(addrStr, &netAddr)) {
		ERROR("failed to encode multi-cast address: %s\n", addrStr);
		return FALSE;
	}
	netPath->multicastAddr = netAddr.s_addr;

	/* multicast send only on specified interface */
	imr.imr_multiaddr.s_addr = netAddr.s_addr;
	imr.imr_interface.s_addr = interfaceAddr.s_addr;
	if (setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_IF, 
		       &imr.imr_interface.s_addr, sizeof(struct in_addr)) < 0
	    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_IF, 
			  &imr.imr_interface.s_addr, sizeof(struct in_addr)) 
	    < 0) {
		PERROR("failed to enable multi-cast on the interface");
		return FALSE;
	}
	/* join multicast group (for receiving) on specified interface */
	if (setsockopt(netPath->eventSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
		       &imr, sizeof(struct ip_mreq)) < 0
	    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
			  &imr, sizeof(struct ip_mreq)) < 0) {
		PERROR("failed to join the multi-cast group");
		return FALSE;
	}
	/* End of General multicast Ip address init */

	/* Init Peer multicast IP address */
	memcpy(addrStr, PEER_PTP_DOMAIN_ADDRESS, NET_ADDRESS_LENGTH);

	if (!inet_aton(addrStr, &netAddr)) {
		ERROR("failed to encode multi-cast address: %s\n", addrStr);
		return FALSE;
	}
	netPath->peerMulticastAddr = netAddr.s_addr;

	/* multicast send only on specified interface */
	imr.imr_multiaddr.s_addr = netAddr.s_addr;
	imr.imr_interface.s_addr = interfaceAddr.s_addr;
	if (setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_IF, 
		       &imr.imr_interface.s_addr, sizeof(struct in_addr)) < 0
	    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_IF, 
			  &imr.imr_interface.s_addr, sizeof(struct in_addr)) 
	    < 0) {
		PERROR("failed to enable multi-cast on the interface");
		return FALSE;
	}
	/* join multicast group (for receiving) on specified interface */
	if (setsockopt(netPath->eventSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
		       &imr, sizeof(struct ip_mreq)) < 0
	    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
			  &imr, sizeof(struct ip_mreq)) < 0) {
		PERROR("failed to join the multi-cast group");
		return FALSE;
	}
	/* End of Peer multicast Ip address init */


	/* set socket time-to-live to 1 */

	if (setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_TTL, 
		       &rtOpts->ttl, sizeof(int)) < 0
	    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_TTL, 
			  &rtOpts->ttl, sizeof(int)) < 0) {
		PERROR("failed to set the multi-cast time-to-live");
		return FALSE;
	}
	/* enable loopback */
	temp = 1;
	if (setsockopt(netPath->eventSock, IPPROTO_IP, IP_MULTICAST_LOOP, 
		       &temp, sizeof(int)) < 0
	    || setsockopt(netPath->generalSock, IPPROTO_IP, IP_MULTICAST_LOOP, 
			  &temp, sizeof(int)) < 0) {
		PERROR("failed to enable multi-cast loopback");
		return FALSE;
	}
	/* make PTP_Timestamps available through recvmsg() */
	temp = 1;
#if defined(linux)
	if (setsockopt(netPath->eventSock, SOL_SOCKET, SO_TIMESTAMP, 
		       &temp, sizeof(int)) < 0
	    || setsockopt(netPath->generalSock, SOL_SOCKET, SO_TIMESTAMP, 
			  &temp, sizeof(int)) < 0) {
		PERROR("failed to enable receive time stamps");
		return FALSE;
	}
#else /* FreeBSD */
	if (setsockopt(netPath->eventSock, SOL_SOCKET, SO_BINTIME, 
		       &temp, sizeof(int)) < 0
	    || setsockopt(netPath->generalSock, SOL_SOCKET, SO_BINTIME, 
			  &temp, sizeof(int)) < 0) {
		PERROR("failed to enable receive time stamps");
		return FALSE;
	}
#endif
	return TRUE;
}

/*Check if data have been received*/
int 
netSelect(TimeInternal * timeout, NetPath * netPath)
{
	int ret, nfds;
	fd_set readfds;
	struct timeval tv, *tv_ptr;

	if (timeout < 0)
		return FALSE;

	FD_ZERO(&readfds);
	FD_SET(netPath->eventSock, &readfds);
	FD_SET(netPath->generalSock, &readfds);

	if (timeout) {
		tv.tv_sec = timeout->seconds;
		tv.tv_usec = timeout->nanoseconds / 1000;
		tv_ptr = &tv;
	} else
		tv_ptr = 0;

	if (netPath->eventSock > netPath->generalSock)
		nfds = netPath->eventSock;
	else
		nfds = netPath->generalSock;

	ret = select(nfds + 1, &readfds, 0, 0, tv_ptr) > 0;

	if (ret < 0) {
		if (errno == EAGAIN || errno == EINTR)
			return 0;
	}
	return ret;
}




/** 
 * store received data from network to "buf" , get and store the
 * SO_PTP_Timestamp value in "time" for an event message
 * 
 * @param buf 
 * @param time 
 * @param netPath 
 * 
 * @return 
 */
ssize_t 
netRecvEvent(Octet * buf, TimeInternal * time, NetPath * netPath)
{
	ssize_t ret;
	struct msghdr msg;
	struct iovec vec[1];
	struct sockaddr_in from_addr;

	union {
		struct cmsghdr cm;
		char	control[CMSG_SPACE(sizeof(struct timeval))];
	}     cmsg_un;

	struct cmsghdr *cmsg;
#if defined(linux)
	struct timeval *tv;
#else
	struct timespec ts;
#endif

	vec[0].iov_base = buf;
	vec[0].iov_len = PACKET_SIZE;

	memset(&msg, 0, sizeof(msg));
	memset(&from_addr, 0, sizeof(from_addr));
	memset(buf, 0, PACKET_SIZE);
	memset(&cmsg_un, 0, sizeof(cmsg_un));

	msg.msg_name = (caddr_t)&from_addr;
	msg.msg_namelen = sizeof(from_addr);
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsg_un.control;
	msg.msg_controllen = sizeof(cmsg_un.control);
	msg.msg_flags = 0;

	ret = recvmsg(netPath->eventSock, &msg, MSG_DONTWAIT);
	if (ret <= 0) {
		if (errno == EAGAIN || errno == EINTR)
			return 0;

		return ret;
	}
	if (msg.msg_flags & MSG_TRUNC) {
		ERROR("received truncated message\n");
		return 0;
	}
	/* get time stamp of packet */
	if (!time) {
		ERROR("null receive time stamp argument\n");
		return 0;
	}
	if (msg.msg_flags & MSG_CTRUNC) {
		ERROR("received truncated ancillary data\n");
		return 0;
	}
	if (msg.msg_controllen < sizeof(cmsg_un.control)) {
		ERROR("received short ancillary data (%ld/%ld)\n",
		    (long)msg.msg_controllen, (long)sizeof(cmsg_un.control));

		return 0;
	}
#if defined(linux)
	tv = 0;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; 
	     cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET && 
		    cmsg->cmsg_type == SCM_TIMESTAMP)
			tv = (struct timeval *)CMSG_DATA(cmsg);
	}

	if (tv) {
		time->seconds = tv->tv_sec;
		time->nanoseconds = tv->tv_usec * 1000;
		DBGV("kernel recv time stamp %us %dns\n", 
		     time->seconds, time->nanoseconds);
#else /* FreeBSD has more accurate time stamps */
	bzero(&ts, sizeof(ts));
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; 
	     cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET && 
		    cmsg->cmsg_type == SCM_BINTIME)
			bintime2timespec((struct bintime *)CMSG_DATA(cmsg),
					 &ts);
	}

	if (ts.tv_sec != 0) {
		time->seconds = ts.tv_sec;
		time->nanoseconds = ts.tv_nsec;
		DBGV("kernel recv time stamp %us %dns\n", time->seconds, time->nanoseconds);
#endif /* Linux or FreeBSD */
	} else {
		/*
		 * do not try to get by with recording the time here, better
		 * to fail because the time recorded could be well after the
		 * message receive, which would put a big spike in the
		 * offset signal sent to the clock servo
		 */
		DBG("no receive time stamp\n");
		return 0;
	}

	return ret;
}



/** 
 * 
 * store received data from network to "buf" get and store the
 * SO_PTP_Timestamp value in "time" for a general message
 * 
 * @param buf 
 * @param time 
 * @param netPath 
 * 
 * @return 
 */

ssize_t 
netRecvGeneral(Octet * buf, TimeInternal * time, NetPath * netPath)
{
	ssize_t ret;
	struct msghdr msg;
	struct iovec vec[1];
	struct sockaddr_in from_addr;

	union {
		struct cmsghdr cm;
		char	control[CMSG_SPACE(sizeof(struct timeval))];
	}     cmsg_un;

	struct cmsghdr *cmsg;
#if defined(linux)
	struct timeval *tv;
#else
	struct timespec ts;
#endif 
	vec[0].iov_base = buf;
	vec[0].iov_len = PACKET_SIZE;

	memset(&msg, 0, sizeof(msg));
	memset(&from_addr, 0, sizeof(from_addr));
	memset(buf, 0, PACKET_SIZE);
	memset(&cmsg_un, 0, sizeof(cmsg_un));

	msg.msg_name = (caddr_t)&from_addr;
	msg.msg_namelen = sizeof(from_addr);
	msg.msg_iov = vec;
	msg.msg_iovlen = 1;
	msg.msg_control = cmsg_un.control;
	msg.msg_controllen = sizeof(cmsg_un.control);
	msg.msg_flags = 0;

	ret = recvmsg(netPath->generalSock, &msg, MSG_DONTWAIT);
	if (ret <= 0) {
		if (errno == EAGAIN || errno == EINTR)
			return 0;

		return ret;
	}
	if (msg.msg_flags & MSG_TRUNC) {
		ERROR("received truncated message\n");
		return 0;
	}
	/* get time stamp of packet */
	if (!time) {
		ERROR("null receive time stamp argument\n");
		return 0;
	}
	if (msg.msg_flags & MSG_CTRUNC) {
		ERROR("received truncated ancillary data\n");
		return 0;
	}
	if (msg.msg_controllen < sizeof(cmsg_un.control)) {
		ERROR("received short ancillary data (%ld/%ld)\n",
		    (long)msg.msg_controllen, (long)sizeof(cmsg_un.control));

		return 0;
	}
#if defined(linux)
	tv = 0;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; 
	     cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET && 
		    cmsg->cmsg_type == SCM_TIMESTAMP)
			tv = (struct timeval *)CMSG_DATA(cmsg);
	}

	if (tv) {
		time->seconds = tv->tv_sec;
		time->nanoseconds = tv->tv_usec * 1000;
		DBGV("kernel recv time stamp %us %dns\n", 
		     time->seconds, time->nanoseconds);
#else /* FreeBSD has more accurate time stamps */
	bzero(&ts, sizeof(ts));
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; 
	     cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET && 
		    cmsg->cmsg_type == SCM_BINTIME)
			bintime2timespec((struct bintime *)CMSG_DATA(cmsg),
					 &ts);
	}

	if (ts.tv_sec != 0) {
		time->seconds = ts.tv_sec;
		time->nanoseconds = ts.tv_nsec;
		DBGV("kernel recv time stamp %us %dns\n", time->seconds, time->nanoseconds);
#endif /* Linux or FreeBSD */
	} else {
		/*
		 * do not try to get by with recording the time here, better
		 * to fail because the time recorded could be well after the
		 * message receive, which would put a big spike in the
		 * offset signal sent to the clock servo
		 */
		DBG("no receive time stamp\n");
		return 0;
	}

	return ret;
}

ssize_t netSend(Octet *buf, UInteger16 length, Integer32 sock, Integer32 toaddr, UInteger16 toport)
{
        ssize_t ret;
        struct sockaddr_in addr;

        addr.sin_family = AF_INET;
        addr.sin_port = htons(toport);
        addr.sin_addr.s_addr = toaddr;

        ret = sendto(sock, buf, length, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
        if(ret <= 0)
                DBGV("error sending message\n");
        return ret;
}

ssize_t 
netSendEvent(Octet * buf, UInteger16 length, NetPath * netPath)
{
	ssize_t ret;

	ret = netSend(buf, length, netPath->eventSock, netPath->multicastAddr, PTP_EVENT_PORT);
	if (ret <= 0)
		DBG("error sending multi-cast event message\n");

	if (netPath->unicastAddr) {
		ret = netSend(buf, length, netPath->eventSock, netPath->unicastAddr, PTP_EVENT_PORT);
		if (ret <= 0)
			DBG("error sending uni-cast event message\n");
                // Must loop back the packet since are not using multicast. 
		ret = netSend(buf, length, netPath->eventSock, htonl(0x7f000001), PTP_EVENT_PORT);
		if (ret <= 0)
			DBG("error looping back uni-cast event message\n");
	}
	return ret;
}

ssize_t 
netSendGeneral(Octet * buf, UInteger16 length, NetPath * netPath)
{
	ssize_t ret;

	ret = netSend(buf, length, netPath->generalSock, netPath->multicastAddr, PTP_GENERAL_PORT);
	if (ret <= 0)
		DBG("error sending multi-cast general message\n");

	if (netPath->unicastAddr) {
		ret = netSend(buf, length, netPath->generalSock, netPath->unicastAddr, PTP_GENERAL_PORT);
		if (ret <= 0)
			DBG("error sending uni-cast general message\n");
	}
	return ret;
}

ssize_t 
netSendPeerGeneral(Octet * buf, UInteger16 length, NetPath * netPath)
{
	ssize_t ret;

	ret = netSend(buf, length, netPath->generalSock, netPath->peerMulticastAddr, PTP_GENERAL_PORT);
	if (ret <= 0)
		DBG("error sending multi-cast peer general message\n");

	if (netPath->unicastAddr) {
		ret = netSend(buf, length, netPath->generalSock, netPath->unicastAddr, PTP_GENERAL_PORT);
		if (ret <= 0)
			DBG("error sending uni-cast peer general message\n");
	}
	return ret;
}

ssize_t 
netSendPeerEvent(Octet * buf, UInteger16 length, NetPath * netPath)
{
	ssize_t ret;

	ret = netSend(buf, length, netPath->eventSock, netPath->peerMulticastAddr, PTP_EVENT_PORT);
	if (ret <= 0)
		DBG("error sending multi-cast peer event message\n");

	if (netPath->unicastAddr) {
		ret = netSend(buf, length, netPath->eventSock, netPath->unicastAddr, PTP_EVENT_PORT);
		if (ret <= 0)
			DBG("error sending uni-cast peer event message\n");
	}
	return ret;
}
