inittest list-liblong tc/list-liblong
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar t liblong.a" work true
rundiff true
