#!/usr/bin/env python
import numpy as np
import numpy.linalg as la

def nErr(w, X, y):
    return np.count_nonzero(np.sign(X.dot(w)) != y)

def read_hwfile(fname):
    dim = 3
    dat = np.fromfile(fname, sep='   ')
    nDat = dat.shape[0]/dim
    dat = dat.reshape((nDat, dim))
    lab = dat[:,dim-1].copy()
    dat[:,(dim-1)] = 1
    return dat, lab, nDat

def pinv_regu(X, y, lmbda):
    A = X.T.dot(X)
    n = X.shape[1]
    A[np.diag_indices(n)] += lmbda
    w = la.solve(A, y.dot(X))
    return w

def prob13_to_20():
    trnDat, trnLab, nTrn = read_hwfile('hw4_train.bin')
    tstDat, tstLab, nTst = read_hwfile('hw4_test.bin')
    nTrnInv = 1.0/nTrn
    nTstInv = 1.0/nTst

    print "For problem 13~15"
    lmbda = 1E-10
    for p in xrange(-10, 3):
        w = pinv_regu(trnDat, trnLab, lmbda)
        Ein = nErr(w, trnDat, trnLab)*nTrnInv
        Eout = nErr(w, tstDat, tstLab)*nTstInv
        print "log10(lambda) = {}:\n\tw = {}\n\tEin = {:.3f}, Eout = {:.3f}".format(p, w, Ein, Eout)
        lmbda *= 10

    print "For problem 13~18"
    trnDatTrn = trnDat[:120,:]
    trnDatVal = trnDat[-80:,:]
    trnLabTrn = trnLab[:120]
    trnLabVal = trnLab[-80:]
    lmbda = 1E-10
    for p in xrange(-10, 3):
        w = pinv_regu(trnDatTrn, trnLabTrn, lmbda)
        Etrn = nErr(w, trnDatTrn, trnLabTrn) / 120.0
        Eval = nErr(w, trnDatVal, trnLabVal) /  80.0
        Eout = nErr(w, tstDat, tstLab) * nTstInv
        lmbda *= 10
        print "log10(lambda) = {}:\n\tEtrn = {:.3f}, Eval = {:.3f}, Eout = {:.3f}".format(p, Etrn, Eval, Eout)

    print "For problem 19~20"
    nFold = 5
    nPerFold = nTrn/nFold
    lmbda = 1E-10
    for p in xrange(-10, 3):
        nEcv = 0
        for i in xrange(nFold):
            outStart = i*nPerFold
            outEnd = (i+1)*nPerFold
            idxCVTrn = np.concatenate((np.arange(outStart), np.arange(outEnd, nTrn)))
            w = pinv_regu(trnDat[idxCVTrn,:], trnLab[idxCVTrn], lmbda)
            nEcv += nErr(w, trnDat[outStart:outEnd,:], trnLab[outStart:outEnd])
        Ecv = nEcv * nTrnInv
        lmbda *= 10
        print "log10(lambda) = {}:\n\tEcv = {:.3f}".format(p, Ecv)



if __name__ == '__main__':
    np.set_printoptions(precision=3)

    print "Result of 13~20 is:"
    prob13_to_20()
