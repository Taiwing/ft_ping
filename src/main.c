/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:30:15 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/31 22:02:37 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static char	*get_options(int argc, char **argv)
{
	int			opt;
	t_optdata	optd;
	char		**args;

	ft_bzero((void *)&optd, sizeof(t_optdata));
	init_getopt(&optd, FT_PING_OPT, NULL, NULL);
	args = ft_memalloc((argc + 1) * sizeof(char *));
	ft_memcpy((void *)args, (void *)argv, argc * sizeof(char *));
	*args = (char *)g_cfg->exec_name;
	while ((opt = ft_getopt(argc, args, &optd)) == 'v')
		g_cfg->verbose = 1;
	if (g_cfg->verbose)
		ft_printf("%s: VERBOSE MODE ON\n", g_cfg->exec_name);
	if (opt != -1)
	{
		ft_printf(FT_PING_HELP, g_cfg->exec_name);
		ft_exit(NULL, opt != 'h');
	}
	ft_memdel((void **)&args);
	return (argv[optd.optind]);
}

static void	get_destinfo(void)
{
	int				ret;
	char			*err;
	struct addrinfo	hints;

	err = NULL;
	ft_bzero((void *)&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_ICMP;
	if ((ret = getaddrinfo(g_cfg->dest, NULL, NULL, &g_cfg->destinfo)))
		ft_asprintf(&err, "%s: %s", g_cfg->dest, gai_strerror(ret));
	if (!err)
		g_cfg->dest_addr_in = (struct sockaddr_in *)g_cfg->destinfo->ai_addr;
	if (!err && !inet_ntop(AF_INET, (void *)&g_cfg->dest_addr_in->sin_addr,
		g_cfg->dest_ip, INET_ADDRSTRLEN))
		ft_asprintf(&err, "inet_ntop: %s", strerror(errno));
	if (err)
		ft_exit(err, EXIT_FAILURE);
}

static void	ping_cleanup(void)
{
	if (g_cfg->destinfo)
		freeaddrinfo(g_cfg->destinfo);
}

static int	setup_socket(void)
{
	int				ttl;
	char			*err;
	int				sockfd;
	struct timeval	timeout;

	err = NULL;
	ttl = PING_TTL;
	timeout.tv_usec = 0;
	timeout.tv_sec = PING_TIMEOUT;
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		ft_asprintf(&err, "socket: %s", strerror(errno));
	if (!err && (setsockopt(sockfd, SOL_IP, IP_TTL, (void *)&ttl,
		sizeof(int)) < 0 || setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
		(void *)&timeout, sizeof(struct timeval)) < 0))
		ft_asprintf(&err, "setsockopt: %s", strerror(errno));
	if (err)
		ft_exit(err, EXIT_FAILURE);
	ft_printf("SOCKET created successfully\n");
	return (sockfd);
}

static unsigned short	checksum(unsigned short *data, size_t sz)
{
	uint32_t		sum;
	unsigned short	res;

	for (sum = 0; sz >= sizeof(unsigned short); sz -= sizeof(unsigned short))
		sum += *data++;
	if (sz)
		sum += *((unsigned char *)data);
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	res = (unsigned short)~sum;
	return (res);
}

static void	ping(int sockfd)
{
	char				*err;

	err = NULL;
	g_cfg->request.hdr.checksum = 0;
	//++g_cfg->request.hdr.un.echo.sequence;
	g_cfg->request.hdr.un.echo.sequence = 321; //TEST
	g_cfg->request.hdr.checksum =
		checksum((void *)&g_cfg->request, sizeof(t_ping_packet));
	if (sendto(sockfd, (void *)&g_cfg->request, sizeof(t_ping_packet), 0,
		g_cfg->destinfo->ai_addr, sizeof(struct sockaddr)) < 0)
		ft_asprintf(&err, "sendto: %s", strerror(errno));
	else
		ft_printf("ICMP ECHO packet sent successfully\n");
	if (!err && (g_cfg->rd = recvmsg(sockfd, &g_cfg->response, 0)) < 0)
		ft_asprintf(&err, "recvmsg: %s", strerror(errno));
	else if (!err)
		ft_printf("ICMP ECHO response received successfully (%d bytes)\n",
			(int)g_cfg->rd);
	switch (g_cfg->response.msg_flags)
	{
		case MSG_CTRUNC:	ft_printf("msg_flags = MSG_CTRUNC\n");	break;
		case MSG_EOR:		ft_printf("msg_flags = MSG_EOR\n");		break;
		case MSG_OOB:		ft_printf("msg_flags = MSG_OOB\n");		break;
		case MSG_TRUNC:		ft_printf("msg_flags = MSG_TRUNC\n");	break;
		default:													break;
	}
	if (!err && !inet_ntop(AF_INET, (void *)&g_cfg->resp_ip_hdr->ip_src.s_addr,
		(void *)g_cfg->resp_ip, INET_ADDRSTRLEN))
		ft_asprintf(&err, "inet_ntop: %s", strerror(errno));
	if (err)
		ft_exit(err, EXIT_FAILURE);
	ft_printf("response ip: %s\n", g_cfg->resp_ip);
}

static void	build_config(int argc, char **argv)
{
	g_cfg->exec_name = ft_exec_name(*argv);
	ft_exitmsg((char *)g_cfg->exec_name);
	if (!(g_cfg->dest = get_options(argc, argv)))
		ft_exit("usage error: Destination address required", EXIT_FAILURE);
	get_destinfo();
	g_cfg->request.hdr.type = ICMP_ECHO;
	g_cfg->request.hdr.un.echo.id = getpid();
	g_cfg->iov.iov_base = (void *)g_cfg->iov_buffer;
	g_cfg->iov.iov_len = MSG_BUFLEN;
	g_cfg->response = (struct msghdr){
		NULL, 0, &g_cfg->iov, 1, &g_cfg->msg_buffer, MSG_BUFLEN, 0
	};
	g_cfg->resp_ip_hdr = (struct ip *)g_cfg->iov_buffer;
	g_cfg->resp_icmp_hdr =
		(struct icmphdr *)(g_cfg->iov_buffer + sizeof(struct ip));
}

t_pingcfg	*g_cfg = NULL;

int	main(int argc, char **argv)
{
	int			sockfd;
	t_pingcfg	cfg = { 0 };

	g_cfg = &cfg;
	ft_atexit(ping_cleanup);
	build_config(argc, argv);
	if (getuid())
		ft_exit("user is not root", EXIT_FAILURE);
	sockfd = setup_socket();
	ft_printf("PING %s (%s) %zu(%zu) bytes of data.\n", g_cfg->dest,
		g_cfg->dest_ip, sizeof(g_cfg->request.data),
		sizeof(g_cfg->request) + sizeof(struct ip));
	ping(sockfd);
	ft_exit(NULL, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
