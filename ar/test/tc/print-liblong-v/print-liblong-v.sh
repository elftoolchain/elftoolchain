inittest print-liblong-v tc/print-liblong-v
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar vp liblong.a" work true
rundiff true
