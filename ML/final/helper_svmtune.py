#!/usr/bin/env python

if __name__ == '__main__':
    import numpy as np
    # svm 3.20
    import svmutil as SVM
    from final_utils import read_hwfile

    # initialize data
    dat, lab, nDat = read_hwfile('ml14fall_train_align.dat.hog.dat', 169)
    nVal = nDat/5
    nTrn = nDat-nVal
    datTrn = dat[:nTrn]
    labTrn = lab[:nTrn]
    datVal = dat[-nVal:]
    labVal = lab[-nVal:]
    print "#trn = {}, #val = {}".format(nTrn, nVal)

    # SVM
    ##### LINEAR #####
    # param = SVM.svm_parameter('-t 0 -c 5 -h 0') # 82%

    ##### RBF level 1 grid #####
    # param = SVM.svm_parameter('-t 2 -c 1.126    -g 11.26    -h 0') # 75%

    # param = SVM.svm_parameter('-t 2 -c 112.6    -g 1.126    -h 0') # 87.5%
    # param = SVM.svm_parameter('-t 2 -c 11.26    -g 1.126    -h 0') # 87.9%
    # param = SVM.svm_parameter('-t 2 -c 1.126    -g 1.126    -h 0') # 86%
    # param = SVM.svm_parameter('-t 2 -c 0.1126   -g 1.126    -h 0') # 79%
    # param = SVM.svm_parameter('-t 2 -c 0.01126  -g 1.126    -h 0') # %

    # param = SVM.svm_parameter('-t 2 -c 112.6    -g 0.1126   -h 0') # 83.9%
    # param = SVM.svm_parameter('-t 2 -c 11.26    -g 0.1126   -h 0') # 83.9
    # param = SVM.svm_parameter('-t 2 -c 1.126    -g 0.1126   -h 0') # 82%
    # param = SVM.svm_parameter('-t 2 -c 0.1126   -g 0.1126   -h 0') # 64%
    # param = SVM.svm_parameter('-t 2 -c 0.01126  -g 0.1126   -h 0') # 

    ##### RBF level 2 grid #####
    # param = SVM.svm_parameter('-t 2 -c 2  -g 1 -h 0') # 87.5%
    # param = SVM.svm_parameter('-t 2 -c 5  -g 1 -h 0') # 88.06%
    # param = SVM.svm_parameter('-t 2 -c 10 -g 1 -h 0') # 87.9

    ##### RBF level 3 grid #####
    # param = SVM.svm_parameter('-t 2 -c 5  -g 0.2 -h 0') # 
    # param = SVM.svm_parameter('-t 2 -c 5  -g 0.5 -h 0') # 86%
    # param = SVM.svm_parameter('-t 2 -c 5  -g 1   -h 0') # 88.06%
    param = SVM.svm_parameter('-t 2 -c 5  -g 2   -h 0') # 88.06%
    # param = SVM.svm_parameter('-t 2 -c 5  -g 5   -h 0') # 86.55%

    problem = SVM.svm_problem(labTrn.tolist(), datTrn.tolist())
    model = SVM.svm_train(problem, param)
    SVM.svm_predict(labVal.tolist(), datVal.tolist(), model)
