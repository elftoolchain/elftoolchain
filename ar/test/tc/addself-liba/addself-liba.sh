inittest addself-liba tc/addself-liba
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "../ar cru liba.a liba.a" work true
rundiff true
