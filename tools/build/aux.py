#!/usr/bin/python

import os
import stat
import shutil
from SCons.Tool import install

def InstallAndPreserveSymlinks(dest, source, env):
    """Install a source file or directory into a destination by copying,
    (including copying permission/mode bits) and preserving symlinks."""

    if os.path.isdir(source):
        if os.path.exists(dest):
            if not os.path.isdir(dest):
                raise SCons.Errors.UserError("cannot overwrite non-directory `%s' with a directory `%s'" % (str(dest), str(source)))
        else:
            parent = os.path.split(dest)[0]
            if not os.path.exists(parent):
                os.makedirs(parent)
        install.scons_copytree(source, dest, symlinks=True)
    else:
        if os.path.islink(source):
            if not os.path.exists(dest):
                linkto = os.readlink(source)
                os.symlink(linkto, dest)
        else:
            shutil.copy2(source, dest)
            st = os.stat(source)
            os.chmod(dest, stat.S_IMODE(st[stat.ST_MODE]) | stat.S_IWRITE)

    return 0

