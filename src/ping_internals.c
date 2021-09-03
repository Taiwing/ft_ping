/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_internals.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/02 18:59:02 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/03 14:16:41 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

unsigned short	checksum(unsigned short *data, size_t sz)
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

unsigned int	reply_error(void)
{
	struct ip		*ip = g_cfg->resp_ip_hdr;
	struct icmphdr	*icmp = g_cfg->resp_icmp_hdr;
	char			*data = g_cfg->resp_data;
	unsigned short	sum = ip->ip_sum, len = ip->ip_len;
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
	if (len < sizeof(*icmp) || sum != checksum((unsigned short *)icmp, len))
		return (PING_ICMP_HDR);
	if (icmp->type != ICMP_ECHOREPLY)
		ret |= PING_ICMP_TYPE;
	len -= sizeof(*icmp);
	if (!ret && (icmp->un.echo.sequence != g_cfg->request.hdr.un.echo.sequence
		|| icmp->un.echo.id != g_cfg->request.hdr.un.echo.id
		|| len != (unsigned short)g_cfg->datasize || ft_memcmp((void *)data,
		(void *)g_cfg->request.data, len)))
		return (PING_FOREIGN_REPLY);
	return (ret);
}

static int	print_echo_error(int rep_err)
{
	if (rep_err & PING_IP_HDR)
		return (ft_dprintf(2, "%s: invalid IP header\n", g_cfg->exec_name));
	else if (rep_err & PING_FOREIGN_REPLY)
		return (ft_dprintf(2, "%s: lost package\n", g_cfg->exec_name));
	ft_dprintf(2, "From %s", g_cfg->resp_ip);
	if (rep_err & PING_ICMP_HDR)
		return (ft_dprintf(2, " Invalid ICMP header\n"));
	if ((rep_err & PING_IP_SOURCE) && !(rep_err & PING_ICMP_TYPE))
		return (ft_dprintf(2, " Invalid reply source\n"));
	ft_dprintf(2, " icmp_seq=%d ", g_cfg->sent);
	if (g_cfg->resp_icmp_hdr->type == ICMP_DEST_UNREACH)
		ft_dprintf(2, "Destination Host Unreachable\n");
	else if (g_cfg->resp_icmp_hdr->type == ICMP_TIME_EXCEEDED)
		ft_dprintf(2, "Time to live exceeded\n");
	else
		ft_dprintf(2, "Unknown Error\n");
	return (1);
}

static double	reply_time(struct timeval *received)
{
	struct timeval	sent;
	double			time = 0.0;

	ft_memcpy((void *)&sent, (void *)g_cfg->resp_data, sizeof(sent));
	time = (double)(received->tv_sec - sent.tv_sec) * 1000
		+ (double)(received->tv_usec - sent.tv_usec) / 1000;
	g_cfg->min_ms = time < g_cfg->min_ms || g_cfg->received == 1 ?
		time : g_cfg->min_ms;
	g_cfg->max_ms = time > g_cfg->max_ms ? time : g_cfg->max_ms;
	g_cfg->sum_ms += time;
	g_cfg->ssum_ms += time * time;
	return (time);
}

void	print_echo_reply(int rep_err, struct timeval *received)
{
	double	time;

	if (!rep_err)
	{
		if (g_cfg->print_time)
			time = reply_time(received);
		ft_printf("%zd bytes from %s", g_cfg->rd - sizeof(struct ip),
			g_cfg->dest);
		if (!g_cfg->dest_is_ip)
			ft_printf(" (%s)", g_cfg->resp_ip);
		ft_printf(": icmp_seq=%hu ttl=%hhu",
			g_cfg->resp_icmp_hdr->un.echo.sequence, g_cfg->resp_ip_hdr->ip_ttl);
		if (g_cfg->print_time)
			ft_printf(" time=%.*f ms", time,
				3 - (time >= 1.0) - (time >= 10.0) - (time >= 100.0));
		ft_putchar('\n');
	}
	else
		print_echo_error(rep_err);
}
