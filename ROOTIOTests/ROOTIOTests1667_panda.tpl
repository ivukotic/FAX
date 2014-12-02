Job (
 name = '' ,
 outputsandbox = [] ,
 info = JobInfo (
    ) ,
 inputdata = DQ2Dataset (
    match_ce_all = False ,
    datatype = '' ,
    failover = False ,
    accessprotocol = '' ,
    guids = [] ,
    check_md5sum = False ,
    exclude_names = [] ,
    dataset = ####DATASET#### ,
    min_num_files = 0 ,
    tagdataset = [] ,
    names = ["group.test.hc.NTUP_SMWZ.root"] ,
    number_of_files = 1 ,
    use_aodesd_backnav = False ,
    type = '####INPUTTYPE####'
    ) ,
 merger = None ,
 inputsandbox = [ ] ,
 application = Athena (
    atlas_dbrelease = '' ,
    atlas_project = '' ,
    atlas_release = '17.6.0' ,
    collect_stats = True ,
    atlas_environment = [] ,
    exclude_package = [] ,
    atlas_production = '' ,
    options = '%IN' ,
    atlas_use_AIDA = False ,
    atlas_cmtconfig = 'x86_64-slc5-gcc43-opt',
    max_events = -999 ,
    exclude_from_user_area = [] ,
    atlas_run_dir = './' ,
    atlas_exetype = 'EXE' ,
    athena_compile = False ,
    trf_parameter = {} ,
    atlas_run_config = {'input': {'noInput': True}, 'other': {}, 'output': {'alloutputs': []}} ,
    option_file = [ File (
		name = '####JOBOPTIONS####' ,       
		subdir = '.'
       ) , ] ,
    group_area = File (
    name = '' ,
    subdir = '.'
    ) ,
    user_area = File (
       name = '####USERAREA####' ,
       subdir = '.'
    ) 
    ) ,
 outputdata = DQ2OutputDataset (
    datasetname = '####OUTPUTDATASETNAME####',  
    outputdata = ['info.txt'] ,
    local_location = '' ,
    location = ''
   ) ,
 splitter = DQ2JobSplitter (
    update_siteindex = False ,
    numsubjobs = 1 ,
    use_lfc = False ,
    use_blacklist = False ,
    numfiles = 1
    ) ,
 backend = Panda (
    nobuild = True ,
    site = ####SITES#### ,
    requirements = PandaRequirements (
	enableJEM = False ,
       	configJEM = '+debug'
	)
    )  
 ) 
