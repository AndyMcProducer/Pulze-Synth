#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class FourOscProAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit FourOscProAudioProcessorEditor(FourOscProAudioProcessor&);
    ~FourOscProAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void visibilityChanged() override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    class NeonLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        NeonLookAndFeel();
        void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional,
                              float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
        void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void styleKnob(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void styleToggle(juce::ToggleButton& button, const juce::String& text);
    void styleCombo(juce::ComboBox& box);
    void updateOscTabVisibility();
    void applyFactoryPreset(int presetIndex);
    void buildPresetList();
    void showPresetMenu();
    void selectPresetIndex(int presetIndex);
    void selectRandomPreset();
    void stepPresetByCategory(int direction);
    void applySkin(int skinIndex);
    juce::File getUserPresetDirectory() const;
    void saveCurrentAsUserPreset();
    bool loadUserPresetFromFile(const juce::File& presetFile);
    void registerArpHookTarget(juce::Component& component, const juce::String& paramId, float defaultDepth);
    void setArpHookEnabled(const juce::String& paramId, bool enabled, float depthOverride = -1.0f);
    void clearArpHooks();
    void applyArpHookMotion();
    void applyMotionHookPresetConfig(int presetIndex);
    juce::Rectangle<int> getArpMeterBounds() const;
    juce::Colour getArpModeColor(int arpMode) const;
    void spawnSparkBurst(int step, juce::Colour modeColor);

    struct Spark
    {
        juce::Point<float> pos;
        juce::Point<float> vel;
        juce::Colour color;
        float life = 0.0f;
    };
    struct ArpHookTarget
    {
        juce::Component* component = nullptr;
        juce::String paramId;
        float defaultDepth = 0.15f;
        float depth = 0.15f;
        bool hooked = false;
        bool baseLatched = false;
        float baseNorm = 0.0f;
        float phaseOffset = 0.0f;
    };

    FourOscProAudioProcessor& processor;
    NeonLookAndFeel neonLookAndFeel;
    juce::Random uiRandom;
    std::array<Spark, 40> sparks {};
    int lastVisualStep = -1;
    bool lastVisualActive = false;
    float panelBreathPhase = 0.0f;
    float panelBreath = 0.0f;
    float bootSequenceT = 0.0f;
    bool bootSequenceDone = false;
    float randomDicePressAnim = 0.0f;
    juce::Image diceIconImage;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label presetLabel;
    juce::Label skinLabel;
    juce::Label glowLabel;
    juce::Label synthSectionLabel;
    juce::Label oscSectionLabel;
    juce::Label arpSectionLabel;
    juce::Label fxSectionLabel;

    juce::Slider masterGainSlider;
    juce::Slider driveSlider;
    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;
    juce::Slider madnezzSlider;
    juce::Slider arpGateSlider;
    juce::Slider subAmountSlider;
    juce::Slider subFreqSlider;
    juce::Slider fxSpaceSlider;
    juce::Slider delayMixSlider;
    juce::Slider reverbMixSlider;
    juce::Slider driftSlider;
    juce::Slider bassCompSlider;
    juce::Slider analogWarmSlider;
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;
    juce::Slider unisonDetuneSlider;
    juce::Slider stereoSpreadSlider;
    juce::Slider monoWidthSlider;
    juce::Slider vibratoDepthSlider;
    juce::Slider vibratoRateSlider;
    juce::Slider pitchEnvTimeSlider;
    juce::Slider panSweepAmountSlider;
    juce::Slider panSweepRateSlider;
    juce::Slider chorusRateSlider;
    juce::Slider chorusDepthSlider;
    juce::Slider chorusMixSlider;
    juce::Slider phaserMixSlider;
    juce::Slider delayTimeSlider;
    juce::Slider delayFeedbackSlider;
    juce::Slider reverbSizeSlider;
    juce::Slider reverbDampingSlider;
    juce::Slider postSatSlider;
    juce::Slider limiterThresholdSlider;
    juce::Slider glowIntensitySlider;

    juce::Label masterGainLabel;
    juce::Label driveLabel;
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label madnezzLabel;
    juce::Label arpGateLabel;
    juce::Label subAmountLabel;
    juce::Label subFreqLabel;
    juce::Label fxSpaceLabel;
    juce::Label delayMixLabel;
    juce::Label reverbMixLabel;
    juce::Label driftLabel;
    juce::Label bassCompLabel;
    juce::Label analogWarmLabel;
    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;
    juce::Label unisonDetuneLabel;
    juce::Label stereoSpreadLabel;
    juce::Label monoWidthLabel;
    juce::Label vibratoDepthLabel;
    juce::Label vibratoRateLabel;
    juce::Label pitchEnvTimeLabel;
    juce::Label panSweepAmountLabel;
    juce::Label panSweepRateLabel;
    juce::Label chorusRateLabel;
    juce::Label chorusDepthLabel;
    juce::Label chorusMixLabel;
    juce::Label phaserMixLabel;
    juce::Label delayTimeLabel;
    juce::Label delayFeedbackLabel;
    juce::Label reverbSizeLabel;
    juce::Label reverbDampingLabel;
    juce::Label postSatLabel;
    juce::Label limiterThresholdLabel;
    std::array<juce::Label, 4> oscHeadLabels;
    std::array<juce::Label, 4> oscWaveLabels;
    std::array<juce::Label, 4> oscOctaveLabels;
    std::array<juce::Label, 4> oscSemitoneLabels;
    std::array<juce::Label, 4> oscLevelLabels;
    std::array<juce::Label, 4> oscTuneLabels;
    std::array<juce::Label, 4> oscPanLabels;
    std::array<juce::Label, 4> oscPhaseLabels;
    std::array<juce::Label, 4> oscPitchEnvLabels;
    std::array<juce::Label, 4> oscPitchShapeLabels;

    juce::ToggleButton arpEnabledButton;
    juce::ToggleButton arpLatchButton;
    juce::ToggleButton retriggerButton;
    juce::ToggleButton delaySyncButton;
    juce::ToggleButton chorusSyncButton;
    juce::ToggleButton limiterOnButton;
    std::array<juce::ToggleButton, 5> oscTabButtons;
    int selectedOscTab = 4;

    juce::TextButton presetBox;
    juce::TextButton presetPrevButton;
    juce::TextButton presetNextButton;
    juce::TextButton presetRandomButton;
    juce::ComboBox unisonVoicesBox;
    juce::ComboBox skinBox;
    juce::ComboBox arpDivisionBox;
    juce::ComboBox arpModeBox;
    juce::ComboBox arpOctavesBox;
    juce::ComboBox delayDivisionBox;
    juce::ComboBox chorusDivisionBox;
    std::array<juce::ComboBox, 4> oscWaveBoxes;
    std::array<juce::ComboBox, 4> oscOctaveBoxes;
    std::array<juce::ComboBox, 4> oscPitchShapeBoxes;
    std::array<juce::Slider, 4> oscLevelSliders;
    std::array<juce::Slider, 4> oscSemitoneSliders;
    std::array<juce::Slider, 4> oscTuneSliders;
    std::array<juce::Slider, 4> oscPanSliders;
    std::array<juce::Slider, 4> oscPhaseSliders;
    std::array<juce::Slider, 4> oscPitchEnvSliders;

    std::unique_ptr<SliderAttachment> masterGainAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> cutoffAttachment;
    std::unique_ptr<SliderAttachment> resonanceAttachment;
    std::unique_ptr<SliderAttachment> madnezzAttachment;
    std::unique_ptr<SliderAttachment> arpGateAttachment;
    std::unique_ptr<SliderAttachment> subAmountAttachment;
    std::unique_ptr<SliderAttachment> subFreqAttachment;
    std::unique_ptr<SliderAttachment> fxSpaceAttachment;
    std::unique_ptr<SliderAttachment> delayMixAttachment;
    std::unique_ptr<SliderAttachment> reverbMixAttachment;
    std::unique_ptr<SliderAttachment> driftAttachment;
    std::unique_ptr<SliderAttachment> bassCompAttachment;
    std::unique_ptr<SliderAttachment> analogWarmAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    std::unique_ptr<SliderAttachment> unisonDetuneAttachment;
    std::unique_ptr<SliderAttachment> stereoSpreadAttachment;
    std::unique_ptr<SliderAttachment> monoWidthAttachment;
    std::unique_ptr<SliderAttachment> vibratoDepthAttachment;
    std::unique_ptr<SliderAttachment> vibratoRateAttachment;
    std::unique_ptr<SliderAttachment> pitchEnvTimeAttachment;
    std::unique_ptr<SliderAttachment> panSweepAmountAttachment;
    std::unique_ptr<SliderAttachment> panSweepRateAttachment;
    std::unique_ptr<SliderAttachment> chorusRateAttachment;
    std::unique_ptr<SliderAttachment> chorusDepthAttachment;
    std::unique_ptr<SliderAttachment> chorusMixAttachment;
    std::unique_ptr<SliderAttachment> phaserMixAttachment;
    std::unique_ptr<SliderAttachment> delayTimeAttachment;
    std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<SliderAttachment> reverbSizeAttachment;
    std::unique_ptr<SliderAttachment> reverbDampingAttachment;
    std::unique_ptr<SliderAttachment> postSatAttachment;
    std::unique_ptr<SliderAttachment> limiterThresholdAttachment;

    std::unique_ptr<ButtonAttachment> arpEnabledAttachment;
    std::unique_ptr<ButtonAttachment> arpLatchAttachment;
    std::unique_ptr<ButtonAttachment> retriggerAttachment;
    std::unique_ptr<ButtonAttachment> delaySyncAttachment;
    std::unique_ptr<ButtonAttachment> chorusSyncAttachment;
    std::unique_ptr<ButtonAttachment> limiterOnAttachment;

    std::unique_ptr<ComboAttachment> unisonVoicesAttachment;
    std::unique_ptr<ComboAttachment> arpDivisionAttachment;
    std::unique_ptr<ComboAttachment> arpModeAttachment;
    std::unique_ptr<ComboAttachment> arpOctavesAttachment;
    std::unique_ptr<ComboAttachment> delayDivisionAttachment;
    std::unique_ptr<ComboAttachment> chorusDivisionAttachment;
    std::array<std::unique_ptr<ComboAttachment>, 4> oscWaveAttachments;
    std::array<std::unique_ptr<ComboAttachment>, 4> oscOctaveAttachments;
    std::array<std::unique_ptr<ComboAttachment>, 4> oscPitchShapeAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 4> oscLevelAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 4> oscSemitoneAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 4> oscTuneAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 4> oscPanAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 4> oscPhaseAttachments;
    std::array<std::unique_ptr<SliderAttachment>, 4> oscPitchEnvAttachments;

    juce::StringArray allPresetNames;
    int currentPresetIndex = 0;
    float glowIntensity = 1.2f;
    float audioGlowEnv = 0.0f;
    float audioGlowVisual = 0.0f;
    float audioGlowHoldSeconds = 0.0f;
    std::vector<ArpHookTarget> arpHookTargets;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FourOscProAudioProcessorEditor)
};
