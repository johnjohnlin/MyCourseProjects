#!/usr/bin/env python
import numpy as np
from math import log, sqrt

def cal_err_num(x, y, s, theta):
    return np.count_nonzero(((x>theta) == (s>0)) != (y>0))

def decision_stump(x, y):
    nTest = x.shape[0]
    xArg = x.argsort()
    ySorted = y[xArg]
    def cumsum_min_wrap(a):
        ac = a.cumsum()
        i = ac.argmin()
        return i, -ac[i]

    i  ,minEle   = cumsum_min_wrap(ySorted)
    i_r,minEle_r = cumsum_min_wrap(ySorted[::-1])
    if minEle >= minEle_r:
        s = 1
        reti = i
    else:
        s = -1
        reti = nTest-2-i_r

    get_res = lambda i: x[xArg[i]]
    if reti == -1:
        theta = get_res(0)
    elif reti == nTest-1:
        theta = get_res(-1)
    else:
        theta = 0.5*(get_res(reti)+get_res(reti+1))
    nErr = cal_err_num(x, y, s, theta)
    return (s, theta, nErr)

def prob17_18():
    nTest = 20
    nTestInv = 1.0/nTest
    nRep = 5000
    sumRes = np.zeros(4)

    for ind in xrange(nRep):
        x = np.random.random(nTest)*2-1
        y = np.sign(x)
        flip = np.random.randint(0, 5, nTest) == 0
        y[flip] = -y[flip]

        s, theta, nErr = decision_stump(x, y)
        sumRes += np.array([s, theta, nErr*nTestInv, 0.5+0.3*s*(abs(theta)-1)])

    return sumRes/nRep

def read_hwfile(fname):
    dat = np.fromfile(fname, sep='   ')
    nDat = dat.shape[0]/10
    dat = dat.reshape((nDat, 10))
    lab = dat[:,9].copy()
    dat = dat[:,0:9].T.copy()
    return dat, lab, nDat

def prob19_20():
    trnDat, trnLab, nTrn = read_hwfile('hw2_train.bin')
    tstDat, tstLab, nTst = read_hwfile('hw2_test.bin')

    minNErr = nTrn+1
    minI = 0
    minS = 0
    minTheta = 0
    for d in xrange(9):
        s, theta, nErr = decision_stump(trnDat[d,:], trnLab)
        if minNErr > nErr:
            minNErr = nErr
            minI = d
            minS = s
            minTheta = theta

    Ein  = minNErr / float(nTrn)
    Eout = cal_err_num(tstDat[minI,:], tstLab, minS, minTheta) / float(nTst)
    return (Ein, Eout)

def nav_quad(a, b, c):
    ainv2 = 1.0/(2*a)
    sD = sqrt(b*b - 4*a*c)
    return ((-b+sD)*ainv2, (-b+sD)*ainv2)

def prob_4_5(N):
    delta = 0.05
    dvc = 50
    # [a]
    a = sqrt(8.0/N*(log(4)+log(2*N)*dvc-log(delta)))
    # [b]
    b = sqrt(16.0/N*(log(2)+log(N)*dvc-log(delta)))
    # [c]
    c = sqrt(2*(log(2*N)+dvc*log(N))/N) + sqrt(-2.0/N*log(delta)) + 1.0/N
    # [d]
    p = N
    q = -2
    r = -( log(6) + log(2*N)*dvc - log(delta) )
    d = nav_quad(p, q, r)
    # [e]
    p = 2*N-4
    q = -4
    r = -( log(4) + log(N)*dvc*2 - log(delta) )
    e = nav_quad(p, q, r)
    return (a, b, c, d, e)

if __name__ == '__main__':
    print '=============4_5============='
    print prob_4_5(10000)
    print prob_4_5(5)
    print '============17_18============'
    a = prob17_18()
    print "Avg Ein  is {}".format(a[2])
    print "Avg Eout is {}".format(a[3])

    print '============19_20============'
    Ein, Eout = prob19_20()
    print "Avg Ein  is {}".format(Ein)
    print "Avg Eout is {}".format(Eout)
