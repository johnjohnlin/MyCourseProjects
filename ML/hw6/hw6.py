#!/usr/bin/env python
import numpy as np
from sklearn import tree as SK_Tree

def read_hwfile(fname):
    datLab = np.fromfile(fname, sep='   ')
    datLab = datLab.reshape((-1, 3))
    return datLab[:,:-1], datLab[:,-1], datLab.shape[0]

def dstump_predict(dat, feat_idx, thres, invSign):
    lab = np.sign(dat[:,feat_idx]-thres)
    return -lab if invSign else lab

def adaboost_predict(dat, stumpArray, alphas):
    Gx = np.zeros(dat.shape[0])
    for i in xrange(stumpArray.shape[0]):
        Gx += dstump_predict(dat, *stumpArray[i]) * alphas[i]
    return np.sign(Gx)

_eqs = '='*60
def banner(info):
    print eqs
    print info.center(31).center(60, '|')
    print eqs

def print_banner(f):
    def wrapper(*args, **kwargs):
        banner(f.__name__)
        return f(*args, **kwargs)
    return wrapper

@print_banner
def prob12_to_15():
    trnDat, trnLab, nTrn = read_hwfile('hw6_train.bin')
    tstDat, tstLab, nTst = read_hwfile('hw6_test.bin')

    nTrnInv = 1.0 / nTrn
    nDim = trnDat.shape[1]
    assert nDim == tstDat.shape[1]

    # adaboost
    niter = 300

    # allocate memory
    g = np.empty(niter, dtype=[('feat_idx', 'i4'), ('thres', 'f4'), ('invSign', 'i4')])
    alphas = np.zeros(niter, dtype='f4')
    weight = np.empty(nTrn, dtype='f4')
    weight.fill(1.0/nTrn)

    # initialize sorted datum (transposed)
    sortIdx = np.empty((nDim, nTrn), dtype='i4')
    labSortByAxis = np.empty((nDim, nTrn), dtype='f4')
    for d in xrange(nDim):
        sortIdx[d] = trnDat[:,d].argsort()
        labSortByAxis[d] = trnLab[sortIdx[d]]
    trnDatSorted = trnDat.T.copy()
    trnDatSorted.sort()

    # boosting
    for i in xrange(niter):
        bestStump = None
        errMin = np.infty

        for j in xrange(nDim):
            # pos/neg: label -1 or 1, upper/lower: greater or less than cut
            sumPosLower = 0
            sumNegLower = 0
            weightSorted = weight[sortIdx[j]].copy()
            sumPosUpper = weightSorted[labSortByAxis[j] > 0].sum()
            sumNegUpper = weightSorted[labSortByAxis[j] < 0].sum()

            # print "==="
            for k in xrange(nTrn-1):
                if labSortByAxis[j,k] > 0:
                    sumPosLower += weightSorted[k]
                    sumPosUpper -= weightSorted[k]
                else:
                    sumNegLower += weightSorted[k]
                    sumNegUpper -= weightSorted[k]

                errNormal   = sumNegUpper + sumPosLower
                errInverted = sumNegLower + sumPosUpper
                shouldInvert = errNormal >= errInverted
                err = errInverted if shouldInvert else errNormal
                # print labSortByAxis[j,k], j, k, errNormal, errInverted, sumPosLower, sumPosUpper, sumNegLower, sumNegUpper
                assert sumPosLower >= -1e-4 and sumPosUpper >= -1e-4 and sumNegLower >= -1e-4 and sumNegUpper >=  -1e-4

                if err < errMin:
                    errMin = err
                    bestStump = (j, 0.5*(trnDatSorted[j,k]+trnDatSorted[j,k+1]), shouldInvert)
                    # print "!"
            # end for each cut along certain dimension
        # end one dimension

        errRate = errMin / weight.sum()
        learnRatio = np.sqrt((1-errRate)/errRate)
        alphas[i] = np.log(learnRatio)
        g[i] = bestStump
        assert errRate <= 0.5 and learnRatio >= 1.0

        wrongIdx = dstump_predict(trnDat, *bestStump) != trnLab
        weight[~wrongIdx] *= 1.0 / learnRatio
        weight[ wrongIdx] *= learnRatio

        # testing
        Ein  = np.count_nonzero(adaboost_predict(trnDat, g[:i+1], alphas) != trnLab)
        Eout = np.count_nonzero(adaboost_predict(tstDat, g[:i+1], alphas) != tstLab)
        print "iter {:3d}, NErrIn {:4d}, NErrOut {:4d}, usum {:3.3f}, alpha {:0.4f}".format(\
            i, Ein, Eout, weight.sum(), alphas[i]\
        )

    # end one iteration

@print_banner
def prob16_to_18():
    trnDat, trnLab, nTrn = read_hwfile('hw6_train.bin')
    tstDat, tstLab, nTst = read_hwfile('hw6_test.bin')

    decTree = SK_Tree.DecisionTreeClassifier()
    decTree.fit(trnDat, trnLab)
    preLab17 = decTree.predict(trnDat)
    preLab18 = decTree.predict(tstDat)
    Ein  = float(np.count_nonzero(preLab17 != trnLab)) / nTrn
    Eout = float(np.count_nonzero(preLab18 != tstLab)) / nTst

    # imaging binary tree structure
    print "16: #branch = {}".format((decTree.tree_.capacity-1)>>1)
    print "17: Ein  = {}".format(Ein )
    print "18: Eout = {}".format(Eout)

@print_banner
def prob19_20():
    trnDat, trnLab, nTrn = read_hwfile('hw6_train.bin')
    tstDat, tstLab, nTst = read_hwfile('hw6_test.bin')
    nTrnInv = 1.0/nTrn
    nTstInv = 1.0/nTst

    niter = 300
    dtreeList = []
    for i in xrange(niter):
        bagIdx = np.random.randint(nTrn, size=nTrn)
        bagDat = trnDat[bagIdx]
        bagLab = trnLab[bagIdx]

        dtree = SK_Tree.DecisionTreeClassifier()
        dtree.fit(bagDat, bagLab)
        dtreeList.append(dtree)

    sum_gt_trn = np.zeros(nTrn)
    sum_gt_tst = np.zeros(nTst)
    Ein_gt_List = []
    Eout_gt_List = []
    for i, dtree in enumerate(dtreeList):
        predictY_gt_trn = dtree.predict(trnDat)
        predictY_gt_tst = dtree.predict(tstDat)

        sum_gt_trn += predictY_gt_trn
        sum_gt_tst += predictY_gt_tst

        predictY_Gt_trn = np.sign(sum_gt_trn)
        predictY_Gt_tst = np.sign(sum_gt_tst)

        Ein_gt  = nTrnInv * np.count_nonzero(trnLab != predictY_gt_trn)
        Eout_gt = nTstInv * np.count_nonzero(tstLab != predictY_gt_tst)
        Eout_Gt = nTstInv * np.count_nonzero(tstLab != predictY_Gt_tst)

        Ein_gt_List.append(Ein_gt)
        Eout_gt_List.append(Eout_gt)
        print "Iter {:3d}: Eo(gt)={:.3f}, Eo(Gt)={:.3f}, avg Ei(gt)={:.3f}, avg Eo(gt)={:.3f}, var Eo(gt)={:3f}".format(\
            i, Eout_gt, Eout_Gt, np.mean(Ein_gt_List), np.mean(Eout_gt_List), np.var(Eout_gt_List)
        )

if __name__ == '__main__':
    eqs = '='*60

    prob12_to_15()
    prob16_to_18()
    prob19_20()
