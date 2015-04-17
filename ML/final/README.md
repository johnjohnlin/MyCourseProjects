# Files
## Running files
### run\_align.py
Use this to align (crop) training data and testing data, _file name is fixed_

Input and output are both libsvm format

### run\_genhog.py
Extract hog features

Usage:

`<exectuable> <libsvm file> <W> <H>`

It will generate file `<libsvm file>.hog.dat`

### run\_hogsvm.py
Used for running SVM classfier on HOG, _file name is fixed_

## Helper files

### helper\_recstr.py
Used to convert libsvm format to png files, which are sorted by classes

Usage:

`<exectuable> <libsvm file> <W> <H>`

It will generate directory `<file>_dir`

### helper\_svmtune.py
Used for tuning SVM on HOG, _file name is fixed_

### helper\_copy\_label.py
Used to copy label (uploaded results.txt format) to libsvm format

Usage:

`<exectuable> <label> <libsvm file> <output libsvm file>`

## Librarie files
### hog.py, final\_utils.py
Libraries

---

# How to run

## Train in phase 1
1. Run `run_align.py` to align all data
1. Run `run_genhog.py ml14fall_train_align.dat 120 140` and `run_genhog.py ml14fall_test1_no_answer_align.dat 120 140` to generate HOG
1. Run `run_hogsvm.py` to get `result.txt`

## Train in phase 2 (With phase 1 golden answer)
1. Concatenate the 2 files with labels
1. Rename the files...
1. The same as the previous flow

# Libraries used
* libsvm (3.20) Python interface
* Python sklearn (0.14.1)
* OpenCV (2.4.9) Python interface

---

# Results
We achieve rank 9/60 using *only SVM*, no NN.
To our knowledge, this is the best result of SVM out of all teams.
Besides, it's very fast compared to NN, the whole flow spent no more than 20 mins.
We use Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz without threading.
