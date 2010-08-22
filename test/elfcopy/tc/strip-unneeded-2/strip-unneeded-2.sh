inittest strip-unneeded-2 tc/strip-unneeded-2
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../strip --strip-unneeded -o elfcopy.1 elfcopy" work true
rundiff true
