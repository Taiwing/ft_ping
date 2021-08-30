/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:30:15 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/30 14:46:15 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

void	get_options(t_pingcfg *cfg, int argc, char **argv)
{
	int			opt;
	t_optdata	optd;
	char		**args;

	ft_bzero((void *)&optd, sizeof(t_optdata));
	init_getopt(&optd, FT_PING_OPT, NULL, NULL);
	args = ft_memalloc((argc + 1) * sizeof(char *));
	ft_memcpy((void *)args, (void *)argv, argc * sizeof(char *));
	*args = (char *)cfg->exec_name;
	while ((opt = ft_getopt(argc, args, &optd)) == 'v')
		cfg->verbose = 1;
	if (cfg->verbose)
		ft_printf("%s: VERBOSE MODE ON\n", cfg->exec_name);
	if (opt != -1)
	{
		ft_printf(FT_PING_HELP, cfg->exec_name);
		ft_exit(NULL, opt != 'h');
	}
	ft_memdel((void **)&args);
}

int	main(int argc, char **argv)
{
	t_pingcfg	cfg;

	ft_bzero((void *)&cfg, sizeof(t_pingcfg));
	cfg.exec_name = ft_exec_name(*argv);
	ft_exitmsg((char *)cfg.exec_name);
	get_options(&cfg, argc, argv);
	ft_printf("This is %s!\n", cfg.exec_name);
	ft_exit(NULL, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
