package edu.uchicago.xrootd4j;

import java.net.InetSocketAddress;
import java.security.GeneralSecurityException;
import java.util.Map;
import java.util.MissingResourceException;
import java.util.Properties;

import javax.security.auth.Subject;

import org.dcache.xrootd.core.XrootdException;
import org.dcache.xrootd.plugins.AuthorizationHandler;
import org.dcache.xrootd.protocol.XrootdProtocol;
import org.dcache.xrootd.protocol.XrootdProtocol.FilePerm;
import org.slf4j.LoggerFactory;
import org.slf4j.Logger;

public class AtlasAuthorizationHandler implements AuthorizationHandler {

	final static Logger log = LoggerFactory.getLogger(AtlasAuthorizationHandler.class);

	private String SRM_HOST = "";

	private static RucioN2N rucio = null;

	public AtlasAuthorizationHandler(RucioN2N rc, Properties properties) throws IllegalArgumentException, MissingResourceException {

		rucio = rc;

		SRM_HOST = properties.getProperty("srm_host");

		if (SRM_HOST == null) {
			log.error("*** Error: SRM_HOST parameter not defined. Please set it (in etc/dcache.conf)  and restart the server.");
			throw new IllegalArgumentException("SRM_HOST parameter not defined. Please set it (in etc/dcache.conf) and restart the server.");
		} else {
			log.info("Setting SRM_HOST to: " + SRM_HOST);
		}

	}


	@Override
	public String authorize(Subject subject, InetSocketAddress localAddress, InetSocketAddress remoteAddress, String path, Map<String, String> opaque,
			int request, FilePerm mode) throws SecurityException, GeneralSecurityException, XrootdException {

		log.info("GOT to translate: " + path);

		if (path.startsWith("pnfs/") || path.startsWith("/pnfs/")) {
			return path;
		}

		String LFN = path;
		if (!LFN.startsWith("/atlas/rucio")) {
			log.error("*** Error: gLFN must start with /atlas/rucio. ");
			throw new XrootdException(XrootdProtocol.kXR_NotFound, "*** Error: LFN must start with /atlas/. ");
		}

		if (LFN.startsWith("/atlas/rucio/")) {
			String pfn = rucio.translate(LFN);
			if (pfn == null) {
				log.info("rucio name not found.");
				pfn = "";
				throw new XrootdException(XrootdProtocol.kXR_NotFound, "rucio name not found. ");
			} else {
				log.info("rucio translated name: " + pfn);
				return pfn;
			}
		}
		throw new XrootdException(XrootdProtocol.kXR_NotFound, "*** Error: File not Found.");
	}

}
