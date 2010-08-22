inittest strip-all-5 tc/strip-all-5
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../strip pkill" work true
rundiff true
