package edu.uchicago;

import junit.framework.TestCase;
import edu.uchicago.xrootd4j.RucioN2N;
import java.util.Properties;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class RucioN2NTest extends TestCase {

	final static Logger log = LoggerFactory.getLogger(RucioN2NTest.class);
	private Properties p=new Properties();
	
	public void testRucioInitialization() {
		log.info("test initialization...");
		p.setProperty("xrootd.n2n.site","MWT2");
		RucioN2N rucio=new RucioN2N(p);
		assert(rucio!=null);
		String gLFN3="/atlas/rucio/user/ivukotic:user.ivukotic.xrootd.mwt2-1M";
		String pfn3=rucio.translate(gLFN3);
		log.info(pfn3);
		assertTrue(pfn3!=null);
		log.info("---------------- done. ");
	}
	
}
