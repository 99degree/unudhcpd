## unudhcpd

This is an extremely basic DHCP server that only issues 1 IP address to any client that sends a DHCP{OFFER,REQUEST}.

It is meant to be used on an interface that will only ever see 1 client at a
time, but where the client's MAC may change, making it inconvenient to try and
configure leasing, etc on other DHCP servers. With unudhcpd, the client always
gets an IP quickly.


## Usage
```
Usage:
        unudhcpd -i <interface> [-s <server IP>] [-p <server port] [-c <client IP>]
Where:
        -i  network interface to bind to
        -s  server IP {default: 172.168.1.1}
        -p  server port {default: 67}
        -c  client IP to issue for DHCP requests {default: 172.168.1.2}
```

## Installation

Project uses meson, so:

```
$ meson _build
...
$ meson compile -C _build
...
$ sudo meson install -C _build
...
```
