/******************************************************************************/
/*                                                                            */
/*                     X r d O u c N a m e 2 N a m e L F C . c c              */
/*                                                                            */
/******************************************************************************/

// This file implements an instance of the XrdOucName2Name abstract class
// which uses the LFC to resolve LFNs to PFNs.  
//
// Initial version written by cgw@hep.uchicago.edu Oct 2010
// Support RUCIO global logical file name by Wei Yang, yangw@slac.stanford.edu June, 2013

const char* XrdOucName2NameLFCCVSID = "$Id: XrdOucName2NameLFC.cc,v 1.21 2011/12/13 16:06:40 sarah Exp $";
const char* version = "$Revision: 1.31 $";

#define LFC_CACHE_TTL 2*3600
#define LFC_CACHE_MAXSIZE 500000

using namespace std;

// C++ headers
//#include <string>
#include <deque>
#include <vector>
#include <fstream>
#include <sstream>

// C headers
#include <errno.h>
#include <glob.h>
#include <pthread.h>

#include <sys/types.h>
#include <dirent.h>

// for gettimefoday
#include <sys/time.h>

// Xrootd headers
#include "XrdOuc/XrdOucEnv.hh"
#include "XrdOuc/XrdOucName2Name.hh"
#include "XrdSys/XrdSysPlatform.hh"

// LFC headers
#include <sys/types.h>
#define NSTYPE_LFC
#include <lfc_api.h>
#include <serrno.h>
extern "C"
{
    int Cthread_init();
}

// My headers
#include "String.hh"
#ifndef STANDALONE   /// Move this into XrdMsgStream
#include "XrdMsgStream.hh"
#else
#define XrdMsgStream ostream
#define XrdSysError ostream
#endif

#include "rucioN2N.hh"

// Hash_map implementation
#ifdef CXX0X
#include <unordered_map>
#else // Use hash_map for now...
#include <ext/hash_map>
using __gnu_cxx::hash_map;
using __gnu_cxx::hash;
// Template specialization to hash String type
namespace __gnu_cxx {
    template<> struct hash<String> {
	size_t operator()( const String& x ) const {
	    return hash<const char*>()(x.c_str());
	}
   };
}
#endif

// Debugging
#include <assert.h>

// Some helpful typedefs/macros
typedef vector<String> List;
#define for_each(i, v) for(List::iterator (i) = (v).begin(); (i) != (v).end(); (i)++)


class XrdOucLFC : public XrdOucName2Name
{
public:
    virtual int lfn2pfn(const char* lfn, char* buff, int blen);
    virtual int lfn2rfn(const char* lfn, char* buff, int blen);
    virtual int pfn2lfn(const char* lfn, char* buff, int blen);

    XrdOucLFC(XrdSysError *erp, const char* parms);
    ~XrdOucLFC();

    // Methods
    int start_lfc_session();
    int parse_parameters(String);
    friend XrdOucName2Name *XrdOucgetName2Name(XrdOucgetName2NameArgs);

private:
    // Params
    String lfc_host;
    String root; // should we perhaps use 'lroot' or 'rroot' here?
    List match_list;
    List nomatch_list;
    List dcache_pool_list;
    bool force_direct; // if set, only read files from dcache pools, no dcap
    List rucioprefix_list;

    // LFC utility functions
    String query_lfc(String lfn);
    String lfn_is_pfn(String lfn);
    List find_matching_lfc_dirs(String lfn);
    List rewrite_lfn(String lfn);

    // dcache utility functions
    String get_pnfsid(String pfn);

    // Cache implementation
    class PfnRecord
    {
    public:
	String pfn;
	time_t timestamp;
	String id;
	bool is_direct;
	PfnRecord(String _pfn, time_t _timestamp, String _id, bool _is_direct) {
	    pfn = _pfn;
	    timestamp = _timestamp;
	    id = _id;
	    is_direct = _is_direct;
	};
	PfnRecord() {};
    };
    typedef hash_map<String, PfnRecord> Cache;
    Cache cache_by_lfn;
    deque<Cache::iterator> cache_by_time;
    int lfc_cache_ttl;
    int lfc_cache_maxsize;
    void insert_cache(const char*, String, time_t, String id="", bool direct=false);

