inittest elfcopy-noops-7 tc/elfcopy-noops-7
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy sections.o.debug sections.o.debug.1" work true
rundiff true
