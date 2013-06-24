j = Job()
j.application=Athena()
j.application.atlas_exetype = 'EXE'

j.inputdata = DQ2Dataset()
j.inputdata.dataset = ['data11_7TeV.00183581.physics_Egamma.merge.NTUP_EGAMMA.f385_m884_p536_tid424367_00']
j.inputdata.names=['NTUP_EGAMMA.424367._000112.root.1']

j.application.option_file = 'runEGAMMA.sh'
j.application.options = "%IN"
j.application.athena_compile = False

j.application.prepare()

j.backend=Panda()
#j.backend.site='ANALY_FZK'
j.backend.site='ANALY_ARC'
j.backend.nobuild=True
j.backend.requirements.enableJEM = True
j.backend.requirements.configJEM = '+debug;WN_trigger=ApproveSelectedTrigger,DiscardAllTrigger,LogDumpTrigger;LogDumpTrigger_resolveIDs=True'

j.outputdata=DQ2OutputDataset()
j.outputdata.outputdata=['info.txt' ]
j.splitter = DQ2JobSplitter()
j.splitter.numsubjobs = 1

j.submit()