    // Locks
    pthread_mutex_t cache_mutex;
    pthread_mutex_t lfc_mutex;
    void lock_cache() {pthread_mutex_lock(&cache_mutex);}
    void unlock_cache() {pthread_mutex_unlock(&cache_mutex);}
    void lock_lfc() {pthread_mutex_lock(&lfc_mutex);}
    void unlock_lfc() {pthread_mutex_unlock(&lfc_mutex);}
    
    // Misc
    XrdMsgStream *eDest;
    bool session_initialized;
};

XrdOucLFC::XrdOucLFC(XrdSysError* erp, const char* parms)
{
    session_initialized = false;
    force_direct = false;
    lfc_cache_ttl = LFC_CACHE_TTL;
    lfc_cache_maxsize = LFC_CACHE_MAXSIZE;

    pthread_mutex_init(&cache_mutex, NULL);
    pthread_mutex_init(&lfc_mutex, NULL);
    
#ifndef STANDALONE    
    eDest = new XrdMsgStream(erp);
#else
    eDest = erp;
#endif
    *eDest << "XRD-LFC v" << String(version).split(" ")[1] << " starting, parameters: " <<  parms << endl;
}
  
XrdOucLFC::~XrdOucLFC()
{
    pthread_mutex_destroy(&cache_mutex);
    pthread_mutex_destroy(&lfc_mutex);
    if (session_initialized) 
	(void) lfc_endsess();
    cache_by_lfn.clear();
    cache_by_time.clear();
}

int XrdOucLFC::lfn2pfn(const char* lfn, char  *buff, int blen)
{
    Cache::iterator it;
    pair <Cache::iterator, bool> ret;
    int s1, s2;
    
    time_t now;
    String pfn;
    bool cache_hit;
    ostringstream tmp;

    *eDest << "XRD-LFC: lookup " << lfn << endl;

    // Clear expired cache entries
    now = time(NULL);

    cache_hit = false;
    lock_cache();

    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));

    while (!cache_by_time.empty() &&
	   now - (it=cache_by_time.front())->second.timestamp > lfc_cache_ttl) {
	cache_by_lfn.erase(it);
	cache_by_time.pop_front();
    }

    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));

    // Check cache for lfn
    PfnRecord rec;
    if ( (it = cache_by_lfn.find(lfn)) != cache_by_lfn.end()) {
	cache_hit = true;
	rec = it->second;
    }
    unlock_cache();

    if (cache_hit) {
	if (rec.is_direct) {
	    // make sure file has not been migrated off pool
	    // small race cond. still possible, but this helps 
	    if (access(rec.pfn, R_OK)==0) {
		// Copy result to caller's buffer & return
		strncpy(buff, rec.pfn, blen);
		*eDest << "XRD-LFC: cache hit, direct mode, return " << buff << endl;
		return 0;
	    }
	} else if (!force_direct) {
	    strncpy(buff, rec.pfn, blen);
	    *eDest << "XRD-LFC: cache hit, return " << buff << endl;
	    return 0;
	} else {
            pfn = rec.pfn;
	}
    } else {
	if (! strncmp(lfn, "/atlas/rucio", 12)) { // rucio gLFN
           char *sfn = rucio_n2n_glfn(lfn);
           pfn = sfn;
           free(sfn); 
	} else if (! strncmp(lfn, "/atlas/dq2", 10)) { // dq2 gLFN
	    List possibles = rewrite_lfn(lfn);
	    for_each (it, possibles) {
		if (pfn=query_lfc(*it))
		    break;
	    }
	    //	}
	    //#if 0
/*
	    if (!pfn) {
	      for_each (it, possibles) {
		List possibles2 = find_matching_lfc_dirs(*it);
		for_each (it2, possibles2) {
		  if (pfn=query_lfc(*it2))
		    break;
		}
		//if (pfn)
		//    break;
	      }
	    }
*/
	} else // don't do N2N 
            pfn = lfn;
       
	//#endif    
    }
    if (!pfn) {
	*eDest << "XRD-LFC: no valid replica for " << lfn << endl;
	return -ENOENT;
    }

    // See if we have the file in a local dcache pool
    String local_file;
    bool can_access = false;
    String id;
    
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    if (!dcache_pool_list.empty()) {
	if (cache_hit && rec.id)
	    id = rec.id;
	else
	    id = get_pnfsid(pfn);
	if (id) {
	    for_each (dcache_pool, dcache_pool_list) {
		local_file = *dcache_pool + "/" + id;
		if ( can_access = (access(local_file, R_OK)==0) ) {
		    pfn = local_file;
		    break;
		}
	    }
	}
    }
    gettimeofday(&t1, NULL);
    float diff = (t1.tv_sec + t1.tv_usec/1000000.0) - (t0.tv_sec + t0.tv_usec/1000000.0);
    *eDest << "XRD-LFC: timing info: pool check took " << diff << " seconds" << endl;

    // Cache LFC reply & other (pnfs) info
    if (! strncmp(lfn, "/atlas/rucio", 12) || ! strncmp(lfn, "/atlas/dq2", 10)) insert_cache(lfn, pfn, now, id, can_access);

    if (force_direct && !can_access) {
	*eDest << "XRD-LFC: no direct access to " << pfn << " as "  << id << endl;
	return -ENOENT;
    }

    // Copy result to caller's buffer and report success
    strncpy(buff, pfn, blen);

    *eDest << "XRD-LFC: return " << buff << " cache size=" << cache_by_time.size() << endl;
    return 0;
}

