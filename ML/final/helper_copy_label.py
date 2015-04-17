#!/usr/bin/env python
if __name__ == "__main__":
    from itertools import izip
    from sys import argv

    with open(argv[1], 'r') as fpLabel,\
         open(argv[2], 'r') as fpData,\
         open(argv[3], 'w') as fpMerge:
        for lLabel, lData in izip(iter(fpLabel.readline, ''), iter(fpData.readline, '')):
            l = lLabel.strip() + ' ' + lData.strip().partition(' ')[2] + '\n'
            fpMerge.write(l)
