import os
import zipfile
import time
import datetime
import argparse

parser = argparse.ArgumentParser(description='Package Torque2D NaCL assets.')
parser.add_argument("-s", "--srcdir", dest="srcdir", required=False, help="Torque2D root directory")
parser.add_argument("-d", "--dstdir", dest="dstdir", required=False, help="Packaged asset destination directory")
options = parser.parse_args()

def zipdir(path, prefix, zip):
	for root, dirs, files in os.walk(path):
		for file in files:
                        fullPath = os.path.join(root, file)
			zip.write(fullPath, fullPath.replace(prefix, '', 1))

def writeDateTimeStamp(destDir):
	timeStamp = datetime.datetime.utcnow()
	timeFormat = '%b %d %Y %H:%M:%S'
	stringTime = timeStamp.strftime(timeFormat)

	outputfile = open(destDir+'Torque2DAssetStamp.txt', "w")
	outputfile.write(stringTime)
	outputfile.flush()
	outputfile.close()

if __name__ == '__main__':

	srcdir = '../../'
	if(options.srcdir):
		srcdir = options.srcdir
	dstdir = ''
	if(options.dstdir):
		dstdir = options.dstdir

	zip = zipfile.ZipFile(dstdir+'Torque2DNaClAssets.zip', 'w', zipfile.ZIP_DEFLATED)
	zip.write(srcdir+'main.cs', 'main.cs')
	zipdir(srcdir+'modules/', srcdir, zip)
	zip.close()
	writeDateTimeStamp(dstdir)
