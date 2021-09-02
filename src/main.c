#include "ft_ping.h"

static int	setup_socket(void)
{
	int				ttl;
	int				sockfd;
	struct timeval	timeout;

	ttl = g_cfg->ttl_arg ? ft_atoi(g_cfg->ttl_arg) : PING_TTL;
	timeout.tv_usec = 0;
	timeout.tv_sec = PING_TIMEOUT;
	timeout.tv_sec = g_cfg->timeout_arg ?
		atoi(g_cfg->timeout_arg) : PING_TIMEOUT;
	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		ft_asprintf(&g_cfg->err, "socket: %s", strerror(errno));
	if (!g_cfg->err && (setsockopt(sockfd, SOL_IP, IP_TTL, (void *)&ttl,
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
	while ((opt = ft_getopt(argc, args, &optd)) >= 0)
		switch (opt)
		{
			case 'W':	g_cfg->timeout_arg = optd.optarg;	break;
			case 't':	g_cfg->ttl_arg = optd.optarg;		break;
			case 'v':	++g_cfg->verbose;					break;
			default:
				ft_printf(FT_PING_HELP, g_cfg->exec_name);
				ft_exit(NULL, opt != 'h');
		}
	ft_memdel((void **)&args);
	return (argv[optd.optind]);
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
