#include <jni.h>
#include "essentia/algorithmfactory.h"
#include "essentia/pool.h"
#include "essentia/essentia.h"
#include "essentia/algorithm.h"
#include <iostream>
#include <fstream>

using namespace essentia;
using namespace essentia::standard;
using namespace std;

template <typename T>
void printVector(const std::vector<T>& vec) {
    for (const auto& elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

// Ensure Essentia is initialized only once (singleton pattern)
void initializeEssentia() {
    static bool initialized = false;
    if (!initialized) {
        essentia::init();
        initialized = true;
    }
}

// Computes average energy (vector) based on the analysed track to figure out which is the general energy of the song
jfloat computeAverageEnergy(const vector<Real>& values) {
    if (values.empty()) {
        return 0.0f;  // Return 0 or appropriate value for empty vector
    }

    Real sum = 0.0;
    for (Real value : values) {
        sum += value;
    }
    Real average = sum / values.size();
    return static_cast<jfloat>(average);
}

std::string getCamelotCode(const std::string& key, const std::string& scale) {
    std::map<std::pair<std::string, std::string>, std::string> camelotMap = {
        {{"C", "major"}, "8B"}, {{"C#", "major"}, "3B"}, {{"Db", "major"}, "3B"}, {{"D", "major"}, "10B"},
        {{"D#", "major"}, "5B"}, {{"Eb", "major"}, "5B"}, {{"E", "major"}, "12B"}, {{"F", "major"}, "7B"},
        {{"F#", "major"}, "2B"}, {{"Gb", "major"}, "2B"}, {{"G", "major"}, "9B"}, {{"G#", "major"}, "4B"}, {{"Ab", "major"}, "4B"},
        {{"A", "major"}, "11B"}, {{"A#", "major"}, "6B"}, {{"Bb", "major"}, "6B"}, {{"B", "major"}, "1B"},

        {{"C", "minor"}, "5A"}, {{"C#", "minor"}, "12A"}, {{"Db", "minor"}, "12A"}, {{"D", "minor"}, "7A"},
        {{"D#", "minor"}, "2A"}, {{"Eb", "minor"}, "2A"}, {{"E", "minor"}, "9A"}, {{"F", "minor"}, "4A"},
        {{"F#", "minor"}, "11A"}, {{"Gb", "minor"}, "11A"}, {{"G", "minor"}, "6A"}, {{"G#", "minor"}, "1A"}, {{"Ab", "minor"}, "1A"},
        {{"A", "minor"}, "8A"}, {{"A#", "minor"}, "3A"}, {{"Bb", "minor"}, "3A"}, {{"B", "minor"}, "10A"}
    };

    auto it = camelotMap.find({key, scale});
    if (it != camelotMap.end()) {
        return it->second;
    }
    return "Unknown";
}

extern "C" {

JNIEXPORT void JNICALL Java_com_caveman_dual_DualApplication_initEssentia(JNIEnv *env, jclass cls) {
    initializeEssentia();
}

JNIEXPORT void JNICALL Java_com_caveman_dual_DualApplication_shutdownEssentia(JNIEnv *env, jclass cls) {
    essentia::shutdown();
}

JNIEXPORT jobjectArray JNICALL Java_com_caveman_dual_DualApplication_analyzeAudio(JNIEnv *env, jclass cls, jstring audioPath, jstring stringProfileType) {
    const char* nativeAudioPath = env->GetStringUTFChars(audioPath, nullptr);
    const char* profileTypeChars = env->GetStringUTFChars(stringProfileType, nullptr);
        
    if (!nativeAudioPath || !profileTypeChars) return nullptr;

    string profileType(profileTypeChars);
    
    try {
        // Setup the algorithms needed for BPM, key, and energy extraction
        AlgorithmFactory& factory = AlgorithmFactory::instance();
        Algorithm* audioLoader = factory.create("MonoLoader", "filename", nativeAudioPath, "sampleRate", 44100);
        Algorithm* rhythmExtractor = factory.create("RhythmExtractor2013", "method", "multifeature");
        Algorithm* keyExtractor = factory.create("KeyExtractor", "profileType", profileType);
        Algorithm* levelExtractor = factory.create("LevelExtractor");
        Algorithm* dynamicComplexity = factory.create("DynamicComplexity");
        Algorithm* spectralContrast = factory.create("SpectralContrast");
        

        vector<Real> audio;
        audioLoader->output("audio").set(audio);
        audioLoader->compute();

        // Outputs of the rhythm extractor
        Real bpm;
        std::vector<Real> ticks;
        Real confidence;
        std::vector<Real> estimates;
        std::vector<Real> bpmIntervals;

        rhythmExtractor->input("signal").set(audio);
        rhythmExtractor->output("bpm").set(bpm);
        rhythmExtractor->output("ticks").set(ticks);
        rhythmExtractor->output("confidence").set(confidence);
        rhythmExtractor->output("estimates").set(estimates);
        rhythmExtractor->output("bpmIntervals").set(bpmIntervals);

        rhythmExtractor->compute();

        // Extract key and scale
        string key, scale;
        Real strength;
        keyExtractor->input("audio").set(audio);
        keyExtractor->output("key").set(key);
        keyExtractor->output("scale").set(scale);
        keyExtractor->output("strength").set(strength);
        keyExtractor->compute();

        cout << "C++ output: Extracted Key: " << key << endl;
        cout << "C++ output: Extracted Scale: " << scale << endl;

        string camelotCode = getCamelotCode(key, scale);

        // Extract energy (level)
        vector<Real> levels;
        levelExtractor->input("signal").set(audio);
        levelExtractor->output("loudness").set(levels);
        levelExtractor->compute();

        Real averageLevel = computeAverageEnergy(levels);

        // Song structure analysis
        Real dynamicComplexityValue, loudness;
        vector<Real> spectralContrastValues;

        dynamicComplexity->input("signal").set(audio);
        dynamicComplexity->output("dynamicComplexity").set(dynamicComplexityValue);
        dynamicComplexity->output("loudness").set(loudness);
        dynamicComplexity->compute();

        frameCutter->input("signal").set(audio);
        frameCutter->output("frame").set(frame);

        windowing->input("frame").set(frame);
        windowing->output("frame").set(windowedFrame);

        spectrum->input("frame").set(windowedFrame);
        spectrum->output("spectrum").set(spectrumFrame);

        spectralContrast->input("spectrum").set(spectrumFrame);
        spectralContrast->output("spectralContrast").set(spectralContrastValues);

        while (true) {
            frameCutter->compute();
            if (frame.empty()) break;
            windowing->compute();
            spectrum->compute();
            spectralContrast->compute();
        }

        // Clean up
        delete audioLoader;
        delete rhythmExtractor;
        delete keyExtractor;
        delete levelExtractor;
        delete dynamicComplexity;
        delete frameCutter;
        delete windowing;
        delete spectrum;
        delete spectralContrast;

        // Format the results as strings
        char bpmStr[50];
        sprintf(bpmStr, "%.2f", bpm);
        char levelStr[50];
        sprintf(levelStr, "%.2f", averageLevel);

        // Create string array to return
        jobjectArray result = env->NewObjectArray(3, env->FindClass("java/lang/String"), nullptr);
        env->SetObjectArrayElement(result, 0, env->NewStringUTF(bpmStr));
        env->SetObjectArrayElement(result, 1, env->NewStringUTF(levelStr));
        env->SetObjectArrayElement(result, 2, env->NewStringUTF(camelotCode.c_str()));

        env->ReleaseStringUTFChars(audioPath, nativeAudioPath);
        return result;
    } catch (const std::exception& e) {
        cerr << "Error during Essentia analysis: " << e.what() << endl;
        env->ReleaseStringUTFChars(audioPath, nativeAudioPath);
        return nullptr;
    }
}
}
