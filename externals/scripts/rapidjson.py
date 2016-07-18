#!/usr/bin/python3

import sys
import os
import subprocess
import zipfile
import multiprocessing

TMP_DIR = '/tmp'
ARCHIVE_NAME = sys.argv[1]
EXTERNALS_BUILD_PATH = sys.argv[2]

RAPIDJSON_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.tar')[0])
INC_DIR = '{0}/rapidjson/include'.format(EXTERNALS_BUILD_PATH)

# Remove old lib
subprocess.check_call('rm -rf {0}/rapidjson'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(RAPIDJSON_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('tar -xzvf {0} -C {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(RAPIDJSON_TMP_DIR)

# Copy to repository
os.makedirs(INC_DIR)
subprocess.check_call('cp -R include/* {0}'.format(INC_DIR), shell=True)






