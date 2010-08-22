inittest elfcopy-noops-4 tc/elfcopy-noops-4
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../elfcopy vi vi.new" work true
rundiff true
