inittest print-libmix tc/print-libmix
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar p libmix.a" work true
rundiff true
