inittest strip-all-1 tc/strip-all-1
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../strip ls" work true
rundiff true
