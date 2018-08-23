#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "t_tools.h"
#include "t_dlink.h"


char	g_sBuf[128];

struct hostent *e_gethostbyname(char *hostname)
{
	static struct hostent host;
	static unsigned long addr;
	static char *paddr;

/*	DelSpace(hostname); */
	if (!strlen(hostname)) return((struct hostent *)0);
	addr = inet_addr(hostname);
	if( addr != INADDR_NONE ){
		memset(&host, 0, sizeof(host));
		host.h_length = sizeof(addr);
		host.h_addr_list = &paddr;
		paddr = (char *)&addr;
		return(&host);
	} else
		return(gethostbyname(hostname));
}

struct servent *e_getservbyname(char *servname, char *proto)
{
	static struct servent serv;
	struct servent *sp;
	int i;

/*	DelSpace(servname); */
	if (!strlen(servname)) return((struct servent *)0);
	sp = getservbyname(servname, proto);
	if (!sp) {
		for (i = 0; i < strlen(servname); i++)
			if ((servname[i] < '0') || (servname[i] > '9')) break;
		if (i == strlen(servname)) {
			memset(&serv, 0, sizeof(serv));
			serv.s_port = htons(atoi(servname));
			sp = &serv;
		}
	}
	return(sp);
}

int tcp_listen( char *hostname, char *service )
{
	struct servent *sp;
	struct sockaddr_in local;
	struct hostent* h;
	struct	linger ling;
 	int sock;
	int on;

	if ( (sp = e_getservbyname(service, "tcp")) == NULL )
	{
		return(-1);
	}

	if (!(h = e_gethostbyname(hostname)))
	{
		return(-1);
	}

	bzero((char *)&local, sizeof(local));
	local.sin_family = AF_INET;
    bcopy(h->h_addr, &local.sin_addr, h->h_length);
	local.sin_port = sp->s_port;

	if ( (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
	{
		return(-2);
	}

	on=1;
	ling.l_onoff = 1;
	ling.l_linger = 0;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(int));
	setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(int));
	setsockopt(sock,SOL_SOCKET,SO_LINGER,&ling,sizeof(struct linger));

	if ( bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0 )
	{
		close(sock);
		return(-3);
	}

	if (listen(sock, 1000 ) < 0){
                close(sock);
                return(-4);
        }

	return(sock);
}

int tcp_connect( char *hostname, char *servicename, char *localhost, char *localservicename )
{
    int    sock;
    struct servent *sp;
    struct sockaddr_in remote, local;
    struct hostent* h;

    if ( (sp = e_getservbyname(servicename, "tcp")) == NULL )
    {
        return(-1);
    }

    if (!(h = e_gethostbyname(hostname)))
    {
        return(-1);
    }

    bzero(&remote, sizeof(remote));
    remote.sin_family = AF_INET;
    bcopy(h->h_addr, &remote.sin_addr, h->h_length);
    remote.sin_port = sp->s_port;

    if ( (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 )
    {
        return(-1);
    }

    if ( localhost[0] == '\0' || localservicename[0] == '\0' )
    {
        bzero(&local, sizeof(local));
        local.sin_family = AF_INET;

        if ( localhost[0] == '\0' )
            local.sin_addr.s_addr = htonl( INADDR_ANY );
        else
        {
            if (!(h = e_gethostbyname(localhost)))
            {
                return(-1);
            }
            bcopy(h->h_addr, &local.sin_addr, h->h_length);
        }

        if ( localservicename[0] == '\0' )
        {
            local.sin_port = 0;
        }
        else
        {
            if ( (sp = e_getservbyname(localservicename, "tcp")) == NULL )
            {
                return(-1);
            }
            local.sin_port = sp->s_port;
        }

        if ( bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0 )
        {
            close(sock);
            return(-1);
        }
    }

    if ( connect(sock, (struct sockaddr *)&remote, sizeof(remote)) < 0 )
    {
        close(sock);
        return(-1);
    }

    return(sock);
}

int tcp_close( int sock )
{
	close(sock);
        return 0;
}

int tcp_read( register int fd, register char *ptr, register int n )
{
	int nleft = n, nread;

	while ( nleft > 0 )
	{
		nread = read(fd, ptr, nleft);
		if (nread < 0)
		{
			return(-1);
		}
		else if (nread == 0) break;
/*
{
	char tmp[2000]; int i;
	tmp[0] = 0;
	for (i = 0; i < nread; i++)
		sprintf(tmp + strlen(tmp), "%02x ", (unsigned char)(ptr[i]));
	write_debug("%s", tmp);
}
*/
		nleft -= nread;
		ptr += nread;
	}
	return(n - nleft);
}

int tcp_write( register int fd, register char *ptr, register int n )
{
	int nleft = n, wc;

	while ( nleft > 0 )
	{
		wc = write(fd, ptr, nleft);
		if ( wc <= 0 )
		{
			return(-1);
		}
		nleft -= wc;
		ptr += wc;
	}
	return(n - nleft);
}

char *get_client_ip( int sock )
{
	struct sockaddr_in sa;
	int sa_len;

	sa_len = sizeof(struct sockaddr_in);
	getpeername( sock, (struct sockaddr *)&sa, (socklen_t *)&sa_len );
	sprintf( g_sBuf, "%s", inet_ntoa( sa.sin_addr ) );
	return ( g_sBuf );
}

int get_client_port( int sock )
{
	struct sockaddr_in sa;
	int sa_len;

	sa_len = sizeof(struct sockaddr_in);
	getpeername( sock, (struct sockaddr *)&sa, (socklen_t *)&sa_len );
	return ntohs( sa.sin_port );
}


char *get_server_ip( int sock )
{
	struct sockaddr_in sa;
	int sa_len;

	sa_len = sizeof(struct sockaddr_in);
	getsockname( sock, (struct sockaddr *)&sa, (socklen_t *)&sa_len );
	sprintf( g_sBuf, "%s", inet_ntoa( sa.sin_addr ) );
	return ( g_sBuf );
}


int get_server_port( int sock )
{
	struct sockaddr_in sa;
	int sa_len;

	sa_len = sizeof(struct sockaddr_in);
	getsockname( sock, (struct sockaddr *)&sa, (socklen_t *)&sa_len );
	return ntohs( sa.sin_port );
}


int	is_same_addr( const char *addr1, const char *addr2 )
{
	struct hostent *host;
	char	**addrs;
	char	ip[29];

	memset( ip, 0, sizeof( ip ));

	host = gethostbyname( addr2 );
	if( host == NULL ) return 0;
	if( host->h_addrtype != AF_INET ) return 0;
	addrs = host->h_addr_list;
	strcpy( ip, (char *)inet_ntoa(*(struct in_addr *)*addrs ));

	return !strcmp( addr1, ip );
}
