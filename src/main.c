/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:30:15 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/30 14:22:48 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

int	main(int argc, char **argv)
{
	int			opt;
	t_pingcfg	cfg;
	t_optdata	optd;

	(void)argc;
	ft_bzero((void *)&cfg, sizeof(t_pingcfg));
	ft_bzero((void *)&optd, sizeof(t_optdata));
	cfg.exec_name = ft_exec_name(*argv);
	ft_exitmsg((char *)cfg.exec_name);
	init_getopt(&optd, FT_PING_OPT, NULL, NULL);
	while ((opt = ft_getopt(argc, argv, &optd)) == 'v')
		cfg.verbose = 1;
	if (cfg.verbose)
		ft_printf("%s: VERBOSE MODE ON\n", cfg.exec_name);
	if (opt != -1)
	{
		ft_printf(FT_PING_HELP, cfg.exec_name);
		ft_exit(NULL, opt != 'h');
	}
	ft_printf("This is %s!\n", cfg.exec_name);
	ft_exit(NULL, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
