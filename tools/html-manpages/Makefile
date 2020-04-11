# Publish the Elftoolchain project's manual pages as static files on
# Sourceforge.
#
# $Id$

TOP=		../..

CSS=		mandoc.css
MANDOC_OPTIONS=	-Thtml -O style=mandoc.css,man='%N.%S.html' -mdoc \
		-I os=elftoolchain

# The list of manual pages to convert to HTML.
#
# This stanza selects all file names ending in ".<digit>", which may be
# too permissive.
MAN_SRCS!=	cd ${TOP} && find . -name tet -prune -o \
			-type f -name '*.[0-9]' -print

# The list of directories that contain manual pages.
MAN_DIRS=	${MAN_SRCS:H:u}

# Where the generated HTML resides.
STAGING_DIRECTORY=	${.OBJDIR}/man

# The location of the manual page collection on SF's website.
SF_WEBDIR=	${USER}@web.sourceforge.net:/home/project-web/elftoolchain/htdocs/man

.MAIN:	all

all:	${STAGING_DIRECTORY}/${CSS} build-html build-mlinks

# Copy the CSS file to the staging directory.
${STAGING_DIRECTORY}/${CSS}: ${CSS}
	mkdir -p ${STAGING_DIRECTORY}
	cp ${.CURDIR}/${CSS} ${STAGING_DIRECTORY}/${CSS}

# Translate each manual page to HTML form.
build-html:	.PHONY
.for M in ${MAN_SRCS:S,^./,,g}
	mandoc ${MANDOC_OPTIONS} ${TOP}/${M} > ${STAGING_DIRECTORY}/${M:T}.html
.endfor

# Add symbolic links from manual pages to their aliases.
#
# 1. Build a list of directories with manual pages.
# 2. Invoke ${MAKE} -V '${MLINKS}' to extract (origin, destination)
#    pairs of manual pages.
# 3. Invoke 'ln -s' for each such pair of pages.
build-mlinks:	build-html .PHONY
.for D in ${MAN_DIRS}
	cd ${TOP}/${D} && ${MAKE} -V '$${MLINKS}' | egrep -v '^$$' |\
	(read line; echo $${line}; set -- $${line}; \
	 while [ $${#} -gt 0 ]; do \
		(cd ${STAGING_DIRECTORY}; ln -s $${1}.html $${2}.html); \
		shift; shift; \
	 done)
.endfor

# Publish the site to Sourceforge.Net.
publish:	build-mlinks	.PHONY
	rsync ${STAGING_DIRECTORY}/ ${SF_WEBDIR}/

clean:	.PHONY
	rm -rf ${STAGING_DIRECTORY}
