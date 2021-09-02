/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/02 20:01:18 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/02 20:52:02 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static int	ft_secatoi(int *dest, int min, int max, const char *nptr)
{
	long long int	nb;
	long long int	sign;

	nb = 0;
	sign = 1;
	if (*nptr == '-' || *nptr == '+')
		sign = *nptr++ == '-' ? -1 : sign;
	while (*nptr > 47 && *nptr < 58 && nb <= max)
		nb = (nb * 10) + ((*nptr++) - 48);
	if (*nptr && (*nptr <= 47 || *nptr >= 58))
		return (-3);
	else if (nb > max)
		return (-1);
	else if (nb * sign < min)
		return (-2);
	*dest = nb * sign;
	return (0);
}

void		intopt(int *dest, t_optdata *optd, int min, int max)
{
	int	ret;

	if ((ret = ft_secatoi(dest, min, max, optd->optarg)))
	{
		if (ret == -3)
			ft_asprintf(&g_cfg->err, "invalid argument: '%s'", optd->optarg);
		else
			ft_asprintf(&g_cfg->err, "invalid argument: '%s': "
				"out of range: %d <= value <= %d", optd->optarg, min, max);
		ft_exit(g_cfg->err, EXIT_FAILURE);
	}
}


char		*get_options(int argc, char **argv)
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
			case 'W': intopt(&g_cfg->timeout, &optd, 1, INT_MAX);	break;
			case 'c': intopt(&g_cfg->count, &optd, 1, INT_MAX);		break;
			case 't': intopt(&g_cfg->ttl, &optd, 1, 255);			break;
			case 'v': ++g_cfg->verbose;								break;
			default:
				ft_printf(FT_PING_HELP, g_cfg->exec_name);
				ft_exit(NULL, opt != 'h');
		}
	ft_memdel((void **)&args);
	return (argv[optd.optind]);
}
