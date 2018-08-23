#ifndef _T_NETLIB_H
#define _T_NETLIB_H
#include <sys/socket.h>
#include <netdb.h>

struct hostent *e_gethostbyname(char *hostname);
struct servent *e_getservbyname(char *servname, char *proto);
int 	tcp_listen( char *hostname, char *service );
int 	tcp_connect( char *hostname, char *servicename, char *localhost, char *localservicename );
int 	tcp_close( int sock );
int 	tcp_read( register int fd, register char *ptr, register int n );
int		tcp_write( register int fd, register char *ptr, register int n );
char	*get_client_ip( int sock );
int 	get_client_port( int sock );
char 	*get_server_ip( int sock );
int 	get_server_port( int sock );
int		is_same_addr( const char *addr1, const char *addr2 );
#endif
