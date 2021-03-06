Authorization and mapping plugin for xrootd4j and dCache
========================================================

This is an authorization and mapping plugin for xrootd4j and dCache.

To compile the plugin, run:

    mvn package


Using the plugin with dCache
----------------------------

To use this plugin with dCache, place the directory containing this
file in /usr/local/share/dcache/plugins/

To enable the plugin, define the following property in dcache.conf:

    xrootd.xrootdPlugins=authn:none,authz:atlas-name-to-name-plugin
    
in addition one needs to add:
    xrootd.n2n.site=rc_site
    proper value for "rc_site" you may find here: http://atlas-agis-api.cern.ch/request/service/query/get_se_services/?json&flavour=XROOTD
    
probably never needed still if someone wants to hide a SpaceToken from FAX, just add this variable listing all of the space tokens you want exposed.
    xrootd.n2n.overwriteSE=/pnfs/uchicago.edu/atlasgroupdisk,/pnfs/uchicago.edu/atlasproddisk
    
