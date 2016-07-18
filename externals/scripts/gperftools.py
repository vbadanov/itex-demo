#!/usr/bin/python3

import sys
import os
import subprocess
import zipfile
import multiprocessing
import platform


# Skip for OS X (Darwin)
if platform.platform().startswith('Darwin'):
    sys.exit(0);


TMP_DIR = '/tmp'
ARCHIVE_NAME = sys.argv[1]
EXTERNALS_BUILD_PATH = sys.argv[2]

GPERFTOOLS_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.tar')[0])
BUILD_INSTALL_PREFIX = '{0}/gperftools'.format(EXTERNALS_BUILD_PATH)

# Remove old lib
subprocess.check_call('rm -rf {0}/gperftools'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(GPERFTOOLS_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('tar -xzvf {0} -C {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(GPERFTOOLS_TMP_DIR)

# Build
subprocess.check_call('./configure --prefix={0}'.format(BUILD_INSTALL_PREFIX), shell=True)
subprocess.check_call('make -j{0}'.format(multiprocessing.cpu_count()), shell=True)

# Copy to repository
subprocess.check_call('make -j{0} install'.format(multiprocessing.cpu_count()), shell=True)



