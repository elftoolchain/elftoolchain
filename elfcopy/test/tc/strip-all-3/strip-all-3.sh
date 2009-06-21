inittest strip-all-3 tc/strip-all-3
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../strip tcsh" work true
rundiff true
