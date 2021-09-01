/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:30:15 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/01 15:22:38 by yforeau          ###   ########.fr       */
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
	struct addrinfo	hints;

	ft_bzero((void *)&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	if ((ret = getaddrinfo(g_cfg->dest, NULL, NULL, &g_cfg->destinfo)))
		ft_asprintf(&g_cfg->err, "%s: %s", g_cfg->dest, gai_strerror(ret));
	if (!g_cfg->err)
		g_cfg->dest_addr_in = (struct sockaddr_in *)g_cfg->destinfo->ai_addr;
	if (!g_cfg->err && !inet_ntop(AF_INET,
		(void *)&g_cfg->dest_addr_in->sin_addr,
		g_cfg->dest_ip, INET_ADDRSTRLEN))
		ft_asprintf(&g_cfg->err, "inet_ntop: %s", strerror(errno));
	if (g_cfg->err)
		ft_exit(g_cfg->err, EXIT_FAILURE);
}

static int	setup_socket(void)
{
	int				ttl;
	int				sockfd;
	struct timeval	timeout;

	ttl = PING_TTL;
	timeout.tv_usec = 0;
	timeout.tv_sec = PING_TIMEOUT;
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		ft_asprintf(&g_cfg->err, "socket: %s", strerror(errno));
	if (!g_cfg->err && (setsockopt(sockfd, SOL_IP, IP_TTL, (void *)&ttl,
		sizeof(int)) < 0 || setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
		(void *)&timeout, sizeof(struct timeval)) < 0))
		ft_asprintf(&g_cfg->err, "setsockopt: %s", strerror(errno));
	if (g_cfg->err)
		ft_exit(g_cfg->err, EXIT_FAILURE);
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

static void	echo_request(int sockfd)
{
	g_cfg->request.hdr.checksum = 0;
	++g_cfg->request.hdr.un.echo.sequence;
	g_cfg->request.hdr.checksum =
		checksum((void *)&g_cfg->request, sizeof(t_ping_packet));
	if (sendto(sockfd, (void *)&g_cfg->request, sizeof(t_ping_packet), 0,
		g_cfg->destinfo->ai_addr, sizeof(struct sockaddr)) < 0)
		ft_asprintf(&g_cfg->err, "sendto: %s", strerror(errno));
	else
	{
		++g_cfg->sent;
		if (gettimeofday(&g_cfg->sent_ts, NULL) < 0)
			ft_asprintf(&g_cfg->err, "gettimeofday: %s", strerror(errno));
	}
}

static unsigned int	reply_error(void)
{
	struct ip		*ip = g_cfg->resp_ip_hdr;
	struct icmphdr	*icmp = g_cfg->resp_icmp_hdr;
	unsigned short	sum = ip->ip_sum;
	unsigned short	len = ip->ip_len;
	unsigned int	ret = 0;

	ip->ip_sum = 0;
	ft_memswap((void *)&len, sizeof(len));
	if (ip->ip_hl != 5 || ip->ip_v != 4 || ip->ip_p != 1 || len < sizeof(*ip)
		|| sum != checksum((unsigned short *)ip, sizeof(*ip)))
		return (PING_IP_HDR);
	if (ip->ip_src.s_addr != g_cfg->dest_addr_in->sin_addr.s_addr)
		ret |= PING_IP_SOURCE;
	sum = icmp->checksum;
	icmp->checksum = 0;
	len -= sizeof(*ip);
	if (len < sizeof (*icmp) || sum != checksum((unsigned short *)icmp, len))
		ret |= PING_ICMP_HDR;
	return (icmp->type ? ret | PING_ICMP_TYPE : ret);
}

static void	print_echo_reply(int rep_err)
{
	double		time = 0.0;

	time = (double)(g_cfg->received_ts.tv_sec - g_cfg->sent_ts.tv_sec) * 1000
		+ (double)(g_cfg->received_ts.tv_usec - g_cfg->sent_ts.tv_usec) / 1000;
	if (!rep_err)
	{
		g_cfg->min_ms = time < g_cfg->min_ms || g_cfg->received == 1 ?
			time : g_cfg->min_ms;
		g_cfg->max_ms = time > g_cfg->max_ms ? time : g_cfg->max_ms;
		g_cfg->sum_ms += time;
		g_cfg->ssum_ms += time * time;
		//if input dest is IP
		ft_printf("%zd bytes from %s: ", g_cfg->rd - sizeof(struct ip),
			g_cfg->resp_ip);
		//TODO: else print hostname or FDQN shit
		ft_printf("icmp_seq=%hu ttl=%hhu time=%.*f ms\n",
			g_cfg->resp_icmp_hdr->un.echo.sequence, g_cfg->resp_ip_hdr->ip_ttl,
			time, 3 - (time >= 1.0) - (time >= 10.0) - (time >= 100.0));
	}
	if (rep_err & PING_IP_HDR)
		ft_printf("PING_IP_HDR\n");
	if (rep_err & PING_IP_SOURCE)
		ft_printf("PING_IP_SOURCE\n");
	if (rep_err & PING_ICMP_HDR)
		ft_printf("PING_ICMP_HDR\n");
	if (rep_err & PING_ICMP_TYPE)
		ft_printf("PING_ICMP_TYPE\n");
}

