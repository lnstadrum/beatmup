#
#    Beatmup image and signal processing library
#    Copyright (C) 2020, lnstadrum
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
from setuptools import setup, Command
from shutil import copyfile, move, rmtree
from glob import glob
import platform, os

lib_file = None
rootdir = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)

# returns location of the binary in build folder
def get_binary_location():
    path = os.path.join(rootdir, 'build')
    if platform.system() == 'Windows':
        return os.path.join(path, 'Release', '*.pyd')
    if platform.system() == 'Linux':
        return os.path.join(path, 'beatmup.*.so')
    raise RuntimeError('OS not supported: ' + platform.system())


# define cleanup command
class CleanCommand(Command):
    description = "custom clean command that forcefully removes dist/build directories"
    user_options = []
    def initialize_options(self):
        pass
    def finalize_options(self):
        pass
    def run(self):
        global lib_file
        os.remove(lib_file)
        rmtree('beatmup.egg-info')
        rmtree('build')


# copy the binary to the current working dir
for file in glob(get_binary_location()):
    lib_file = os.path.basename(file)
    copyfile(file, lib_file)


# subclass bdist_wheel command
from wheel.bdist_wheel import bdist_wheel as BdistWheelCommandBase
class BdistWheelCommand(BdistWheelCommandBase):
    def finalize_options(self):
        BdistWheelCommandBase.finalize_options(self)
        self.root_is_pure = False   # this makes the wheel platform-dependent


# grab readme
with open(os.path.join(rootdir, 'README.md'), 'r') as file:
    long_description = file.read()


# build package
setup(
    name='beatmup',
    version='2.0',
    author='lnstadrum',
    author_email='',
    description='Image and signal asynchronous processing library. A lot of fun',
    long_description=long_description,
    long_description_content_type='text/markdown',
    platforms=platform.system(),
    url='https://github.com/lnstadrum/beatmup',
    packages=[''],
    package_data={'': [
        lib_file,
        'beatmup_keras.py'
    ]},
    include_package_data=True,
    cmdclass={
        'bdist_wheel': BdistWheelCommand,
        'clean': CleanCommand
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "Topic :: Scientific/Engineering :: Image Processing",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)"
    ]
)
