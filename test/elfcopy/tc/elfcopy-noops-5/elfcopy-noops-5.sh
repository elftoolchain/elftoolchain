inittest elfcopy-noops-5 tc/elfcopy-noops-5
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy tcsh tcsh.new" work true
rundiff true
