#!/usr/bin/python3

import sys
import codecs
from Crypto.PublicKey import RSA
from hexdump import hexdump

# Crypto.PublicKey.RSA.generate(bits, randfunc=None, e=65537)

def n_to_hex(n):
    text = hex(n)[2:].rstrip("L")
    if len(text) % 2 == 0:
        return codecs.decode(text, "hex")
    else:
        return codecs.decode("0"+text, "hex")

def read_big_int_le(text):
    return int(text[::-1].encode("hex"), 16)

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print ("Usage: python rsakeypem <private.pem>")
        quit()

    print ("Open", sys.argv[1])

    f = open(sys.argv[1],'r')
    keyPair = RSA.import_key(f.read())

    print ("Dump private key")
    keyPair_d = n_to_hex(keyPair.d)
    hexdump(keyPair_d)
    with open("./private_key", "wb") as f:
        f.write(keyPair_d)

    print ("Dump public key")
    keyPair_n = n_to_hex(keyPair.n)
    hexdump(keyPair_n)
    with open("./public_key", "wb") as f:
        f.write(keyPair_n)

    print ("Exponent: %s" % hex(keyPair.e))
    print ("First factor of the RSA modulus: %s" % hex(keyPair.p))
    print ("Second factor of the RSA modulus: %s" % hex(keyPair.q))
    print ("Chinese remainder component: %s" % hex(keyPair.u))

