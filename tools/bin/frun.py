#!/usr/bin/env python

import sys
import os

try:
    import dq2.clientapi.cli.cliutil
    from dq2.common.cli.DQDashboardTool import DQDashboardTool
    from dq2.clientapi.cli.cliutil import getDQ2
    from dq2.filecatalog.lfc.lfcconventions import to_native_lfn
except ImportError:
    print "Environment not set [error importing DQ2 dependencies]!"
    sys.exit(1)


class LFNs (DQDashboardTool):
    usage=''
    version=''
    description=''
    toolOptions = [ dq2.clientapi.cli.cliutil.opt_dataset_version  ]

    def __init__ (self, idsn):
        DQDashboardTool.__init__(self)
        self.dsn=idsn
 
    def execute(self):
        dq = getDQ2(self)
        ret = dq.listFilesInDataset(self.dsn, version=self.options.version)

        try:
            entry = ret[0]
        except IndexError, e:
            sys.exit(0)
        
        roots=[]
        
        redir='glrd.usatlas.org'
        if os.environ.get("XRDREDIRECTOR") != None:
            redir=os.environ.get("XRDREDIRECTOR")
 
        for guid in entry:
            gfn='root://'+redir+'/' + to_native_lfn(self.dsn, entry[guid]['lfn']).replace("/grid", "")
            if gfn.count('.root')>0:
                roots.append(gfn)
        print roots
        with open('pfn.tmp','w') as f:
            f.write(",".join(roots))
        f.close()




iDS=None

for clp in sys.argv:
    print clp	
    clpw = clp.split('=')
    if clpw[0]=='--inDS':
		iDS=clpw[1]

allARGS=['prun']
for i,clp in enumerate(sys.argv):
    clpf=clp.split('=')[0]
    if i<1 or clpf=='--inDS' or clpf=='--pfnList': continue
    allARGS.append(clp)

sys.argv=[]

if iDS is None:
    print 'This is FRUN meaning you have to give it inDS'
    sys.exit(0)
else:
    LFNs(iDS).execute()
    

if os.path.exists('pfn.tmp'):
    import subprocess
    allARGS.append('--pfnList=pfn.tmp')
    print 'submitting prun job:',' '.join(allARGS)
    p = subprocess.Popen(allARGS, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    print out
    print err
