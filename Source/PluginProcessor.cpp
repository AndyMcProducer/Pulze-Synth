#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <array>
#include <cmath>
#include <utility>
#include <vector>

namespace
{
constexpr int kOscCount = 4;
constexpr int kMaxUnison = 8;
constexpr int kHaasBufferSize = 2048;

namespace ParamIDs
{
static constexpr auto masterGain = "masterGain";
static constexpr auto retrigger = "retrigger";
static constexpr auto drift = "drift";
static constexpr auto drive = "drive";
static constexpr auto cutoff = "cutoff";
static constexpr auto resonance = "resonance";
static constexpr auto bassCompAmount = "bassCompAmount";
static constexpr auto analogWarm = "analogWarm";
static constexpr auto madnezz = "madnezz";
static constexpr auto attackMs = "attackMs";
static constexpr auto decayMs = "decayMs";
static constexpr auto sustain = "sustain";
static constexpr auto releaseMs = "releaseMs";
static constexpr auto unisonVoices = "unisonVoices";
static constexpr auto unisonDetune = "unisonDetune";
static constexpr auto stereoSpread = "stereoSpread";
static constexpr auto monoWidth = "monoWidth";
static constexpr auto vibratoDepthCents = "vibratoDepthCents";
static constexpr auto vibratoRateHz = "vibratoRateHz";
static constexpr auto pitchEnvTimeMs = "pitchEnvTimeMs";
static constexpr auto panSweepAmount = "panSweepAmount";
static constexpr auto panSweepRateHz = "panSweepRateHz";
static constexpr auto chorusRate = "chorusRate";
static constexpr auto chorusDepth = "chorusDepth";
static constexpr auto chorusMix = "chorusMix";
static constexpr auto phaserMix = "phaserMix";
static constexpr auto chorusSync = "chorusSync";
static constexpr auto chorusSyncDivision = "chorusSyncDivision";
static constexpr auto fxSpace = "fxSpace";
static constexpr auto delayTimeMs = "delayTimeMs";
static constexpr auto delayFeedback = "delayFeedback";
static constexpr auto delayMix = "delayMix";
static constexpr auto delaySync = "delaySync";
static constexpr auto delaySyncDivision = "delaySyncDivision";
static constexpr auto reverbSize = "reverbSize";
static constexpr auto reverbDamping = "reverbDamping";
static constexpr auto reverbMix = "reverbMix";
static constexpr auto postSaturation = "postSaturation";
static constexpr auto limiterOn = "limiterOn";
static constexpr auto limiterThresholdDb = "limiterThresholdDb";
static constexpr auto arpEnabled = "arpEnabled";
static constexpr auto arpDivision = "arpDivision";
static constexpr auto arpGate = "arpGate";
static constexpr auto arpMode = "arpMode";
static constexpr auto arpLatch = "arpLatch";
static constexpr auto arpOctaves = "arpOctaves";
static constexpr auto subAmount = "subAmount";
static constexpr auto subFrequencyHz = "subFrequencyHz";
} // namespace ParamIDs

static juce::String oscEnabledId(int index)    { return "osc" + juce::String(index + 1) + "Enabled"; }
static juce::String oscLevelId(int index)      { return "osc" + juce::String(index + 1) + "Level"; }
static juce::String oscTuneCentsId(int index)  { return "osc" + juce::String(index + 1) + "TuneCents"; }
static juce::String oscOctaveId(int index)     { return "osc" + juce::String(index + 1) + "Octave"; }
static juce::String oscSemitoneId(int index)   { return "osc" + juce::String(index + 1) + "Semitone"; }
static juce::String oscPanId(int index)        { return "osc" + juce::String(index + 1) + "Pan"; }
static juce::String oscPhaseDegId(int index)   { return "osc" + juce::String(index + 1) + "PhaseDeg"; }
static juce::String oscPitchEnvSemiId(int index){ return "osc" + juce::String(index + 1) + "PitchEnvSemi"; }
static juce::String oscPitchEnvShapeId(int index){ return "osc" + juce::String(index + 1) + "PitchEnvShape"; }
static juce::String oscWaveId(int index)       { return "osc" + juce::String(index + 1) + "Wave"; }

static float getDelaySyncMultiplier(int choice)
{
    switch (choice)
    {
        case 0: return 4.0f;             // 1/1
        case 1: return 2.0f;             // 1/2
        case 2: return 1.0f;             // 1/4
        case 3: return 0.5f;             // 1/8
        case 4: return 0.75f;            // 1/8D
        case 5: return 1.0f / 3.0f;      // 1/8T
        case 6: return 0.25f;            // 1/16
        case 7: return 0.375f;           // 1/16D
        case 8: return 1.0f / 6.0f;      // 1/16T
        default: return 0.5f;
    }
}

static float getChorusSyncMultiplier(int choice)
{
    switch (choice)
    {
        case 0: return 4.0f;     // 1/1 cycle
        case 1: return 2.0f;     // 1/2 cycle
        case 2: return 1.0f;     // 1/4 cycle
        case 3: return 0.5f;     // 1/8 cycle
        default: return 1.0f;
    }
}

static float getArpSyncMultiplier(int choice)
{
    switch (choice)
    {
        case 0: return 1.0f;             // 1/4
        case 1: return 0.5f;             // 1/8
        case 2: return 0.75f;            // 1/8D
        case 3: return 1.0f / 3.0f;      // 1/8T
        case 4: return 0.25f;            // 1/16
        case 5: return 0.375f;           // 1/16D
        case 6: return 1.0f / 6.0f;      // 1/16T
        case 7: return 0.125f;           // 1/32
        default: return 0.25f;
    }
}

class ExpAdsr
{
public:
    void prepare(double newSampleRate)
    {
        sampleRate = juce::jmax(1.0, newSampleRate);
    }

    void setParameters(float attackMs, float decayMs, float sustainValue, float releaseMs)
    {
        sustain = juce::jlimit(0.0f, 1.0f, sustainValue);
        attackCoeff = timeToCoeff(attackMs);
        decayCoeff = timeToCoeff(decayMs);
        releaseCoeff = timeToCoeff(releaseMs);
    }

    void noteOn()   { state = State::attack; }
    void noteOff()  { state = State::release; }
    bool isActive() const { return state != State::idle; }

    float getNextSample()
    {
        switch (state)
        {
            case State::idle:
                return 0.0f;
            case State::attack:
                level += (1.0f - level) * (1.0f - attackCoeff);
                if (level >= 0.999f)
                    state = State::decay;
                return level;
            case State::decay:
                level += (sustain - level) * (1.0f - decayCoeff);
                if (std::abs(level - sustain) <= 0.0005f)
                    state = State::sustain;
                return level;
            case State::sustain:
                return sustain;
            case State::release:
                level += (0.0f - level) * (1.0f - releaseCoeff);
                if (level <= 0.0001f)
                {
                    level = 0.0f;
                    state = State::idle;
                }
                return level;
        }

        return 0.0f;
    }

private:
    enum class State { idle, attack, decay, sustain, release };

    float timeToCoeff(float ms) const
    {
        const auto sec = juce::jmax(0.001f, ms * 0.001f);
        return std::exp(-1.0f / (sec * static_cast<float>(sampleRate)));
    }

    double sampleRate = 44100.0;
    State state = State::idle;
    float sustain = 0.8f;
    float level = 0.0f;
    float attackCoeff = 0.0f;
    float decayCoeff = 0.0f;
    float releaseCoeff = 0.0f;
};

class FourOscSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override      { return true; }
    bool appliesToChannel(int) override   { return true; }
};

class FourOscVoice final : public juce::SynthesiserVoice
{
public:
    explicit FourOscVoice(FourOscParameterRefs& inParams) : params(inParams) {}

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<FourOscSound*>(sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        baseFrequency = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
        noteVelocity = velocity;
        noteSampleIndex = 0;

        updateEnvelopeParams();

        const auto shouldRetrigger = (params.retrigger->load() >= 0.5f);
        if (shouldRetrigger)
        {
            for (int osc = 0; osc < kOscCount; ++osc)
                for (int lane = 0; lane < kMaxUnison; ++lane)
                    phases[osc][lane] = 0.0f;
        }

        env.noteOn();
    }

    void stopNote(float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            env.noteOff();
        }
        else
        {
            env.noteOff();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        ensurePrepared();
        updateEnvelopeParams();
        updateFilterParams();

        auto* left = outputBuffer.getWritePointer(0);
        auto* right = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1) : nullptr;

        const float driveDb = params.drive->load();
        const float driveLinear = juce::Decibels::decibelsToGain(driveDb);
        const float driveComp = 1.0f / juce::jmax(1.0f, driveLinear);

