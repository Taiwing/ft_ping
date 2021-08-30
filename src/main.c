/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:30:15 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/30 19:05:28 by yforeau          ###   ########.fr       */
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

	ft_bzero((void *)&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_ICMP;
	if ((ret = getaddrinfo(g_cfg->dest, NULL, NULL, &g_cfg->destinfo)))
	{
		ft_asprintf(&err, "%s: %s", g_cfg->dest, gai_strerror(ret));
		ft_exit(err, EXIT_FAILURE);
	}
	g_cfg->addr_in = (struct sockaddr_in *)g_cfg->destinfo->ai_addr;
	if (!inet_ntop(AF_INET, (void *)&g_cfg->addr_in->sin_addr,
		g_cfg->ip, INET_ADDRSTRLEN))
		ft_exit("inet_ntop error", EXIT_FAILURE);
}

static void	ping_cleanup(void)
{
	if (g_cfg->destinfo)
		freeaddrinfo(g_cfg->destinfo);
}

t_pingcfg	*g_cfg = NULL;

int	main(int argc, char **argv)
{
	t_pingcfg	cfg = { 0 };

	g_cfg = &cfg;
	ft_atexit(ping_cleanup);
	g_cfg->exec_name = ft_exec_name(*argv);
	ft_exitmsg((char *)g_cfg->exec_name);
	if (!(g_cfg->dest = get_options(argc, argv)))
		ft_exit("usage error: Destination address required", EXIT_FAILURE);
	//TODO: check ICMP socket permission with getpid and getuid
	get_destinfo();
	ft_printf("PING %s (%s) 56(84) bytes of data.\n",
		g_cfg->dest, g_cfg->ip);
	ft_exit(NULL, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
