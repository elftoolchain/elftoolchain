inittest extract-liba-v tc/extract-liba-v
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar vx liba.a" work true
rundiff true
