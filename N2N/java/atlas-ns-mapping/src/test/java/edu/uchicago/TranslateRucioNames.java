package edu.uchicago;

import static org.junit.Assert.*;

import java.util.Properties;

import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.uchicago.xrootd4j.RucioN2N;

public class TranslateRucioNames {

	final static Logger log = LoggerFactory.getLogger(TranslateRucioNames.class);
	
	@Test
	public void test() {
		log.info("test both initialization and translation.");

		Properties p=new Properties();
		p.setProperty("site","MWT2");
		
		RucioN2N rucio=new RucioN2N(p);
		

		log.info("test translation...");
		String gLFN1="/atlas/rucio/mc12_8TeV:NTUP_TOP.01213741._012015.root.1";
		String pfn1=rucio.translate(gLFN1);
		log.info(pfn1);
		assertTrue(pfn1!=null);
		log.info("---------------- done 1. ");
		
		String pfn2=rucio.translate(gLFN1);
		assertTrue(pfn2!=null);

		log.info("---------------- done 2. ");
		
	}

}
