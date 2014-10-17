/******************************************************************************/
/*                                                                            */
/*                     X r d O u c N a m e 2 N a m e L F C . c c              */
/*                                                                            */
/******************************************************************************/

// This file implements an instance of the XrdOucName2Name abstract class
// which resolve LFNs to PFNs according to ATLAS RUCIO convention
//
// Initial version written by cgw@hep.uchicago.edu Oct 2010
// Support RUCIO global logical file name by Wei Yang, yangw@slac.stanford.edu June, 2013
// Drop support of LFC by Ilija Vukotic (ivukotic@uchicago.edu) and Wei Yang, January, 2014

const char* XrdOucName2NameLFCCVSID = "$Id: XrdOucName2NameLFC.cc,v 1.21 2011/12/13 16:06:40 sarah Exp $";
const char* version = "$Revision: 2.1 $";

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
#include "XrdVersion.hh"
XrdVERSIONINFO(XrdOucgetName2Name, "FAX-N2N");

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
bool prllstat = false;

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
    int parse_parameters(String);
    friend XrdOucName2Name *XrdOucgetName2Name(XrdOucgetName2NameArgs);

private:
    // Params
    List rucioprefix_list;
    List siteprefixreplace; // this is a private feature to only needed at SLAC. Has nothing to do with rucio.

    // Cache implementation
    class PfnRecord
    {
    public:
	String pfn;
	time_t timestamp;
	PfnRecord(String _pfn, time_t _timestamp) {
	    pfn = _pfn;
	    timestamp = _timestamp;
	};
	PfnRecord() {};
    };
    typedef hash_map<String, PfnRecord> Cache;
    Cache cache_by_lfn;
    deque<Cache::iterator> cache_by_time;
    int lfc_cache_ttl;
    int lfc_cache_maxsize;
    void insert_cache(const char*, String, time_t);

    // Locks
    pthread_mutex_t cache_mutex;
    void lock_cache() {pthread_mutex_lock(&cache_mutex);}
    void unlock_cache() {pthread_mutex_unlock(&cache_mutex);}
    
    // Misc
    XrdMsgStream *eDest;
};

XrdOucLFC::XrdOucLFC(XrdSysError* erp, const char* parms)
{
    lfc_cache_ttl = LFC_CACHE_TTL;
    lfc_cache_maxsize = LFC_CACHE_MAXSIZE;

    pthread_mutex_init(&cache_mutex, NULL);
    
#ifndef STANDALONE    
    eDest = new XrdMsgStream(erp);
#else
    eDest = erp;
#endif
    *eDest << "XRD-N2N v" << String(version).split(" ")[1] << " starting, parameters: " <<  parms << endl;
}
  
XrdOucLFC::~XrdOucLFC()
{
    pthread_mutex_destroy(&cache_mutex);
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

    if (strncmp(lfn, "/atlas/rucio", 12)) { // not a rucio gLFN
        strncpy(buff, lfn, blen);
        return 0;
    }

    *eDest << "XRD-N2N: lookup " << lfn << endl;

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
	strncpy(buff, rec.pfn, blen);
	*eDest << "XRD-N2N: cache hit, return " << buff << endl;
	return 0;
    } else {
        char *sfn = rucio_n2n_glfn(lfn);
        pfn = sfn;
        free(sfn); 
    }
    if (!pfn) {
	*eDest << "XRD-N2N: no valid replica for " << lfn << endl;
	return -ENOENT;
    }
    if (siteprefixreplace.size() == 2 && pfn.find(siteprefixreplace[0], 0) == 0) // Like SLAC only
        pfn.replace(0, siteprefixreplace[0].size(), siteprefixreplace[1]);

    // Cache LFC reply & other (pnfs) info
    if (! strncmp(lfn, "/atlas/rucio", 12) || ! strncmp(lfn, "/atlas/dq2", 10)) insert_cache(lfn, pfn, now);

    // Copy result to caller's buffer and report success
    strncpy(buff, pfn, blen);

    *eDest << "XRD-N2N: return " << buff << " cache size=" << cache_by_time.size() << endl;
    return 0;
}

void XrdOucLFC::insert_cache(const char* lfn, String pfn, time_t time)
{
    pair <Cache::iterator, bool> ret;
    Cache::iterator it;
    bool insertion_done;
    int s1, s2;
    lock_cache();

    assert ( (s1 = cache_by_lfn.size()) == (s2 = cache_by_time.size()));

    ret = cache_by_lfn.insert(pair<String, PfnRecord>(lfn, PfnRecord(pfn, time)));
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

#include <openssl/ssl.h>

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
/*
  It seems after calling libcurl (in above function if we pull rucio prefix from AGIS) we need to call the 
  following OpenSSL function. Otherwise, the program will crash. This only happens on RHEL5 platform, probably
  due to the openssl and libcurl versions on rhel5)
*/
//    OpenSSL_add_all_algorithms();
//    OpenSSL_add_all_ciphers();
//    OpenSSL_add_all_digests();

    if (inst->rucioprefix_list.size() != 0) rucio_n2n_init((XrdMsgStream*)(inst->eDest), inst->rucioprefix_list, prllstat);

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
        if (*it == "prllstat") { // see parallelstat in rucioN2N.cc
            prllstat = true;
            continue;
        }
	List keyval = it->split("=");
	if (keyval.size() != 2) {
	    // ERROR
	    *eDest << "XRD-N2N: Invalid parameter " << *it << endl;
	    return 1;
	}
	String key = keyval[0];
	String val = keyval[1];
	if (key == "cache_ttl") {
	    if (! (stringstream(val) >> lfc_cache_ttl) ) {
		*eDest << "XRD-N2N: Invalid numeric parameter " << val << endl;
		return 2;
	    }
	} else if (key == "cache_maxsize") {
	    if (! (stringstream(val) >> lfc_cache_maxsize) ) {
		*eDest << "XRD-N2N: Invalid numeric parameter " << val << endl;
		return 2;
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
	}
    }
    if (siteprefixstr != NULL) { // paramenter rucioprefix is provided
        *eDest << "XRD-N2N: Customer RUCIO prefix list " << siteprefixstr << endl;
    } else if (sitename != "") { // use sitename to get site prefix from AGIS
        *eDest << "XRD-N2N: Getting site " << sitename << " prefix list from AGIS ... " << endl;
        siteprefixstr = rucio_get_siteprefix(eDest, AGISurl, sitename.c_str());
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

int XrdOucLFC::lfn2rfn(const char* lfn, char  *buff, int blen)
{
    if (strncmp(lfn, "/atlas/rucio", 12)) { // not a rucio gLFN
        strncpy(buff, lfn, blen);
        return 0;
    }
    else {
        *eDest << "XRD-N2N: lfn2rfn not implemented" << endl;
        return -EOPNOTSUPP;
    }
}

int XrdOucLFC::pfn2lfn(const char* pfn, char  *buff, int blen)
{
    *eDest << "XRD-N2N: pfn2lfn not implemented" << endl;
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
