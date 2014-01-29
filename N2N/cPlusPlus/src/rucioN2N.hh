/******************************************************************************/
/* rucioN2N.hh                                                                */
/*                                                                            */
/* (c) 2010 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/* Author: Wei Yang (SLAC National Accelerator Laboratory, 2013)              */
/******************************************************************************/

char AGISurl[] = "http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&flavour=XROOTD";

// List is a String list (defined in String.hh) of site prefixes.
void rucio_n2n_init(XrdMsgStream*, List);

// lfn : the global logical file Name
char* rucio_n2n_glfn(const char *lfn);

// rucio_get_siteprefix returns a comma seperated list of rucio site prefixes for "mysite"
char *rucio_get_siteprefix(const char* AGISurl, const char* mysite);
