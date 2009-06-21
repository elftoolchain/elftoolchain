TET_ROOT?=	/usr/tet
LIBDIR=		$(TET_ROOT)/lib/tet3
INCDIR=		$(TET_ROOT)/inc/tet3
CSTD=		-std=iso9899:1999
CFLAGS=		-I$(INCDIR)
TC=		nm_option-tc
NM=		../../../nm

nm:
	cd ../../../ && make

$(TC): nm_option.c nm $(INCDIR)/tet_api.h
	$(CC) -DNM=\"$(.CURDIR)/$(NM)\" $(CFLAGS) \
		-o $(TC) nm_option.c \
		$(LIBDIR)/tcm.o $(LIBDIR)/libapi.a

clean:
	rm -f *.o $(TC)

lint:
	lint $(CFLAGS) nm_opton.c -ltcm
