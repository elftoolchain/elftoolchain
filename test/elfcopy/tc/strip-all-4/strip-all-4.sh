inittest strip-all-4 tc/strip-all-4
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../strip vi" work true
rundiff true
