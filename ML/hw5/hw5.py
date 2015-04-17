#!/usr/bin/env python
import numpy as np
import numpy.linalg as la
# svm 3.20
import svmutil as SVM
from itertools import count, imap
from os import devnull
import sys

def prob2_to_4():
    x = np.array([
        [ 1,  0],
        [ 0,  1],
        [ 0, -1],
        [-1,  0],
        [ 0,  2],
        [ 0, -2],
        [-2,  0]
    ])
    y = np.array([-1,-1,-1,1,1,1,1])

    print "===prob 2==="
    xf = np.fliplr(x.copy())
    print xf*xf - 2*x + np.array([[3,-3]])

    print "===prob 3==="
    prob  = SVM.svm_problem(y.tolist(), x.tolist())
    param = SVM.svm_parameter('-t 1 -c 100 -d 2 -r 1 -g 1') # very large C for hard margin
    m = SVM.svm_train(prob, param)
    sumA = 0
    poly = [0] * 6 # xx, xy, yy, x, y, 1
    for i in xrange(m.l):
        idx = m.sv_indices[i]
        alphay = m.sv_coef[0][i]
        alpha = abs(m.sv_coef[0][i])
        print "{:d} {:+1.2f}".format(idx, alpha)
        sumA += alpha
        v = x[idx-1]
        poly[0] += alphay*v[0]*v[0]
        poly[1] += alphay*v[1]*v[0]*2
        poly[2] += alphay*v[1]*v[1]
        poly[3] += alphay*v[0]*2
        poly[4] += alphay*v[1]*2
        # poly[5] += alphay*1 # no need because Sum(alphay) = 0
    poly[5] -= m.rho[0]
    print "Sum of alpha is {:1.3f}\nb = {}".format(sumA, m.rho[0])
    print "{:+2.2f}xx {:+2.2f}xy {:+2.2f}yy {:+2.2f}x {:+2.2f}y {:+2.2f}".format(*poly)

def to_1_vs_all(n, arr):
    y = np.ones(arr.shape[0])
    flip = arr[:,0] != n
    y[flip] = -1
    x = arr[:,1:]
    return y, x

def prob15_to_20():
    trnDat = np.fromfile('features.train.dat', sep=' ').reshape((-1, 3))
    tstDat = np.fromfile('features.test.dat', sep=' ').reshape((-1, 3))

    # problem 15
    param15 = SVM.svm_parameter('-t 0 -c 0.01 -h 0')
    y, x = to_1_vs_all(0, trnDat)
    prob  = SVM.svm_problem(y.tolist(), x.tolist())
    m = SVM.svm_train(prob, param15)

    w = np.zeros(2)
    for i in xrange(m.l):
        idx = m.sv_indices[i]
        alphay = m.sv_coef[0][i]
        w += alphay * x[idx-1]
    print w[0]

    # problem 16 17
    print "===prob 16~17==="
    param16 = SVM.svm_parameter('-t 1 -c 0.01 -d 2 -g 1 -r 1 -h 0')
    for d in xrange(0, 10, 2):
        print "\nvvv digit = {} vvv".format(d)
        y, x = to_1_vs_all(d, trnDat)
        yl = y.tolist()
        xl = x.tolist()
        prob  = SVM.svm_problem(yl, xl)
        m = SVM.svm_train(prob, param16)
        SVM.svm_predict(yl, xl, m)
        sA = 0
        for i in xrange(m.l):
            idx = m.sv_indices[i]
            sA += abs(m.sv_coef[0][i])
        print "sum of alpha = {}".format(sA)
        print "^^^ digit = {} ^^^".format(d)

    # problem 18
    print "===prob 18==="
    Cs = [0.001, 0.01, 0.1, 1.0, 10]
    yTrn, xTrn = [a.tolist() for a in to_1_vs_all(0, trnDat)]
    yTst, xTst = [a.tolist() for a in to_1_vs_all(0, tstDat)]
    for C in Cs:
        print "\nvvv C = {} vvv".format(C)
        param18 = SVM.svm_parameter('-h 0 -t 2 -c {:f} -g 100'.format(C))
        prob  = SVM.svm_problem(yTrn, xTrn)
        m = SVM.svm_train(prob, param18)
        # Eout and sum of violation
        # NOTE: pVal is list of list[1]... and Eout[0] is what we want
        Eout, pVal = SVM.svm_predict(yTst, xTst, m)[1:]
        err = sum(imap(lambda v, x: max(1-v[0]*x, 0), pVal, yTst))
        # |w| square in hyperplane
        # NOTE: I think it can be calculated by this way
        # > wTw = -m.obj-C*err
        # However, the objective function is printed only...
        print "^^^ C = {}, sum of err = {}, Eout = {}^^^".format(C, err, Eout[0])

    # problem 19
    print "===prob 19==="
    gammas = [1, 10, 100, 1000, 10000]
    for gamma in gammas:
        print "\nvvv gamma = {} vvv".format(gamma)
        param19 = SVM.svm_parameter('-h 0 -t 2 -c 0.1 -g {:f} -q'.format(gamma))
        prob  = SVM.svm_problem(yTrn, xTrn)
        m = SVM.svm_train(prob, param19)
        SVM.svm_predict(yTst, xTst, m)
        print "^^^ gamma = {} ^^^".format(gamma)

    # problem 20
    print "===prob 20==="
    niter = 100
    nsample = 1000
    cntGammaUsed = np.zeros(len(gammas), dtype=np.uint)
    param20s = [SVM.svm_parameter('-q -h 0 -t 2 -c 0.1 -g {:f}'.format(gamma)) for gamma in gammas]

    yTrnArr, xTrnArr = to_1_vs_all(0, trnDat)
    yxTrnArr = np.hstack((yTrnArr[:,None], xTrnArr))
    assert len(yxTrnArr) > nsample
    shuffleAgain = len(yxTrnArr)/nsample
    shuffleCounter = shuffleAgain

    fp_null = open(devnull, 'w')
    stdout_tmp = sys.stdout
    sys.stdout = fp_null
    for i in xrange(niter):
        # init the problem
        if shuffleCounter == shuffleAgain:
            shuffleCounter = 0
            np.random.shuffle(yxTrnArr)
        ofs = shuffleCounter * nsample
        prob = SVM.svm_problem(yxTrnArr[ofs:ofs+nsample,0].tolist(), yxTrnArr[ofs:ofs+nsample,1:].tolist())
        shuffleCounter += 1

        # all gamma
        maxGammaIdx = None
        maxAcc = 0
        for j, param20 in enumerate(param20s):
            m = SVM.svm_train(prob, param20)
            p_acc = SVM.svm_predict(yTst, xTst, m)[1][0]
            if p_acc > maxAcc:
                maxAcc = p_acc
                maxGammaIdx = j
        cntGammaUsed[maxGammaIdx] += 1
    sys.stdout = stdout_tmp
    for gamma, cnt in zip(gammas, cntGammaUsed):
        print "gamma = {:f} is used {:d} times".format(gamma, cnt)


if __name__ == '__main__':
    print "===prob 2~4==="
    prob2_to_4()
    print "===prob 15~20==="
    prob15_to_20()

