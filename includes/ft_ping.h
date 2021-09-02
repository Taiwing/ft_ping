/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:34:28 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/02 20:46:25 by yforeau          ###   ########.fr       */
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
# include <limits.h>

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
** dest_is_ip: boolean set to 1 if dest is an IP
** timeout: timeout value for ECHO_REPLY
** count: number of ECHO_REQUEST to send
** ttl: IP time to live
** verbose: boolean set to 1 if -v is set
** destinfo: result of getaddrinfo call (to be freed)
** dest_addr_in: sockaddr_in cast of sockaddr pointer
** iov_buffer: raw data read from socket
** msg_buffer: data passed through the kernel
** request: ECHO_REQUEST packet to be sent
** response: ECHO_REPLY response structure for recvmsg call
** dest_ip: ip string from inet_ntop
** resp_ip: ip string from inet_ntop
** iov: io vector (structure of buffers for recvmsg)
** rd: number of bytes read from socket
** resp_ip_hdr: ip header cast of the reply packet
** resp_icmp_hdr: icmp header cast of the reply packet
** sent: number of packets successfully sent
** received: number of packets successfully received
** errors: number of requests returning an error
** err: error string (will exit if set)
** sockfd: file descriptor of socket
** start_ts: timestamp at start of program
** end_ts: timestamp at end of program (on SIGINT)
** sent_ts: timestamp at sent echo request
** received_ts: timestamp at received echo reply
** min_ms: fastest reply
** max_ms: slowest reply
** sum_ms: sum of all reply times
** ssum_ms: sum of all reply times squared
** avg_ms: average reply (sum_ms / received)
** mdev_ms: standard deviation
*/
typedef struct			s_pingcfg
{
	const char			*exec_name;
	const char			*dest;
	int					dest_is_ip;
	int					timeout;
	int					count;
	int					ttl;
	int					verbose;
	struct addrinfo		*destinfo;
	struct sockaddr_in	*dest_addr_in;
	char				iov_buffer[MSG_BUFLEN];
	char				msg_buffer[MSG_BUFLEN];
	t_ping_packet		request;
	struct msghdr		response;
	char				dest_ip[INET_ADDRSTRLEN + 1];
	char				resp_ip[INET_ADDRSTRLEN + 1];
	struct iovec		iov;
	ssize_t				rd;
	struct ip			*resp_ip_hdr;
	struct icmphdr		*resp_icmp_hdr;
	unsigned int		sent;
	unsigned int		received;
	unsigned int		errors;
	char				*err;
	int					sockfd;
	struct timeval		start_ts;
	struct timeval		end_ts;
	struct timeval		sent_ts;
	struct timeval		received_ts;
	double				min_ms;
	double				max_ms;
	double				sum_ms;
	double				ssum_ms;
	double				avg_ms;
	double				mdev_ms;
}						t_pingcfg;

/*
** Global instance of the structure
*/
extern t_pingcfg		*g_cfg;

/*
** Ping macros
*/
# define	FT_PING_OPT		"W:c:t:vh"
# define	FT_PING_HELP	"Usage:\n\t%s [options] <destination>\n"\
	"Options:\n\t<destination>\t\thostname or IPv4 address\n"\
	"\t-W timeout\t\ttime to wait for a response, in seconds\n"\
	"\t-c count\t\tstop after sending count ECHO_REQUEST packets\n"\
	"\t-t ttl\t\t\tIP time to live\n"\
	"\t-v\t\t\tverbose output\n"\
	"\t-h\t\t\tprint help and exit\n"
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

/*
** Ping functions
*/
void			ping(int sig);
void			ping_int_handler(int sig);
void			ping_cleanup(void);
void			print_echo_reply(int rep_err);
unsigned int	reply_error(void);
unsigned short	checksum(unsigned short *data, size_t sz);
char			*get_options(int argc, char **argv);

#endif
