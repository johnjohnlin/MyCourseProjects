#!/usr/bin/env python
import unittest
import final_utils as fu
import numpy as np

class FmtReaderTest(unittest.TestCase):
    def setUp(self):
        self.cvt1 = fu.LibsvmFmtConverter((3, 3))

    def test_forward(self):
        a = self.cvt1.line2IdxValArray("1:0.1 4:0.2 9:0.3")
        self.assertEquals(3, len(a))

        def test_b_and_c(x):
            self.assertAlmostEquals(x[0][1], 0.0)
            self.assertAlmostEquals(x[0][0], 0.1)
            self.assertAlmostEquals(x[1][0], 0.2)
            self.assertAlmostEquals(x[2][2], 0.3)

        b = np.zeros((3,3), dtype='f')
        self.cvt1.idxValArray2NpArray(a, b)
        test_b_and_c(b)

        c = self.cvt1.idxValArray2NpArray(a)
        test_b_and_c(c)

        self.assertTrue(np.allclose(b, c))

        self.assertEquals(3, fu.Norm0(a))
        self.assertAlmostEquals(0.6, fu.Norm1(a))

    def test_empty(self):
        self.assertFalse(self.cvt1.idxValArray2NpArray(self.cvt1.line2IdxValArray("")).any())

    def test_backward(self):
        a = np.random.random((3, 3))
        b = self.cvt1.npArray2IdxValArray(a)
        c = self.cvt1.idxValArray2NpArray(b)
        self.assertTrue(np.allclose(a, c))

class AlignTest(unittest.TestCase):
    def setUp(self):
        self.by = 7
        self.bx = 2
        self.align = fu.ZodiacAlign((30,30), (40,40), 0.01)
        self.testArr = np.zeros((20,20), dtype='f')
        self.testArr[self.by:self.by+10, self.bx:self.bx+5] = 1

    def test_calculate(self):
        cropWindow = self.align._cal_crop_window(self.testArr)
        self.assertEquals(cropWindow[0][0], self.by)
        self.assertEquals(cropWindow[0][1], self.by+10-1)
        self.assertEquals(cropWindow[1][0], self.bx)
        self.assertEquals(cropWindow[1][1], self.bx+ 5-1)

        innerRealSize, nPad = self.align._cal_reshape_and_padding_size(cropWindow)
        self.assertEquals(innerRealSize[0], 30)
        self.assertLess(innerRealSize[1], 20)
        self.assertEquals(40, innerRealSize[0]+sum(nPad[0]))
        self.assertEquals(40, innerRealSize[1]+sum(nPad[1]))

        imAlign = self.align._pad(self.testArr, cropWindow, innerRealSize, nPad)
        self.assertEquals(imAlign.shape, (40,40))

if __name__ == '__main__':
    unittest.main(verbosity=2)
