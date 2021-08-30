/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:34:28 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/30 19:05:26 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
# define FT_PING_H

# include "libft.h"
# include <netdb.h>
# include <arpa/inet.h>

/*
** Ping configuration structure
**
** exec_name: name of the ft_ping executable
*/
typedef struct			s_pingcfg
{
	const char			*exec_name;
	const char			*dest;
	int					verbose;
	struct addrinfo		*destinfo;
	struct sockaddr_in	*addr_in;
	char				ip[INET_ADDRSTRLEN + 1];
}						t_pingcfg;

/*
** Global instance of the structure
*/
extern t_pingcfg		*g_cfg;

#define	FT_PING_OPT		"vh"
#define	FT_PING_HELP	"Usage:\n\t%s [options] <destination>\n"\
	"Options:\n\t<destination>\t\thostname or IPv4 address\n"\
	"\t-h\t\t\tprint help and exit\n\t-v\t\t\tverbose output\n"

#endif
