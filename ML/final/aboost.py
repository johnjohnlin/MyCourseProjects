#!/usr/bin/env python
if __name__ == '__main__':
    from sklearn.ensemble import AdaBoostClassifier as ABC
    from sklearn.tree import DecisionTreeClassifier as DTC
    import numpy as np
    from sklearn.metrics import accuracy_score
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


    classfier = ABC(DTC(max_depth=6, max_features=1), n_estimators=50000)
    classfier.fit(datTrn, labTrn)

    for i, labPre in enumerate(classfier.staged_predict(datVal)):
	if i % 10 == 9:
	    print accuracy_score(labPre, labVal)
