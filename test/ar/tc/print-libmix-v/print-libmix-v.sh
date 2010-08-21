inittest print-libmix-v tc/print-libmix-v
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar pv libmix.a" work true
rundiff true
