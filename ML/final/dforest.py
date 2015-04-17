#!/usr/bin/env python
if __name__ == '__main__':
    from sklearn import tree as SK_Tree
    import numpy as np
    from final_utils import read_hwfile

    # initialize data
    dat, lab, nDat = read_hwfile('ml14fall_train_align.dat.hog.dat', 169)
    nVal = nDat/5
    nTrn = nDat-nVal
    datTrn = dat[:nTrn]
    labTrn = lab[:nTrn].astype('i4')
    datVal = dat[-nVal:]
    labVal = lab[-nVal:].astype('i4')
    print "#trn = {}, #val = {}".format(nTrn, nVal)

    niter = 1000
    voteRes = np.zeros((nVal, 32), dtype='i4')
    for i in xrange(niter):
        ridx = np.random.randint(168, size=10)
        datTrn_small = datTrn[:,ridx].copy()
        datVal_small = datVal[:,ridx].copy()

        decTree = SK_Tree.DecisionTreeClassifier()
        decTree.fit(datTrn_small, labTrn)
        labPre = decTree.predict(datVal_small).astype('i4')

        for d in xrange(nVal):
            voteRes[d,labPre[d]] += 1

        if i % 100 == 0:
            voteWinner = np.argmax(voteRes, 1)
            Eval = float(np.count_nonzero(voteWinner != labVal)) / nVal
            print "after iter {}, Eval = {}".format(i, Eval)
