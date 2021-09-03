#include "ft_ping.h"

static void	ping_cleanup(void)
{
	if (g_cfg->destinfo)
		freeaddrinfo(g_cfg->destinfo);
}

static int	setup_socket(void)
{
	int				sockfd;
	struct timeval	timeout;

	g_cfg->ttl = g_cfg->ttl ? g_cfg->ttl : PING_TTL;
	g_cfg->timeout = g_cfg->timeout ? g_cfg->timeout : PING_TIMEOUT;
	timeout.tv_usec = 0;
	timeout.tv_sec = g_cfg->timeout;
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		ft_asprintf(&g_cfg->err, "socket: %s", strerror(errno));
	if (!g_cfg->err && (setsockopt(sockfd, SOL_IP, IP_TTL, (void *)&g_cfg->ttl,
		sizeof(int)) < 0 || setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
		(void *)&timeout, sizeof(struct timeval)) < 0))
		ft_asprintf(&g_cfg->err, "setsockopt: %s", strerror(errno));
	if (g_cfg->err)
		ft_exit(g_cfg->err, EXIT_FAILURE);
	return (sockfd);
}

static void	get_destinfo(void)
{
	int				ret;
	struct addrinfo	hints;

	ft_bzero((void *)&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	if ((ret = getaddrinfo(g_cfg->dest, NULL, &hints, &g_cfg->destinfo)))
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

static void	build_config(int argc, char **argv)
{
	char	buf[sizeof(struct in_addr)];

	g_cfg->exec_name = ft_exec_name(*argv);
	ft_exitmsg((char *)g_cfg->exec_name);
	if (g_cfg->err)
		ft_exit(g_cfg->err, EXIT_FAILURE);
	if (!(g_cfg->dest = get_options(argc, argv)))
		ft_exit("usage error: Destination address required", EXIT_FAILURE);
	if ((g_cfg->dest_is_ip = inet_pton(AF_INET, g_cfg->dest, (void *)buf)) < 0)
		ft_asprintf(&g_cfg->err, "inet_pton: %s", strerror(errno));
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
	g_cfg->resp_data = (char *)
		(g_cfg->iov_buffer + sizeof(struct ip) + sizeof(struct icmphdr));
	g_cfg->count = g_cfg->count ? g_cfg->count : -1;
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
