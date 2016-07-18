#!/usr/bin/python3

import sys
import os
import subprocess
import zipfile
import multiprocessing

TMP_DIR = '/tmp'
ARCHIVE_NAME = sys.argv[1]
EXTERNALS_BUILD_PATH = sys.argv[2]

CGATE_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.x86_64.tar')[0])
BUILD_INSTALL_PREFIX = '{0}/cgate'.format(EXTERNALS_BUILD_PATH)

# Remove old lib
subprocess.check_call('rm -rf {0}/cgate'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(CGATE_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('tar -xzvf {0} -C {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(CGATE_TMP_DIR)

# Copy to repository
os.makedirs(BUILD_INSTALL_PREFIX)
subprocess.check_call('cp -R {0}/* {1}'.format(CGATE_TMP_DIR, BUILD_INSTALL_PREFIX), shell=True)

# Copy scheme and other configs - NOTE: you can get scheme from ftp://ftp.moex.com/pub/FORTS/test/Scheme/
subprocess.check_call('cp -R {0}/../plaza2/* {1}'.format(EXTERNALS_BUILD_PATH, BUILD_INSTALL_PREFIX), shell=True)
