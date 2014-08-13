The file host_to_site.txt is loaded by the GLED collector and is used to map servers to site names.
It can be obtained by doing:
	curl http://git.cern.ch/pubweb/FAX.git/blob_plain/HEAD:/mapping/host_to_site.txt

The file atlas_topology.json is used by the DDM dashboard team in order to resolve ambiguities on their side.
It can be obtained by doing:
	curl http://git.cern.ch/pubweb/FAX.git/blob_plain/HEAD:/mapping/atlas_topology.json
