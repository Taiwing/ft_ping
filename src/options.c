/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   options.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/02 20:01:18 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/03 14:02:19 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static int	ft_secatoi(int *dest, int min, int max, const char *nptr)
{
	long long int	nb;
	long long int	sign;

	if (!*nptr)
		return (-3);
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

void		pattern_option(const char *arg, int print)
{
	char		buf[PATTERN_BUF + 1] = { 0 }, p[PATTERN_MAX] = { 0 };
	const char	*hex = "0123456789abcdefABCDEF";
	size_t		sz, i, ds = DATASIZE;

	for (i = 0; arg[i] && !g_cfg->err; ++i)
		if (!ft_strchr(hex, arg[i]))
			ft_asprintf(&g_cfg->err,
				"patterns must be specified as hex digits: %s", arg + i);
	if (g_cfg->err)
		ft_exit(g_cfg->err, EXIT_FAILURE);
	ft_strncpy(buf, arg, PATTERN_BUF);
	for (i = 0; buf[i]; ++i)
	{
		buf[i] = ft_tolower(buf[i]);
		if (i & 1)
			p[i / 2] <<= 4;
		p[i / 2] |= ft_strchr(hex, buf[i]) - hex;
	}
	if (print)
		ft_printf("PATTERN: 0x%s\n", buf);
	sz = i / 2 + i % 2;
	for (i = sizeof(struct timeval); sz && i + sz <= ds; i += sz)
		ft_memcpy((void *)g_cfg->request.data + i, (void *)p, sz);
	if (sz && i < ds)
		ft_memcpy((void *)g_cfg->request.data + i, (void *)p, ds - i);
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
			case 'c': intopt(&g_cfg->count, &optd, 1, INT_MAX);		break;
			case 'p': pattern_option(optd.optarg, 0);				break;
			case 's': intopt(&g_cfg->datasize, &optd, 0, DATASIZE);	break;
			case 't': intopt(&g_cfg->ttl, &optd, 1, 255);			break;
			case 'v': ++g_cfg->verbose;								break;
			case 'W': intopt(&g_cfg->timeout, &optd, 1, INT_MAX);	break;
			default:
				ft_printf(FT_PING_HELP, g_cfg->exec_name);
				ft_exit(NULL, opt != 'h');
		}
	ft_memdel((void **)&args);
	return (argv[optd.optind]);
}
