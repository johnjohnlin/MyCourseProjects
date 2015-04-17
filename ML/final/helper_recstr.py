#!/usr/bin/env python
if __name__ == '__main__':
    from scipy.misc import imsave
    import sys, os
    from numpy import empty, zeros
    import final_utils as fu

    def write_px(splits, cl, ith):
        _cvt.idxValArray2NpArray(
            _cvt.line2IdxValArray(splits),
            _im
        )
        MKDIR(_pngDirPathFmt.format(cl))
        imsave(_pngPathFmt.format(cl, ith), _im)

    def MKDIR(path):
        if not os.path.isdir(path):
            os.mkdir(path)

    # main
    if len(sys.argv) != 4:
        raise AttributeError("{} <fname> <W> <H>".format(sys.argv[0]))

    _W = int(sys.argv[2])
    _H = int(sys.argv[3])
    _im = empty((_H,_W))
    _fname = sys.argv[1]
    _dname = _fname + "_dir"
    _pngDirPathFmt = _dname + "/{:05d}"
    _pngPathFmt    = _pngDirPathFmt + "/{:05d}.png"
    _cvt = fu.LibsvmFmtConverter((_H,_W))
    MKDIR(_dname)

    with open(_fname, 'r') as fp:
        for i, l in enumerate(iter(fp.readline, '')):
            l = l.split(' ', 1)
            cl = int(l[0])
            if len(l) == 2:
                write_px(l[1], cl, i)
            else:
                write_px('', cl, i)
