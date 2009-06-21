TET_ROOT?=	/usr/tet
LIBDIR=		$(TET_ROOT)/lib/tet3
INCDIR=		$(TET_ROOT)/inc/tet3
CSTD=		-std=iso9899:1999
CFLAGS=		-I$(INCDIR)
TC=		nm_object1-tc
NM=		../../../nm
TESTFILE=	test_obj

$(TESTFILE):
	uudecode $(TESTFILE).uu

nm:
	cd ../../../ && make

$(TC): ../common/test_nm.c nm $(TESTFILE) $(INCDIR)/tet_api.h
	$(CC) -DNM=\"$(.CURDIR)/$(NM)\" -DTC_DIR=\"$(.CURDIR)\" \
		-DTESTFILE=\"$(TESTFILE)\" $(CFLAGS) \
		-o $(TC) ../common/test_nm.c \
		$(LIBDIR)/tcm.o $(LIBDIR)/libapi.a

clean:
	rm -f *.o $(TESTFILE) $(TC)

lint:
	lint $(CFLAGS) $(TC).c -ltcm
