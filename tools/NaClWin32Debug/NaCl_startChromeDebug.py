import os
import sys
import argparse
import subprocess

parser = argparse.ArgumentParser(description='File wrapper.')
parser.add_argument("-d", "--chrome-data-dir", dest="chromeDataDir", required=True, help="REQUIRED. chrome_data directory")
options = parser.parse_args()

chromePath = os.getenv("CHROME_PATH")
if chromePath == None:
	print "CHROME_PATH environment variable not set. Aborting"
	sys.exit(0)

command = chromePath
commandARGSBasic = " --enable-nacl-debug --no-sandbox localhost:5103/Torque2DNaCl.html --enable-nacl"
commandARGS = " --enable-nacl-debug --no-sandbox localhost:5103/Torque2DNaCl.html --user-data-dir=\"" + options.chromeDataDir + "\" --enable-nacl"
#--user-data-dir=\"" + options.chromeDataDir + "\""
commandARGS = commandARGS.replace('\\', '/')

subprocess.call(command+commandARGS)
#subprocess.call([command, commandARGSBasic], shell=False)
#os.system(command)
