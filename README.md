# ft_ping

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
sockets for crafting custom ip packets and read responses.

## Usage

ft\_ping sends an ICMP ECHO\_REQUEST packet and waits for an ECHO\_REPLY. It
loops and sends one probe per second by default.

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