        for (int i = 0; i < numSamples; ++i)
        {
            const float envValue = env.getNextSample();
            if (! env.isActive())
            {
                clearCurrentNote();
                break;
            }

            const int unisonCount = getUnisonCount();
            const float detuneWidthCents = params.unisonDetune->load();
            const float stereoSpread = params.stereoSpread->load();
            const float monoWidth = juce::jlimit(0.0f, 1.0f, params.monoWidth->load());
            const float vibratoDepthCents = juce::jlimit(0.0f, 120.0f, params.vibratoDepthCents->load());
            const float vibratoRate = juce::jlimit(0.05f, 14.0f, params.vibratoRateHz->load());
            const float pitchEnvTimeMs = juce::jlimit(5.0f, 4000.0f, params.pitchEnvTimeMs->load());
            const float panSweepAmount = juce::jlimit(0.0f, 1.0f, params.panSweepAmount->load());
            const float panSweepRate = juce::jlimit(0.0f, 8.0f, params.panSweepRateHz->load());
            const float madnezz = juce::jlimit(0.0f, 1.0f, params.madnezz->load());
            const float driftNorm = juce::jlimit(0.0f, 1.0f, params.drift->load() / 8.0f);
            const float warmNorm = juce::jlimit(0.0f, 1.0f, params.analogWarm->load());
            const float organicAmt = juce::jlimit(0.0f, 0.45f, 0.04f + 0.18f * driftNorm + 0.14f * warmNorm
                                                               + 0.09f * (vibratoDepthCents / 80.0f));
            const float organicRateHz = 0.03f + 0.20f * driftNorm + 0.16f * organicAmt;
            organicPhase += juce::MathConstants<float>::twoPi * organicRateHz / static_cast<float>(getSampleRate());
            if (organicPhase > juce::MathConstants<float>::twoPi)
                organicPhase -= juce::MathConstants<float>::twoPi;
            vibratoPhase += juce::MathConstants<float>::twoPi * vibratoRate / static_cast<float>(getSampleRate());
            if (vibratoPhase > juce::MathConstants<float>::twoPi)
                vibratoPhase -= juce::MathConstants<float>::twoPi;
            panSweepPhase += juce::MathConstants<float>::twoPi * panSweepRate / static_cast<float>(getSampleRate());
            if (panSweepPhase > juce::MathConstants<float>::twoPi)
                panSweepPhase -= juce::MathConstants<float>::twoPi;
            const float madRateHz = 0.14f + madnezz * 0.48f;
            madnessPhase += juce::MathConstants<float>::twoPi * madRateHz / static_cast<float>(getSampleRate());
            if (madnessPhase > juce::MathConstants<float>::twoPi)
                madnessPhase -= juce::MathConstants<float>::twoPi;
            const float cutoffRateHz = 0.035f + madnezz * 0.42f;
            madnessCutoffPhase += juce::MathConstants<float>::twoPi * cutoffRateHz / static_cast<float>(getSampleRate());
            if (madnessCutoffPhase > juce::MathConstants<float>::twoPi)
                madnessCutoffPhase -= juce::MathConstants<float>::twoPi;
            float preFilterL = 0.0f;
            float preFilterR = 0.0f;

            for (int lane = 0; lane < unisonCount; ++lane)
            {
                const float lanePos = getLanePosition(lane, unisonCount);
                const float laneDetune = getSupersawDetuneCents(lanePos, detuneWidthCents);
                const float lanePanOffset = lanePos * stereoSpread * 0.55f;
                float laneL = 0.0f;
                float laneR = 0.0f;

                for (int osc = 0; osc < kOscCount; ++osc)
                {
                    if (params.oscEnabled[osc]->load() < 0.5f)
                        continue;

                    updateDrift(osc, lane);

                    const auto wave = static_cast<int>(params.oscWave[osc]->load());
                    const float level = params.oscLevel[osc]->load();
                    const float tuneCents = params.oscTuneCents[osc]->load();
                    const int octaveChoice = static_cast<int>(params.oscOctave[osc]->load());
                    const float octaveCents = static_cast<float>(octaveChoice - 2) * 1200.0f;
                    const float semitoneCents = params.oscSemitone[osc]->load() * 100.0f;
                    const float pitchEnvSemi = params.oscPitchEnvSemi[osc]->load();
                    const float pitchEnvProgress = juce::jlimit(0.0f, 4.0f, static_cast<float>(noteSampleIndex) / juce::jmax(1.0f, pitchEnvTimeMs * 0.001f * static_cast<float>(getSampleRate())));
                    const int pitchEnvShape = juce::jlimit(0, 2, static_cast<int>(params.oscPitchEnvShape[osc]->load()));
                    float pitchEnvValue = 0.0f;
                    if (pitchEnvShape == 0) // Exp
                        pitchEnvValue = std::exp(-4.0f * pitchEnvProgress);
                    else if (pitchEnvShape == 1) // Linear
                        pitchEnvValue = juce::jmax(0.0f, 1.0f - pitchEnvProgress);
                    else // S-curve
                    {
                        const float x = juce::jlimit(0.0f, 1.0f, 1.0f - pitchEnvProgress);
                        pitchEnvValue = x * x * (3.0f - 2.0f * x);
                    }
                    const float pitchEnvCents = pitchEnvSemi * 100.0f * pitchEnvValue;
                    const float vibratoLfo = std::sin(vibratoPhase + static_cast<float>(osc) * 0.87f + lanePos * 0.63f);
                    const float vibratoCents = vibratoDepthCents * vibratoLfo;
                    const float organicLfo = std::sin(organicPhase + static_cast<float>(osc) * 0.71f + lanePos * 0.53f + static_cast<float>(lane) * 0.17f);
                    const float organicLfo2 = std::sin(organicPhase * 0.39f + static_cast<float>(osc) * 1.63f + static_cast<float>(lane) * 0.29f);
                    const float organicCents = organicAmt * (22.0f * organicLfo + 8.0f * organicLfo2);
                    const float motionA = std::sin(madnessPhase + static_cast<float>(osc) * 1.1f + lanePos * 1.7f);
                    const float motionB = std::sin(madnessPhase * 0.53f + static_cast<float>(osc) * 2.4f + static_cast<float>(lane) * 0.61f);
                    const float madnessCents = madnezz * (190.0f * motionA + 78.0f * motionB);
                    const float totalCents = tuneCents + semitoneCents + octaveCents + laneDetune + driftCurrentCents[osc][lane]
                                           + pitchEnvCents + vibratoCents + organicCents + madnessCents;
                    const float freq = baseFrequency * std::pow(2.0f, totalCents / 1200.0f);

                    const float increment = juce::MathConstants<float>::twoPi * freq / static_cast<float>(getSampleRate());
                    phases[osc][lane] = std::fmod(phases[osc][lane] + increment, juce::MathConstants<float>::twoPi);

                    const float phaseLfo = std::sin(madnessPhase * 1.31f + static_cast<float>(osc) * 0.91f + static_cast<float>(lane) * 0.38f);
                    const float phaseOffset = params.oscPhaseDeg[osc]->load() * (juce::MathConstants<float>::pi / 180.0f)
                                            + madnezz * phaseLfo * juce::MathConstants<float>::pi * 0.58f
                                            + organicAmt * organicLfo2 * juce::MathConstants<float>::pi * 0.18f;
                    float phase = std::fmod(phases[osc][lane] + phaseOffset, juce::MathConstants<float>::twoPi);
                    if (phase < 0.0f)
                        phase += juce::MathConstants<float>::twoPi;

                    const float levelWarp = 1.0f + madnezz * 0.33f * motionB;
                    const float oscSample = juce::jmax(0.0f, level * levelWarp) * renderWave(wave, phase, osc, lane);
                    const float sweep = panSweepAmount * std::sin(panSweepPhase + static_cast<float>(osc) * 1.17f);
                    const float madnessPan = madnezz * 0.58f * std::sin(madnessPhase * 0.79f + static_cast<float>(osc) * 1.9f);
                    const float organicPan = organicAmt * 0.22f * std::sin(organicPhase * 0.57f + static_cast<float>(osc) * 1.43f + static_cast<float>(lane) * 0.41f);
                    const float totalPan = juce::jlimit(-1.0f, 1.0f, params.oscPan[osc]->load() + lanePanOffset + sweep + organicPan + madnessPan);
                    const auto pan = getPanGains(totalPan);
                    laneL += oscSample * pan.first;
                    laneR += oscSample * pan.second;
                }

                laneL *= (1.0f / static_cast<float>(kOscCount));
                laneR *= (1.0f / static_cast<float>(kOscCount));
                preFilterL += laneL;
                preFilterR += laneR;
            }

            const float unisonNorm = 1.0f / std::sqrt(static_cast<float>(unisonCount));
            preFilterL *= unisonNorm;
            preFilterR *= unisonNorm;

            // Width control: 0.0 = mono, 1.0 = full stereo.
            const float mono = 0.5f * (preFilterL + preFilterR);
            preFilterL = mono + (preFilterL - mono) * monoWidth;
            preFilterR = mono + (preFilterR - mono) * monoWidth;

            // Haas-style widening for Stereo Spread.
            const int maxHaasSamples = juce::jlimit(1, 384, static_cast<int>(0.012f * getSampleRate()));
            const int haasSamples = juce::jlimit(0, maxHaasSamples, static_cast<int>(stereoSpread * static_cast<float>(maxHaasSamples)));
            const int readIndex = (haasWriteIndex - haasSamples + kHaasBufferSize) % kHaasBufferSize;
            const float delayedL = haasBufferL[static_cast<size_t>(readIndex)];
            const float delayedR = haasBufferR[static_cast<size_t>(readIndex)];
            haasBufferL[static_cast<size_t>(haasWriteIndex)] = preFilterL;
            haasBufferR[static_cast<size_t>(haasWriteIndex)] = preFilterR;
            haasWriteIndex = (haasWriteIndex + 1) % kHaasBufferSize;
            if (haasSamples > 0)
            {
                const float haasMix = juce::jlimit(0.0f, 0.65f, 0.18f + stereoSpread * 0.42f);
                preFilterL = preFilterL * (1.0f - haasMix) + delayedR * haasMix;
                preFilterR = preFilterR * (1.0f - haasMix) + delayedL * haasMix;
            }

            // Nonlinear pre-filter drive for richer harmonics.
            const float saturatedL = std::tanh(preFilterL * driveLinear) * driveComp;
            const float saturatedR = std::tanh(preFilterR * driveLinear) * driveComp;
            const float baseCutoff = params.cutoff->load();
            const float resonance = params.resonance->load();
            const float cutoffLfo = 0.5f + 0.5f * std::sin(madnessCutoffPhase + madnessPhase * 0.37f);
            const float cutoffSemitoneSweep = (cutoffLfo * 2.0f - 1.0f) * (7.0f * madnezz);
            const float modCutoff = juce::jlimit(40.0f, 18000.0f, baseCutoff * std::pow(2.0f, cutoffSemitoneSweep / 12.0f));
            filterLeft.setCutoffFrequency(modCutoff);
            filterRight.setCutoffFrequency(modCutoff);
            filterLeft.setResonance(resonance);
            filterRight.setResonance(resonance);
            const float filteredL = filterLeft.processSample(0, saturatedL);
            const float filteredR = filterRight.processSample(0, saturatedR);

            // Resonance compensation: restore some low-end as resonance increases.
            lowBandStateL = lowBandCoeff * lowBandStateL + (1.0f - lowBandCoeff) * filteredL;
            lowBandStateR = lowBandCoeff * lowBandStateR + (1.0f - lowBandCoeff) * filteredR;
            const float compensatedL = filteredL + (bassCompGain - 1.0f) * lowBandStateL;
            const float compensatedR = filteredR + (bassCompGain - 1.0f) * lowBandStateR;
            const float warm = juce::jlimit(0.0f, 1.0f, params.analogWarm->load());
            const float warmDrive = 1.0f + 2.4f * warm;
            const float saturatedWarmL = std::tanh(compensatedL * warmDrive) / juce::jmax(1.0f, warmDrive * 0.8f);
            const float saturatedWarmR = std::tanh(compensatedR * warmDrive) / juce::jmax(1.0f, warmDrive * 0.8f);
            const float warmBlendL = compensatedL + warm * (saturatedWarmL - compensatedL);
            const float warmBlendR = compensatedR + warm * (saturatedWarmR - compensatedR);

            warmStateL = warmLowpassCoeff * warmStateL + (1.0f - warmLowpassCoeff) * warmBlendL;
            warmStateR = warmLowpassCoeff * warmStateR + (1.0f - warmLowpassCoeff) * warmBlendR;
            const float warmOutL = warmBlendL + warm * (warmStateL - warmBlendL);
            const float warmOutR = warmBlendR + warm * (warmStateR - warmBlendR);

            const float outL = warmOutL * envValue * noteVelocity;
            const float outR = warmOutR * envValue * noteVelocity;

            left[startSample + i] += outL;
            if (right != nullptr) right[startSample + i] += outR;
            else left[startSample + i] += 0.5f * (outL + outR);
            ++noteSampleIndex;
        }
    }

