inittest elfcopy-rename-1 tc/elfcopy-rename-1
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy --rename-section .text=.text.newname sym.o" work true
rundiff true