static void	echo_reply(int sockfd)
{
	int			rep_err = 0;

	ft_bzero((void *)g_cfg->iov_buffer, MSG_BUFLEN);
	if ((g_cfg->rd = recvmsg(sockfd, &g_cfg->response, 0)) < 0)
		ft_asprintf(&g_cfg->err, "recvmsg: %s", strerror(errno));
	else if (g_cfg->resp_icmp_hdr->type == ICMP_ECHO
		&& !ft_memcmp((void *)&g_cfg->request, (void *)g_cfg->resp_icmp_hdr,
		sizeof(g_cfg->request))
		&& (g_cfg->rd = recvmsg(sockfd, &g_cfg->response, 0)) < 0)
		ft_asprintf(&g_cfg->err, "recvmsg: %s", strerror(errno));
	else if (!(rep_err = reply_error()))
	{
		++g_cfg->received;
		if (gettimeofday(&g_cfg->received_ts, NULL) < 0)
			ft_asprintf(&g_cfg->err, "gettimeofday: %s", strerror(errno));
	}
	if (!g_cfg->err && !inet_ntop(AF_INET,
		(void *)&g_cfg->resp_ip_hdr->ip_src.s_addr,
		(void *)g_cfg->resp_ip, INET_ADDRSTRLEN))
		ft_asprintf(&g_cfg->err, "inet_ntop: %s", strerror(errno));
	if (!g_cfg->err)
		print_echo_reply(rep_err);
}

static void	ping(int sig)
{
	(void)sig;
	echo_request(g_cfg->sockfd);
	if (!g_cfg->err)
		echo_reply(g_cfg->sockfd);
	if (!g_cfg->err && gettimeofday(&g_cfg->end_ts, NULL) < 0)
		ft_asprintf(&g_cfg->err, "gettimeofday: %s", strerror(errno));
	else if (!g_cfg->err)
	{
		alarm(1);
		if (!g_cfg->start_ts.tv_sec && !g_cfg->start_ts.tv_usec)
			ft_memcpy((void *)&g_cfg->start_ts,
				(void *)&g_cfg->end_ts, sizeof(struct timeval));
	}
	else
		ft_exit(g_cfg->err, EXIT_FAILURE);
}

static void	build_config(int argc, char **argv)
{
	g_cfg->exec_name = ft_exec_name(*argv);
	ft_exitmsg((char *)g_cfg->exec_name);
	if (g_cfg->err)
		ft_exit(g_cfg->err, EXIT_FAILURE);
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

static void	ping_cleanup(void)
{
	if (g_cfg->destinfo)
		freeaddrinfo(g_cfg->destinfo);
}

static void	ping_int_handler(int sig)
{
	uint64_t		time;
	unsigned int	loss;

	(void)sig;
	loss = 0;
	if (g_cfg->sent)
		loss = 100
		- (unsigned int)(100 * ((double)g_cfg->received/(double)g_cfg->sent));
	time = (g_cfg->end_ts.tv_sec - g_cfg->start_ts.tv_sec) * 1000;
	time += (g_cfg->end_ts.tv_usec - g_cfg->start_ts.tv_usec) / 1000;
	ft_printf("\n--- %s ping statistics ---\n%u packets transmitted, "
		"%u received, %u%% packet loss, time %llums\n", g_cfg->dest,
		g_cfg->sent, g_cfg->received, loss, time);
	if (g_cfg->received)
	{
		g_cfg->avg_ms = g_cfg->sum_ms / (double)g_cfg->received;
		g_cfg->mdev_ms = ft_sqrt(g_cfg->ssum_ms / g_cfg->received
			- g_cfg->avg_ms * g_cfg->avg_ms);
		ft_printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
			g_cfg->min_ms, g_cfg->avg_ms, g_cfg->max_ms, g_cfg->mdev_ms);
	}
	ft_exit(NULL, EXIT_SUCCESS);
}

t_pingcfg	*g_cfg = NULL;

int	main(int argc, char **argv)
{
	t_pingcfg	cfg = { 0 };

	g_cfg = &cfg;
	ft_atexit(ping_cleanup);
	if (signal(SIGINT, ping_int_handler) == SIG_ERR)
		ft_asprintf(&g_cfg->err, "signal: %s", strerror(errno));
	else if (signal(SIGALRM, ping) == SIG_ERR)
		ft_asprintf(&g_cfg->err, "signal: %s", strerror(errno));
	build_config(argc, argv);
	if (getuid())
		ft_exit("user is not root", EXIT_FAILURE);
	g_cfg->sockfd = setup_socket();
	ft_printf("PING %s (%s) %zu(%zu) bytes of data.\n", g_cfg->dest,
		g_cfg->dest_ip, sizeof(g_cfg->request.data),
		sizeof(g_cfg->request) + sizeof(struct ip));
	ping(0);
	while (42);
	return (EXIT_SUCCESS);
}
