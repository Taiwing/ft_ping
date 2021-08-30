/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:30:15 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/30 18:17:35 by yforeau          ###   ########.fr       */
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

static struct addrinfo	*get_destinfo(void)
{
	int				ret;
	char			*err;
	struct addrinfo	*res;
	struct addrinfo	hints;

	ft_bzero((void *)&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_ICMP;
	if (!(ret = getaddrinfo(g_cfg->destination, NULL, NULL, &res)))
		ft_printf("getaddrinfo: HUGE SUUCCESSSS\n");
	ft_printf("getaddrinfo return: %d\n", ret);
	if (ret)
	{
		ft_asprintf(&err, "%s: %s", g_cfg->destination, gai_strerror(ret));
		ft_exit(err, EXIT_FAILURE);
	}
	return (res);
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
	if (!(g_cfg->destination = get_options(argc, argv)))
		ft_exit("usage error: Destination address required", EXIT_FAILURE);
	//TODO: check ICMP socket permission with getpid and getuid
	g_cfg->destinfo = get_destinfo();
	ft_printf("This is %s!\n", g_cfg->exec_name);
	ft_printf("destination is: %s\n", g_cfg->destination);
	ft_exit(NULL, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
