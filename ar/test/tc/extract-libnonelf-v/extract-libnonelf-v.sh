inittest extract-libnonelf-v tc/extract-libnonelf-v
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar xv libnonelf.a" work true
rundiff true
