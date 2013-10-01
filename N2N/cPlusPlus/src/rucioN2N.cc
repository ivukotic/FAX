/******************************************************************************/
/* rucioN2N.cc                                                                */
/*                                                                            */
/* (c) 2010 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/* Author: Wei Yang (SLAC National Accelerator Laboratory, 2013)              */
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/md5.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "String.hh"
#include "XrdOuc/XrdOucEnv.hh"
#include "XrdPosix/XrdPosixXrootd.hh"
#include "XrdClient/XrdClientEnv.hh"
#include "XrdClient/XrdClientAdmin.hh"

using namespace std;

pthread_t cleaner;
pthread_mutex_t cm;
pthread_cond_t cc;
pthread_attr_t attr;

// create number connections by using multiple login names: e.g. root://rn2nXX@host//.
// this is required by the current (old) xrootd client because calls via the same xrootd 
// login name is sequentialized by the client

#define nXrdConn4n2n 20
short iXrdConn4n2n = 0;

char **sitePrefix;
int nPrefix = 0;

class RucioStorageStatPars {
public:
    pthread_mutex_t *m;
    pthread_cond_t *c;
    short id;
    short *icount;
    char *input;
    char *output;
    RucioStorageStatPars(pthread_mutex_t *xm, 
                         pthread_cond_t *xc, 
                         short xid, 
                         short *xicount, 
                         const char *xinput,
                         char *xoutput) {
        m = xm;
        c = xc;
        id = xid;
        icount = xicount;
        input = strdup(xinput);
        output = xoutput; 
    }
    void FreeIt() { // this frees the memory
        if (m != NULL) free(m);
        if (c != NULL) free(c);
        if (icount != NULL) free(icount);
        if (input != NULL)  free(input);
        input = NULL;
        if (output != NULL) free(output);
    }
    ~RucioStorageStatPars() { // this does not free the memory
        m = NULL;
        c = NULL;
        icount = NULL;
        if (input != NULL) free(input);
        output = NULL;
    }
};

class Garbage {
public:
    pthread_t **tid;
    int nthreads; 
    RucioStorageStatPars *p;
    
    Garbage(pthread_t **xtid, int xn, RucioStorageStatPars *xp) {
        tid = xtid;
        nthreads = xn;
        p = xp;
    }
    Garbage(Garbage *g) {
        tid = g->tid;
        nthreads = g->nthreads;
        p = g->p;
    }
    void FreeIt() {
        for (int i = 0; i<nthreads; i++) {
            pthread_join(*tid[i], NULL);
            free(tid[i]);
        }
        free(tid);
        p->FreeIt();
        delete p;
    }
    ~Garbage() {}
};

typedef vector<Garbage> GarbageCan;
GarbageCan garbageCan;
void *garbageCleaner(void* x) {
    Garbage *g;
    while (true) {
        pthread_mutex_lock(&cm);
        if (garbageCan.size() == 0) pthread_cond_wait(&cc, &cm);
        g = new Garbage(*garbageCan.begin());
        garbageCan.erase(garbageCan.begin());
        pthread_mutex_unlock(&cm);

        g->FreeIt();
        delete g;
    }
}

void dump2GarbageCan(Garbage *g) {
    pthread_mutex_lock(&cm);
    garbageCan.push_back(*g);
    pthread_cond_signal(&cc);
    pthread_mutex_unlock(&cm);
}

int x_stat(const char *path, struct stat *buf) {
    char *pssorigin;
    char rooturl[512];

    if (XrdOucEnv::Import("XRDXROOTD_PROXY", pssorigin)) {
        int i;
        pthread_mutex_lock(&cm);
        i = iXrdConn4n2n;
        iXrdConn4n2n = (iXrdConn4n2n +1) % nXrdConn4n2n;
        pthread_mutex_unlock(&cm);

        sprintf(rooturl, "root://rn2n%d@%s/%s\0", i, pssorigin, path);
// User client admin interface instead of posix interface to get a little better performance (but less portable).
//        return XrdPosixXrootd::Stat(rooturl, buf);

        XrdClientLocate_Info host;
        XrdClientAdmin adm(rooturl);
        adm.Connect();

        return (adm.Locate((kXR_char *)path, host) == true? 0 : 1);
    } else
// For posix storage, doing multiple stat() in parallel is probably not necessary. let's see if there are complains.
        return stat(path, buf);
}

void rucio_n2n_init(List rucioPrefix) {
    int i;
    nPrefix = rucioPrefix.size();
    if (nPrefix == 0) return;

    sitePrefix = (char**)malloc(sizeof(char*) * rucioPrefix.size());
    for (i = 0; i < nPrefix; i++) sitePrefix[i] = strdup(rucioPrefix[i].c_str());

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_mutex_init(&cm, NULL);
    pthread_cond_init(&cc, NULL);
    pthread_create(&cleaner, NULL, garbageCleaner, NULL);

    char *tmp = (char*)malloc(strlen(sitePrefix[0]) + strlen("/rucio"));
    strcpy(tmp, sitePrefix[0]);
    strcat(tmp, "/rucio");
    struct stat stbuf;
    
// initialize xrd connections
    for (i = 0; i<nXrdConn4n2n; i++) x_stat(tmp, &stbuf);
    free(tmp);
} 

