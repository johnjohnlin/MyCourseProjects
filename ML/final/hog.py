import numpy as np
from numpy import linalg as la
from itertools import product
import operator, cv2

# the orientation (theta) convention in this code:
# -y
# ^  /
# | /
# |/ ) theta
# +----> x

def gen_Gaussian(a, sigma):
    sigma2 = sigma*sigma
    return np.exp(a*a*(-0.5/sigma2)) * (1.0/np.sqrt(2*np.pi*sigma2))

_k33 = np.ones((3,3), dtype='u1')
def simple_denoise(img):
    i1 = cv2.morphologyEx(img, cv2.MORPH_DILATE, _k33)
    i2 = cv2.medianBlur(i1, 3)
    i3 = cv2.morphologyEx(i2, cv2.MORPH_ERODE, _k33)
    return i3

def rot_ic(y, x, theta):
    "Rotate vector in image coordinate counterclock-wise (y, x)"
    c = np.cos(theta)
    s = np.sin(theta)
    yprime = y*c - x*s
    xprime = y*s + x*c
    return yprime, xprime

def gen_oriented_gaussian(ksize, sigmaLong, sigmaShort, theta, dtype):
    "Generate oriented Gaussian where sigmaLong is along theta direction"
    # generate mesh
    y = np.arange(ksize[0], dtype=dtype) - (ksize[0]-1)*0.5
    x = np.arange(ksize[1], dtype=dtype) - (ksize[1]-1)*0.5
    xMesh, yMesh = np.meshgrid(x, y)
    yprimeMesh, xprimeMesh = rot_ic(yMesh, xMesh, -theta)

    # calculate weight
    xWeight = gen_Gaussian(xprimeMesh, sigmaLong)
    yWeight = gen_Gaussian(yprimeMesh, sigmaShort)
    orientedKernel = xWeight*yWeight
    return orientedKernel * (1.0/orientedKernel.sum())

