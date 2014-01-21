/******************************************************************************/ /* */ /* X r d O u c N a m e 2 N a m e L F C . c c */ /* */
/******************************************************************************/

// This file implements an instance of the XrdOucName2Name abstract class
// which uses RUCIO to resolve LFNs to PFNs.  
//
// Initial version written by cgw@hep.uchicago.edu Oct 2010
// Support RUCIO global logical file name by Wei Yang, yangw@slac.stanford.edu June, 2013

const char* XrdOucName2NameLFCCVSID = "$Id: XrdOucName2NameLFC.cc,v 1.21 2014/01/20 16:06:40 sarah Exp $";
const char* version = "$Revision: 2.02 $";

#define LFC_CACHE_TTL 2*3600
#define LFC_CACHE_MAXSIZE 500000

#include <iostream>
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


class XrdOucLFC : public XrdOucName2Name {
    public:
        virtual int lfn2pfn(const char* lfn, char* buff, int blen);
        virtual int lfn2rfn(const char* lfn, char* buff, int blen);
        virtual int pfn2lfn(const char* lfn, char* buff, int blen);
    
        XrdOucLFC(XrdSysError *erp, const char* parms);
        ~XrdOucLFC();
    
        // Methods
        int parse_parameters(String);
        friend XrdOucName2Name *XrdOucgetName2Name(XrdOucgetName2NameArgs);
    
    private:
        // Params
        String root; // should we perhaps use 'lroot' or 'rroot' here?
        List match_list;
        List nomatch_list;
        List dcache_pool_list;
        bool force_direct; // if set, only read files from dcache pools, no dcap
        List rucioprefix_list;
        List siteprefixreplace; // this is a private feature to only needed at SLAC. Has nothing to do with rucio.
    
        // LFC utility functions
        String lfn_is_pfn(String lfn);
    
        // dcache utility functions
        // String get_pnfsid(String pfn);
    
