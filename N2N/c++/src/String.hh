#include <string>
#include <vector>

using namespace std;

class String : public string
{
public:
    String() : string() {}
    String(string s) :  string(s) {}
    String(const char* p) :  string(p? p : "") {}
    
    operator bool() { return !empty(); }
    operator const char*() { return c_str(); }
    operator char*() {return const_cast<char*>(c_str()); }

    bool startswith(const String s) { return substr(0, s.size())==s ; }

    // Tokenize a string based on a set of delimiters
    typedef vector<String> List;
    const List split(String delim) {
	List result;
	string::size_type tok_start, tok_end=0;
 	do {
	    if ( (tok_start=find_first_not_of(delim, tok_end)) == string::npos) 
		break;
	    tok_end = find_first_of(delim, tok_start);
	    result.push_back(String(substr(tok_start, 
		   tok_end==string::npos?  tok_end : tok_end-tok_start)));
	} while (tok_end != string::npos);

	return result;
    }
};

// Inverse to split
typedef vector<String> List;
static String join(List v, String delim)
{
    String result;
    for (List::iterator it=v.begin(); it != v.end(); ++it) 
	result += delim + *it;
    return result;
}

