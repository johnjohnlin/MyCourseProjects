#!/usr/bin/env python
import numpy as np
import numpy.linalg as la
from math import log, sqrt, exp

def gen_13_14_15(N):
    x = np.random.uniform(-1, 1, (N, 2))
    xLin  = np.hstack((np.ones((N,1)),x))
    xQuad = np.hstack((xLin, (x[:,0]*x[:,1])[:,None], x*x))

    y = np.sign(xQuad[:,-1] + xQuad[:,-2] - 0.6)
    flip = np.random.randint(0, 10, N) == 0
    y[flip] = -y[flip]
    return x, xLin, xQuad, y

def prob13_14_15():
    N = 1000
    niter = 1000
    wQuadSum = np.zeros(6)
    EinLinSum   = 0
    EoutQuadSum = 0
    for i in xrange(niter):
        x, xLin, xQuad, y = gen_13_14_15(N)

        wLin  = la.pinv(xLin) .dot(y)
        wQuad = la.pinv(xQuad).dot(y)

        # 13
        EinLinSum   += np.count_nonzero(np.sign(xLin.dot(wLin)) != y)
        # 14
        wQuadSum    += wQuad
        # 15
        x, xLin, xQuad, y = gen_13_14_15(N)
        EoutQuadSum += np.count_nonzero(np.sign(xQuad.dot(wQuad)) != y)

    normFac = 1.0/(N*niter)

    return (EinLinSum*normFac, EoutQuadSum*normFac, wQuadSum/niter)

def prob18_19_20():
    trnDat, trnLab, nTrn = read_hwfile('hw3_train.bin')
    tstDat, tstLab, nTst = read_hwfile('hw3_test.bin')
    nTrnInv = 1.0 / nTrn

    niter = 2000
    currIfor20 = 0
    w18 = np.zeros(20)
    w19 = np.zeros(20)
    w20 = np.zeros(20)
    for i in xrange(niter):
        coeff18 = trnLab / (1.0+np.exp(trnDat.dot(w18)*trnLab))
        coeff19 = trnLab / (1.0+np.exp(trnDat.dot(w19)*trnLab))
        negGradE18 = (coeff18[:,None]*trnDat).sum(0) * nTrnInv
        negGradE19 = (coeff19[:,None]*trnDat).sum(0) * nTrnInv
        w18 += negGradE18 * 0.001
        w19 += negGradE19 * 0.01

        sgdX = trnDat[currIfor20,:]
        sgdLab = trnLab[currIfor20]
        coeff20 = sgdLab / (1.0+np.exp(sgdX.dot(w20)*sgdLab))
        sgdNegGradE20 = coeff20 * sgdX
        w20 += sgdNegGradE20 * 0.001
        currIfor20 = 0 if currIfor20 == nTrn-1 else currIfor20+1

    Eout18 = float(np.count_nonzero(np.sign(tstDat.dot(w18)) != tstLab)) / nTst
    Eout19 = float(np.count_nonzero(np.sign(tstDat.dot(w19)) != tstLab)) / nTst
    Eout20 = float(np.count_nonzero(np.sign(tstDat.dot(w20)) != tstLab)) / nTst
    return (Eout18, Eout19, Eout20)


def read_hwfile(fname):
    dim = 21
    dat = np.fromfile(fname, sep='   ')
    nDat = dat.shape[0]/dim
    dat = dat.reshape((nDat, dim))
    lab = dat[:,dim-1].copy()
    dat = dat[:,0:(dim-1)].copy()
    return dat, lab, nDat

def prob_6_to_10():
    E   = lambda u, v: exp(u)+exp(2*v)+exp(u*v)+u*u-2*u*v+2*v*v-3*u-2*v
    Eu  = lambda u, v: exp(u)+v*exp(u*v)+2*u-v*2-3
    Ev  = lambda u, v: 2*exp(2*v)+u*exp(u*v)-2*u+4*v-2
    Euu = lambda u, v: exp(u)+v*v*exp(u)+2
    Euv = lambda u, v: (1+u*v)*exp(u*v)-2
    Evv = lambda u, v: 4*exp(4*v)+u*u*exp(u*v)

    a6 = [Eu(0, 0), Ev(0, 0)]

    a7 = [0, 0]
    for i in range(5):
        EuCur = Eu(*a7)
        EvCur = Ev(*a7)
        a7[0] -= 0.01 * EuCur
        a7[1] -= 0.01 * EvCur

    a10 = [0, 0]
    for i in range(5):
        EuCur = Eu(*a10)
        EvCur = Ev(*a10)
        EuuCur = Euu(*a10)
        EuvCur = Euv(*a10)
        EvvCur = Evv(*a10)

        d = la.pinv(np.array([[EuuCur, EuvCur], [EuvCur, EvvCur]])).dot(np.array([EuCur, EvCur]))
        a10[0] -= d[0]
        a10[1] -= d[1]

    return (a6, E(*a7), E(*a10))


if __name__ == '__main__':
    np.set_printoptions(precision=3)

    print "Ans of 6, 7, 10 is:"
    print prob_6_to_10()

    print '===========13_14_15==========='
    a = prob13_14_15()
    print "Linear, Ein average =  {}".format(a[0])
    print "Vector {}".format(a[2])
    print "Quad,  Eout average =  {}".format(a[1])

    print '===========18_19_20==========='
    a = prob18_19_20()
    print "Eout of 18-20 is {}, {} and {}".format(*a)
