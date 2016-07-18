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

HTTP_PARSER_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.tar')[0])
BUILD_INSTALL_PREFIX = '{0}/http-parser'.format(EXTERNALS_BUILD_PATH)
LIB_DIR = '{0}/http-parser/lib'.format(EXTERNALS_BUILD_PATH)
INC_DIR = '{0}/http-parser/include'.format(EXTERNALS_BUILD_PATH)
LIB_VERSION = ARCHIVE_NAME.split('.tar')[0].split('-')[-1]

# Remove old lib
subprocess.check_call('rm -rf {0}/http-parser'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(HTTP_PARSER_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('tar -xzvf {0} -C {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(HTTP_PARSER_TMP_DIR)

# Build
subprocess.check_call('make -j{0} package library'.format(multiprocessing.cpu_count()), shell=True)

# Copy to repository
os.makedirs(LIB_DIR)
os.makedirs(INC_DIR)

if platform.platform().startswith('Darwin'):
    subprocess.check_call('cp -R libhttp_parser.*.dylib {0}'.format(LIB_DIR), shell=True)
    subprocess.check_call('ln -s libhttp_parser.{0}.dylib {1}/libhttp_parser.dylib'.format(LIB_VERSION, LIB_DIR), shell=True)
else:
    subprocess.check_call('cp -R libhttp_parser.so* {0}'.format(LIB_DIR), shell=True)
    subprocess.check_call('ln -sT libhttp_parser.so.{0} {1}/libhttp_parser.so'.format(LIB_VERSION, LIB_DIR), shell=True)
    
subprocess.check_call('cp -R *.h {0}'.format(INC_DIR), shell=True)