private:
    float renderWave(int wave, float phase, int osc, int lane)
    {
        switch (wave)
        {
            case 0: // Sine
                return std::sin(phase);
            case 1: // Saw
                return (phase / juce::MathConstants<float>::pi) - 1.0f;
            case 2: // Square
                return phase < juce::MathConstants<float>::pi ? 1.0f : -1.0f;
            case 3: // Triangle
            {
                const float x = phase / juce::MathConstants<float>::twoPi;
                return (4.0f * std::abs(x - 0.5f)) - 1.0f;
            }
            case 4: // White Noise
                return random.nextFloat() * 2.0f - 1.0f;
            case 5: // Pink Noise (Paul Kellet refinement)
            {
                const float white = random.nextFloat() * 2.0f - 1.0f;
                auto& b0 = pinkB0[osc][lane];
                auto& b1 = pinkB1[osc][lane];
                auto& b2 = pinkB2[osc][lane];
                auto& b3 = pinkB3[osc][lane];
                auto& b4 = pinkB4[osc][lane];
                auto& b5 = pinkB5[osc][lane];
                auto& b6 = pinkB6[osc][lane];

                b0 = 0.99886f * b0 + white * 0.0555179f;
                b1 = 0.99332f * b1 + white * 0.0750759f;
                b2 = 0.96900f * b2 + white * 0.1538520f;
                b3 = 0.86650f * b3 + white * 0.3104856f;
                b4 = 0.55000f * b4 + white * 0.5329522f;
                b5 = -0.7616f * b5 - white * 0.0168980f;
                const float pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362f;
                b6 = white * 0.115926f;
                return juce::jlimit(-1.0f, 1.0f, pink * 0.11f);
            }
            case 6: // Brown Noise
            {
                const float white = random.nextFloat() * 2.0f - 1.0f;
                auto& brown = brownState[osc][lane];
                brown = juce::jlimit(-1.0f, 1.0f, (brown + 0.045f * white) * 0.995f);
                return brown * 1.25f;
            }
            case 7: // Blue Noise
            {
                const float white = random.nextFloat() * 2.0f - 1.0f;
                auto& prev = bluePrevWhite[osc][lane];
                const float blue = (white - prev) * 0.95f;
                prev = white;
                return juce::jlimit(-1.0f, 1.0f, blue);
            }
            default:
                return 0.0f;
        }
    }

    void ensurePrepared()
    {
        if (isPrepared)
            return;

        env.prepare(getSampleRate());

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = getSampleRate();
        spec.maximumBlockSize = 256;
        spec.numChannels = 1;
        filterLeft.prepare(spec);
        filterRight.prepare(spec);
        filterLeft.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filterRight.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        lowBandCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * 120.0f / static_cast<float>(getSampleRate()));
        lowBandStateL = 0.0f;
        lowBandStateR = 0.0f;
        warmLowpassCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * 8000.0f / static_cast<float>(getSampleRate()));
        warmStateL = 0.0f;
        warmStateR = 0.0f;

        for (int osc = 0; osc < kOscCount; ++osc)
        {
            for (int lane = 0; lane < kMaxUnison; ++lane)
            {
                phases[osc][lane] = random.nextFloat() * juce::MathConstants<float>::twoPi;
                driftCurrentCents[osc][lane] = 0.0f;
                driftTargetCents[osc][lane] = 0.0f;
                driftStepCents[osc][lane] = 0.0f;
                driftSamplesToTarget[osc][lane] = 0;
                pinkB0[osc][lane] = 0.0f;
                pinkB1[osc][lane] = 0.0f;
                pinkB2[osc][lane] = 0.0f;
                pinkB3[osc][lane] = 0.0f;
                pinkB4[osc][lane] = 0.0f;
                pinkB5[osc][lane] = 0.0f;
                pinkB6[osc][lane] = 0.0f;
                brownState[osc][lane] = 0.0f;
                bluePrevWhite[osc][lane] = 0.0f;
            }
        }
        panSweepPhase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        vibratoPhase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        organicPhase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        madnessPhase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        madnessCutoffPhase = random.nextFloat() * juce::MathConstants<float>::twoPi;
        std::fill(haasBufferL.begin(), haasBufferL.end(), 0.0f);
        std::fill(haasBufferR.begin(), haasBufferR.end(), 0.0f);
        haasWriteIndex = 0;

        isPrepared = true;
    }

    void updateEnvelopeParams()
    {
        env.setParameters(params.attackMs->load(),
                          params.decayMs->load(),
                          params.sustain->load(),
                          params.releaseMs->load());
    }

    void updateFilterParams()
    {
        const auto cutoff = params.cutoff->load();
        const auto resonance = params.resonance->load();
        filterLeft.setCutoffFrequency(cutoff);
        filterLeft.setResonance(resonance);
        filterRight.setCutoffFrequency(cutoff);
        filterRight.setResonance(resonance);

        // 0.1..1.6 -> 0.0..1.0 normalized resonance.
        const float resonanceNorm = juce::jlimit(0.0f, 1.0f, (resonance - 0.1f) / 1.5f);
        const float amount = juce::jlimit(0.0f, 1.0f, params.bassCompAmount->load());
        bassCompGain = 1.0f + (0.65f * amount) * resonanceNorm;

        const float warm = juce::jlimit(0.0f, 1.0f, params.analogWarm->load());
        const float warmFc = 12000.0f - (9000.0f * warm);
        warmLowpassCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * warmFc / static_cast<float>(getSampleRate()));
    }

    void updateDrift(int osc, int lane)
    {
        const float driftLimit = params.drift->load();
        if (driftLimit <= 0.001f)
        {
            driftCurrentCents[osc][lane] = 0.0f;
            driftSamplesToTarget[osc][lane] = 0;
            return;
        }

        if (driftSamplesToTarget[osc][lane] <= 0)
        {
            driftTargetCents[osc][lane] = random.nextFloat() * (2.0f * driftLimit) - driftLimit;
            const float durationSec = 0.2f + random.nextFloat() * 1.8f;
            driftSamplesToTarget[osc][lane] = juce::jmax(1, static_cast<int>(durationSec * getSampleRate()));
            driftStepCents[osc][lane] = (driftTargetCents[osc][lane] - driftCurrentCents[osc][lane]) / static_cast<float>(driftSamplesToTarget[osc][lane]);
        }

        driftCurrentCents[osc][lane] += driftStepCents[osc][lane];
        --driftSamplesToTarget[osc][lane];
    }

    int getUnisonCount() const
    {
        const int choice = static_cast<int>(params.unisonVoices->load());
        if (choice == 1) return 2;
        if (choice == 2) return 4;
        if (choice == 3) return 8;
        return 1;
    }

    float getLanePosition(int lane, int laneCount) const
    {
        switch (laneCount)
        {
            case 1:
                return 0.0f;
            case 2:
            {
                static constexpr std::array<float, 2> positions { -1.0f, 1.0f };
                return positions[static_cast<size_t>(lane)];
            }
            case 4:
            {
                static constexpr std::array<float, 4> positions { -1.0f, -0.36f, 0.36f, 1.0f };
                return positions[static_cast<size_t>(lane)];
            }
            case 8:
            {
                static constexpr std::array<float, 8> positions { -1.0f, -0.74f, -0.50f, -0.22f, 0.22f, 0.50f, 0.74f, 1.0f };
                return positions[static_cast<size_t>(lane)];
            }
            default:
                break;
        }

        if (laneCount <= 1)
            return 0.0f;
        return (static_cast<float>(lane) / static_cast<float>(laneCount - 1)) * 2.0f - 1.0f;
    }

    float getSupersawDetuneCents(float lanePos, float detuneWidthCents) const
    {
        const float absPos = std::abs(lanePos);
        const float shaped = std::pow(absPos, 1.35f);
        return std::copysign(shaped * detuneWidthCents, lanePos);
    }

    std::pair<float, float> getPanGains(float pan) const
    {
        pan = juce::jlimit(-1.0f, 1.0f, pan);
        const float angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        return { std::cos(angle), std::sin(angle) };
    }

    FourOscParameterRefs& params;

    ExpAdsr env;
    juce::dsp::StateVariableTPTFilter<float> filterLeft;
    juce::dsp::StateVariableTPTFilter<float> filterRight;
    juce::Random random;

    std::array<std::array<float, kMaxUnison>, kOscCount> phases {};
    std::array<std::array<float, kMaxUnison>, kOscCount> driftCurrentCents {};
    std::array<std::array<float, kMaxUnison>, kOscCount> driftTargetCents {};
    std::array<std::array<float, kMaxUnison>, kOscCount> driftStepCents {};
    std::array<std::array<int, kMaxUnison>, kOscCount> driftSamplesToTarget {};
    std::array<std::array<float, kMaxUnison>, kOscCount> pinkB0 {};
    std::array<std::array<float, kMaxUnison>, kOscCount> pinkB1 {};
    std::array<std::array<float, kMaxUnison>, kOscCount> pinkB2 {};
    std::array<std::array<float, kMaxUnison>, kOscCount> pinkB3 {};
    std::array<std::array<float, kMaxUnison>, kOscCount> pinkB4 {};
    std::array<std::array<float, kMaxUnison>, kOscCount> pinkB5 {};
    std::array<std::array<float, kMaxUnison>, kOscCount> pinkB6 {};
    std::array<std::array<float, kMaxUnison>, kOscCount> brownState {};
    std::array<std::array<float, kMaxUnison>, kOscCount> bluePrevWhite {};

    float baseFrequency = 440.0f;
    float noteVelocity = 0.0f;
    int noteSampleIndex = 0;
    float bassCompGain = 1.0f;
    float lowBandCoeff = 0.98f;
    float lowBandStateL = 0.0f;
    float lowBandStateR = 0.0f;
    float warmLowpassCoeff = 0.9f;
    float warmStateL = 0.0f;
    float warmStateR = 0.0f;
    float panSweepPhase = 0.0f;
    float vibratoPhase = 0.0f;
    float organicPhase = 0.0f;
    float madnessPhase = 0.0f;
    float madnessCutoffPhase = 0.0f;
    std::array<float, kHaasBufferSize> haasBufferL {};
    std::array<float, kHaasBufferSize> haasBufferR {};
    int haasWriteIndex = 0;
    bool isPrepared = false;
};
} // namespace

