Call NaCl_startChromeDebug before NaCL_attachGDB. Chrome will pause until gdb connects.
Call NaCL_attachGDB from the directory containing the manifest and nexe.

Environment variables used by NaCl_attachGDB/NaCl_gdbcmd:

  NACL_IRT - example "C:/Program Files (x86)/Google/Chrome/Application/25.0.1364.97/nacl_irt_x86_64.nexe"

Environment variables used by NaCl_startChromeDebug:

  CHROME_PATH - exampe "C:/Program Files (x86)/Google/Chrome/Application/25.0.1364.97/nacl_irt_x86_64.nexe"

-d/--chrome-data-dir argument to NaCl_startChromeDebug is the full-path to chrome_data directory
  example - C:\dev\GitHub\Torque2D\engine\compilers\NaCl VisualStudio 2010\chrome_data