inittest arscript-4 tc/arscript-4
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar -M < liba.script.bsd" work true
rundiff true