        // Cache implementation
        class PfnRecord {
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

XrdOucLFC::XrdOucLFC(XrdSysError* erp, const char* parms){
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
  
XrdOucLFC::~XrdOucLFC(){
    pthread_mutex_destroy(&cache_mutex);
    pthread_mutex_destroy(&lfc_mutex);
    cache_by_lfn.clear();
    cache_by_time.clear();
}

int XrdOucLFC::lfn2pfn(const char* lfn, char  *buff, int blen) {
    
    if (! strncmp(lfn, "/atlas/dq2", 10)){
        *eDest << "XRD-N2N: old gLFN. will not translate: " << lfn << endl;
        return -ENOENT;
    }
    
    *eDest << "XRD-N2N: looking up: " << lfn << endl;
    
    Cache::iterator it;

    // ************* Clear expired cache entries ******************** //
    time_t now = time(NULL);
    lock_cache();

    int s1, s2;
    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));
    while (!cache_by_time.empty() && now - (it=cache_by_time.front())->second.timestamp > lfc_cache_ttl) {
	    cache_by_lfn.erase(it);
	    cache_by_time.pop_front();
    }
    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));



    // ************** Check cache for lfn  ************************** //
    bool cache_hit=false;
    PfnRecord rec;
    if ( (it = cache_by_lfn.find(lfn)) != cache_by_lfn.end()) {
        cache_hit = true;
        rec = it->second;
    }
    unlock_cache();


    String pfn;

    if (cache_hit) {
        std::cout<<"cache hit: checking what would happend next. "<<rec.pfn<<std::endl;
    	if (rec.is_direct) { // make sure file has not been migrated off pool
    	    // small race cond. still possible, but this helps 
    	    if (access(rec.pfn, R_OK)==0) {  // Copy result to caller's buffer & return
        		strncpy(buff, rec.pfn, blen);
                *eDest << "XRD-N2N: cache hit, direct mode, would return " << buff << endl;
                // return 0;  - commented for test
    	    }
	    } else if (!force_direct) {
	        strncpy(buff, rec.pfn, blen);
            *eDest << "XRD-N2N: cache hit, return, would return " << buff << endl;
            // return 0; - commented for test
	    } else {
            pfn = rec.pfn; 
	    }
        
    }
    // else { -commented for test
    
    if (true){
        if (! strncmp(lfn, "/atlas/rucio", 12)) { // rucio gLFN
           char *sfn = rucio_n2n_glfn(lfn);
           pfn = sfn;
           free(sfn); 
	    } else // don't do N2N 
            pfn = lfn;
        }
        
        if (!pfn) {
            *eDest << "XRD-N2N: no valid replica for " << lfn << endl;
            return -ENOENT;
        }
        
        if (siteprefixreplace.size() == 2 && pfn.find(siteprefixreplace[0], 0) == 0) // Like SLAC only
            pfn.replace(0, siteprefixreplace[0].size(), siteprefixreplace[1]);

        // struct timeval t0, t1; - unimportant
        // gettimeofday(&t0, NULL);
        
        
        // See if we have the file in a local dcache pool
        String local_file;
        bool can_access = false;
        String id="";
    


    //     if (!dcache_pool_list.empty()) {  - what kind of BS is this ?!
    //         if (cache_hit && rec.id)
    //             id = rec.id;
    //         else
    //             id = get_pnfsid(pfn);
    //         
    //         if (id) {
    //             for_each (dcache_pool, dcache_pool_list) {
    //             local_file = *dcache_pool + "/" + id;
    //             can_access = access(local_file, R_OK);
    //             if ( ! can_access ) {
    //                 pfn = local_file;
    //                 break;
    //             }
    //         }
    //         
    //         }
    // }
    
    // gettimeofday(&t1, NULL); - unimportant
    // float diff = (t1.tv_sec + t1.tv_usec/1000000.0) - (t0.tv_sec + t0.tv_usec/1000000.0);
    // *eDest << "XRD-N2N: timing info: pool check took " << diff << " seconds" << endl;

    // Cache LFC reply & other (pnfs) info
    if (! strncmp(lfn, "/atlas/rucio", 12) ) insert_cache(lfn, pfn, now, id, can_access);

    // if (force_direct && !can_access) { - what is this checking for ?! 
    //     *eDest << "XRD-N2N: no direct access to " << pfn << " as "  << id << endl;
    //     return -ENOENT;
    // }

    // Copy result to caller's buffer and report success
    strncpy(buff, pfn, blen);

    *eDest << "XRD-N2N: return " << buff << " cache size=" << cache_by_time.size() << endl;
    return 0;
}