void XrdOucLFC::insert_cache(const char* lfn, String pfn, 
			     time_t time, String id, bool is_direct)
{
    pair <Cache::iterator, bool> ret;
    Cache::iterator it;
    bool insertion_done;
    int s1, s2;
    lock_cache();

    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));

    ret = cache_by_lfn.insert(pair<String, PfnRecord>(
				  lfn, PfnRecord(pfn, time, id, is_direct )));
    it = ret.first;
    insertion_done = ret.second;
    if (!insertion_done) { // entry was already in cache_by_lfn, 
	               // so remove previous entry in cache_by_time
	for (deque<Cache::iterator>::iterator it2 = cache_by_time.begin(); 
	     it2 != cache_by_time.end(); ++it2) {
	    if (*it2 == it) {
		cache_by_time.erase(it2); // invalidates all iterators
		break;
	    }
	}
    }

    cache_by_time.push_back(it);
    
    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));

    // Trim cache as needed, while lock held
    while(cache_by_lfn.size() > lfc_cache_maxsize) {
	it=cache_by_time.front();
	cache_by_lfn.erase(it);
	cache_by_time.pop_front();
    }

    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));

    unlock_cache();
}

String XrdOucLFC::lfn_is_pfn(String lfn)
{
    char* pfn=NULL;
    if (!root)
	return NULL;
    pfn = strstr((char*)lfn, root);
    return pfn;
}

