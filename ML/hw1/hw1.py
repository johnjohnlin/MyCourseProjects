#!/usr/bin/env python
import numpy as np
from hashlib import md5
from struct import pack, unpack
from random import randint

def naivePLA(dat, lab, maxUpdate=-1, f=lambda x: x, eta=1):
    (nDat, dim) = dat.shape
    w = np.zeros(dim)
    u = 0

    curTestIdx = 0
    firstTime = True
    searchIdx = curTestIdx
    while (firstTime or searchIdx != curTestIdx) and (maxUpdate < 0 or u < maxUpdate):
        firstTime = False
        x = dat[f(searchIdx)]
        y = lab[f(searchIdx)]
        searchIdx = searchIdx+1 if searchIdx!=nDat-1 else 0
        if (y>0) != (w.dot(x)>0):
            w += eta*y*x
            firstTime = True
            curTestIdx = searchIdx
            u += 1
            continue
    return (u, w)

def pocketPLA(dat, lab, maxUpdate=-1, eta=1, retWHat = True):
    nDat, dim = dat.shape
    u = 0

    w = np.zeros(dim)
    wHat = w.copy()
    idxErr = np.nonzero(lab > 0)[0]
    nErr = idxErr.shape[0]
    while (maxUpdate < 0 or u < maxUpdate) and nErr:
        u += 1
        rndIdx = idxErr[randint(0, nErr-1)]
        w += eta*lab[rndIdx]*dat[rndIdx]
        idxErr = np.nonzero((dat.dot(w) > 0) != (lab > 0))[0]
        nErrTmp = idxErr.shape[0]
        if nErr == 0:
            break
        elif nErrTmp < nErr:
            nErr = nErrTmp
            wHat = w.copy()
    return (u, wHat if retWHat else w)

def read_hwfile(fname):
    dat = np.fromfile(fname, sep='   ')
    nDat = dat.shape[0]/5
    dat = dat.reshape((nDat, 5))
    lab = dat[:,4].copy()
    dat[:,4] = np.ones(nDat)
    return dat, lab

def prob15_16_17():
    dat, lab = read_hwfile('hw1_15_train.bin')
    nDat = dat.shape[0]

    n = naivePLA(dat, lab, 1000)[0]
    print "15: Loop stops after {} times".format(n)

    sn16 = 0
    sn17 = 0
    n16_17 = 2000
    for i in xrange(n16_17):
        lut = [unpack("i", md5(pack("2i", i, x)).digest()[-4:])[0] % nDat for x in xrange(nDat)]
        sn16 += naivePLA(dat, lab, 1000,
             lambda x: lut[x]
        )[0]
        sn17 += naivePLA(dat, lab, 1000,
             lambda x: lut[x],
             0.5
        )[0]
    print "16: Loop stops after {} times".format(float(sn16)/n16_17)
    print "17: Loop stops after {} times".format(float(sn17)/n16_17)

def prob18_19_20():
    dat , lab  = read_hwfile('hw1_18_train.bin')
    tdat, tlab = read_hwfile('hw1_18_test.bin' )
    nDat  = dat .shape[0]
    nTest = tdat.shape[0]

    cr18 = 0
    cr19 = 0
    cr20 = 0
    sn18 = 0
    sn19 = 0
    sn20 = 0
    n18_19_20 = 2000
    for i in xrange(n18_19_20):
        n18, w18 = pocketPLA(dat, lab,  50)
        cr18 += np.count_nonzero(tdat.dot(w18)*tlab < 0)
        sn18 += n18
        n19, w19 = pocketPLA(dat, lab,  50, retWHat = False)
        cr19 += np.count_nonzero(tdat.dot(w19)*tlab < 0)
        sn19 += n19
        n20, w20 = pocketPLA(dat, lab, 100)
        cr20 += np.count_nonzero(tdat.dot(w20)*tlab < 0)
        sn20 += n20
    print "18: Rate: {}, avg converge {}".format(float(cr18)/(n18_19_20*nTest), float(sn18)/n18_19_20)
    print "19: Rate: {}, avg converge {}".format(float(cr19)/(n18_19_20*nTest), float(sn19)/n18_19_20)
    print "20: Rate: {}, avg converge {}".format(float(cr20)/(n18_19_20*nTest), float(sn20)/n18_19_20)

if __name__ == '__main__':
    print '==========15_16_17=========='
    prob15_16_17()
    print '==========18_19_20=========='
    prob18_19_20()
