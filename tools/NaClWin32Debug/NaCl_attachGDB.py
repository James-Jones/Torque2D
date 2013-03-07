import os
import sys

gdbCommands = 'nacl-manifest \"Torque2D.nmf\"'
gdbCommands += '\n'
gdbCommands += 'nacl-irt '
naclIRT = os.getenv("NACL_IRT")
if naclIRT != None:
	gdbCommands += naclIRT
else:
	print "NACL_IRT environment variable not set. Aborting"
	sys.exit(0)
gdbCommands += '\n'
gdbCommands += 'target remote localhost:4014'

outputfile = open('gdbcmd.txt', "w")
outputfile.write(gdbCommands)
outputfile.flush()
outputfile.close()

os.system("x86_64-nacl-gdb -x gdbcmd.txt")
