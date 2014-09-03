
// Stream class to simplify logging

#include <sstream>
#include <iostream>

#include "XrdSys/XrdSysError.hh"


// Hash_map implementation
#ifdef CXX0X
#include <unordered_map>
#else // Use hash_map for now...
#include <ext/hash_map>
using __gnu_cxx::hash_map;
using __gnu_cxx::hash;
#endif

static pthread_mutex_t m1, m2;
static hash_map<pthread_t, ostringstream*> s_hash;
static XrdSysError *eDest;

class XrdMsgStream
{
public:
    XrdMsgStream() { eDest = NULL;  mutex_init(); }
    XrdMsgStream(XrdSysError *e) { eDest = e; mutex_init(); }
    ~XrdMsgStream() {}

    // delegate << to member 's'
    template <typename T> XrdMsgStream& operator<<(const T& x) { lock(); *get_s() << x; unlock(); }

    // type of std::cout
    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

    // function signature of std::endl
    typedef CoutType& (*StandardEndLine)(CoutType&);

    // define an operator<< to take in std::endl
    XrdMsgStream& operator<<(StandardEndLine manip)
    {
	lock();
	if (*manip != (StandardEndLine)std::endl) {
	    manip(*get_s());
	} else {
	    struct tm loc;
	    time_t now;
	    char *tmp;
	    pthread_t id;

	    id = pthread_self();
	    
	    now = time(NULL);
	    localtime_r(&now, &loc);

	    tmp = (char*) malloc(strlen(get_s()->str().c_str()) + 128);
            if (!tmp) {
                eDest->Say("XRD-N2N: String opr can not allocate memory");
                exit(1);
            }
	    strftime(tmp, 64, "%y%m%d %H:%M:%S", &loc);
	    sprintf(tmp+strlen(tmp), " 0x%x %s", id, get_s()->str().c_str());
            eDest->Say(tmp);

//	    if ((void*)eDest) 
//		eDest->Say(tmp);
//	    else 
//		cout << tmp << std::endl;
	    free(tmp);
	    get_s()->str(""); // Reset string to empty
	}
	unlock();
    }
private:
    ostringstream *get_s() {
	pthread_mutex_lock(&m2); // secondary lock to protect s_hash

	ostringstream *ret;
	pthread_t id = pthread_self();

	hash_map<pthread_t, ostringstream*>::iterator it;

	if ( (it = s_hash.find(id)) != s_hash.end()) {
	    ret = it->second;
	} else {
	    ret = new ostringstream;
	    s_hash[id] = ret;
	}
	pthread_mutex_unlock(&m2);
	return ret;
    }
    void mutex_init() {pthread_mutex_init(&m1, NULL); pthread_mutex_init(&m2, NULL); }
    void lock() {pthread_mutex_lock(&m1);}
    void unlock() {pthread_mutex_unlock(&m1);}
};