String XrdOucLFC::query_lfc(String lfn)
{
    struct lfc_filereplica *rep_entries = NULL;
    char* pfn=NULL;
    int i, status, n_entries;

    char *guid;
    guid = strstr((char*)lfn, "!GUID=");
    // Query LFC
    lock_lfc();
    if (guid == NULL)
        status = lfc_getreplica (lfn, NULL/*guid*/, NULL/*se*/, &n_entries, &rep_entries);
    else
        status = lfc_getreplica (NULL/*lfn*/, (const char*)(guid+6), NULL/*se*/, &n_entries, &rep_entries);
    unlock_lfc();

    if (status) {
	// ERROR
	// TODO recovery possible here, e.g. re-initialize session?
	*eDest << "XRD-LFC " << sstrerror(serrno) << " " << lfn << endl;
	return NULL;
    };
    bool replica_found = false;

    for (i=0; i<n_entries; ++i) {
	pfn = rep_entries[i].sfn;
	// Reply may include empty names
	if (strlen(pfn) == 0) 
	    continue;

	// Check for forbidden substrings
	bool forbidden = false;
	for_each (it, nomatch_list) {
	    if (strstr(pfn, *it)) {
		forbidden = true;
		break;
	    }
	}
	if (forbidden)
	    continue; 

	// Check for required match string, if specified (match is boolean OR)
	bool match_found = match_list.empty();  // empty list is trivial match
	for_each (it, match_list) {
	    if (strstr(pfn, *it)) {
		match_found = true;
		break;
	    }
	}
	if (!match_found)
	    continue;

	// Scan for local filesystem mount point, if specified
	if (root && ! (pfn=strstr(pfn, root))) {
		*eDest << "XRD-LFC: WARNING " << root << " not found" << endl;
		continue;
	} 
	replica_found = true;
	break;
    }
    String ret;
    if (replica_found) 
	ret = String(pfn);
    if (rep_entries)
	free(rep_entries);
    return ret;
}

// Compensate for varying conventions for LFC path
List XrdOucLFC::rewrite_lfn(String lfn)
{
    List ret;
    ret.push_back(lfn);                           // 0) Unmodified LFC path
    if (!lfn.startswith("/grid"))    
	ret.push_back("/grid" + lfn);             // 1) Try adding /grid prefix
    List components = lfn.split("/");
    if (components.size() > 2 && components[0] == "atlas") {
	if (components[1] != "dq2") {             // 2) /atlas/!dq2 -> /grid/atlas/dq2
	    ret.push_back("/grid/atlas/dq2" + 
			  join(List(components.begin()+1, components.end()), "/"));
	}//else if (components[1] != "pathena") { // 3) /atlas/!pathena -> /grid/atlas/pathena
	                                          // 4) etc..
        if (components[2] == "user") {
            ret.push_back("/grid/atlas/users/pathena" +
                        join(List(components.begin()+2, components.end()), "/"));
        }
    }
    return ret;
}

// XXX this might get expensive!
List XrdOucLFC::find_matching_lfc_dirs(String lfn)
{
    List ret;

    //    *eDest << "lfn: " << lfn << endl;

    // #if 0
    List components = lfn.split("/");

    //    *eDest << "size=" <<  components.size() << endl;
    if (components.size() < 3)
	return ret;
    
    List::iterator it=components.end();
    it--;
    String filename=*it;
    //    *eDest << "filename:" << filename << endl; 
   
    it--;
    String dirname = *it;
    //    *eDest << "dirname:  " << dirname << endl;

    String parent_path = join(List(components.begin(), it), "/");
    //    *eDest << "parent_path:  " << parent_path << endl;
 

    // trying in case the name space is using the old format.
    // current: /atlas/dq2/data11_7TeV/NTUP_TOP/f369_m812_p530_p577/data11_7TeV.00180309.physics_Egamma.merge.NTUP_TOP.f369_m812_p530_p577_tid367204_00/	
    // old: /atlas/dq2/data11_7TeV/NTUP_TOP/data11_7TeV.00180309.physics_Egamma.merge.NTUP_TOP.f369_m812_p530_p577_tid367204_00_sub021131151/
    it--;
    String old_path = join(List(components.begin(), it), "/");
    
    
   
    lock_lfc();
    lfc_DIR *lfcdir=lfc_opendir(parent_path);
    struct dirent *dirent=lfc_readdir(lfcdir);
    
    while (dirent!=0) {
      String cName=dirent->d_name;
      if (cName.substr(0,dirname.size())==dirname) {
	//	*eDest << "dname: " <<  parent_path + "/" + cName << endl;
	ret.push_back(parent_path + "/" + cName + "/" + filename );
      }
	  
      dirent=lfc_readdir(lfcdir);
    }
    lfc_closedir(lfcdir);
    
    //try the old path
    // *eDest << "old_path: " << old_path << endl;
    lfcdir=lfc_opendir(old_path);
    dirent=lfc_readdir(lfcdir);
    while (dirent!=0) {
      String cName=dirent->d_name;
      if (cName.substr(0,dirname.size())==dirname) {
	ret.push_back(old_path + "/" + cName + "/" + filename);
	//	*eDest << "old_path: " << old_path + "/" + cName + "/" + filename << endl;
      }
      dirent=lfc_readdir(lfcdir);
    }
    lfc_closedir(lfcdir);
    
    unlock_lfc();
    //#endif
    
    return ret;
}
  
