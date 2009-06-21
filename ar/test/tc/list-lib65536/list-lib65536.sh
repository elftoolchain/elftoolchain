inittest list-lib65536 tc/list-lib65536
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar t lib65536.a" work true
rundiff true
