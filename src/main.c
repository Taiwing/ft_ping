/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/23 04:30:15 by yforeau           #+#    #+#             */
/*   Updated: 2021/08/30 12:54:59 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

int	main(int argc, char **argv)
{
	t_pingcfg	cfg;

	(void)argc;
	ft_bzero((void *)&cfg, sizeof(t_pingcfg));
	cfg.exec_name = ft_exec_name(*argv);
	ft_exitmsg((char *)cfg.exec_name);
	ft_printf("This is %s!\n", cfg.exec_name);
	ft_exit(NULL, EXIT_SUCCESS);
	return (EXIT_SUCCESS);
}
