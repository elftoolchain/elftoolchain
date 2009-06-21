inittest elfcopy-noops-3 tc/elfcopy-noops-3
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy ps ps.new" work true
rundiff true
