#!/usr/bin/env python3

from socket import *
from struct import *

class Ste
def sendpkt(src, dst, eth_type, payload, interface = "tap0"):
    """Send raw Ethernet packet on interface."""

    #assert(len(src) == len(dst) == 6) # 48-bit ethernet addresses
    #assert(len(eth_type) == 2) # 16-bit ethernet type

    s = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)

    # From the docs: "For raw packet
    # sockets the address is a tuple (ifname, proto [,pkttype [,hatype]])"
    s.bind((interface, 0))
    msg = src + dst + eth_type + payload
    sent_bytes = s.send(bytearray(msg, 'utf-8'))
    print("Sent " + str(sent_bytes) + " byte Ethernet packet on " + interface)
    return sent_bytes


if __name__ == "__main__":
    bytes = sendpkt("\xFE\xED\xFA\xCE\xBE\xEF", "\xFE\xED\xFA\xCE\xBE\xEF", "\x7A\x05", "hello", "tap0")
    bytes = sendpkt("111111",
            "111",
            "1",
            "hello", "tap0")
