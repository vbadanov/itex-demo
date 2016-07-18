#!/usr/bin/python3

import sys
import os
import subprocess
import multiprocessing
import platform

TMP_DIR = '/tmp'
ARCHIVE_NAME = sys.argv[1]
EXTERNALS_BUILD_PATH = sys.argv[2]

MATHGL_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.tar')[0])
BUILD_INSTALL_PREFIX = '{0}/mathgl'.format(EXTERNALS_BUILD_PATH)
LIB_DIR = '{0}/lib'.format(BUILD_INSTALL_PREFIX)


# Remove old lib
subprocess.check_call('rm -rf {0}/mathgl'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(MATHGL_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('tar -xzvf {0} -C {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(MATHGL_TMP_DIR)

# Build C++ library
CC = 'gcc'
CXX = 'g++'
if platform.platform().startswith('Darwin'):
    CC='gcc-5'
    CXX='g++-5'
    
subprocess.check_call('cmake -DCMAKE_C_COMPILER={0} -DCMAKE_CXX_COMPILER={1} -DCMAKE_INSTALL_PREFIX={2} -DMGL_HAVE_TYPEOF=0 -DMGL_HAVE_C99_COMPLEX=0 -Denable-jpeg=on -Denable-gif=on -Denable-openmp=on .'.format(CC, CXX, BUILD_INSTALL_PREFIX), shell=True)
subprocess.check_call('cmake -DCMAKE_C_COMPILER={0} -DCMAKE_CXX_COMPILER={1} -DCMAKE_INSTALL_PREFIX={2} -DMGL_HAVE_TYPEOF=0 -DMGL_HAVE_C99_COMPLEX=0 -Denable-jpeg=on -Denable-gif=on -Denable-openmp=on .'.format(CC, CXX, BUILD_INSTALL_PREFIX), shell=True)
subprocess.check_call('make -j{0}'.format(multiprocessing.cpu_count()), shell=True)

# Copy to repository
subprocess.check_call('make -j{0} install'.format(multiprocessing.cpu_count()), shell=True)


