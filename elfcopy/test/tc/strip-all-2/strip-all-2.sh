inittest strip-all-2 tc/strip-all-2
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../strip ps" work true
rundiff true
