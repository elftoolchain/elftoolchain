inittest print-liblong tc/print-liblong
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar p liblong.a" work true
rundiff true
