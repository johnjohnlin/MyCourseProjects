import numpy as np
from scipy.misc import imresize

def read_hwfile(fname, ncol):
    dat = np.fromfile(fname, sep=' ')
    dat = dat.reshape((-1, ncol))
    nDat = dat.shape[0]
    lab = dat[:,0]
    dat = dat[:,1:]
    return dat, lab, nDat

def Norm0(idxValArr):
    return idxValArr.shape[0]

def Norm1(idxValArr):
    return idxValArr['val'].sum()

class LibsvmFmtConverter:
    _dtype = [('idx', 'i4'), ('val', 'f4')]

    def __init__(self, shape, sp=' :'):
        self.shape = shape
        self.sp0   = sp[0]
        self.sp1   = sp[1]

    def line2IdxValArray(self, line):
        """
            Split libsvm format string "4:2.3 44:32.99" using sp[0] then sp[1]
        """
        lineStrip = line.strip()
        if not lineStrip:
            return np.empty(0, dtype=self._dtype)

        lineSplit = lineStrip.split(self.sp0)
        retArray = np.empty(len(lineSplit), dtype=self._dtype)
        for i, sp in enumerate(lineSplit):
            spsp = sp.split(self.sp1)
            retArray[i] = (int(spsp[0])-1, float(spsp[1]))
        return retArray

    def idxValArray2NpArray(self, idxValArr, arr=None):
        """
            Convert numpy structured array(idx, val) to dense numpy array
        """
        if arr == None:
            arr = np.zeros(self.shape)
        else:
            arr.fill(0)
        (arr.ravel())[idxValArr['idx']] = idxValArr['val']
        return arr

    def npArray2IdxValArray(self, arr):
        arrFlat = arr.ravel()
        nz = np.flatnonzero(arrFlat)
        retArray = np.empty(nz.shape[0], dtype=self._dtype)
        retArray['idx'] = nz
        retArray['val'] = arrFlat[nz]
        return retArray

class ZodiacAlign:
    def __init__(self, shapeOutInner, shapeOutOuter, cropRatio):
        self.shapeOutInner = shapeOutInner
        self.shapeOutOuter = shapeOutOuter
        self.cropRatio     = cropRatio
        self.cropThreshold = np.array([cropRatio,1-cropRatio])

    def _clip_cropRange(self, cropRange, l, h):
        c = cropRange.mean()
        cropRangeScaled = (cropRange-c)*(1+2*self.cropRatio) + c + 0.5
        cropRangeScaled = np.clip(cropRangeScaled.astype(np.int_), l, h)
        if cropRangeScaled[0] == cropRangeScaled[1]:
            cropRangeScaled[0] -= 1
            cropRangeScaled[1] += 1
        return np.clip(cropRangeScaled.astype(np.int_), l, h)

    def _cal_crop_window(self, img):
        "Search the crop window containing most intensity of the image"

        yHist = img.sum(1).cumsum()
        xHist = img.sum(0).cumsum()
        cropY = yHist.searchsorted(self.cropThreshold*yHist[-1])
        cropX = xHist.searchsorted(self.cropThreshold*xHist[-1])
        cropY = self._clip_cropRange(cropY, 0, yHist.shape[0])
        cropX = self._clip_cropRange(cropX, 0, xHist.shape[0])
        return cropY, cropX

    def _cal_reshape_and_padding_size(self, cropWindow):
        """
          return value (ny, nx), ((a, b), (c, d))
          (The tuple format are for convininent)
          +--+--+--+
         a|  |  |  |
          +--+--+--+
        ny|  |XX|  |
          +--+--+--+
         b|  |  |  |
          +--+--+--+
           c  nx  d
        We resize the window (cropX, cropY) to XX area afterward.
        EITHER nx == shapeOutInner[1] or ny == shapeOutInner[0]
        (We preserve the aspect ratio)
        The over all size is shapeOutOuter
        """
        dy = float(cropWindow[0][1] - cropWindow[0][0])
        dx = float(cropWindow[1][1] - cropWindow[1][0])
        sy = self.shapeOutInner[0] / dy
        sx = self.shapeOutInner[1] / dx
        s = min(sx, sy)
        ny = min(int(dy*s+0.5), self.shapeOutInner[0])
        nx = min(int(dx*s+0.5), self.shapeOutInner[1])
        padY = self.shapeOutOuter[0] - ny
        padX = self.shapeOutOuter[1] - nx
        padYHalf = padY >> 1
        padXHalf = padX >> 1
        return (ny, nx), ((padYHalf, padY-padYHalf), (padXHalf, padX-padXHalf))

    def _pad(self, im, cropWindow, innerCalculatedSize, nPad):
        imCroped = im[cropWindow[0][0]:cropWindow[0][1],cropWindow[1][0]:cropWindow[1][1]]
        imResized = imresize(imCroped, innerCalculatedSize, mode='F')
        return np.pad(imResized, nPad, 'constant', constant_values=(0,))

    def align(self, imgIn):
        cropWindow = self._cal_crop_window(imgIn)
        innerRealSize, nPad = self._cal_reshape_and_padding_size(cropWindow)
        return self._pad(imgIn, cropWindow, innerRealSize, nPad)

