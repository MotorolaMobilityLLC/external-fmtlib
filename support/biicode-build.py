#!/usr/bin/env python
# Build the project with Biicode.

import bootstrap, glob, os, shutil
from download import Downloader
from subprocess import check_call

os_name = os.environ['TRAVIS_OS_NAME']
cmake_dir = 'cmake-dir'
cmake_program = 'cmake'
if os_name == 'linux':
  # Install newer version of CMake.
  cmake_program = bootstrap.install_cmake(
    'cmake-3.1.1-Linux-i386.tar.gz', check_installed=False, download_dir=None, install_dir='.')
  cmake_dir = glob.glob('cmake*')[0]
  with Downloader().download('http://www.biicode.com/downloads/latest/ubuntu64') as f:
    check_call(['sudo', 'dpkg', '-i', f])
elif os_name == 'osx':
  with Downloader().download('http://www.biicode.com/downloads/latest/macos') as f:
    check_call(['sudo', 'installer', '-pkg', f, '-target', '/'])

env = os.environ.copy()
env['PATH'] = os.path.abspath(os.path.dirname(cmake_program)) + ':' + env['PATH']
env['CMAKE_ROOT'] = os.path.abspath(cmake_dir)

project_dir = 'biicode_project'
check_call(['bii', 'init', project_dir], env=env)
cppformat_dir = os.path.join(project_dir, 'blocks/vitaut/cppformat')
shutil.copytree('.', cppformat_dir,
                ignore=shutil.ignore_patterns('biicode_project', cmake_dir))
for f in glob.glob('support/biicode/*'):
  shutil.copy(f, cppformat_dir)
check_call(['bii', 'cpp:build'], cwd=project_dir, env=env)
