# $Id$

YAML_FILES=	check_elf getclass ehdr fsize newehdr newscn phdr shdr u1 \
		xlate xscn-1 xscn-2 xscn-3

.for f in ${YAML_FILES}
TS_DATA+=	${f}.lsb32 ${f}.lsb64 ${f}.msb32 ${f}.msb64
.endfor

.include "../Makefile.tset"
