#!/usr/bin/python3

import sys
import os
import subprocess
import multiprocessing

TMP_DIR = '/tmp'
ARCHIVE_NAME = sys.argv[1]
EXTERNALS_BUILD_PATH = sys.argv[2]

FLATBUFFERS_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.tar')[0])
BUILD_INSTALL_PREFIX = '{0}/flatbuffers'.format(EXTERNALS_BUILD_PATH)
LIB_DIR = '{0}/lib'.format(BUILD_INSTALL_PREFIX)


# Remove old lib
subprocess.check_call('rm -rf {0}/flatbuffers'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(FLATBUFFERS_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('tar -xzvf {0} -C {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(FLATBUFFERS_TMP_DIR)

# Build C++ library
subprocess.check_call('cmake -DCMAKE_INSTALL_PREFIX={0} .'.format(BUILD_INSTALL_PREFIX), shell=True)
subprocess.check_call('make -j{0}'.format(multiprocessing.cpu_count()), shell=True)

# Copy to repository
subprocess.check_call('make -j{0} install'.format(multiprocessing.cpu_count()), shell=True)

#Build C# .NET library
subprocess.check_call('xbuild /p:Configuration="Release" /p:Platform=AnyCPU /tv:4.0 /p:TargetFrameworkVersion=v4.5 ./net/FlatBuffers/FlatBuffers.csproj', shell=True)
#os.makedirs(LIB_DIR)
subprocess.check_call('cp -R ./net/FlatBuffers/bin/Release/* {0}'.format(LIB_DIR), shell=True)

