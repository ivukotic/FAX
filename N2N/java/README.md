Using the plugin with dCache
----------------------------

This plugin is needed only on the dCache xrootd door node.
The node needs to have dCache PNFS mounted.

The only change needed in the dCache configuration is to add the following lines:

	xrootd/xrootdPlugins=gplazma:gsi,authz:atlas-name-to-name-plugin
	site=<ATLAS_SITE_NAME_AS_DEFINED_IN_AGIS>


if the door node has no access to AGIS configuration needs one more line:
	overwriteSE=/pnfs/pic.es/data/atlas/atlasdatadisk,...
listing all the space tokens defined at the side, that should be accessible through FAX.
