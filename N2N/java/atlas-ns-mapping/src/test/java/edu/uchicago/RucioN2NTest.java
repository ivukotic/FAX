package edu.uchicago;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import edu.uchicago.xrootd4j.RucioN2N;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

/**
 * Unit test for simple App.
 */
public class RucioN2NTest extends TestCase {

	
	private static Logger log = Logger.getLogger(RucioN2NTest.class);
	private Properties p=new Properties();
	private static RucioN2N rucio;
	
	public RucioN2NTest(String testName) {
		super(testName);
		PropertyConfigurator.configure(RucioN2NTest.class.getClassLoader().getResource("log4j.properties"));
		p.setProperty("site","MWT2");
	}

	/**
	 * @return the suite of tests being tested
	 */
	public static Test suite() {
		return new TestSuite(RucioN2NTest.class);
	}


	public void testRucioInitialization() {
		log.info("test initialization...");
		rucio=new RucioN2N(p);
	}
	
	public void testTranslation(){
		log.info("test translation...");
		String gLFN1="/atlas/rucio/mc12_8TeV:NTUP_TOP.01213329._025034.root.1";
		String pfn1=rucio.translate(gLFN1);
		assertTrue(pfn1!=null);
		log.info("done. ");
		String pfn2=rucio.translate(gLFN1);
		assertTrue(pfn2!=null);
		log.info("done. ");
	}
}
