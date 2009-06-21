inittest extract-libmix-v tc/extract-libmix-v
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar xv libmix.a" work true
rundiff true
