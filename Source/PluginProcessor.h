#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <memory>
#include <vector>

struct FourOscParameterRefs
{
    std::atomic<float>* masterGain = nullptr;
    std::atomic<float>* retrigger = nullptr;
    std::atomic<float>* drift = nullptr;
    std::atomic<float>* drive = nullptr;
    std::atomic<float>* cutoff = nullptr;
    std::atomic<float>* resonance = nullptr;
    std::atomic<float>* bassCompAmount = nullptr;
    std::atomic<float>* analogWarm = nullptr;
    std::atomic<float>* madnezz = nullptr;
    std::atomic<float>* attackMs = nullptr;
    std::atomic<float>* decayMs = nullptr;
    std::atomic<float>* sustain = nullptr;
    std::atomic<float>* releaseMs = nullptr;
    std::atomic<float>* unisonVoices = nullptr;
    std::atomic<float>* unisonDetune = nullptr;
    std::atomic<float>* stereoSpread = nullptr;
    std::atomic<float>* monoWidth = nullptr;
    std::atomic<float>* vibratoDepthCents = nullptr;
    std::atomic<float>* vibratoRateHz = nullptr;
    std::atomic<float>* pitchEnvTimeMs = nullptr;
    std::atomic<float>* panSweepAmount = nullptr;
    std::atomic<float>* panSweepRateHz = nullptr;
    std::atomic<float>* chorusRate = nullptr;
    std::atomic<float>* chorusDepth = nullptr;
    std::atomic<float>* chorusMix = nullptr;
    std::atomic<float>* chorusSync = nullptr;
    std::atomic<float>* chorusSyncDivision = nullptr;
    std::atomic<float>* fxSpace = nullptr;
    std::atomic<float>* delayTimeMs = nullptr;
    std::atomic<float>* delayFeedback = nullptr;
    std::atomic<float>* delayMix = nullptr;
    std::atomic<float>* delaySync = nullptr;
    std::atomic<float>* delaySyncDivision = nullptr;
    std::atomic<float>* reverbSize = nullptr;
    std::atomic<float>* reverbDamping = nullptr;
    std::atomic<float>* reverbMix = nullptr;
    std::atomic<float>* arpEnabled = nullptr;
    std::atomic<float>* arpDivision = nullptr;
    std::atomic<float>* arpGate = nullptr;
    std::atomic<float>* arpMode = nullptr;
    std::atomic<float>* arpLatch = nullptr;
    std::atomic<float>* arpOctaves = nullptr;
    std::atomic<float>* subAmount = nullptr;
    std::atomic<float>* subFrequencyHz = nullptr;
    std::array<std::atomic<float>*, 4> oscEnabled {};
    std::array<std::atomic<float>*, 4> oscLevel {};
    std::array<std::atomic<float>*, 4> oscTuneCents {};
    std::array<std::atomic<float>*, 4> oscOctave {};
    std::array<std::atomic<float>*, 4> oscSemitone {};
    std::array<std::atomic<float>*, 4> oscPan {};
    std::array<std::atomic<float>*, 4> oscPhaseDeg {};
    std::array<std::atomic<float>*, 4> oscPitchEnvSemi {};
    std::array<std::atomic<float>*, 4> oscPitchEnvShape {};
    std::array<std::atomic<float>*, 4> oscWave {};
};

class FourOscProAudioProcessor final : public juce::AudioProcessor
{
public:
    FourOscProAudioProcessor();
    ~FourOscProAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float getUiArpPhase() const noexcept { return uiArpPhase.load(); }
    int getUiArpStep() const noexcept { return uiArpStep.load(); }
    bool isUiArpActive() const noexcept { return uiArpActive.load(); }
    float getUiHostBpm() const noexcept { return uiHostBpm.load(); }
    float getUiOutputLevel() const noexcept { return uiOutputLevel.load(); }
    void setArpHookEnabled(const juce::String& paramId, bool enabled, float depth = 0.15f);
    bool isArpHookEnabled(const juce::String& paramId) const;
    void clearArpHooks();

private:
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState parameters;
    juce::dsp::Chorus<float> chorusLeft;
    juce::dsp::Chorus<float> chorusRight;
    juce::dsp::Phaser<float> phaserLeft;
    juce::dsp::Phaser<float> phaserRight;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLeft { 192000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayRight { 192000 };
    juce::dsp::Reverb reverb;
    juce::dsp::Limiter<float> outputLimiter;
    float delayFeedbackStateL = 0.0f;
    float delayFeedbackStateR = 0.0f;
    double currentSampleRate = 44100.0;
    std::array<bool, 128> arpHeldNotes {};
    std::array<bool, 128> arpPhysicalHeld {};
    std::array<float, 128> arpHeldVelocities {};
    int arpHeldCount = 0;
    int arpCurrentStepIndex = 0;
    int arpCurrentNote = -1;
    int arpSamplesUntilNextStep = 0;
    int arpSamplesUntilGateOff = -1;
    bool arpWasEnabled = false;
    bool arpDirectionForward = true;
    bool arpWasLatch = false;
    int arpLastRandomIndex = -1;
    int arpVisualStepCounter = 0;
    juce::Random arpRandom;
    std::atomic<float> uiArpPhase { 0.0f };
    std::atomic<int> uiArpStep { 0 };
    std::atomic<bool> uiArpActive { false };
    std::atomic<float> uiHostBpm { 120.0f };
    std::atomic<float> uiOutputLevel { 0.0f };
    struct ArpHookState
    {
        juce::String paramId;
        bool hooked = false;
        bool baseLatched = false;
        float depth = 0.15f;
        float baseNorm = 0.0f;
        float phaseOffset = 0.0f;
    };
    mutable juce::CriticalSection arpHookLock;
    std::vector<ArpHookState> arpHooks;
    std::unique_ptr<FourOscParameterRefs> parameterRefs;
    float subPhase = 0.0f;
    float subEnvFollower = 0.0f;
    float fxMotionPhaseA = 0.0f;
    float fxMotionPhaseB = 0.0f;
    float fxMotionPhaseC = 0.0f;
    float fxRandomDrift = 0.0f;
    float fxPulseEnv = 0.0f;
    int fxPulseSamplesRemaining = 0;
    juce::Random fxMotionRandom;

    int getArpStepSamples(double hostBpm) const;
    juce::MidiBuffer buildArpMidi(const juce::MidiBuffer& inputMidi, int numSamples, double hostBpm);
    void stopArpNote(juce::MidiBuffer& midi, int sampleOffset);
    void applyArpHookMotionAudioThread(bool arpMotionActive, float arpPhase);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FourOscProAudioProcessor)
};
