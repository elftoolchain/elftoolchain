inittest extract-liblong tc/extract-liblong
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar x liblong.a" work true
rundiff true