void XrdOucLFC::insert_cache(const char* lfn, String pfn, time_t time, String id, bool is_direct) {
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

String XrdOucLFC::lfn_is_pfn(String lfn){
    char* pfn=NULL;
    if (!root)
	return NULL;
    pfn = strstr((char*)lfn, root);
    return pfn;
}

  
XrdOucName2Name *XrdOucgetName2Name(XrdOucgetName2NameArgs){
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

    if (inst->rucioprefix_list.size() != 0) rucio_n2n_init(inst->rucioprefix_list);

    return (XrdOucName2Name *)inst;
}

static List glob(String pat){
    List ret;
    glob_t globbuf;
    
    if (glob(pat, GLOB_ERR, NULL, &globbuf) == 0)
	for (int i=0; i<globbuf.gl_pathc; ++i)
	    ret.push_back(globbuf.gl_pathv[i]);

    globfree(&globbuf);
    return ret;
}


int XrdOucLFC::parse_parameters(String param_str) {
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
	        *eDest << "XRD-N2N: Invalid parameter " << *it << endl;
	        return 1;
	    }
        
	    String key = keyval[0];
	    String val = keyval[1];
	    if (key == "root") {
	        root = val;
	    } else if (key == "match") {
	        match_list = val.split(",");
	    } else if (key == "nomatch") {
	        nomatch_list = val.split(",");
	    } else if (key == "cache_ttl") {
	        if (! (stringstream(val) >> lfc_cache_ttl) ) {
	    	    *eDest << "XRD-N2N: Invalid numeric parameter " << val << endl;
	    	    return 2;
	        }
	    } else if (key == "cache_maxsize") {
	        if (! (stringstream(val) >> lfc_cache_maxsize) ) {
	    	    *eDest << "XRD-N2N: Invalid numeric parameter " << val << endl;
	    	    return 2;
	        }
	    } else if (key.startswith("dcache_pool")) {
	        List l = val.split(",");
	        for_each (p, l) {
		        List g = glob(*p);
		        if (g.empty()) {
		            *eDest << "XRD-N2N: Error, no valid match for " << *p << endl;
		        } else {
		            dcache_pool_list.insert(dcache_pool_list.end(), g.begin(), g.end());
		        }
	        }
        } else if (key == "rucioprefix") {
            rucioprefix_list = val.split(",");
            siteprefixstr = strdup(val.c_str());
        } else if (key == "sitename") {
            sitename = val;
        } else if (key == "pssorigin") {
            XrdOucEnv::Export("XRDXROOTD_PROXY",val.c_str());
        } else if (key == "siteprefixreplace" )  // Like SLAC only
            siteprefixreplace = val.split(":");
	    else {
	        *eDest << "XRD-N2N: Invalid parameter " << key << endl;
	        return 3;
	    }
    }
    if (siteprefixstr != NULL) { // paramenter rucioprefix is provided
        *eDest << "XRD-N2N: Customer RUCIO prefix list " << siteprefixstr << endl;
    } else if (sitename != "") { // use sitename to get site prefix from AGIS
        *eDest << "XRD-N2N: Getting site " << sitename << " prefix list from AGIS ... " << endl;
        siteprefixstr = rucio_get_siteprefix(AGISurl, sitename.c_str());
        if (siteprefixstr == NULL)
            *eDest << "XRD-N2N: RUCIO prefix = none, RUCIO N2N is disabled" << endl;
        else {
            *eDest << "XRD-N2N: prefix list: " << siteprefixstr << endl;
            String tmp = siteprefixstr;
            rucioprefix_list = tmp.split(",");
        }
    }
    else 
        *eDest << "XRD-N2N: RUCIO prefix = none, RUCIO N2N is disabled" << endl;
   
    if (siteprefixstr != NULL) free((void*)siteprefixstr);
    // all done
    return 0;
}

// String XrdOucLFC::get_pnfsid(String pfn){
//     List components = pfn.split("/");
//     String fname = components.back();
//     components.pop_back();
//     String path = join(components, "/");
//     path += "/.(id)(" + fname + ")";
// 
//     String pnfsid = "";
// 
//     ifstream ifs;
//     
//     ifs.exceptions(ifstream::failbit | ifstream::badbit);
//     
//     struct timeval t0, t1;;
//     gettimeofday(&t0, NULL);
// 
//     try {
//     ifs.open(path);
//     ifs >> pnfsid;
//     } catch (...) {
//     char err[64];
//     strerror_r(errno, err, 64);
//     *eDest << "XRD-N2N: Error reading " << path << " " << err << endl;
//     }
//     gettimeofday(&t1, NULL);
//     float diff = (t1.tv_sec + t1.tv_usec/1000000.0) - (t0.tv_sec + t0.tv_usec/1000000.0);
//     *eDest << "XRD-N2N: timing info: pnfsid lookup took " << diff << " seconds" << endl;
//     return pnfsid;
// }


int XrdOucLFC::lfn2rfn(const char* lfn, char  *buff, int blen){
    *eDest << "XRD-LFC: lfn2rfn not implemented" << endl;
    return -EOPNOTSUPP;
}

int XrdOucLFC::pfn2lfn(const char* pfn, char  *buff, int blen){
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
	cerr << "Cannot initialize xrd-n2n" << endl;
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
