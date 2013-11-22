#trying to resolve localredirector

dom=$(dnsdomainname)
#echo "local domain: $dom"

dom="uchicago.edu"

hostn="localredirector.$dom"
#echo $hostn

resolveMe() {
	LR=$(host "$hostn")
}

if  resolveMe ; then
	echo $hostn
	exit
else
	#echo "$hostn does not resolve"
fi



