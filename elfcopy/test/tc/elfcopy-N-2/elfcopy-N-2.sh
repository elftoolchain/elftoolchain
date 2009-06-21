inittest elfcopy-N-2 tc/elfcopy-N-2
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy -N bar2 dup.o dup.o.1" work true
rundiff true
