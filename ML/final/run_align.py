#!/usr/bin/env python
if __name__ == "__main__":
    import final_utils as fu
    import numpy as np

    cvtRd = fu.LibsvmFmtConverter((122,105))
    imgIn = np.empty((122,105))
    align = fu.ZodiacAlign((120,100), (140,120), 0.02)
    cvtWr = fu.LibsvmFmtConverter((140,120))

    print 'ml14fall_train.dat'
    with open('ml14fall_train.dat', 'r') as fpr, open('ml14fall_train_align.dat', 'w') as fpw:
        for i, l in enumerate(iter(fpr.readline, '')):
            s = l.split(' ', 1)
            if len(s) == 1:
                continue
            cls = int(s[0])
            inIdxVal = cvtRd.line2IdxValArray(s[1])
            if fu.Norm0(inIdxVal) < 50:
                continue
            cvtRd.idxValArray2NpArray(inIdxVal, imgIn)
            imgOut = align.align(imgIn)
            outIdxVal = cvtWr.npArray2IdxValArray(imgOut)
            fpw.write('{:d} '.format(cls))
            fpw.write(' '.join(['{:d}:{:.6f}'.format(j, v) for j, v in outIdxVal]))
            fpw.write('\n')
            if (i & 1023) == 0:
                print "{} is processed".format(i)

    print 'ml14fall_test1_no_answer.dat'
    with open('ml14fall_test1_no_answer.dat', 'r') as fpr, open('ml14fall_test1_no_answer_align.dat', 'w') as fpw:
        for i, l in enumerate(iter(fpr.readline, '')):
            s = l.split(' ', 1)
            if len(s) == 1:
                # give up
                fpw.write('-1\n')
                continue
            cls = int(s[0])
            inIdxVal = cvtRd.line2IdxValArray(s[1])
            if fu.Norm0(inIdxVal) < 50:
                # give up again
                fpw.write('-1\n')
                continue
            cvtRd.idxValArray2NpArray(inIdxVal, imgIn)
            imgOut = align.align(imgIn)
            outIdxVal = cvtWr.npArray2IdxValArray(imgOut)
            fpw.write('{:d} '.format(cls))
            fpw.write(' '.join(['{:d}:{:.6f}'.format(j, v) for j, v in outIdxVal]))
            fpw.write('\n')
            if (i & 1023) == 0:
                print "{} is processed".format(i)
