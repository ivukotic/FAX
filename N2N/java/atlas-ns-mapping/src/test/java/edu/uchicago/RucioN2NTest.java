package edu.uchicago;

import static org.junit.Assert.assertTrue;
import junit.framework.TestCase;
import edu.uchicago.xrootd4j.RucioN2N;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class RucioN2NTest extends TestCase {

	private static Logger log = Logger.getLogger(RucioN2NTest.class);
	private Properties p=new Properties();
	
	public void testRucioInitialization() {
		log.info("test initialization...");
		PropertyConfigurator.configure(RucioN2NTest.class.getClassLoader().getResource("log4j.properties"));
		p.setProperty("site","MWT2");
		RucioN2N rucio=new RucioN2N(p);
		assert(rucio!=null);
		String gLFN3="/atlas/rucio/mc12_8TeV:NTUP_TOP.01213737._008157.root.1";
//		String gLFN3="/atlas/rucio/mc12_8TeV:NTUP_TOP.01213735._003982.root.1";
		String pfn3=rucio.translate(gLFN3);
		log.info(pfn3);
		assertTrue(pfn3!=null);
		log.info("---------------- done. ");
	}
	
}
