package com.caveman.dual;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Arrays;

public class DualApplication {

    private static final Logger log = LoggerFactory.getLogger(DualApplication.class);

    static {
        try { // Correct way to load the library
            System.loadLibrary("Dual");
        } catch (UnsatisfiedLinkError e) {
            log.error("Failed to load library: {}", e.getMessage());
            log.error(Arrays.toString(e.getStackTrace()));
        }
    }

    public native void initEssentia();
    public native void shutdownEssentia();
    public native String[] analyzeAudio(String audioPath, String profile);

    public static void main(String[] args) {


        DualApplication analyzer = new DualApplication();
        analyzer.initEssentia();
        String profile = "edma";
        String[] features = analyzer.analyzeAudio("autonomous.aiff", profile);
        if (features != null) {
            log.debug("BPM: {}", features[0]);
            log.debug("Camelot Code: {}", features[1]);
            log.debug("Average Energy Level: {}", features[2]);
        } else {
            log.error("Failed to analyze the audio.");
            log.error("Shutting down...");
        }
        analyzer.shutdownEssentia();
    }
}
