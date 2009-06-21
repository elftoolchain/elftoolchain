inittest elfcopy-L-1 tc/elfcopy-L-1
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy -L bar -L foo2 sym.o sym.o.1" work true
rundiff true
