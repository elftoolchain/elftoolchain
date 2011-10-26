# $Id$
inittest addself-liba tc/addself-liba
extshar ${TESTDIR}
extshar ${RLTDIR}
runcmd "${AR} cru liba.a liba.a" work true
rundiff true
