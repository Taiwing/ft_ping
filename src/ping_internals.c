/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping_internals.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/02 18:59:02 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/02 19:02:25 by yforeau          ###   ########.fr       */
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
	unsigned short	sum = ip->ip_sum;
	unsigned short	len = ip->ip_len;
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
	if (len < sizeof (*icmp) || sum != checksum((unsigned short *)icmp, len))
		ret |= PING_ICMP_HDR;
	return (icmp->type ? ret | PING_ICMP_TYPE : ret);
}

static int	print_echo_error(int rep_err)
{
	if (rep_err & PING_IP_HDR)
		return (ft_dprintf(2, "%s: invalid IP header\n", g_cfg->exec_name));
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

void	print_echo_reply(int rep_err)
{
	double		time = 0.0;

	time = (double)(g_cfg->received_ts.tv_sec - g_cfg->sent_ts.tv_sec) * 1000
		+ (double)(g_cfg->received_ts.tv_usec - g_cfg->sent_ts.tv_usec) / 1000;
	if (!rep_err)
	{
		g_cfg->min_ms = time < g_cfg->min_ms || g_cfg->received == 1 ?
			time : g_cfg->min_ms;
		g_cfg->max_ms = time > g_cfg->max_ms ? time : g_cfg->max_ms;
		g_cfg->sum_ms += time;
		g_cfg->ssum_ms += time * time;
		ft_printf("%zd bytes from %s", g_cfg->rd - sizeof(struct ip),
			g_cfg->dest);
		if (!g_cfg->dest_is_ip)
			ft_printf(" (%s)", g_cfg->resp_ip);
		ft_printf(": icmp_seq=%hu ttl=%hhu time=%.*f ms\n",
			g_cfg->resp_icmp_hdr->un.echo.sequence, g_cfg->resp_ip_hdr->ip_ttl,
			time, 3 - (time >= 1.0) - (time >= 10.0) - (time >= 100.0));
	}
	else
		print_echo_error(rep_err);
}