bool rucioMd5(const char *lfn, char *sfn) {
    unsigned char md5digest[16];
    char md5string[33];

    MD5((const unsigned char*)lfn+13, strlen(lfn)-13, md5digest);
    for(int i = 0; i < 16; ++i)
        sprintf(&md5string[i*2], "%02x", (unsigned int)md5digest[i]);

    char *p1, *p2;
    p1 = (char*)lfn+13;
    p2 = strchr(p1, ':');
// the standard case for a gLFN is /atlas/rucio/scope:file
// we should avoid crashing if someone mistakenly supply /atlas/rucio/scope or /atlas/rucio/:file
    if (p2 == NULL) return false;

    strcpy(sfn, "/rucio/"); 
    strncat(sfn, p1, strlen(p1) - strlen(p2));
    strcat(sfn, "/");
    strncat(sfn, md5string, 2); 
    strcat(sfn, "/");
    strncat(sfn, md5string+2, 2); 
    strcat(sfn, "/");
    strcat(sfn, ++p2);
    return true;
}

void *rucio_storage_stat(void *pars) {
    struct stat buf;
    RucioStorageStatPars *p;
    p = (RucioStorageStatPars*)pars;
 
    int rc;
    rc = x_stat(p->input, &buf);

    pthread_mutex_lock(p->m);
    *(p->icount) -= 1;
    if (p->output[0] == '\0' && rc == 0) strcat(p->output, p->input);
    pthread_cond_signal(p->c);
    pthread_mutex_unlock(p->m);

    delete p;
    pthread_exit(NULL);
}

// export this function
char* rucio_n2n_glfn(const char *lfn) {
    int i; 
    char *pfn;

    pthread_mutex_t *m = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *c = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));;
    short *icount = (short*)malloc(sizeof(short));
    char input[512];
    char *output = (char*)malloc(512);

    pthread_mutex_init(m, NULL);
    pthread_cond_init(c, NULL);
    *icount = nPrefix;
    output[0] = '\0';

    char sfn[512];
    sfn[0] = '\0';

    if (nPrefix == 0 || ! rucioMd5(lfn, sfn)) {
        pfn = strdup(output);
        return pfn;
    }
    pthread_t **ids = (pthread_t**)malloc(sizeof(pthread_t*) * nPrefix);
    RucioStorageStatPars *p;

    for (i=0; i<nPrefix; i++) {
        ids[i] = (pthread_t*)malloc(sizeof(pthread_t));
        strcpy(input, sitePrefix[i]);
        strcat(input, sfn);
        p = new RucioStorageStatPars(m, c, i, icount, input, output);
        pthread_create(ids[i], &attr, rucio_storage_stat, p);
    }

    struct timespec now;
    pthread_mutex_lock(m);
    while (*icount > 0 && output[0] == '\0') {
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_sec += 180;
// potential problem by using pthread_cond_timedwait(): what happens if a positive result returns after timeout period?
// unlike the conventional cases, this will not update the cmsd cache, and file will still be marked by cmsd as not exist.
        if (pthread_cond_timedwait(c, m, &now) == ETIMEDOUT) break;
    }
    pfn = strdup(output);
    pthread_mutex_unlock(m);

// if we get the result (exit or not exist), dump the thread cleaning to garbage cleaner and return.
    p = new RucioStorageStatPars(m, c, i, icount, lfn, output);
    Garbage *g = new Garbage(ids, nPrefix, p);
    dump2GarbageCan(g);
    delete g;
    return pfn;
}

#ifdef STANDALONE
// To comiple
// g++ -g -I/usr/include/xrootd rucioN2N.cc -lcrypto -lrt -lXrdUtils -lXrdPosix -DSTANDALONE -DTESTLOCALSTORAGE -fPIC (optional)
int main(int argc, char *argv[]) {

    char *prefix[32];
#ifdef TESTLOCALSTORAGE
    prefix[0] = strdup("/xrootd/atlas/atlasdatadisk");
    prefix[1] = strdup("/xrootd/atlas/atlasuserdisk");
    prefix[2] = strdup("/xrootd/atlas/atlasscratchdisk");
#else
    XrdOucEnv myEnv;
    prefix[0] = strdup("/atlas/xrootd/atlasdatadisk");
    prefix[1] = strdup("/atlas/xrootd/atlasuserdisk");
    prefix[2] = strdup("/atlas/xrootd/atlasscratchdisk");

    char pssorigin[] = "atl-xrdr:11094";
    XrdOucEnv::Export("XRDXROOTD_PROXY", pssorigin);

    struct stat buf;
// To proper use of XrdPosixXrootd, one has to define a static object
// Also, XrdPosixXrootd isn't thread safe while building physical 
// connection. After that, it is thread safe
    static XrdPosixXrootd abc;
    XrdPosixXrootd::Stat("root://atl-xrdr:11094//atlas/xrootd", &buf);
#endif
    rucio_n2n_init();
    printf("==================================================== \n");
    for (int i=1; i<argc; i++) {
//        printf("searching %s\n", argv[i]);
        char *pfn = rucio_n2n_glfn_stat(prefix, 3, argv[i]);
        printf("pfn : %s\n\n", pfn);
        free(pfn);
    }
    free(prefix[0]);
    free(prefix[1]);
    free(prefix[2]);
    pthread_cancel(cleaner);
    pthread_join(cleaner, NULL);
}
#endif