XrdOucName2Name *XrdOucgetName2Name(XrdOucgetName2NameArgs)
{
    static XrdOucLFC *inst = NULL;

    if (inst) { 
	return (XrdOucName2Name *)inst;
    }

    inst = new XrdOucLFC(eDest, parms);
    if (!inst) {
	return NULL;
    }
    if (inst->parse_parameters(parms)) {
	delete inst;
	return NULL;
    }

    // Initialize Cthread library - should be called before any LFC-API function 
    serrno = 0;
    if (Cthread_init()) {
	*inst->eDest << "XRD-LFC: Cthread_init error: " << sstrerror(serrno) << endl;
	return NULL;
    }

    if (inst->start_lfc_session()){
	delete inst;
	return NULL;
    }

    if (inst->rucioprefix_list.size() != 0) rucio_n2n_init(inst->rucioprefix_list);

    return (XrdOucName2Name *)inst;
}

static List glob(String pat)
{
    List ret;
    glob_t globbuf;
    
    if (glob(pat, GLOB_ERR, NULL, &globbuf) == 0)
	for (int i=0; i<globbuf.gl_pathc; ++i)
	    ret.push_back(globbuf.gl_pathv[i]);

    globfree(&globbuf);
    return ret;
}


int XrdOucLFC::parse_parameters(String param_str)
{
    List tokens = param_str.split(" \t");
    const char *siteprefixstr = NULL;
    String sitename = "";
    char *xrdsite = NULL;

    if (XrdOucEnv::Import("XRDSITE", xrdsite)) sitename = xrdsite;

    for_each (it, tokens) {
	if (*it == "force_direct") {
	    force_direct = true;
	    continue;
	}
	List keyval = it->split("=");
	if (keyval.size() != 2) {
	    // ERROR
	    *eDest << "XRD-LFC: Invalid parameter " << *it << endl;
	    return 1;
	}
	String key = keyval[0];
	String val = keyval[1];
//	if (key == "lfc_host") {
//	    lfc_host = val;
//	} else if (key == "root") {
	if (key == "root") {
	    root = val;
	} else if (key == "match") {
	    match_list = val.split(",");
	} else if (key == "nomatch") {
	    nomatch_list = val.split(",");
	} else if (key == "cache_ttl") {
	    if (! (stringstream(val) >> lfc_cache_ttl) ) {
		*eDest << "XRD-LFC: Invalid numeric parameter " << val << endl;
		return 2;
	    }
	} else if (key == "cache_maxsize") {
	    if (! (stringstream(val) >> lfc_cache_maxsize) ) {
		*eDest << "XRD-LFC: Invalid numeric parameter " << val << endl;
		return 2;
	    }
	} else if (key.startswith("dcache_pool")) {
	    List l = val.split(",");
	    for_each (p, l) {
		List g = glob(*p);
		if (g.empty()) {
		    *eDest << "XRD-LFC: Error, no valid match for " << *p << endl;
		} else {
		    dcache_pool_list.insert(dcache_pool_list.end(), g.begin(), g.end());
		}
	    }
        } else if (key == "rucioprefix") {
            rucioprefix_list = val.split(",");
            siteprefixstr = strdup(val.c_str());
        } else if (key == "sitename") {
            sitename = val;
	} else {
	    // ERROR
	    *eDest << "XRD-LFC: Invalid parameter " << key << endl;
	    return 3;
	}
    }
    if (siteprefixstr != NULL) { // paramenter rucioprefix is provided
        *eDest << "XRD-LFC: Customer RUCIO prefix list " << siteprefixstr << endl;
    } else if (sitename != "") { // use sitename to get site prefix from AGIS
        *eDest << "XRD-LFC: Getting site " << sitename << " prefix list from AGIS ... " << endl;
        siteprefixstr = rucio_get_siteprefix(AGISurl, sitename.c_str());
        if (siteprefixstr == NULL)
            *eDest << "XRD-LFC: RUCIO prefix = none, RUCIO N2N is disabled" << endl;
        else {
            *eDest << "XRD-LFC: prefix list: " << siteprefixstr << endl;
            String tmp = siteprefixstr;
            rucioprefix_list = tmp.split(",");
        }
    }
    else 
        *eDest << "XRD-LFC: RUCIO prefix = none, RUCIO N2N is disabled" << endl;
   
    if (siteprefixstr != NULL) free((void*)siteprefixstr);
    // all done
    return 0;
}
    
