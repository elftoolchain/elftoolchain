inittest compbase_read-liba tc/compbase_read-liba
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar t liba.a ./a1.o" work true
rundiff true