class ModifiedHoG:
    # size format is (h, w)
    def __init__(self,
            windowShape=(128,128), cellShape=(8,8), blockShapeInCell=(2,2), blockStrideInCell=(1,1),
            nabin=4, normalizeBias1=0.1, normalizeBias2=0.001, normalizeThres=0.2):
        """
            windowShape, cellShape, blockShapeInCell, blockStrideInCell => integer pairs
        """
        self.windowShape       = np.array(windowShape      , dtype=np.int)
        self.cellShape         = np.array(cellShape        , dtype=np.int)
        self.blockShapeInCell  = np.array(blockShapeInCell , dtype=np.int)
        self.blockStrideInCell = np.array(blockStrideInCell, dtype=np.int)
        self.nabin             = nabin # number of angle bin
        self.abinSiz           = np.pi / self.nabin
        self.abinSizInv        = self.nabin / np.pi
        self.nCell             = self.windowShape / self.cellShape
        self.normalizeBias1    = normalizeBias1
        self.normalizeBias2    = normalizeBias2
        self.normalizeThres    = normalizeThres

        testShape = self.nCell * self.cellShape
        assert (testShape == self.windowShape).all(), "Window size must be multiple of cell size"

        self.nBlock = (self.nCell - self.blockShapeInCell) / self.blockStrideInCell + 1
        assert self.nBlock.all(), "No valid block according to blockShapeInCell, blockStrideInCell"

        # calculate size
        self.histDim = self.nCell.tolist()
        self.histDim.append(nabin)

        self.fvDim = self.nBlock.tolist()
        self.fvDim.append(blockShapeInCell[0]*blockShapeInCell[1]*nabin)
        self.fvDim1D = reduce(operator.mul, self.fvDim)

        # allocate memories
        self.blurred_buf = np.empty(self.windowShape, dtype='f4')
        self.gy_buf      = np.empty(self.windowShape, dtype='f4')
        self.gx_buf      = np.empty(self.windowShape, dtype='f4')
        self.ang_buf     = np.empty(self.windowShape, dtype='f4')
        self.mag_buf     = np.empty(self.windowShape, dtype='f4')
        self.featureVec = np.empty(self.fvDim, dtype='f4')
        self.histVec    = np.empty(self.histDim, dtype='f4')

        # generate oriented gaussian
        self.kern = []
        w = 5 # magic number 1
        sigmaLong  = 2.0 # magic number 2
        sigmaShort = 0.5 # magic number 3
        for abin in xrange(nabin):
            self.kern.append(gen_oriented_gaussian((w,w), sigmaLong, sigmaShort, abin*self.abinSiz, 'f4'))

        # used in calculating hog
        cly = self.cellShape[0]
        clx = self.cellShape[1]
        centery = (cly-1)>>1
        centerx = (clx-1)>>1
        clyEven = 1-(cly&1)
        clxEven = 1-(clx&1)
        # plus1 suffix does not really mean plus 1, plus 1 only when even cell size
        # When calculating HOG, we average 2 pixel if that direction is even, and
        # we do dummy averaging to simplify the code.
        self.hogSampleY      = slice(centery        , centery        +cly*self.nCell[1]+1, cly)
        self.hogSampleYplus1 = slice(centery+clyEven, centery+clyEven+cly*self.nCell[1]+1, cly)
        self.hogSampleX      = slice(centerx        , centerx        +clx*self.nCell[0]+1, clx)
        self.hogSampleXplus1 = slice(centerx+clxEven, centerx+clxEven+clx*self.nCell[0]+1, clx)

    def _normBlockHist(self, h):
        scale1 = 1.0 / (la.norm(h)+self.normalizeBias1)
        h = np.minimum(h*scale1, self.normalizeThres)
        scale2 = 1.0 / (la.norm(h)+self.normalizeBias2)
        h *= scale2
        return h

    def _fill_histogram(self, patch):
        for abin in xrange(self.nabin):
            # extract line response along some angle

            # oriented gaussian
            cv2.filter2D(patch, -1, self.kern[abin], self.blurred_buf)

            # gradient
            cv2.Sobel(self.blurred_buf, -1, 0, 1, self.gy_buf, ksize=1)
            cv2.Sobel(self.blurred_buf, -1, 1, 0, self.gx_buf, ksize=1)
            cv2.cartToPolar(self.gx_buf, -self.gy_buf, self.mag_buf, self.ang_buf)

            # rotate
            self.ang_buf -= abin * self.abinSiz
            self.ang_buf[self.ang_buf < 0] += 2 * np.pi
            self.ang_buf[self.ang_buf > np.pi] -= np.pi
            # assert (self.ang_buf >= 0).all() and (self.ang_buf <= np.pi).all()

            # Detect vertical gradient
            # (1-xxx) is equivalent to oriataion linear interpolation in HoG.
            thisAngleResponse = self.mag_buf * np.maximum(1 - self.abinSizInv*np.abs(self.ang_buf-np.pi*0.5), 0)

            celly = self.cellShape[0]
            cellx = self.cellShape[1]

            # Box filter*2, equivalent to triangular filter, namely linear interpolation in HoG.
            thisAngleResponse = cv2.boxFilter(thisAngleResponse, -1, (cellx, celly))
            thisAngleResponse = cv2.boxFilter(thisAngleResponse, -1, (cellx, celly))
            np.maximum(thisAngleResponse, 0, thisAngleResponse)
            # Averaging
            thisAngleResponse[self.hogSampleY, self.hogSampleX] += thisAngleResponse[self.hogSampleY, self.hogSampleXplus1]
            thisAngleResponse[self.hogSampleY, self.hogSampleX] += thisAngleResponse[self.hogSampleYplus1, self.hogSampleX]

            self.histVec[:,:,abin] = thisAngleResponse[self.hogSampleY, self.hogSampleX] * 0.25

    def _fill_feature(self):
        nby = self.nBlock[0]
        nbx = self.nBlock[1]
        sby = self.blockShapeInCell[0]
        sbx = self.blockShapeInCell[1]
        for y, x in product(xrange(nby), xrange(nbx)):
            h = self._normBlockHist(self.histVec[y:y+sby,x:x+sbx].flatten())
            self.featureVec[y,x] = h

    def compute(self, patch):
        self._fill_histogram(patch)
        self._fill_feature()
        return self.featureVec
