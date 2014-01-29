/******************************************************************************/
/* rucioGetSitePrefix.cc                                                      */
/*                                                                            */
/* (c) 2010 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/* Author: Wei Yang (SLAC National Accelerator Laboratory, 2013)              */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <curl/curl.h>
#include <json/json.h>

// mimic the curl example at http://curl.haxx.se/libcurl/c/getinmemory.html

struct JsonData {
    char *data;
    size_t size;
};

static size_t JsonDataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct JsonData *mem = (struct JsonData *)userp;
 
    mem->data = (char*)realloc(mem->data, mem->size + realsize + 1);
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    return realsize;
}

int GetJsonData(const char* AGISurl, struct JsonData *chunk) 
{  // caller is responsible to free(chuck->data) 
    CURL *curl_handle;
    CURLcode res;
   
    chunk->data = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk->size = 0;    /* no data at this point */ 
   
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, AGISurl);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, JsonDataCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 60L);
   
    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */ 
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
   
    res = curl_easy_perform(curl_handle);
   
    /* check for errors */ 
    if(res != CURLE_OK) return 0;
    curl_easy_cleanup(curl_handle);
//    curl_global_cleanup();
    return 1;
}

char *str_replace(const char *original, const char *pattern, const char *replacement)
{
    size_t const replen = strlen(replacement);
    size_t const patlen = strlen(pattern);
    size_t const orilen = strlen(original);

    size_t patcnt = 0;
    const char * oriptr;
    const char * patloc;

    // find how many times the pattern occurs in the original string
    for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen) patcnt++;

    // allocate memory for the new string
    size_t const retlen = orilen + patcnt * (replen - patlen);
    char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

    if (returned != NULL)
    {
      // copy the original string, replacing all the instances of the pattern
        char * retptr = returned;
        for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
        {
            size_t const skplen = patloc - oriptr;
            // copy the section until the occurence of the pattern
            strncpy(retptr, oriptr, skplen);
            retptr += skplen;
            // copy the replacement 
            strncpy(retptr, replacement, replen);
            retptr += replen;
        }
        // copy the rest of the string.
        strcpy(retptr, oriptr);
    }
    return returned;
}


char *rucio_get_siteprefix(const char* AGISurl, const char* mysite) 
{
    json_object *root, *site, *readonly, *prefix; 
    const char *sitename, *spprefix, *tmp;
    char  *siteprefix = NULL;
    int i, j;

    struct JsonData jsondata;
    if (!GetJsonData(AGISurl, &jsondata)) return NULL;

    root = json_tokener_parse(jsondata.data);
    for (i = 0; i < json_object_array_length(root); i++) {
        site = json_object_array_get_idx(root, i);
        sitename = str_replace(json_object_to_json_string_ext(json_object_object_get(site, "rc_site"), JSON_C_TO_STRING_PLAIN), 
                               "\"",
                               "");
        if (strcmp(sitename, mysite)) continue;
        
        readonly = json_object_object_get(json_object_object_get(site, "aprotocols"), "r");
        if (readonly == NULL) {
            free((char*)sitename);
            continue;
        }
        for (j = 0; j < json_object_array_length(readonly); j++) {
            prefix = json_object_array_get_idx(json_object_array_get_idx(readonly, j), 2);
            tmp = str_replace(json_object_to_json_string_ext(prefix, JSON_C_TO_STRING_PLAIN), "\\/", "/");
            spprefix = str_replace(tmp, "\"", "");
            if (j == 0)
                siteprefix = strdup(spprefix);    
            else {
                int k = strlen(siteprefix);
                siteprefix = (char*)realloc(siteprefix, k + strlen(spprefix) + 2); 
                memcpy(&siteprefix[k], ",", 1);
                memcpy(&siteprefix[k +1], spprefix, strlen(spprefix)); 
                siteprefix[k + 1 + strlen(spprefix)] = 0;
            }
            if (siteprefix[strlen(siteprefix) -1] == '/') siteprefix[strlen(siteprefix) -1] = 0;
            free((char*)tmp);
            free((char*)spprefix);
        } 
        free((char*)sitename);
        break;
    }

    free(root);
    free(jsondata.data);
    return siteprefix;
}
/*
int main(int argc, char **argv)
{
    char *url = "http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&flavour=XROOTD";
    char *sitename, *siteprefix;

    sitename = argv[1];
    siteprefix = rucio_get_siteprefix(url, sitename);
    printf("site %s : prefixes: %s\n", sitename, siteprefix);
    free(siteprefix);
    return 0;
}
*/
