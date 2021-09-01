/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:34:28 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/01 04:13:59 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include "libft.h"
# include <signal.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>
# include <sys/time.h>
# include <errno.h>

# define	PINGPACK_SIZE	64

/*
** Ping packet structure
**
** hdr: icmp header structure
** data: data buffer
*/

typedef struct			s_ping_packet
{
	struct icmphdr		hdr;
	char				data[PINGPACK_SIZE - sizeof(struct icmphdr)];
}						t_ping_packet;

# define	MSG_BUFLEN		1024
//TODO: maybe remove msg_buffer as it seems to be useless
/*
** Ping configuration structure
**
** exec_name: name of the ft_ping executable
** dest: destination given by user
** verbose: boolean set to 1 if verbose mode is on
** destinfo: result of getaddrinfo call (to be freed)
** dest_addr_in: sockaddr_in cast of sockaddr pointer
** dest_ip: ip string from inet_ntop
** request: ECHO_REQUEST packet to be sent
** resp_ip: ip string from inet_ntop
** iov_buffer: raw data read from socket
** iov: io vector (structure of buffers for recvmsg)
** msg_buffer: data passed through the kernel
** rd: number of bytes read from socket
** response: ECHO_REPLY response structure for recvmsg call
** resp_ip_hdr: ip header cast of the reply packet
** resp_icmp_hdr: icmp header cast of the reply packet
** sent: number of packets successfully sent
** received: number of packets successfully received
** err: error string (will exit if set)
** sockfd: file descriptor of socket
*/
typedef struct			s_pingcfg
{
	const char			*exec_name;
	const char			*dest;
	int					verbose;
	struct addrinfo		*destinfo;
	struct sockaddr_in	*dest_addr_in;
	char				dest_ip[INET_ADDRSTRLEN + 1];
	t_ping_packet		request;
	char				resp_ip[INET_ADDRSTRLEN + 1];
	char				iov_buffer[MSG_BUFLEN];
	struct iovec		iov;
	char				msg_buffer[MSG_BUFLEN];
	ssize_t				rd;
	struct msghdr		response;
	struct ip			*resp_ip_hdr;
	struct icmphdr		*resp_icmp_hdr;
	unsigned int		sent;
	unsigned int		received;
	char				*err;
	int					sockfd;
	struct timeval		start;
	struct timeval		end;
}						t_pingcfg;

/*
** Global instance of the structure
*/
extern t_pingcfg		*g_cfg;

/*
** Ping macros
*/
# define	FT_PING_OPT		"vh"
# define	FT_PING_HELP	"Usage:\n\t%s [options] <destination>\n"\
	"Options:\n\t<destination>\t\thostname or IPv4 address\n"\
	"\t-h\t\t\tprint help and exit\n\t-v\t\t\tverbose output\n"
# define	PING_TTL		255
# define	PING_TIMEOUT	5

/*
** Ping reply errors
*/
enum e_ping_errors {
	PING_IP_HDR		= 0x01,
	PING_IP_SOURCE	= 0x02,
	PING_ICMP_HDR	= 0x04,
	PING_ICMP_TYPE	= 0x08,
};

#endif
