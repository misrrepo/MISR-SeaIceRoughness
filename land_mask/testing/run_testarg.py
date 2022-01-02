#!/usr/bin/env python
from subprocess import call

print("-> hello Ehsan!")

exe_name = "testarg"

# cmd = (' "./%s", "%s", "%s", "%s"' % (exe_name ,'ehsan', 'ali', 'iman'))

# cmd = ("./testarg", "ehsan", "ali", "iman")
cmd = ( "%s %s %s %s" % ("./testarg", "ehsan", "ali", "iman") )

print(cmd)

call(cmd, shell=True)  # cmd= is a string