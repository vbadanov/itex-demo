#!/usr/bin/python3

import sys
import os
import subprocess
import zipfile
import multiprocessing
import platform

TMP_DIR = '/tmp'
ARCHIVE_NAME = sys.argv[1]
EXTERNALS_BUILD_PATH = sys.argv[2]

GTEST_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.zip')[0])
LIB_DIR = '{0}/gtest/lib'.format(EXTERNALS_BUILD_PATH)
INC_DIR = '{0}/gtest/include'.format(EXTERNALS_BUILD_PATH)

# Remove old lib
subprocess.check_call('rm -rf {0}/gtest'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(GTEST_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('unzip {0} -d {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(GTEST_TMP_DIR)

# Build
if platform.platform().startswith('Darwin'):
    subprocess.check_call('cmake -DCMAKE_CXX_COMPILER=g++-5 .', shell=True)
else:
    subprocess.check_call('cmake .', shell=True)
    
subprocess.check_call('make -j{0}'.format(multiprocessing.cpu_count()), shell=True)

# Copy to repository
os.makedirs(LIB_DIR)
os.makedirs(INC_DIR)
subprocess.check_call('cp -R libgtest* {0}'.format(LIB_DIR), shell=True)
subprocess.check_call('cp -R include/* {0}'.format(INC_DIR), shell=True)






