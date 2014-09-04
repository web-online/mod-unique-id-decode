#!/usr/bin/python

#
# python implementation of mod_unique_id_uudecoder
#

from __future__ import print_function
from collections import namedtuple
import datetime
import getopt
import socket
import struct
import sys

unique_id_rec = namedtuple('unique_id_rec', 'stamp in_addr pid counter thread_index')
uuencoder = [
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '@', '-',
]


def unique_id_uudecode(s_unique_id):
    struct_format = '!IIIHI'
    unique_id_rec_total_size = struct.calcsize(struct_format)
    unique_id_rec_size_uu = (unique_id_rec_total_size*8+5)/6
    index = [0,0,0,0]
    k = 0
    y = bytearray().zfill(unique_id_rec_total_size)
    for i in xrange(0, unique_id_rec_total_size, 3):
        index[0] = uuencoder.index(s_unique_id[k]); k += 1
        index[1] = uuencoder.index(s_unique_id[k]); k += 1
        # first 6 bits + next 2 bits
        y[i] = ((index[0] << 2) & 0xfc) | ((index[1] >> 4) & 0x03)

        if (k == unique_id_rec_size_uu): break
        index[2] = uuencoder.index(s_unique_id[k]); k += 1
        # remaining 4 bits + next 4 bits
        y[i+1] = ((index[1] << 4) & 0xf0) | ((index[2] >> 2) & 0x0f);

        if (k == unique_id_rec_size_uu): break
        index[3] = uuencoder.index(s_unique_id[k]); k += 1
        # remaining 2 bits + next 6 bits (& 0x3f probably not necessary)
        y[i+2] = ((index[2] << 6) & 0xc0) | (index[3] & 0x3f);

    s = struct.unpack(struct_format, y)
    return unique_id_rec(
        datetime.datetime.fromtimestamp(s[0]),
        socket.inet_ntoa(buffer(y, 4, 4)),
        s[2],
        s[3],
        s[4]
    )


def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ti:", ["test","id="])
    except getopt.GetoptError as err:
        print(str(err), file=sys.stderr)
        sys.exit(2)
    for opt, arg in opts:
        if opt in ["-t", "--test"]:
            None
        elif opt in ["-i", "--id"]:
            unique_id = unique_id_uudecode(arg)
            print("unique_id.stamp = %s" % unique_id.stamp.ctime())
            print("unique_id.in_addr = %s" % unique_id.in_addr)
            print("unique_id.pid = %d" % unique_id.pid)
            print("unique_id.counter = %d" % unique_id.counter)
            print("unique_id.thread_index = %d" % unique_id.thread_index)
        else:
            assert False, "unhandled option"


if __name__ == '__main__':
    main()