FourOscProAudioProcessor::FourOscProAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    auto getRaw = [this] (const juce::String& id) { return parameters.getRawParameterValue(id); };

    parameterRefs = std::make_unique<FourOscParameterRefs>();
    parameterRefs->masterGain = getRaw(ParamIDs::masterGain);
    parameterRefs->retrigger = getRaw(ParamIDs::retrigger);
    parameterRefs->drift = getRaw(ParamIDs::drift);
    parameterRefs->drive = getRaw(ParamIDs::drive);
    parameterRefs->cutoff = getRaw(ParamIDs::cutoff);
    parameterRefs->resonance = getRaw(ParamIDs::resonance);
    parameterRefs->bassCompAmount = getRaw(ParamIDs::bassCompAmount);
    parameterRefs->analogWarm = getRaw(ParamIDs::analogWarm);
    parameterRefs->madnezz = getRaw(ParamIDs::madnezz);
    parameterRefs->attackMs = getRaw(ParamIDs::attackMs);
    parameterRefs->decayMs = getRaw(ParamIDs::decayMs);
    parameterRefs->sustain = getRaw(ParamIDs::sustain);
    parameterRefs->releaseMs = getRaw(ParamIDs::releaseMs);
    parameterRefs->unisonVoices = getRaw(ParamIDs::unisonVoices);
    parameterRefs->unisonDetune = getRaw(ParamIDs::unisonDetune);
    parameterRefs->stereoSpread = getRaw(ParamIDs::stereoSpread);
    parameterRefs->monoWidth = getRaw(ParamIDs::monoWidth);
    parameterRefs->vibratoDepthCents = getRaw(ParamIDs::vibratoDepthCents);
    parameterRefs->vibratoRateHz = getRaw(ParamIDs::vibratoRateHz);
    parameterRefs->pitchEnvTimeMs = getRaw(ParamIDs::pitchEnvTimeMs);
    parameterRefs->panSweepAmount = getRaw(ParamIDs::panSweepAmount);
    parameterRefs->panSweepRateHz = getRaw(ParamIDs::panSweepRateHz);
    parameterRefs->chorusRate = getRaw(ParamIDs::chorusRate);
    parameterRefs->chorusDepth = getRaw(ParamIDs::chorusDepth);
    parameterRefs->chorusMix = getRaw(ParamIDs::chorusMix);
    parameterRefs->chorusSync = getRaw(ParamIDs::chorusSync);
    parameterRefs->chorusSyncDivision = getRaw(ParamIDs::chorusSyncDivision);
    parameterRefs->fxSpace = getRaw(ParamIDs::fxSpace);
    parameterRefs->delayTimeMs = getRaw(ParamIDs::delayTimeMs);
    parameterRefs->delayFeedback = getRaw(ParamIDs::delayFeedback);
    parameterRefs->delayMix = getRaw(ParamIDs::delayMix);
    parameterRefs->delaySync = getRaw(ParamIDs::delaySync);
    parameterRefs->delaySyncDivision = getRaw(ParamIDs::delaySyncDivision);
    parameterRefs->reverbSize = getRaw(ParamIDs::reverbSize);
    parameterRefs->reverbDamping = getRaw(ParamIDs::reverbDamping);
    parameterRefs->reverbMix = getRaw(ParamIDs::reverbMix);
    parameterRefs->arpEnabled = getRaw(ParamIDs::arpEnabled);
    parameterRefs->arpDivision = getRaw(ParamIDs::arpDivision);
    parameterRefs->arpGate = getRaw(ParamIDs::arpGate);
    parameterRefs->arpMode = getRaw(ParamIDs::arpMode);
    parameterRefs->arpLatch = getRaw(ParamIDs::arpLatch);
    parameterRefs->arpOctaves = getRaw(ParamIDs::arpOctaves);
    parameterRefs->subAmount = getRaw(ParamIDs::subAmount);
    parameterRefs->subFrequencyHz = getRaw(ParamIDs::subFrequencyHz);
    parameterRefs->subAmount = getRaw(ParamIDs::subAmount);
    parameterRefs->subFrequencyHz = getRaw(ParamIDs::subFrequencyHz);

    for (int i = 0; i < kOscCount; ++i)
    {
        parameterRefs->oscEnabled[i] = getRaw(oscEnabledId(i));
        parameterRefs->oscLevel[i] = getRaw(oscLevelId(i));
        parameterRefs->oscTuneCents[i] = getRaw(oscTuneCentsId(i));
        parameterRefs->oscOctave[i] = getRaw(oscOctaveId(i));
        parameterRefs->oscSemitone[i] = getRaw(oscSemitoneId(i));
        parameterRefs->oscPan[i] = getRaw(oscPanId(i));
        parameterRefs->oscPhaseDeg[i] = getRaw(oscPhaseDegId(i));
        parameterRefs->oscPitchEnvSemi[i] = getRaw(oscPitchEnvSemiId(i));
        parameterRefs->oscPitchEnvShape[i] = getRaw(oscPitchEnvShapeId(i));
        parameterRefs->oscWave[i] = getRaw(oscWaveId(i));
    }

    for (int i = 0; i < 16; ++i)
        synth.addVoice(new FourOscVoice(*parameterRefs));

    synth.addSound(new FourOscSound());
}

