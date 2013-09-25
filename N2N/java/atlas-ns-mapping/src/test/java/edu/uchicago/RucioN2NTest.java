package edu.uchicago;

import static org.junit.Assert.assertTrue;
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
		p.setProperty("site","MWT2");
		RucioN2N rucio=new RucioN2N(p);
		assert(rucio!=null);
		String gLFN3="/atlas/rucio/mc12_8TeV:NTUP_TOP.01213741._012015.root.1";
//		String gLFN3="/atlas/rucio/mc12_8TeV:NTUP_TOP.01213735._003982.root.1";
		String pfn3=rucio.translate(gLFN3);
		log.info(pfn3);
		assertTrue(pfn3!=null);
		log.info("---------------- done. ");
	}
	
}
