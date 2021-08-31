/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:34:28 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/31 19:38:44 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include "libft.h"
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>
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
** resp_addr_in: sockaddr_in response address
** resp_ip: ip string from inet_ntop
** iov_buffer: raw data read from socket
** iov: io vector (structure of buffers for recvmsg)
** msg_buffer: data passed through the kernel
** rd: number of bytes read from socket
** response: ECHO_REPLY response structure for recvmsg call
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
	struct sockaddr_in	resp_addr_in;
	char				resp_ip[INET_ADDRSTRLEN + 1];
	char				iov_buffer[MSG_BUFLEN];
	struct iovec		iov;
	char				msg_buffer[MSG_BUFLEN];
	ssize_t				rd;
	struct msghdr		response;
	struct ip			*resp_ip_hdr;
	struct icmphdr		*resp_icmp_hdr;
}						t_pingcfg;

/*
** Global instance of the structure
*/
extern t_pingcfg		*g_cfg;

# define	FT_PING_OPT		"vh"
# define	FT_PING_HELP	"Usage:\n\t%s [options] <destination>\n"\
	"Options:\n\t<destination>\t\thostname or IPv4 address\n"\
	"\t-h\t\t\tprint help and exit\n\t-v\t\t\tverbose output\n"
# define	PING_TTL		255
# define	PING_TIMEOUT	5

#endif