int XrdOucLFC::start_lfc_session()
{
    char comment[80];
    int status;

    if (!lfc_host) {
	if (! (lfc_host=getenv("LFC_HOST"))) {
	    session_initialized = false;
	    *eDest << "XRD-LFC: LFC_HOST not set" << endl;
	    return -EINVAL;
	}
    }

    strcpy(comment, "XRD-LFC@");
    gethostname(comment+8, 72);

    if (status=lfc_startsess(lfc_host, comment)) {
	session_initialized = false;
	*eDest << "XRD-LFC: Unable to open lfc session on " << lfc_host << ": " << sstrerror(serrno) << endl;
	return status;
    }
    session_initialized = true;
    return 0;
}

String XrdOucLFC::get_pnfsid(String pfn)
{
    List components = pfn.split("/");
    String fname = components.back();
    components.pop_back();
    String path = join(components, "/");
    path += "/.(id)(" + fname + ")";

    String pnfsid = "";

    ifstream ifs;
    
    ifs.exceptions(ifstream::failbit | ifstream::badbit);
    
    struct timeval t0, t1;;
    gettimeofday(&t0, NULL);

    try {
	ifs.open(path);
	ifs >> pnfsid;
    } catch (...) {
	char err[64];
	strerror_r(errno, err, 64);
	*eDest << "XRD-LFC: Error reading " << path << " " << err << endl;
    }
    gettimeofday(&t1, NULL);
    float diff = (t1.tv_sec + t1.tv_usec/1000000.0) - (t0.tv_sec + t0.tv_usec/1000000.0);
    *eDest << "XRD-LFC: timing info: pnfsid lookup took " << diff << " seconds" << endl;
    return pnfsid;
}

int XrdOucLFC::lfn2rfn(const char* lfn, char  *buff, int blen)
{
    *eDest << "XRD-LFC: lfn2rfn not implemented" << endl;
    return -EOPNOTSUPP;
}

int XrdOucLFC::pfn2lfn(const char* pfn, char  *buff, int blen)
{
    *eDest << "XRD-LFC: pfn2lfn not implemented" << endl;
    return -EOPNOTSUPP;
}

#ifdef STANDALONE
#include <iostream>
int main(int argc, char **argv)
{
    XrdSysError *erp = &cout;
    const char  *config = argv[0];
    String parms, line;
    char pfn[512];

    for (int i=1; i<argc; ++i) {
	if (i>1)
	    parms += " ";
	parms +=  argv[i];
    }

    XrdOucName2Name *lfc  = (XrdOucLFC*)XrdOucgetName2Name(erp, config,
							   parms, NULL, NULL);
    
    if (!lfc) {
	cerr << "Cannot initialize xrd-lfc" << endl;
	return 1;
    }
    while (cin >> line) {
        if (!lfc->lfn2pfn(line.c_str(), pfn, 512)) {
            cout << "SUCCESS " << pfn << endl;
        } else {
            cout << "FAIL" << endl;
        }
    }
}
#endif
