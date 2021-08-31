/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:34:28 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/31 11:56:25 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include "libft.h"
# include <netdb.h>
# include <arpa/inet.h>
# include <netinet/ip_icmp.h>
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

/*
** Ping configuration structure
**
** exec_name: name of the ft_ping executable
** dest: destination given by user
** verbose: boolean set to 1 if verbose mode is on
** destinfo: result of getaddrinfo call (to be freed)
** addr_in: sockaddr_in cast of sockaddr pointer
** ip: ip string from inet_ntop
** request: ECHO_REQUEST packet to be sent
*/
typedef struct			s_pingcfg
{
	const char			*exec_name;
	const char			*dest;
	int					verbose;
	struct addrinfo		*destinfo;
	struct sockaddr_in	*addr_in;
	char				ip[INET_ADDRSTRLEN + 1];
	t_ping_packet		request;
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