FourOscProAudioProcessor::~FourOscProAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout FourOscProAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::masterGain, "Master Gain",
                                                            juce::NormalisableRange<float>(-24.0f, 12.0f, 0.01f), -3.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::retrigger, "Retrigger", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::drift, "Drift (Cents)",
                                                            juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f), 3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::drive, "Drive (dB)",
                                                            juce::NormalisableRange<float>(0.0f, 24.0f, 0.01f), 6.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::cutoff, "Cutoff",
                                                            juce::NormalisableRange<float>(40.0f, 18000.0f, 0.01f, 0.35f), 8000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::resonance, "Resonance",
                                                            juce::NormalisableRange<float>(0.1f, 1.6f, 0.001f), 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::bassCompAmount, "Bass Comp",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.6f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::analogWarm, "Analog Warm",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::madnezz, "Madnezz",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::attackMs, "Attack (ms)",
                                                            juce::NormalisableRange<float>(1.0f, 5000.0f, 0.01f, 0.4f), 8.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::decayMs, "Decay (ms)",
                                                            juce::NormalisableRange<float>(1.0f, 5000.0f, 0.01f, 0.4f), 180.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::sustain, "Sustain",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::releaseMs, "Release (ms)",
                                                            juce::NormalisableRange<float>(1.0f, 8000.0f, 0.01f, 0.4f), 280.0f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParamIDs::unisonVoices, "Unison Voices",
                                                             juce::StringArray { "1", "2", "4", "8" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::unisonDetune, "Unison Detune (Cents)",
                                                            juce::NormalisableRange<float>(0.0f, 35.0f, 0.01f), 8.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::stereoSpread, "Stereo Spread",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.75f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::monoWidth, "Mono Width",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::vibratoDepthCents, "Vibrato Depth (Cents)",
                                                            juce::NormalisableRange<float>(0.0f, 80.0f, 0.01f), 8.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::vibratoRateHz, "Vibrato Rate (Hz)",
                                                            juce::NormalisableRange<float>(0.05f, 12.0f, 0.001f, 0.5f), 5.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::pitchEnvTimeMs, "Pitch Env Time (ms)",
                                                            juce::NormalisableRange<float>(5.0f, 3000.0f, 0.01f, 0.4f), 220.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::panSweepAmount, "Pan Sweep Amount",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::panSweepRateHz, "Pan Sweep Rate (Hz)",
                                                            juce::NormalisableRange<float>(0.0f, 8.0f, 0.001f, 0.5f), 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::chorusRate, "Chorus Rate (Hz)",
                                                            juce::NormalisableRange<float>(0.05f, 8.0f, 0.001f, 0.5f), 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::chorusDepth, "Chorus Depth",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::chorusMix, "Chorus Mix",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::phaserMix, "Phaser Mix",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::chorusSync, "Chorus Sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParamIDs::chorusSyncDivision, "Chorus Sync Div",
                                                             juce::StringArray { "1/1", "1/2", "1/4", "1/8" }, 2));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::fxSpace, "FX Space",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.45f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::delayTimeMs, "Delay Time (ms)",
                                                            juce::NormalisableRange<float>(40.0f, 1500.0f, 0.01f, 0.35f), 340.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::delayFeedback, "Delay Feedback",
                                                            juce::NormalisableRange<float>(0.0f, 0.92f, 0.0001f), 0.34f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::delayMix, "Delay Mix",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.18f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::delaySync, "Delay Sync", true));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParamIDs::delaySyncDivision, "Delay Sync Div",
                                                             juce::StringArray { "1/1", "1/2", "1/4", "1/8", "1/8D", "1/8T", "1/16", "1/16D", "1/16T" }, 3));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::reverbSize, "Reverb Size",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.55f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::reverbDamping, "Reverb Damping",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::reverbMix, "Reverb Mix",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::postSaturation, "Post Saturation",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.15f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::limiterOn, "Limiter On", true));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::limiterThresholdDb, "Limiter Threshold (dB)",
                                                            juce::NormalisableRange<float>(-12.0f, 0.0f, 0.01f), -0.8f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::arpEnabled, "Arp Enabled", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParamIDs::arpDivision, "Arp Division",
                                                             juce::StringArray { "1/4", "1/8", "1/8D", "1/8T", "1/16", "1/16D", "1/16T", "1/32" }, 4));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::arpGate, "Arp Gate",
                                                            juce::NormalisableRange<float>(0.05f, 0.98f, 0.0001f), 0.58f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParamIDs::arpMode, "Arp Mode",
                                                             juce::StringArray { "Up", "Down", "UpDown", "Random" }, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::arpLatch, "Arp Latch", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParamIDs::arpOctaves, "Arp Octaves",
                                                             juce::StringArray { "1", "2", "3" }, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::subAmount, "Sub Amount",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::subFrequencyHz, "Sub Frequency (Hz)",
                                                            juce::NormalisableRange<float>(30.0f, 150.0f, 0.01f, 0.4f), 60.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::subAmount, "Sub Amount",
                                                            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParamIDs::subFrequencyHz, "Sub Frequency (Hz)",
                                                            juce::NormalisableRange<float>(30.0f, 150.0f, 0.01f, 0.4f), 60.0f));

    const juce::StringArray waveChoices { "Sine", "Saw", "Square", "Triangle", "White Noise", "Pink Noise", "Brown Noise", "Blue Noise" };

    for (int i = 0; i < kOscCount; ++i)
    {
        const auto idx = juce::String(i + 1);
        layout.add(std::make_unique<juce::AudioParameterBool>(oscEnabledId(i), "Osc " + idx + " Enabled", true));
        layout.add(std::make_unique<juce::AudioParameterFloat>(oscLevelId(i), "Osc " + idx + " Level",
                                                                juce::NormalisableRange<float>(0.0f, 1.0f, 0.0001f), 0.25f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(oscOctaveId(i), "Osc " + idx + " Octave",
                                                                 juce::StringArray { "-2", "-1", "0", "+1", "+2" }, 2));
        layout.add(std::make_unique<juce::AudioParameterFloat>(oscSemitoneId(i), "Osc " + idx + " Semitone",
                                                                juce::NormalisableRange<float>(-12.0f, 12.0f, 1.0f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(oscTuneCentsId(i), "Osc " + idx + " Tune (Cents)",
                                                                juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(oscPanId(i), "Osc " + idx + " Pan",
                                                                juce::NormalisableRange<float>(-1.0f, 1.0f, 0.0001f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(oscPhaseDegId(i), "Osc " + idx + " Phase (Deg)",
                                                                juce::NormalisableRange<float>(-180.0f, 180.0f, 0.1f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(oscPitchEnvSemiId(i), "Osc " + idx + " Pitch Env (Semi)",
                                                                juce::NormalisableRange<float>(-48.0f, 48.0f, 0.01f), 0.0f));
        layout.add(std::make_unique<juce::AudioParameterChoice>(oscPitchEnvShapeId(i), "Osc " + idx + " Pitch Env Shape",
                                                                 juce::StringArray { "Exp", "Linear", "S" }, 0));
        layout.add(std::make_unique<juce::AudioParameterChoice>(oscWaveId(i), "Osc " + idx + " Wave", waveChoices, 1));
    }

    return layout;
}

const juce::String FourOscProAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FourOscProAudioProcessor::acceptsMidi() const
{
    return true;
}

bool FourOscProAudioProcessor::producesMidi() const
{
    return false;
}

bool FourOscProAudioProcessor::isMidiEffect() const
{
    return false;
}

double FourOscProAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FourOscProAudioProcessor::getNumPrograms()
{
    return 1;
}

int FourOscProAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FourOscProAudioProcessor::setCurrentProgram(int)
{
}

const juce::String FourOscProAudioProcessor::getProgramName(int)
{
    return {};
}

void FourOscProAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void FourOscProAudioProcessor::prepareToPlay(double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 1024;
    spec.numChannels = 1;

    chorusLeft.prepare(spec);
    chorusRight.prepare(spec);
    chorusLeft.setCentreDelay(7.0f);
    chorusRight.setCentreDelay(8.0f);
    chorusLeft.setFeedback(0.0f);
    chorusRight.setFeedback(0.0f);

    phaserLeft.prepare(spec);
    phaserRight.prepare(spec);
    phaserLeft.setRate(0.28f);
    phaserRight.setRate(0.31f);
    phaserLeft.setDepth(0.7f);
    phaserRight.setDepth(0.75f);
    phaserLeft.setFeedback(0.18f);
    phaserRight.setFeedback(0.2f);
    phaserLeft.setMix(0.0f);
    phaserRight.setMix(0.0f);

    delayLeft.prepare(spec);
    delayRight.prepare(spec);
    delayLeft.reset();
    delayRight.reset();
    delayFeedbackStateL = 0.0f;
    delayFeedbackStateR = 0.0f;

    reverb.reset();
    outputLimiter.prepare({ sampleRate, 1024u, static_cast<juce::uint32>(juce::jmax(1, getTotalNumOutputChannels())) });
    outputLimiter.setThreshold(-0.8f);
    outputLimiter.setRelease(40.0f);

    arpHeldNotes.fill(false);
    arpPhysicalHeld.fill(false);
    arpHeldVelocities.fill(0.0f);
    arpHeldCount = 0;
    arpCurrentStepIndex = 0;
    arpCurrentNote = -1;
    arpSamplesUntilNextStep = 0;
    arpSamplesUntilGateOff = -1;
    arpWasEnabled = false;
    arpDirectionForward = true;
    arpWasLatch = false;
    arpLastRandomIndex = -1;
    arpVisualStepCounter = 0;
    uiArpPhase.store(0.0f);
    uiArpStep.store(0);
    uiArpActive.store(false);
    uiHostBpm.store(120.0f);
    uiOutputLevel.store(0.0f);
    subPhase = 0.0f;
    subEnvFollower = 0.0f;
    fxMotionPhaseA = fxMotionRandom.nextFloat() * juce::MathConstants<float>::twoPi;
    fxMotionPhaseB = fxMotionRandom.nextFloat() * juce::MathConstants<float>::twoPi;
    fxMotionPhaseC = fxMotionRandom.nextFloat() * juce::MathConstants<float>::twoPi;
    fxRandomDrift = 0.0f;
    fxPulseEnv = 0.0f;
    fxPulseSamplesRemaining = 0;
}

void FourOscProAudioProcessor::releaseResources()
{
}

bool FourOscProAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

int FourOscProAudioProcessor::getArpStepSamples(double hostBpm) const
{
    const int divisionChoice = static_cast<int>(parameters.getRawParameterValue(ParamIDs::arpDivision)->load());
    const float quarterNoteSeconds = 60.0f / static_cast<float>(juce::jmax(1.0, hostBpm));
    const float stepSeconds = quarterNoteSeconds * getArpSyncMultiplier(divisionChoice);
    return juce::jmax(1, static_cast<int>(std::round(stepSeconds * static_cast<float>(currentSampleRate))));
}

void FourOscProAudioProcessor::stopArpNote(juce::MidiBuffer& midi, int sampleOffset)
{
    if (arpCurrentNote >= 0)
    {
        midi.addEvent(juce::MidiMessage::noteOff(1, arpCurrentNote), sampleOffset);
        arpCurrentNote = -1;
        arpSamplesUntilGateOff = -1;
    }
}

void FourOscProAudioProcessor::setArpHookEnabled(const juce::String& paramId, bool enabled, float depth)
{
    const float clampedDepth = juce::jlimit(0.01f, 0.48f, depth);
    const juce::ScopedLock lock(arpHookLock);

    for (auto& h : arpHooks)
    {
        if (h.paramId != paramId)
            continue;

        if (! enabled)
        {
            if (h.baseLatched)
            {
                if (auto* p = parameters.getParameter(paramId))
                    p->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, h.baseNorm));
            }
            h.baseLatched = false;
            h.hooked = false;
            h.depth = clampedDepth;
            return;
        }

        h.hooked = true;
        h.depth = clampedDepth;
        return;
    }

    if (enabled)
    {
        ArpHookState h;
        h.paramId = paramId;
        h.hooked = true;
        h.depth = clampedDepth;
        h.phaseOffset = static_cast<float>((paramId.hashCode() & 0xFFFF) / 65535.0) * juce::MathConstants<float>::twoPi;
        arpHooks.push_back(h);
    }
}

bool FourOscProAudioProcessor::isArpHookEnabled(const juce::String& paramId) const
{
    const juce::ScopedLock lock(arpHookLock);
    for (const auto& h : arpHooks)
    {
        if (h.paramId == paramId)
            return h.hooked;
    }
    return false;
}

void FourOscProAudioProcessor::clearArpHooks()
{
    const juce::ScopedLock lock(arpHookLock);
    for (auto& h : arpHooks)
    {
        if (h.baseLatched)
        {
            if (auto* p = parameters.getParameter(h.paramId))
                p->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, h.baseNorm));
        }
        h.baseLatched = false;
        h.hooked = false;
    }
}

void FourOscProAudioProcessor::applyArpHookMotionAudioThread(bool arpMotionActive, float arpPhase)
{
    const juce::ScopedLock lock(arpHookLock);
    const float twoPi = juce::MathConstants<float>::twoPi;
    for (auto& h : arpHooks)
    {
        if (! h.hooked)
            continue;

        auto* p = parameters.getParameter(h.paramId);
        if (p == nullptr)
            continue;

        if (! arpMotionActive)
        {
            if (h.baseLatched)
                p->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, h.baseNorm));
            h.baseLatched = false;
            continue;
        }

        if (! h.baseLatched)
        {
            h.baseNorm = p->getValue();
            h.baseLatched = true;
        }

        const bool isPanParam = h.paramId.endsWithIgnoreCase("Pan");
        const bool isRepeatParam = h.paramId == "delayMix" || h.paramId == "reverbMix"
                                || h.paramId == "postSaturation" || h.paramId == "phaserMix";

        const float baseLfo = std::sin(twoPi * arpPhase + h.phaseOffset);
        // Soft pseudo-random movement: combines unrelated low-rate oscillators.
        const float randomLfo = 0.66f * std::sin(twoPi * (arpPhase * 0.37f) + h.phaseOffset * 1.73f)
                              + 0.34f * std::sin(twoPi * (arpPhase * 1.91f) + h.phaseOffset * 0.41f);
        float motion = isPanParam ? (0.58f * baseLfo + 0.42f * randomLfo) : baseLfo;

        // Optional pulse/repeat accents for motion texture.
        if (isRepeatParam)
        {
            const float pulseRate = 5.0f + std::fmod(h.phaseOffset * 3.1f, 4.0f);
            const float pulseCarrier = std::sin(twoPi * (arpPhase * pulseRate) + h.phaseOffset * 1.9f);
            const float gateLfo = std::sin(twoPi * (arpPhase * 0.27f) + h.phaseOffset * 5.3f);
            const float gate = (gateLfo > 0.15f) ? 1.0f : 0.0f;
            const float pulse = gate * juce::jlimit(0.0f, 1.0f, (pulseCarrier - 0.78f) * 4.5f);
            motion += pulse * 0.6f;
        }

        const float motionScale = isPanParam ? 0.72f : 1.0f;
        const float targetNorm = juce::jlimit(0.0f, 1.0f, h.baseNorm + motion * h.depth * motionScale);
        p->setValueNotifyingHost(targetNorm);
    }
}

juce::MidiBuffer FourOscProAudioProcessor::buildArpMidi(const juce::MidiBuffer& inputMidi, int numSamples, double hostBpm)
{
    juce::MidiBuffer out;
    std::vector<std::pair<int, juce::MidiMessage>> events;
    events.reserve(64);

    {
        juce::MidiBuffer::Iterator it(inputMidi);
        juce::MidiMessage msg;
        int samplePosition = 0;
        while (it.getNextEvent(msg, samplePosition))
            events.emplace_back(samplePosition, msg);
    }

    const int stepSamples = getArpStepSamples(hostBpm);
    const float gate = juce::jlimit(0.05f, 0.98f, parameters.getRawParameterValue(ParamIDs::arpGate)->load());
    const int gateSamples = juce::jlimit(1, stepSamples - 1, static_cast<int>(std::round(static_cast<float>(stepSamples) * gate)));
    const bool latchEnabled = parameters.getRawParameterValue(ParamIDs::arpLatch)->load() >= 0.5f;
    const int modeChoice = static_cast<int>(parameters.getRawParameterValue(ParamIDs::arpMode)->load());
    const int octavesChoice = static_cast<int>(parameters.getRawParameterValue(ParamIDs::arpOctaves)->load());
    const int octaveRange = juce::jlimit(1, 3, octavesChoice + 1);

    if (! latchEnabled && arpWasLatch)
    {
        arpHeldCount = 0;
        for (int note = 0; note < 128; ++note)
        {
            const bool held = arpPhysicalHeld[static_cast<size_t>(note)];
            arpHeldNotes[static_cast<size_t>(note)] = held;
            if (! held)
                arpHeldVelocities[static_cast<size_t>(note)] = 0.0f;
            if (held)
                ++arpHeldCount;
        }
    }

    size_t eventIndex = 0;
    for (int sample = 0; sample < numSamples; ++sample)
    {
        while (eventIndex < events.size() && events[eventIndex].first == sample)
        {
            const auto& msg = events[eventIndex].second;
            if (msg.isNoteOn())
            {
                const int note = msg.getNoteNumber();
                const bool alreadyPhysicallyHeld = arpPhysicalHeld[static_cast<size_t>(note)];
                arpPhysicalHeld[static_cast<size_t>(note)] = true;

                if (latchEnabled)
                {
                    // Latch: first press toggles note in/out of arp set, key release doesn't remove it.
                    if (! alreadyPhysicallyHeld)
                    {
                        if (arpHeldNotes[static_cast<size_t>(note)])
                        {
                            arpHeldNotes[static_cast<size_t>(note)] = false;
                            arpHeldVelocities[static_cast<size_t>(note)] = 0.0f;
                            if (arpHeldCount > 0)
                                --arpHeldCount;
                        }
                        else
                        {
                            arpHeldNotes[static_cast<size_t>(note)] = true;
                            arpHeldVelocities[static_cast<size_t>(note)] = msg.getFloatVelocity();
                            ++arpHeldCount;
                        }
                    }
                }
                else
                {
                    if (! arpHeldNotes[static_cast<size_t>(note)])
                        ++arpHeldCount;
                    arpHeldNotes[static_cast<size_t>(note)] = true;
                    arpHeldVelocities[static_cast<size_t>(note)] = msg.getFloatVelocity();
                }
            }
            else if (msg.isNoteOff())
            {
                const int note = msg.getNoteNumber();
                arpPhysicalHeld[static_cast<size_t>(note)] = false;
                if (! latchEnabled)
                {
                    if (arpHeldNotes[static_cast<size_t>(note)] && arpHeldCount > 0)
                        --arpHeldCount;
                    arpHeldNotes[static_cast<size_t>(note)] = false;
                    arpHeldVelocities[static_cast<size_t>(note)] = 0.0f;
                }
            }
            else
            {
                out.addEvent(msg, sample);
            }

            ++eventIndex;
        }

        if (arpHeldCount <= 0 && arpCurrentNote >= 0)
            stopArpNote(out, sample);

        if (arpSamplesUntilGateOff == 0)
            stopArpNote(out, sample);

        if (arpSamplesUntilNextStep <= 0)
        {
            if (arpHeldCount > 0)
            {
                std::vector<int> heldNotes;
                heldNotes.reserve(static_cast<size_t>(arpHeldCount));
                for (int note = 0; note < 128; ++note)
                {
                    if (arpHeldNotes[static_cast<size_t>(note)])
                        heldNotes.push_back(note);
                }

                if (! heldNotes.empty())
                {
                    std::vector<int> sequenceNotes;
                    std::vector<float> sequenceVels;
                    sequenceNotes.reserve(heldNotes.size() * static_cast<size_t>(octaveRange));
                    sequenceVels.reserve(sequenceNotes.capacity());
                    for (int octave = 0; octave < octaveRange; ++octave)
                    {
                        const int semitones = octave * 12;
                        for (int baseNote : heldNotes)
                        {
                            const int note = baseNote + semitones;
                            if (note < 128)
                            {
                                sequenceNotes.push_back(note);
                                sequenceVels.push_back(arpHeldVelocities[static_cast<size_t>(baseNote)]);
                            }
                        }
                    }

                    if (! sequenceNotes.empty())
                    {
                        const int sequenceSize = static_cast<int>(sequenceNotes.size());
                        int selectedIndex = 0;

                        switch (modeChoice)
                        {
                            case 1: // Down
                            {
                                if (arpCurrentStepIndex <= 0 || arpCurrentStepIndex >= sequenceSize)
                                    arpCurrentStepIndex = sequenceSize - 1;
                                selectedIndex = arpCurrentStepIndex;
                                arpCurrentStepIndex = (arpCurrentStepIndex - 1 + sequenceSize) % sequenceSize;
                                break;
                            }
                            case 2: // UpDown
                            {
                                if (sequenceSize <= 1)
                                {
                                    arpCurrentStepIndex = 0;
                                    selectedIndex = 0;
                                }
                                else
                                {
                                    arpCurrentStepIndex = juce::jlimit(0, sequenceSize - 1, arpCurrentStepIndex);
                                    selectedIndex = arpCurrentStepIndex;

                                    if (arpDirectionForward)
                                    {
                                        if (arpCurrentStepIndex >= sequenceSize - 1)
                                        {
                                            arpDirectionForward = false;
                                            --arpCurrentStepIndex;
                                        }
                                        else
                                        {
                                            ++arpCurrentStepIndex;
                                        }
                                    }
                                    else
                                    {
                                        if (arpCurrentStepIndex <= 0)
                                        {
                                            arpDirectionForward = true;
                                            ++arpCurrentStepIndex;
                                        }
                                        else
                                        {
                                            --arpCurrentStepIndex;
                                        }
                                    }
                                }
                                break;
                            }
                            case 3: // Random
                            {
                                selectedIndex = arpRandom.nextInt(sequenceSize);
                                if (sequenceSize > 1 && selectedIndex == arpLastRandomIndex)
                                    selectedIndex = (selectedIndex + 1) % sequenceSize;
                                arpLastRandomIndex = selectedIndex;
                                break;
                            }
                            default: // Up
                            {
                                if (arpCurrentStepIndex >= sequenceSize || arpCurrentStepIndex < 0)
                                    arpCurrentStepIndex = 0;
                                selectedIndex = arpCurrentStepIndex;
                                arpCurrentStepIndex = (arpCurrentStepIndex + 1) % sequenceSize;
                                break;
                            }
                        }

                        stopArpNote(out, sample);
                        const int note = sequenceNotes[static_cast<size_t>(selectedIndex)];
                        const float vel = juce::jlimit(0.05f, 1.0f, sequenceVels[static_cast<size_t>(selectedIndex)] > 0.0f
                                                                        ? sequenceVels[static_cast<size_t>(selectedIndex)]
                                                                        : 0.8f);
                        out.addEvent(juce::MidiMessage::noteOn(1, note, vel), sample);
                        arpCurrentNote = note;
                        arpSamplesUntilGateOff = gateSamples;
                        arpSamplesUntilNextStep = stepSamples;
                        arpVisualStepCounter = (arpVisualStepCounter + 1) % 16;
                        uiArpStep.store(arpVisualStepCounter);
                    }
                }
            }
            else
            {
                arpCurrentStepIndex = 0;
                arpSamplesUntilNextStep = 0;
                arpDirectionForward = true;
                arpLastRandomIndex = -1;
            }
        }

        if (arpSamplesUntilNextStep > 0)
            --arpSamplesUntilNextStep;
        if (arpSamplesUntilGateOff > 0)
            --arpSamplesUntilGateOff;
    }

    const float phase = stepSamples > 0 ? (1.0f - static_cast<float>(arpSamplesUntilNextStep) / static_cast<float>(stepSamples)) : 0.0f;
    uiArpPhase.store(juce::jlimit(0.0f, 1.0f, phase));
    uiArpActive.store(arpHeldCount > 0);
    arpWasLatch = latchEnabled;
    return out;
}

void FourOscProAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    double hostBpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo position;
        if (playHead->getCurrentPosition(position) && position.bpm > 0.0)
            hostBpm = position.bpm;
    }
    uiHostBpm.store(static_cast<float>(hostBpm));
    const float quarterNoteSeconds = 60.0f / static_cast<float>(hostBpm);
    const float dtBlock = static_cast<float>(buffer.getNumSamples()) / static_cast<float>(juce::jmax(1.0, currentSampleRate));
    const float fxSpaceBase = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::fxSpace)->load());
    const float madAmount = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::madnezz)->load());
    const float movement = juce::jlimit(0.0f, 1.0f, fxSpaceBase * 0.62f + madAmount * 0.48f);
    const float twoPi = juce::MathConstants<float>::twoPi;

    fxMotionPhaseA += twoPi * (0.045f + 0.20f * movement) * dtBlock;
    fxMotionPhaseB += twoPi * (0.015f + 0.11f * movement) * dtBlock;
    fxMotionPhaseC += twoPi * (0.19f + 0.72f * movement) * dtBlock;
    if (fxMotionPhaseA > twoPi) fxMotionPhaseA -= twoPi;
    if (fxMotionPhaseB > twoPi) fxMotionPhaseB -= twoPi;
    if (fxMotionPhaseC > twoPi) fxMotionPhaseC -= twoPi;

    fxRandomDrift += (fxMotionRandom.nextFloat() * 2.0f - 1.0f) * 0.055f * movement;
    fxRandomDrift *= 0.985f;
    fxRandomDrift = juce::jlimit(-1.0f, 1.0f, fxRandomDrift);

    if (fxPulseSamplesRemaining <= 0)
    {
        const float triggerProb = (0.002f + movement * 0.010f) * (static_cast<float>(buffer.getNumSamples()) / 256.0f);
        if (fxMotionRandom.nextFloat() < triggerProb)
        {
            fxPulseSamplesRemaining = static_cast<int>((0.010f + fxMotionRandom.nextFloat() * 0.055f) * currentSampleRate);
            fxPulseEnv = 0.14f + 0.64f * fxMotionRandom.nextFloat();
        }
    }
    if (fxPulseSamplesRemaining > 0)
    {
        fxPulseSamplesRemaining -= buffer.getNumSamples();
        fxPulseEnv *= 0.90f;
    }
    else
    {
        fxPulseEnv *= 0.84f;
    }
    fxPulseEnv = juce::jlimit(0.0f, 1.0f, fxPulseEnv);

    const float motionLfo = 0.58f * std::sin(fxMotionPhaseA)
                          + 0.30f * std::sin(fxMotionPhaseB + 0.66f * std::sin(fxMotionPhaseC))
                          + 0.12f * std::sin(fxMotionPhaseC);
    const float motionRand = juce::jlimit(-1.0f, 1.0f, motionLfo * 0.72f + fxRandomDrift * 0.28f);

    const bool arpEnabled = parameters.getRawParameterValue(ParamIDs::arpEnabled)->load() >= 0.5f;
    juce::MidiBuffer midiForSynth;
    if (arpEnabled)
    {
        midiForSynth = buildArpMidi(midiMessages, buffer.getNumSamples(), hostBpm);
    }
    else
    {
        if (arpWasEnabled)
        {
            stopArpNote(midiMessages, 0);
            arpCurrentStepIndex = 0;
        }

        juce::MidiBuffer::Iterator it(midiMessages);
        juce::MidiMessage msg;
        int samplePosition = 0;
        while (it.getNextEvent(msg, samplePosition))
        {
            if (msg.isNoteOn())
            {
                const int note = msg.getNoteNumber();
                arpPhysicalHeld[static_cast<size_t>(note)] = true;
                if (! arpHeldNotes[static_cast<size_t>(note)])
                    ++arpHeldCount;
                arpHeldNotes[static_cast<size_t>(note)] = true;
                arpHeldVelocities[static_cast<size_t>(note)] = msg.getFloatVelocity();
            }
            else if (msg.isNoteOff())
            {
                const int note = msg.getNoteNumber();
                arpPhysicalHeld[static_cast<size_t>(note)] = false;
                if (arpHeldNotes[static_cast<size_t>(note)] && arpHeldCount > 0)
                    --arpHeldCount;
                arpHeldNotes[static_cast<size_t>(note)] = false;
                arpHeldVelocities[static_cast<size_t>(note)] = 0.0f;
            }
        }

        arpWasLatch = false;
        uiArpActive.store(false);
        uiArpPhase.store(0.0f);
        midiForSynth = midiMessages;
    }
    arpWasEnabled = arpEnabled;
    applyArpHookMotionAudioThread(arpEnabled && uiArpActive.load(), juce::jlimit(0.0f, 1.0f, uiArpPhase.load()));

    synth.renderNextBlock(buffer, midiForSynth, 0, buffer.getNumSamples());

    // Global sub oscillator mixed from a fixed-frequency sine, amplitude-gated by synth output energy.
    const float subAmount = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::subAmount)->load());
    if (subAmount > 0.0001f)
    {
        const float subFreq = juce::jlimit(30.0f, 150.0f, parameters.getRawParameterValue(ParamIDs::subFrequencyHz)->load());
        const float inc = juce::MathConstants<float>::twoPi * subFreq / static_cast<float>(currentSampleRate);
        auto* leftSub = buffer.getWritePointer(0);
        auto* rightSub = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dryL = leftSub[i];
            const float dryR = (rightSub != nullptr ? rightSub[i] : dryL);
            const float inputMag = 0.5f * (std::abs(dryL) + std::abs(dryR));
            subEnvFollower = 0.995f * subEnvFollower + 0.005f * inputMag;
            const float gate = juce::jlimit(0.0f, 1.0f, subEnvFollower * 4.0f);
            const float sub = std::sin(subPhase) * (0.28f * subAmount) * gate;

            leftSub[i] += sub;
            if (rightSub != nullptr)
                rightSub[i] += sub;

            subPhase += inc;
            if (subPhase > juce::MathConstants<float>::twoPi)
                subPhase -= juce::MathConstants<float>::twoPi;
        }
    }

    const float chorusRate = parameters.getRawParameterValue(ParamIDs::chorusRate)->load();
    const float chorusDepth = parameters.getRawParameterValue(ParamIDs::chorusDepth)->load();
    const float chorusMix = parameters.getRawParameterValue(ParamIDs::chorusMix)->load();
    float effectiveChorusRate = chorusRate;
    const bool chorusSyncEnabled = parameters.getRawParameterValue(ParamIDs::chorusSync)->load() >= 0.5f;
    if (chorusSyncEnabled)
    {
        const int chorusSyncChoice = static_cast<int>(parameters.getRawParameterValue(ParamIDs::chorusSyncDivision)->load());
        const float cycleSeconds = juce::jmax(0.001f, quarterNoteSeconds * getChorusSyncMultiplier(chorusSyncChoice));
        effectiveChorusRate = juce::jlimit(0.05f, 8.0f, 1.0f / cycleSeconds);
    }

    const float chorusRateMod = juce::jlimit(0.05f, 8.0f, effectiveChorusRate * (1.0f + movement * 0.30f * motionRand));
    const float chorusDepthMod = juce::jlimit(0.0f, 1.0f, chorusDepth + movement * 0.12f * motionRand + fxPulseEnv * 0.08f);
    const float chorusMixMod = juce::jlimit(0.0f, 1.0f, chorusMix + movement * 0.08f * (0.5f + 0.5f * std::sin(fxMotionPhaseB)) + fxPulseEnv * 0.05f);
    chorusLeft.setRate(chorusRateMod);
    chorusRight.setRate(chorusRateMod * 1.03f);
    chorusLeft.setDepth(chorusDepthMod);
    chorusRight.setDepth(juce::jlimit(0.0f, 1.0f, chorusDepthMod * 0.95f));
    chorusLeft.setMix(chorusMixMod);
    chorusRight.setMix(chorusMixMod);

    auto* left = buffer.getWritePointer(0);
    {
        auto block = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(0);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        chorusLeft.process(context);
    }
    if (buffer.getNumChannels() > 1)
    {
        auto block = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(1);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        chorusRight.process(context);
    }

    const float phaserSpace = fxSpaceBase;
    const float phaserMix = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::phaserMix)->load() * phaserSpace);
    if (phaserMix > 0.0001f)
    {
        const float phaserRate = (0.18f + phaserMix * 1.7f) * (1.0f + movement * 0.42f * (0.4f * motionRand + 0.6f * std::sin(fxMotionPhaseC)));
        phaserLeft.setRate(juce::jlimit(0.05f, 8.0f, phaserRate));
        phaserRight.setRate(juce::jlimit(0.05f, 8.0f, phaserRate * 1.04f));
        phaserLeft.setDepth(juce::jlimit(0.0f, 1.0f, 0.55f + phaserMix * 0.38f + movement * 0.10f * motionRand));
        phaserRight.setDepth(juce::jlimit(0.0f, 1.0f, 0.60f + phaserMix * 0.33f - movement * 0.08f * motionRand));
        phaserLeft.setFeedback(juce::jlimit(0.0f, 0.95f, 0.12f + phaserMix * 0.45f + fxPulseEnv * 0.08f));
        phaserRight.setFeedback(juce::jlimit(0.0f, 0.95f, 0.16f + phaserMix * 0.42f + fxPulseEnv * 0.06f));
        phaserLeft.setMix(phaserMix);
        phaserRight.setMix(phaserMix);

        {
            auto block = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(0);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            phaserLeft.process(context);
        }
        if (buffer.getNumChannels() > 1)
        {
            auto block = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(1);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            phaserRight.process(context);
        }
    }

    const float fxSpace = fxSpaceBase;
    float effectiveDelayTimeMs = parameters.getRawParameterValue(ParamIDs::delayTimeMs)->load();
    const bool delaySyncEnabled = parameters.getRawParameterValue(ParamIDs::delaySync)->load() >= 0.5f;
    if (delaySyncEnabled)
    {
        const int delaySyncChoice = static_cast<int>(parameters.getRawParameterValue(ParamIDs::delaySyncDivision)->load());
        effectiveDelayTimeMs = quarterNoteSeconds * 1000.0f * getDelaySyncMultiplier(delaySyncChoice);
    }

    effectiveDelayTimeMs = juce::jmax(1.0f, effectiveDelayTimeMs * (1.0f + movement * 0.075f * motionRand));
    const float delayFeedback = juce::jlimit(0.0f, 0.97f, parameters.getRawParameterValue(ParamIDs::delayFeedback)->load()
                                                          + movement * 0.10f * motionRand + fxPulseEnv * 0.08f);
    const float delayWet = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::delayMix)->load() * fxSpace
                                                     + movement * 0.08f * (0.5f + 0.5f * std::sin(fxMotionPhaseA))
                                                     + fxPulseEnv * 0.08f);
    const float delayDry = 1.0f - delayWet;
    const float delaySamplesL = juce::jlimit(1.0f, 191999.0f, effectiveDelayTimeMs * 0.001f * static_cast<float>(currentSampleRate));
    const float delaySamplesR = juce::jlimit(1.0f, 191999.0f, delaySamplesL * 1.013f);
    delayLeft.setDelay(delaySamplesL);
    delayRight.setDelay(delaySamplesR);

    if (buffer.getNumChannels() > 1)
    {
        auto* right = buffer.getWritePointer(1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float inL = left[i];
            const float inR = right[i];
            const float delayedL = delayLeft.popSample(0);
            const float delayedR = delayRight.popSample(0);

            delayFeedbackStateL = inL + delayedL * delayFeedback;
            delayFeedbackStateR = inR + delayedR * delayFeedback;
            delayLeft.pushSample(0, delayFeedbackStateL);
            delayRight.pushSample(0, delayFeedbackStateR);

            left[i] = inL * delayDry + delayedL * delayWet;
            right[i] = inR * delayDry + delayedR * delayWet;
        }
    }
    else
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float in = left[i];
            const float delayed = delayLeft.popSample(0);
            delayFeedbackStateL = in + delayed * delayFeedback;
            delayLeft.pushSample(0, delayFeedbackStateL);
            left[i] = in * delayDry + delayed * delayWet;
        }
    }

    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::reverbSize)->load()
                                                      + movement * 0.10f * (0.5f + 0.5f * std::sin(fxMotionPhaseB)));
    reverbParams.damping = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::reverbDamping)->load()
                                                     + movement * 0.08f * (0.5f + 0.5f * std::sin(fxMotionPhaseC + 1.1f)));
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    reverbParams.wetLevel = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::reverbMix)->load() * fxSpace
                                                      + movement * 0.08f * (0.5f + 0.5f * std::sin(fxMotionPhaseA + 0.4f))
                                                      + fxPulseEnv * 0.05f);
    reverbParams.dryLevel = 1.0f - reverbParams.wetLevel;
    reverb.setParameters(reverbParams);

    {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        reverb.process(context);
    }

    const float postSat = juce::jlimit(0.0f, 1.0f, parameters.getRawParameterValue(ParamIDs::postSaturation)->load());
    if (postSat > 0.0001f)
    {
        const float satDrive = 1.0f + postSat * 4.5f;
        const float satNorm = 1.0f / std::tanh(satDrive);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* d = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                const float dry = d[i];
                const float wet = std::tanh(dry * satDrive) * satNorm;
                d[i] = dry + postSat * (wet - dry);
            }
        }
    }

    const auto gainDb = parameters.getRawParameterValue(ParamIDs::masterGain)->load();
    buffer.applyGain(juce::Decibels::decibelsToGain(gainDb));

    const bool limiterEnabled = parameters.getRawParameterValue(ParamIDs::limiterOn)->load() >= 0.5f;
    if (limiterEnabled)
    {
        const float limiterThresholdDb = parameters.getRawParameterValue(ParamIDs::limiterThresholdDb)->load();
        outputLimiter.setThreshold(limiterThresholdDb);
        outputLimiter.setRelease(55.0f);
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);
        outputLimiter.process(context);
    }

    // UI meter for audio-reactive visuals in the editor.
    float sumSquares = 0.0f;
    int sampleCount = 0;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        const auto* channelData = buffer.getReadPointer(ch);
        const int n = buffer.getNumSamples();
        for (int i = 0; i < n; ++i)
            sumSquares += channelData[i] * channelData[i];
        sampleCount += n;
    }
    const float rms = (sampleCount > 0) ? std::sqrt(sumSquares / static_cast<float>(sampleCount)) : 0.0f;
    const float normalized = juce::jlimit(0.0f, 1.0f, std::pow(rms * 2.4f, 0.6f));
    uiOutputLevel.store(normalized);
}

bool FourOscProAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FourOscProAudioProcessor::createEditor()
{
    return new FourOscProAudioProcessorEditor(*this);
}

void FourOscProAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    const auto state = parameters.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void FourOscProAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
    {
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FourOscProAudioProcessor();
}
