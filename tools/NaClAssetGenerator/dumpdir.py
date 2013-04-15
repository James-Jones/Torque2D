import os
import shutil
import argparse

parser = argparse.ArgumentParser(description='Package Torque2D NaCL assets.')
parser.add_argument("-s", "--srcdir", dest="srcdir", required=False, help="Torque2D root directory")
parser.add_argument("-d", "--dstdir", dest="dstdir", required=False, help="Packaged asset destination directory")
options = parser.parse_args()

#print os.listdir('../../modules')

#In order to support Platform::dumpDirectories and Platform::dumpPath for NaCl platform
#we create a file in each directory which lists the directories and another which lists files it contains.
#A file containing all directories will also be created.
#There is a NaCl C/C++ API (PP_DirectoryEntry_Dev) but it is still under development.
#Using javascript to generate the data risks being slow and awkward (js<->NaCl communication, plus file io is asynchronous.)
#The method used here means that run-time generated files/directories will not be shown.

def dumpAllDirs(writeRoot):
	naclfulldirs = open( os.path.join(writeRoot, 'alldirs.txt'), "a")
	for root, dirs, files in os.walk('../../modules'):
		#for name in files:
		#	print os.path.join(root, name)
		for d in dirs:
			naclfulldirs.write(os.path.join(root.replace('../../', '', 1), d+'\n'))
	naclfulldirs.close()

def writeRootFiles( rootReadDir, rootWriteDir):
	dirContents = os.listdir(rootReadDir)
	
	if not os.path.exists(rootWriteDir):
		os.makedirs(rootWriteDir)

	for entry in dirContents:
		if os.path.isdir( os.path.abspath(os.path.join(rootReadDir, entry)) ):

			writeRootFiles(os.path.join(rootReadDir, entry), os.path.join(rootWriteDir, entry))
			nacldirs = open( os.path.join(rootWriteDir, 'dirs.txt'), "a")
			nacldirs.write(entry+'\n')
			nacldirs.close()
		else:
			naclfiles = open(os.path.join(rootWriteDir, 'files.txt'),  "a")
			naclfiles.write(entry+'\n')
			naclfiles.close()
			
if __name__ == '__main__':

	srcdir = '../../'
	if(options.srcdir):
		srcdir = os.path.abspath(options.srcdir)
	dstdir = ''
	if(options.dstdir):
		dstdir = os.path.abspath(options.dstdir)
		
	shutil.rmtree(srcdir + '/dirdump/', ignore_errors=True)
	writeRootFiles(srcdir, dstdir+'/dirdump/')

