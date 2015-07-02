ccflags-$(CONFIG_MBED_TLS) += -Iinclude/linux
MBED_TLS_OBJS = \
		bignum.o \
		rsa.o \
		pk.o \
		md.o \
		pkwrite.o \
		asn1parse.o \
		asn1write.o \
		base64.o \
		sha256.o \
		pem.o \
		oid.o \
		x509_create.o \
		x509write_csr.o \
		x509_crt.o \
		pk_wrap.o \
		md_wrap.o \
	    pkparse.o \
		des.o \
		sha1.o \
		md5.o 


obj-$(CONFIG_MBED_TLS) += $(MBED_TLS_OBJS)
