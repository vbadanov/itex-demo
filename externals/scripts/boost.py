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

TEMPLATE_BUILD_STR = './bjam toolset={0} address-model=64 --build-type=minimal --layout=system link=shared variant={1} -j{2} --without-python stage'
    
LIB_DIR = '{0}/boost/lib'.format(EXTERNALS_BUILD_PATH)
BOOST_TMP_DIR = '{0}/{1}'.format(TMP_DIR, ARCHIVE_NAME.split('.tar')[0])

# Remove old lib
subprocess.check_call('rm -rf {0}/boost'.format(EXTERNALS_BUILD_PATH), shell=True)
subprocess.check_call('rm -rf {0}'.format(BOOST_TMP_DIR), shell=True)

# Unpack archive to temp dir
subprocess.check_call('tar -xjvf {0} -C {1}'.format(ARCHIVE_NAME, TMP_DIR), shell=True)
os.chdir(BOOST_TMP_DIR)

TOOLSET = 'gcc'
if platform.platform().startswith('Darwin'):
    TOOLSET = 'darwin'
    

# Bootsrap boost build system
subprocess.check_call('./bootstrap.sh', shell=True)

def build_and_copy(variant):
    # Create dirs in repository
    dst_dir = '{0}/{1}'.format(LIB_DIR, variant) if variant != "release_only" else LIB_DIR
    os.makedirs(dst_dir)
    
    # Build boost libs and copy to repo
    subprocess.check_call('./bjam --clean', shell=True)
    subprocess.check_call(TEMPLATE_BUILD_STR.format(TOOLSET, "release" if variant == "release_only" else variant, multiprocessing.cpu_count()), shell=True)
    subprocess.check_call('cp -R ./stage/lib/* {0}'.format(dst_dir), shell=True)


# Build release only
build_and_copy('release_only')

# Copy includes
dst_dir = '{0}/boost/include/boost'.format(EXTERNALS_BUILD_PATH)
os.makedirs(dst_dir)
subprocess.check_call('cp -R ./boost/* {0}'.format(dst_dir), shell=True)


