#!/usr/bin/env python
if __name__ == '__main__':
    import numpy as np
    # svm 3.20
    import svmutil as SVM
    from os.path import isfile
    from itertools import izip
    from final_utils import read_hwfile

    # initialize data
    datTrn, labTrn, nTrn = read_hwfile('ml14fall_train_align.dat.hog.dat', 169)
    datTst, labTst, nTst = read_hwfile('ml14fall_test1_no_answer_align.dat.hog.dat', 169)

    save_model_name = 'svm_train.model'
    if isfile(save_model_name):
        model = SVM.svm_load_model(save_model_name)
    else:
        param = SVM.svm_parameter('-t 2 -c 5 -g 2 -h 0')
        problem = SVM.svm_problem(labTrn.tolist(), datTrn.tolist())
        model = SVM.svm_train(problem, param)
        SVM.svm_save_model(save_model_name, model)

    p_label, p_acc, p_val = SVM.svm_predict(labTst.tolist(), datTst.tolist(), model)

    with open("result.txt", 'w') as fp:
        for label in p_label:
            fp.write('{:d}\n'.format(int(label)))
