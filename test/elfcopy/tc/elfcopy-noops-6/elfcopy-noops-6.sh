inittest elfcopy-noops-6 tc/elfcopy-noops-6
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy mcs.o mcs.o.1" work true
rundiff true
