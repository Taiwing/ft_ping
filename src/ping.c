/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ping.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yforeau <yforeau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/09/02 18:53:56 by yforeau           #+#    #+#             */
/*   Updated: 2021/09/02 18:56:41 by yforeau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ping.h"

static void	echo_request(int sockfd)
{
	g_cfg->request.hdr.checksum = 0;
	++g_cfg->request.hdr.un.echo.sequence;
	g_cfg->request.hdr.checksum =
		checksum((void *)&g_cfg->request, sizeof(t_ping_packet));
	if (sendto(sockfd, (void *)&g_cfg->request, sizeof(t_ping_packet), 0,
		g_cfg->destinfo->ai_addr, sizeof(struct sockaddr)) < 0)
		ft_asprintf(&g_cfg->err, "sendto: %s", strerror(errno));
	else
	{
		++g_cfg->sent;
		if (gettimeofday(&g_cfg->sent_ts, NULL) < 0)
			ft_asprintf(&g_cfg->err, "gettimeofday: %s", strerror(errno));
	}
}

static void	echo_reply(int sockfd)
{
	int			rep_err = 0;

	ft_bzero((void *)g_cfg->iov_buffer, MSG_BUFLEN);
	if ((g_cfg->rd = recvmsg(sockfd, &g_cfg->response, 0)) < 0)
		ft_asprintf(&g_cfg->err, "recvmsg: %s", strerror(errno));
	else if (g_cfg->resp_icmp_hdr->type == ICMP_ECHO
		&& !ft_memcmp((void *)&g_cfg->request, (void *)g_cfg->resp_icmp_hdr,
		sizeof(g_cfg->request))
		&& (g_cfg->rd = recvmsg(sockfd, &g_cfg->response, 0)) < 0)
		ft_asprintf(&g_cfg->err, "recvmsg: %s", strerror(errno));
	else if (!(rep_err = reply_error()))
	{
		++g_cfg->received;
		if (gettimeofday(&g_cfg->received_ts, NULL) < 0)
			ft_asprintf(&g_cfg->err, "gettimeofday: %s", strerror(errno));
	}
	else
		++g_cfg->errors;
	if (!g_cfg->err && !inet_ntop(AF_INET,
		(void *)&g_cfg->resp_ip_hdr->ip_src.s_addr,
		(void *)g_cfg->resp_ip, INET_ADDRSTRLEN))
		ft_asprintf(&g_cfg->err, "inet_ntop: %s", strerror(errno));
	if (!g_cfg->err)
		print_echo_reply(rep_err);
}

void	ping_cleanup(void)
{
	if (g_cfg->destinfo)
		freeaddrinfo(g_cfg->destinfo);
}

void	ping_int_handler(int sig)
{
	uint64_t		time;
	unsigned int	loss;

	(void)sig;
	loss = 0;
	if (g_cfg->sent)
		loss = 100
		- (unsigned int)(100 * ((double)g_cfg->received/(double)g_cfg->sent));
	time = (g_cfg->end_ts.tv_sec - g_cfg->start_ts.tv_sec) * 1000;
	time += (g_cfg->end_ts.tv_usec - g_cfg->start_ts.tv_usec) / 1000;
	ft_printf("\n--- %s ping statistics ---\n%u packets transmitted, "
		"%u received, ", g_cfg->dest, g_cfg->sent, g_cfg->received);
	if (g_cfg->errors)
		ft_printf("+%u errors, ", g_cfg->errors);
	ft_printf("%u%% packet loss, time %llums\n", loss, time);
	if (g_cfg->received)
	{
		g_cfg->avg_ms = g_cfg->sum_ms / (double)g_cfg->received;
		g_cfg->mdev_ms = ft_sqrt(g_cfg->ssum_ms / g_cfg->received
			- g_cfg->avg_ms * g_cfg->avg_ms);
		ft_printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
			g_cfg->min_ms, g_cfg->avg_ms, g_cfg->max_ms, g_cfg->mdev_ms);
	}
	ft_exit(NULL, EXIT_SUCCESS);
}

void	ping(int sig)
{
	(void)sig;
	echo_request(g_cfg->sockfd);
	if (!g_cfg->err)
		echo_reply(g_cfg->sockfd);
	if (!g_cfg->err && gettimeofday(&g_cfg->end_ts, NULL) < 0)
		ft_asprintf(&g_cfg->err, "gettimeofday: %s", strerror(errno));
	else if (!g_cfg->err)
	{
		alarm(1);
		if (!g_cfg->start_ts.tv_sec && !g_cfg->start_ts.tv_usec)
			ft_memcpy((void *)&g_cfg->start_ts,
				(void *)&g_cfg->end_ts, sizeof(struct timeval));
	}
	else
		ft_exit(g_cfg->err, EXIT_FAILURE);
}
