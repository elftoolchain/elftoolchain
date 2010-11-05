#
# Libelf by Example
#
# $Id$

TOP = ../..

DOC=			libelf-by-example

SRCS=			libelf-by-example.tex prog1.txt prog2.txt prog3.txt \
			prog4.txt prog5.txt prog6.txt

IMAGES_MP=		fig-arstr.mp fig-ariter.mp fig-data.mp \
			fig-eident.mp fig-elfhdrlayout.mp fig-elflayout.mp \
			fig-filemem.mp \
			fig-phdrlayout.mp \
			fig-scn.mp fig-shdrlayout.mp fig-strtab.mp \
			fig-versions.mp 

.include "${TOP}/mk/elftoolchain.tex.mk"

check-example-syntax: .PHONY
.for f in ${SRCS:Mprog*}
	@c=$$(basename ${f} .txt).c; sed -e 's/@[^@]*@//' \
		${.CURDIR}/${f} > ${.OBJDIR}/$${c}; \
	echo -n $${c} ' '; cc ${.OBJDIR}/$${c} -L${.CURDIR}/${TOP}/libelf \
		-lelf && rm ${.OBJDIR}/$${c}
.endfor
	@echo
