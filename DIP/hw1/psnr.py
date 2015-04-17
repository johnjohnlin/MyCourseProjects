#!/usr/bin/env python
import cv2, math, numpy
from sys import argv

if len(argv) != 3:
	print "Usage: " + argv[0] + " <image>"
	exit()

i1Name = argv[1]
i2Name = argv[2]

i1 = cv2.imread(i1Name, 0).astype(numpy.int32)
i2 = cv2.imread(i2Name, 0).astype(numpy.int32)

idiff = i1-i2
idiff = idiff * idiff
diffMean = numpy.mean(idiff)
PSNR = 10 * math.log10(255*255/diffMean)
print PSNR
