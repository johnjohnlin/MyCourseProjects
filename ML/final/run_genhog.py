#!/usr/bin/env python
if __name__ == '__main__':
    import hog, cv2, sys
    import numpy as np
    import final_utils as fu

    # main
    if len(sys.argv) != 4:
        raise AttributeError("{} <fname> <W> <H>".format(sys.argv[0]))

    _fname = sys.argv[1]
    _W = int(sys.argv[2])
    _H = int(sys.argv[3])

    # 0.02 for normalized images
    hogCvt = hog.ModifiedHoG((_H, _W), (20, 20), (7, 6), (1, 1), 4, 0.001, 0.001, 0.2)
    cvt = fu.LibsvmFmtConverter((_H, _W))
    img = np.empty((_H, _W), dtype='f4')

    np.set_printoptions(precision = 3, suppress = True)
    with open(_fname) as fpSVM, open(_fname+'.hog.dat', 'w') as fpHOG:
        for i, l in enumerate(iter(fpSVM.readline, '')):
            l = l.split(' ', 1)
            cl = l[0]
            if len(l) == 2:
                cvt.idxValArray2NpArray(
                    cvt.line2IdxValArray(l[1]),
                    img
                )
            else:
                img.fill(0)
            feat = hogCvt.compute(hog.simple_denoise(img))
            fpHOG.write(cl + ' ')
            fpHOG.write(' '.join(['{:.6f}'.format(x) for x in feat.ravel()]))
            fpHOG.write('\n')
            if (i & 1023) == 0:
                print "{} procesed".format(i)
