# ft\_ping

Implementation of the ping command in C. This program sends ping probes to a
given host and waits for the responses. This is useful to check if a given
server is up or down and diagnostic eventual connexion problems from the the
probes round trip time values.

<br />
<p align="center">
	<img src="https://github.com/Taiwing/ft_ping/blob/master/resources/ping-screenshot.png?raw=true" alt="ping-screenshot" style="width: 50%;"/>
</p>

## Setup

```shell
# clone it with the libft submodule
git clone --recurse-submodules https://github.com/Taiwing/ft_ping
# build it
cd ft_ping/ && make
# run it
sudo ./ft_ping example.com
```

As shown above this program needs sudo rights. This is because ft\_ping uses raw
sockets for crafting custom ip packets and read responses. If you do not have
root access on your machine but docker is available, then execute the following
commands to run ft\_ping:

```shell
# build docker image and run it
./setup-docker.bash
# run ft_ping inside the container
./ft_ping example.com
```

## Usage

ft\_ping sends an ICMP ECHO\_REQUEST packet and waits for an ECHO\_REPLY. It
loops and sends one probe per second by default. If the count option is not
given to ft\_ping it will need to be manually stopped with a Ctrl+C, otherwise
it will go on forever.

```
Usage:
	ft_ping [options] <destination>
Options:
	<destination>		hostname or IPv4 address
	-c count		stop after sending count ECHO_REQUEST packets
	-h			print help and exit
	-p pattern		up to 16 "pad" bytes to fill out the packet
	-s packetsize		number of data bytes to send
	-t ttl			IP time to live
	-v			verbose output
	-W timeout		time to wait for a response, in seconds
```

> This program only handles valid local or remote hostnames and IPv4.

#### example:

```shell
# send 3 pings to google.com and exit
sudo ./ft_ping -c 3 google.com
```

possible output:

```
PING google.com (172.217.19.238) 56(84) bytes of data.
64 bytes from google.com (172.217.19.238): icmp_seq=1 ttl=116 time=7.14 ms
64 bytes from google.com (172.217.19.238): icmp_seq=2 ttl=116 time=7.81 ms
64 bytes from google.com (172.217.19.238): icmp_seq=3 ttl=116 time=8.23 ms

--- google.com ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2016ms
rtt min/avg/max/mdev = 7.142/7.727/8.231/0.448 ms
```

## How it works

ft\_ping crafts ICMP packets (Internet Control Message Protocol) with the type
field set to ECHO\_REQUEST. The ICMP protocol is on the the Network layer of the
TCP/IP
[OSI model](https://www.imperva.com/learn/application-security/osi-model/),
on the same level as the IP protocol since it handles errors and diagnostic
messages directly related to IP. If the target host is up and not configured to
ignore ECHO\_REQUESTS it should send back an ECHO\_REPLY response.

#### ECHO\_REQUEST example:

IP header:

| version | total length | time to live | protocol | source ip | destination ip |
|---------|--------------|--------------|----------|-----------|----------------|
| 4       | 84           | 255          | ICMP     | 1.2.3.4   | 93.184.216.34  |

> Some fields have been removed for clarity. For a complete overview of the IPv4
> protocol check [this page](https://en.wikipedia.org/wiki/IPv4).

ICMP header:

| type              | code | checksum | identifier | sequence number |
|-------------------|------|----------|------------|-----------------|
| 8 (ECHO\_REQUEST) | 0    | 0xffff   | 4321       | 1               |

> Every field is present but this strucuture is specific to ICMP ECHO packets.
> For a complete overview of ICMP check
> [this page](https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol).

The IP header is 20 bytes long, ICMP is 8 bytes and there is 56 bytes of data
by default which is why the total length IP field amounts to 84 bytes. The ICMP
identifier is chosen by ft\_ping and is simply set to its PID. This is useful to
make sure that the response is intended for this process. The sequence number is
an increasing counter of the ECHO\_REQUEST probes sent by ft\_ping to the given
host. The 56 bytes of data can be anything and is usually filled with zeroes
except if the pattern option is used.

#### ECHO\_REPLY example:

IP header:

| version | total length | time to live | protocol | source ip     | destination ip |
|---------|--------------|--------------|----------|---------------|----------------|
| 4       | 84           | 56           | ICMP     | 93.184.216.34 | 1.2.3.4        |

ICMP header:

| type            | code | checksum | identifier | sequence number |
|-----------------|------|----------|------------|-----------------|
| 0 (ECHO\_REPLY) | 0    | 0xffff   | 4321       | 1               |

The reply is almost identical to the request except that the source and
destination IP addresses are reversed and the ICMP type field is set to
ECHO\_REPLY. The data is also typically identical to the request's data payload.

To compute the RTT (Round Trip Time) - which is also called the ping - ft\_ping
simply substracts the reply departure timestamp from the request arrival. So
this value basically represents how long it takes for a packet to reach the
target host and come back.
