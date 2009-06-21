inittest extract-liba tc/extract-liba
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar x liba.a" work true
rundiff true
