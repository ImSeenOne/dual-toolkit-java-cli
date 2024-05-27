package com.caveman.dual;


public class DualApplication {

    static {
        try {
            System.loadLibrary("Dual"); // Correct way to load the library
        } catch (UnsatisfiedLinkError e) {
            System.out.println("Failed to load library: " + e.getMessage());
            e.printStackTrace();
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
            System.out.println("BPM: " + features[0]);
            System.out.println("Camelot Code: " + features[1]);
            System.out.println("Average Energy Level: " + features[2]);
        } else {
            System.out.println("Failed to analyze the audio.");
        }
        analyzer.shutdownEssentia();
    }
}
