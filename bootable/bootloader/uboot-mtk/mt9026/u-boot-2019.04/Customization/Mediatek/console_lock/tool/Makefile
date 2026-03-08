CC		= gcc
LIB_CRYPTO	= -lcrypto
CFLAGS		+= -Wall -std=c99
TARG		= auth-server
#TARG		= usb-auth-server
MINI_GMP_SRC	= ../src/mini-gmp.c

CONVERT_KEY	= keys/rsapem2key.py

INC = -I../include

all:
	$(CC) $(CFLAGS) -o $(TARG) $(INC) auth.c $(MINI_GMP_SRC) $(LIB_CRYPTO)
#	$(CC) $(CFLAGS) -o $(TARG) $(INC) usb-auth.c $(MINI_GMP_SRC) $(LIB_CRYPTO)

convert_key:
	@echo "Please make sure you have the python3 env"
	-python3 $(CONVERT_KEY) $(PEM_PATH)

import_key: convert_key
	@echo "Install private keys..."
	mv private_key ./keys/DEFAULT
	@echo "Install public keys..."
	mv public_key ./keys/DEFAULT

clean:
	rm -rf $(TARG)

