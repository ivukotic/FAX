package edu.uchicago;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import edu.uchicago.xrootd4j.RucioN2N;

@RunWith(Suite.class)
@SuiteClasses({ RucioN2N.class, TranslateRucioNames.class })
public class AllTests {

}
