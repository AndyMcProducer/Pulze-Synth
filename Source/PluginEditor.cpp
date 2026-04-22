#include "PluginEditor.h"

namespace
{
juce::Colour kPanelBg = juce::Colour::fromRGB(10, 16, 30);
juce::Colour kAccent = juce::Colour::fromRGB(18, 216, 255);
juce::Colour kAccentSoft = juce::Colour::fromRGB(60, 120, 255);
juce::Colour kText = juce::Colour::fromRGB(190, 235, 255);
constexpr auto kUiSkinIndexStateKey = "uiSkinIndex";
constexpr auto kUiPresetIndexStateKey = "uiPresetIndex";
constexpr auto kUserPresetExtension = ".pulzepreset";
constexpr auto kDiceImageAbsolutePath = "E:\\Web stuff\\P2PDAW\\4osc synth\\Assets\\Dice.png";
}

FourOscProAudioProcessorEditor::NeonLookAndFeel::NeonLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, kAccent);
    setColour(juce::Slider::rotarySliderFillColourId, kAccent);
    setColour(juce::Slider::rotarySliderOutlineColourId, kAccentSoft.withAlpha(0.4f));
    setColour(juce::Label::textColourId, kText);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(12, 24, 45));
    setColour(juce::ComboBox::outlineColourId, kAccentSoft.withAlpha(0.6f));
    setColour(juce::ComboBox::textColourId, kText);
}

void FourOscProAudioProcessorEditor::NeonLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height)).reduced(6.0f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    g.setColour(kAccentSoft.withAlpha(0.2f));
    g.fillEllipse(bounds);
    g.setColour(kAccentSoft.withAlpha(0.65f));
    g.drawEllipse(bounds, 1.6f);

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour(kAccent.withAlpha(0.95f));
    g.strokePath(valueArc, juce::PathStrokeType(2.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.addRoundedRectangle(-1.6f, -radius + 8.0f, 3.2f, radius * 0.52f, 1.6f);
    g.setColour(kText);
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void FourOscProAudioProcessorEditor::NeonLookAndFeel::drawToggleButton(
    juce::Graphics& g, juce::ToggleButton& button, bool, bool)
{
    const auto text = button.getButtonText().trim();
    const bool isTopTab = (text == "1" || text == "2" || text == "3" || text == "4" || text == "FX");
    if (isTopTab)
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        const bool on = button.getToggleState();

        if (on)
        {
            juce::ColourGradient grad(juce::Colour::fromRGB(24, 58, 92), bounds.getX(), bounds.getY(),
                                      juce::Colour::fromRGB(11, 33, 59), bounds.getX(), bounds.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, 2.0f);
            g.setColour(kAccent.withAlpha(0.95f));
            g.drawRoundedRectangle(bounds, 2.0f, 1.55f);
        }
        else
        {
            g.setColour(juce::Colour::fromRGB(8, 18, 34));
            g.fillRoundedRectangle(bounds, 2.0f);
            g.setColour(kAccentSoft.withAlpha(0.2f));
            g.drawRoundedRectangle(bounds, 2.0f, 0.8f);
        }

        // Active underline + glow for segmented-tab feel.
        if (on)
        {
            auto underline = bounds.withY(bounds.getBottom() - 4.0f).withHeight(2.2f).reduced(6.0f, 0.0f);
            g.setColour(kAccent.withAlpha(0.32f));
            g.fillRoundedRectangle(underline.expanded(0.0f, 1.6f), 1.8f);
            g.setColour(kAccent.withAlpha(0.95f));
            g.fillRoundedRectangle(underline, 1.3f);
        }

        g.setColour(on ? juce::Colour::fromRGB(120, 238, 255) : kText.withAlpha(0.76f));
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.drawFittedText(text, button.getLocalBounds(), juce::Justification::centred, 1);
        return;
    }

    auto bounds = button.getLocalBounds().toFloat().reduced(1.5f);
    g.setColour(juce::Colour::fromRGB(15, 28, 48));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(button.getToggleState() ? kAccent : kAccentSoft.withAlpha(0.45f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.4f);
    g.setColour(button.getToggleState() ? kAccent : kText.withAlpha(0.8f));
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
}

FourOscProAudioProcessorEditor::FourOscProAudioProcessorEditor(FourOscProAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&neonLookAndFeel);

    titleLabel.setText("PULZE", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setColour(juce::Label::textColourId, kAccent);
    titleLabel.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Subtractive Synth", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.85f));
    subtitleLabel.setFont(juce::FontOptions(14.0f));
    addAndMakeVisible(subtitleLabel);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.85f));
    presetLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    addAndMakeVisible(presetLabel);

    presetBox.setButtonText("INIT - Init");
    presetBox.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(12, 24, 45));
    presetBox.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(16, 32, 58));
    presetBox.setColour(juce::TextButton::textColourOffId, kText);
    presetBox.setColour(juce::TextButton::textColourOnId, kText);
    presetBox.onClick = [this] { showPresetMenu(); };
    addAndMakeVisible(presetBox);

    presetPrevButton.setButtonText("<");
    presetPrevButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(12, 24, 45));
    presetPrevButton.setColour(juce::TextButton::textColourOffId, kText);
    presetPrevButton.onClick = [this]
    {
        if (allPresetNames.isEmpty())
            return;
        stepPresetByCategory(-1);
    };
    addAndMakeVisible(presetPrevButton);

    presetNextButton.setButtonText(">");
    presetNextButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(12, 24, 45));
    presetNextButton.setColour(juce::TextButton::textColourOffId, kText);
    presetNextButton.onClick = [this]
    {
        if (allPresetNames.isEmpty())
            return;
        stepPresetByCategory(1);
    };
    addAndMakeVisible(presetNextButton);

    presetRandomButton.setButtonText("");
    presetRandomButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetRandomButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    presetRandomButton.setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    presetRandomButton.setColour(juce::TextButton::textColourOnId, juce::Colours::transparentBlack);
    presetRandomButton.onClick = [this]
    {
        randomDicePressAnim = 1.0f;
        selectRandomPreset();
    };
    addAndMakeVisible(presetRandomButton);
    diceIconImage = juce::ImageFileFormat::loadFrom(juce::File(kDiceImageAbsolutePath));

    skinLabel.setText("SKIN", juce::dontSendNotification);
    skinLabel.setJustificationType(juce::Justification::centredRight);
    skinLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(skinLabel);

    styleCombo(skinBox);
    skinBox.addItemList(juce::StringArray { "Blue", "Green", "Purple", "Amber" }, 1);
    skinBox.onChange = [this]
    {
        applySkin(juce::jmax(0, skinBox.getSelectedItemIndex()));
    };
    skinBox.setSelectedItemIndex(0, juce::dontSendNotification);

    glowLabel.setText("GLOW", juce::dontSendNotification);
    glowLabel.setJustificationType(juce::Justification::centredRight);
    glowLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    addAndMakeVisible(glowLabel);

    glowIntensitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    glowIntensitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 18);
    glowIntensitySlider.setRange(0.4, 1.8, 0.01);
    glowIntensitySlider.setSkewFactor(1.2);
    glowIntensitySlider.setValue(1.2, juce::dontSendNotification);
    glowIntensitySlider.onValueChange = [this]
    {
        glowIntensity = static_cast<float>(glowIntensitySlider.getValue());
        repaint();
    };
    addAndMakeVisible(glowIntensitySlider);
    glowLabel.setVisible(false);
    glowIntensitySlider.setVisible(false);
    glowIntensitySlider.setEnabled(false);
    glowIntensity = 1.2f;

    buildPresetList();

    synthSectionLabel.setText("SYNTH CORE", juce::dontSendNotification);
    oscSectionLabel.setText("EDIT PAGE", juce::dontSendNotification);
    arpSectionLabel.setText("SYNC ARP", juce::dontSendNotification);
    fxSectionLabel.setText("SPACE FX", juce::dontSendNotification);
    for (auto* label : { &synthSectionLabel, &oscSectionLabel, &arpSectionLabel, &fxSectionLabel })
    {
        label->setColour(juce::Label::textColourId, kAccent.withAlpha(0.95f));
        label->setFont(juce::FontOptions(14.0f, juce::Font::bold));
        addAndMakeVisible(*label);
    }

    styleKnob(masterGainSlider, masterGainLabel, "MASTER");
    styleKnob(driveSlider, driveLabel, "DRIVE");
    styleKnob(cutoffSlider, cutoffLabel, "CUTOFF");
    styleKnob(resonanceSlider, resonanceLabel, "RES");
    styleKnob(madnezzSlider, madnezzLabel, "MADNEZZ");
    styleKnob(arpGateSlider, arpGateLabel, "GATE");
    styleKnob(subAmountSlider, subAmountLabel, "SUB AMT");
    styleKnob(subFreqSlider, subFreqLabel, "SUB FREQ");
    styleKnob(fxSpaceSlider, fxSpaceLabel, "SPACE");
    styleKnob(delayMixSlider, delayMixLabel, "DELAY MIX");
    styleKnob(reverbMixSlider, reverbMixLabel, "REVERB MIX");
    styleKnob(phaserMixSlider, phaserMixLabel, "PHASER");

    auto styleSmall = [this] (juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
        s.setColour(juce::Slider::textBoxTextColourId, kText);
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
        s.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
        addAndMakeVisible(s);

        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centredLeft);
        l.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        l.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(l);
    };

    styleSmall(driftSlider, driftLabel, "DRIFT");
    styleSmall(bassCompSlider, bassCompLabel, "BASS COMP");
    styleSmall(analogWarmSlider, analogWarmLabel, "ANALOG WARM");
    styleSmall(attackSlider, attackLabel, "ATTACK");
    styleSmall(decaySlider, decayLabel, "DECAY");
    styleSmall(sustainSlider, sustainLabel, "SUSTAIN");
    styleSmall(releaseSlider, releaseLabel, "RELEASE");
    styleSmall(unisonDetuneSlider, unisonDetuneLabel, "UNISON DETUNE");
    styleSmall(stereoSpreadSlider, stereoSpreadLabel, "STEREO SPREAD");
    styleSmall(monoWidthSlider, monoWidthLabel, "MONO WIDTH");
    styleSmall(vibratoDepthSlider, vibratoDepthLabel, "VIB DEPTH");
    styleSmall(vibratoRateSlider, vibratoRateLabel, "VIB RATE");
    styleSmall(pitchEnvTimeSlider, pitchEnvTimeLabel, "P ENV TIME");
    styleSmall(panSweepAmountSlider, panSweepAmountLabel, "PAN SWEEP");
    styleSmall(panSweepRateSlider, panSweepRateLabel, "SWEEP RATE");
    styleSmall(chorusRateSlider, chorusRateLabel, "CH RATE");
    styleSmall(chorusDepthSlider, chorusDepthLabel, "CH DEPTH");
    styleSmall(chorusMixSlider, chorusMixLabel, "CH MIX");
    styleSmall(delayTimeSlider, delayTimeLabel, "DLY TIME");
    styleSmall(delayFeedbackSlider, delayFeedbackLabel, "DLY FB");
    styleSmall(reverbSizeSlider, reverbSizeLabel, "REV SIZE");
    styleSmall(reverbDampingSlider, reverbDampingLabel, "REV DAMP");
    styleSmall(postSatSlider, postSatLabel, "POST SAT");
    styleSmall(limiterThresholdSlider, limiterThresholdLabel, "LIM THR");

    styleToggle(arpEnabledButton, "ARP ON");
    styleToggle(arpLatchButton, "LATCH");
    styleToggle(retriggerButton, "RETRIGGER");
    styleToggle(delaySyncButton, "DELAY SYNC");
    styleToggle(chorusSyncButton, "CHORUS SYNC");
    styleToggle(limiterOnButton, "LIMITER");

    styleCombo(unisonVoicesBox);
    styleCombo(arpDivisionBox);
    styleCombo(arpModeBox);
    styleCombo(arpOctavesBox);
    styleCombo(delayDivisionBox);
    styleCombo(chorusDivisionBox);

    unisonVoicesBox.addItemList(juce::StringArray { "Unison: 1", "Unison: 2", "Unison: 4", "Unison: 8" }, 1);
    arpDivisionBox.addItemList(juce::StringArray { "Arp: 1/4", "Arp: 1/8", "Arp: 1/8D", "Arp: 1/8T",
                                                   "Arp: 1/16", "Arp: 1/16D", "Arp: 1/16T", "Arp: 1/32" }, 1);
    arpModeBox.addItemList(juce::StringArray { "Mode: Up", "Mode: Down", "Mode: UpDown", "Mode: Random" }, 1);
    arpOctavesBox.addItemList(juce::StringArray { "Octaves: 1", "Octaves: 2", "Octaves: 3" }, 1);
    delayDivisionBox.addItemList(juce::StringArray { "Delay: 1/1", "Delay: 1/2", "Delay: 1/4", "Delay: 1/8",
                                                     "Delay: 1/8D", "Delay: 1/8T", "Delay: 1/16", "Delay: 1/16D", "Delay: 1/16T" }, 1);
    chorusDivisionBox.addItemList(juce::StringArray { "Chorus: 1/1", "Chorus: 1/2", "Chorus: 1/4", "Chorus: 1/8" }, 1);

    auto& state = processor.getValueTreeState();
    masterGainAttachment = std::make_unique<SliderAttachment>(state, "masterGain", masterGainSlider);
    driveAttachment = std::make_unique<SliderAttachment>(state, "drive", driveSlider);
    cutoffAttachment = std::make_unique<SliderAttachment>(state, "cutoff", cutoffSlider);
    resonanceAttachment = std::make_unique<SliderAttachment>(state, "resonance", resonanceSlider);
    madnezzAttachment = std::make_unique<SliderAttachment>(state, "madnezz", madnezzSlider);
    arpGateAttachment = std::make_unique<SliderAttachment>(state, "arpGate", arpGateSlider);
    subAmountAttachment = std::make_unique<SliderAttachment>(state, "subAmount", subAmountSlider);
    subFreqAttachment = std::make_unique<SliderAttachment>(state, "subFrequencyHz", subFreqSlider);
    fxSpaceAttachment = std::make_unique<SliderAttachment>(state, "fxSpace", fxSpaceSlider);
    delayMixAttachment = std::make_unique<SliderAttachment>(state, "delayMix", delayMixSlider);
    reverbMixAttachment = std::make_unique<SliderAttachment>(state, "reverbMix", reverbMixSlider);
    driftAttachment = std::make_unique<SliderAttachment>(state, "drift", driftSlider);
    bassCompAttachment = std::make_unique<SliderAttachment>(state, "bassCompAmount", bassCompSlider);
    analogWarmAttachment = std::make_unique<SliderAttachment>(state, "analogWarm", analogWarmSlider);
    attackAttachment = std::make_unique<SliderAttachment>(state, "attackMs", attackSlider);
    decayAttachment = std::make_unique<SliderAttachment>(state, "decayMs", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(state, "sustain", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(state, "releaseMs", releaseSlider);
    unisonDetuneAttachment = std::make_unique<SliderAttachment>(state, "unisonDetune", unisonDetuneSlider);
    stereoSpreadAttachment = std::make_unique<SliderAttachment>(state, "stereoSpread", stereoSpreadSlider);
    monoWidthAttachment = std::make_unique<SliderAttachment>(state, "monoWidth", monoWidthSlider);
    vibratoDepthAttachment = std::make_unique<SliderAttachment>(state, "vibratoDepthCents", vibratoDepthSlider);
    vibratoRateAttachment = std::make_unique<SliderAttachment>(state, "vibratoRateHz", vibratoRateSlider);
    pitchEnvTimeAttachment = std::make_unique<SliderAttachment>(state, "pitchEnvTimeMs", pitchEnvTimeSlider);
    panSweepAmountAttachment = std::make_unique<SliderAttachment>(state, "panSweepAmount", panSweepAmountSlider);
    panSweepRateAttachment = std::make_unique<SliderAttachment>(state, "panSweepRateHz", panSweepRateSlider);
    chorusRateAttachment = std::make_unique<SliderAttachment>(state, "chorusRate", chorusRateSlider);
    chorusDepthAttachment = std::make_unique<SliderAttachment>(state, "chorusDepth", chorusDepthSlider);
    chorusMixAttachment = std::make_unique<SliderAttachment>(state, "chorusMix", chorusMixSlider);
    phaserMixAttachment = std::make_unique<SliderAttachment>(state, "phaserMix", phaserMixSlider);
    delayTimeAttachment = std::make_unique<SliderAttachment>(state, "delayTimeMs", delayTimeSlider);
    delayFeedbackAttachment = std::make_unique<SliderAttachment>(state, "delayFeedback", delayFeedbackSlider);
    reverbSizeAttachment = std::make_unique<SliderAttachment>(state, "reverbSize", reverbSizeSlider);
    reverbDampingAttachment = std::make_unique<SliderAttachment>(state, "reverbDamping", reverbDampingSlider);
    postSatAttachment = std::make_unique<SliderAttachment>(state, "postSaturation", postSatSlider);
    limiterThresholdAttachment = std::make_unique<SliderAttachment>(state, "limiterThresholdDb", limiterThresholdSlider);

    arpEnabledAttachment = std::make_unique<ButtonAttachment>(state, "arpEnabled", arpEnabledButton);
    arpLatchAttachment = std::make_unique<ButtonAttachment>(state, "arpLatch", arpLatchButton);
    retriggerAttachment = std::make_unique<ButtonAttachment>(state, "retrigger", retriggerButton);
    delaySyncAttachment = std::make_unique<ButtonAttachment>(state, "delaySync", delaySyncButton);
    chorusSyncAttachment = std::make_unique<ButtonAttachment>(state, "chorusSync", chorusSyncButton);
    limiterOnAttachment = std::make_unique<ButtonAttachment>(state, "limiterOn", limiterOnButton);

    unisonVoicesAttachment = std::make_unique<ComboAttachment>(state, "unisonVoices", unisonVoicesBox);
    arpDivisionAttachment = std::make_unique<ComboAttachment>(state, "arpDivision", arpDivisionBox);
    arpModeAttachment = std::make_unique<ComboAttachment>(state, "arpMode", arpModeBox);
    arpOctavesAttachment = std::make_unique<ComboAttachment>(state, "arpOctaves", arpOctavesBox);
    delayDivisionAttachment = std::make_unique<ComboAttachment>(state, "delaySyncDivision", delayDivisionBox);
    chorusDivisionAttachment = std::make_unique<ComboAttachment>(state, "chorusSyncDivision", chorusDivisionBox);
    for (int i = 0; i < 4; ++i)
    {
        const auto idx = juce::String(i + 1);
        oscWaveAttachments[static_cast<size_t>(i)] = std::make_unique<ComboAttachment>(state, "osc" + idx + "Wave",
                                                                                        oscWaveBoxes[static_cast<size_t>(i)]);
        oscOctaveAttachments[static_cast<size_t>(i)] = std::make_unique<ComboAttachment>(state, "osc" + idx + "Octave",
                                                                                          oscOctaveBoxes[static_cast<size_t>(i)]);
        oscPitchShapeAttachments[static_cast<size_t>(i)] = std::make_unique<ComboAttachment>(state, "osc" + idx + "PitchEnvShape",
                                                                                              oscPitchShapeBoxes[static_cast<size_t>(i)]);
        oscLevelAttachments[static_cast<size_t>(i)] = std::make_unique<SliderAttachment>(state, "osc" + idx + "Level",
                                                                                          oscLevelSliders[static_cast<size_t>(i)]);
        oscSemitoneAttachments[static_cast<size_t>(i)] = std::make_unique<SliderAttachment>(state, "osc" + idx + "Semitone",
                                                                                             oscSemitoneSliders[static_cast<size_t>(i)]);
        oscTuneAttachments[static_cast<size_t>(i)] = std::make_unique<SliderAttachment>(state, "osc" + idx + "TuneCents",
                                                                                         oscTuneSliders[static_cast<size_t>(i)]);
        oscPanAttachments[static_cast<size_t>(i)] = std::make_unique<SliderAttachment>(state, "osc" + idx + "Pan",
                                                                                        oscPanSliders[static_cast<size_t>(i)]);
        oscPhaseAttachments[static_cast<size_t>(i)] = std::make_unique<SliderAttachment>(state, "osc" + idx + "PhaseDeg",
                                                                                          oscPhaseSliders[static_cast<size_t>(i)]);
        oscPitchEnvAttachments[static_cast<size_t>(i)] = std::make_unique<SliderAttachment>(state, "osc" + idx + "PitchEnvSemi",
                                                                                             oscPitchEnvSliders[static_cast<size_t>(i)]);
    }

    for (int i = 0; i < 5; ++i)
    {
        const auto tabText = (i < 4) ? juce::String(i + 1) : juce::String("FX");
        styleToggle(oscTabButtons[static_cast<size_t>(i)], tabText);
        const auto idx = i;
        oscTabButtons[static_cast<size_t>(i)].onClick = [this, idx]
        {
            selectedOscTab = idx;
            updateOscTabVisibility();
        };
    }

    for (int i = 0; i < 4; ++i)
    {
        auto& head = oscHeadLabels[static_cast<size_t>(i)];
        head.setText("OSC " + juce::String(i + 1), juce::dontSendNotification);
        head.setJustificationType(juce::Justification::centredLeft);
        head.setColour(juce::Label::textColourId, kAccent.withAlpha(0.95f));
        head.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        addAndMakeVisible(head);

        auto& waveLabel = oscWaveLabels[static_cast<size_t>(i)];
        waveLabel.setText("WAVE", juce::dontSendNotification);
        waveLabel.setJustificationType(juce::Justification::centredLeft);
        waveLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        waveLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(waveLabel);

        auto& levelLabel = oscLevelLabels[static_cast<size_t>(i)];
        levelLabel.setText("LEVEL", juce::dontSendNotification);
        levelLabel.setJustificationType(juce::Justification::centredLeft);
        levelLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        levelLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(levelLabel);

        auto& octaveLabel = oscOctaveLabels[static_cast<size_t>(i)];
        octaveLabel.setText("OCT", juce::dontSendNotification);
        octaveLabel.setJustificationType(juce::Justification::centredLeft);
        octaveLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        octaveLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(octaveLabel);

        auto& semitoneLabel = oscSemitoneLabels[static_cast<size_t>(i)];
        semitoneLabel.setText("SEMI", juce::dontSendNotification);
        semitoneLabel.setJustificationType(juce::Justification::centredLeft);
        semitoneLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        semitoneLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(semitoneLabel);

        auto& tuneLabel = oscTuneLabels[static_cast<size_t>(i)];
        tuneLabel.setText("TUNE", juce::dontSendNotification);
        tuneLabel.setJustificationType(juce::Justification::centredLeft);
        tuneLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        tuneLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(tuneLabel);

        auto& panLabel = oscPanLabels[static_cast<size_t>(i)];
        panLabel.setText("PAN", juce::dontSendNotification);
        panLabel.setJustificationType(juce::Justification::centredLeft);
        panLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        panLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(panLabel);

        auto& phaseLabel = oscPhaseLabels[static_cast<size_t>(i)];
        phaseLabel.setText("PHASE", juce::dontSendNotification);
        phaseLabel.setJustificationType(juce::Justification::centredLeft);
        phaseLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        phaseLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(phaseLabel);

        auto& pitchEnvLabel = oscPitchEnvLabels[static_cast<size_t>(i)];
        pitchEnvLabel.setText("P ENV", juce::dontSendNotification);
        pitchEnvLabel.setJustificationType(juce::Justification::centredLeft);
        pitchEnvLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        pitchEnvLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(pitchEnvLabel);

        auto& pitchShapeLabel = oscPitchShapeLabels[static_cast<size_t>(i)];
        pitchShapeLabel.setText("P SHP", juce::dontSendNotification);
        pitchShapeLabel.setJustificationType(juce::Justification::centredLeft);
        pitchShapeLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.86f));
        pitchShapeLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        addAndMakeVisible(pitchShapeLabel);

        auto& waveBox = oscWaveBoxes[static_cast<size_t>(i)];
        styleCombo(waveBox);
        waveBox.addItemList(juce::StringArray { "Sine", "Saw", "Square", "Triangle", "White Noise", "Pink Noise", "Brown Noise", "Blue Noise" }, 1);

        auto& octaveBox = oscOctaveBoxes[static_cast<size_t>(i)];
        styleCombo(octaveBox);
        octaveBox.addItemList(juce::StringArray { "-2", "-1", "0", "+1", "+2" }, 1);

        auto& pitchShapeBox = oscPitchShapeBoxes[static_cast<size_t>(i)];
        styleCombo(pitchShapeBox);
        pitchShapeBox.addItemList(juce::StringArray { "Exp", "Linear", "S" }, 1);

        auto& levelSlider = oscLevelSliders[static_cast<size_t>(i)];
        levelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        levelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
        levelSlider.setColour(juce::Slider::textBoxTextColourId, kText);
        levelSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
        levelSlider.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
        addAndMakeVisible(levelSlider);

        auto& semitoneSlider = oscSemitoneSliders[static_cast<size_t>(i)];
        semitoneSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        semitoneSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
        semitoneSlider.setColour(juce::Slider::textBoxTextColourId, kText);
        semitoneSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
        semitoneSlider.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
        addAndMakeVisible(semitoneSlider);

        auto& tuneSlider = oscTuneSliders[static_cast<size_t>(i)];
        tuneSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        tuneSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
        tuneSlider.setColour(juce::Slider::textBoxTextColourId, kText);
        tuneSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
        tuneSlider.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
        addAndMakeVisible(tuneSlider);

        auto& panSlider = oscPanSliders[static_cast<size_t>(i)];
        panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        panSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
        panSlider.setColour(juce::Slider::textBoxTextColourId, kText);
        panSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
        panSlider.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
        addAndMakeVisible(panSlider);

        auto& phaseSlider = oscPhaseSliders[static_cast<size_t>(i)];
        phaseSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        phaseSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
        phaseSlider.setColour(juce::Slider::textBoxTextColourId, kText);
        phaseSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
        phaseSlider.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
        addAndMakeVisible(phaseSlider);

        auto& pitchEnvSlider = oscPitchEnvSliders[static_cast<size_t>(i)];
        pitchEnvSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        pitchEnvSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 56, 18);
        pitchEnvSlider.setColour(juce::Slider::textBoxTextColourId, kText);
        pitchEnvSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
        pitchEnvSlider.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
        addAndMakeVisible(pitchEnvSlider);
    }

    // Right-click hook targets for arp-driven macro motion.
    registerArpHookTarget(cutoffSlider, "cutoff", 0.16f);
    registerArpHookTarget(resonanceSlider, "resonance", 0.08f);
    registerArpHookTarget(phaserMixSlider, "phaserMix", 0.22f);
    registerArpHookTarget(delayMixSlider, "delayMix", 0.16f);
    registerArpHookTarget(reverbMixSlider, "reverbMix", 0.14f);
    registerArpHookTarget(postSatSlider, "postSaturation", 0.12f);
    registerArpHookTarget(panSweepAmountSlider, "panSweepAmount", 0.18f);
    registerArpHookTarget(panSweepRateSlider, "panSweepRateHz", 0.12f);
    for (int i = 0; i < 4; ++i)
    {
        registerArpHookTarget(oscPhaseSliders[static_cast<size_t>(i)], "osc" + juce::String(i + 1) + "PhaseDeg", 0.22f);
        registerArpHookTarget(oscPanSliders[static_cast<size_t>(i)], "osc" + juce::String(i + 1) + "Pan", 0.18f);
    }

    updateOscTabVisibility();
    startTimerHz(60);
    setSize(1120, 760);
    bootSequenceDone = true;
    bootSequenceT = 1.25f;
    // Restore UI-only selections from processor state so GUI close/reopen does not reset.
    auto& uiState = processor.getValueTreeState().state;
    const int restoredSkin = juce::jlimit(0, juce::jmax(0, skinBox.getNumItems() - 1),
                                          static_cast<int>(uiState.getProperty(kUiSkinIndexStateKey, 0)));
    skinBox.setSelectedItemIndex(restoredSkin, juce::dontSendNotification);
    applySkin(restoredSkin);

    const int restoredPreset = static_cast<int>(uiState.getProperty(kUiPresetIndexStateKey, 0));
    if (restoredPreset >= 0 && restoredPreset < allPresetNames.size())
    {
        currentPresetIndex = restoredPreset;
        presetBox.setButtonText(allPresetNames[restoredPreset]);
    }
}

FourOscProAudioProcessorEditor::~FourOscProAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void FourOscProAudioProcessorEditor::visibilityChanged()
{
    if (isShowing())
    {
        startTimerHz(60);
        return;
    }

    stopTimer();
    audioGlowEnv = 0.0f;
    audioGlowVisual = 0.0f;
    audioGlowHoldSeconds = 0.0f;
}

void FourOscProAudioProcessorEditor::registerArpHookTarget(juce::Component& component, const juce::String& paramId, float defaultDepth)
{
    ArpHookTarget target;
    target.component = &component;
    target.paramId = paramId;
    target.defaultDepth = juce::jlimit(0.01f, 0.48f, defaultDepth);
    target.depth = target.defaultDepth;
    target.phaseOffset = static_cast<float>((paramId.hashCode() & 0xFFFF) / 65535.0) * juce::MathConstants<float>::twoPi;
    arpHookTargets.push_back(target);
    component.addMouseListener(this, false);
}

void FourOscProAudioProcessorEditor::setArpHookEnabled(const juce::String& paramId, bool enabled, float depthOverride)
{
    for (auto& target : arpHookTargets)
    {
        if (target.paramId != paramId)
            continue;

        const float depth = (depthOverride > 0.0f) ? juce::jlimit(0.01f, 0.48f, depthOverride) : target.defaultDepth;
        target.hooked = enabled;
        target.baseLatched = false;
        target.depth = depth;
        processor.setArpHookEnabled(paramId, enabled, depth);
        return;
    }
}

void FourOscProAudioProcessorEditor::clearArpHooks()
{
    processor.clearArpHooks();
    for (auto& target : arpHookTargets)
    {
        target.hooked = false;
        target.baseLatched = false;
    }
}

void FourOscProAudioProcessorEditor::applyArpHookMotion()
{
    // Hook motion now runs in the processor so it continues with GUI closed.
}

void FourOscProAudioProcessorEditor::applyMotionHookPresetConfig(int presetIndex)
{
    clearArpHooks();
    constexpr int basePresetCount = 44;
    constexpr int motionPresetCount = 50;
    constexpr int motionStart = basePresetCount;
    constexpr int motionEnd = motionStart + motionPresetCount;
    if (presetIndex < motionStart || presetIndex >= motionEnd)
        return;

    const int variant = presetIndex - motionStart;
    const float t = static_cast<float>(variant % 50) / 49.0f;

    setArpHookEnabled("cutoff", true, 0.10f + 0.12f * t);
    setArpHookEnabled("phaserMix", true, 0.12f + 0.16f * t);
    setArpHookEnabled("panSweepAmount", true, 0.09f + 0.11f * t);
    setArpHookEnabled("panSweepRateHz", true, 0.04f + 0.06f * t);

    if ((variant % 2) == 0) setArpHookEnabled("resonance", true, 0.05f + 0.08f * t);
    if ((variant % 3) != 0) setArpHookEnabled("delayMix", true, 0.08f + 0.10f * t);
    if ((variant % 4) != 0) setArpHookEnabled("reverbMix", true, 0.06f + 0.09f * t);
    if ((variant % 5) == 0) setArpHookEnabled("postSaturation", true, 0.05f + 0.08f * t);

    for (int i = 0; i < 4; ++i)
    {
        if (((variant + i) % 2) == 0)
            setArpHookEnabled("osc" + juce::String(i + 1) + "PhaseDeg", true, 0.14f + 0.14f * t);
        const float panDepth = juce::jlimit(0.05f, 0.30f, 0.07f + 0.05f * static_cast<float>(i) + 0.10f * t);
        setArpHookEnabled("osc" + juce::String(i + 1) + "Pan", true, panDepth);
    }
}

void FourOscProAudioProcessorEditor::mouseUp(const juce::MouseEvent& event)
{
    if (! event.mods.isPopupMenu())
        return;

    juce::Component* clicked = event.eventComponent;
    for (auto& target : arpHookTargets)
    {
        if (target.component != clicked)
            continue;

        const bool hookedNow = processor.isArpHookEnabled(target.paramId);
        target.hooked = hookedNow;
        juce::PopupMenu menu;
        menu.addItem(1, hookedNow ? "Unhook From ARP" : "Hook To ARP");
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(clicked),
                           [this, paramId = target.paramId, hookedNow] (int result)
                           {
                               if (result == 1)
                                   setArpHookEnabled(paramId, ! hookedNow);
                           });
        return;
    }
}

void FourOscProAudioProcessorEditor::styleKnob(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    slider.setColour(juce::Slider::textBoxTextColourId, kText);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(12, 24, 45));
    slider.setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.5f));
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, kText.withAlpha(0.92f));
    label.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    addAndMakeVisible(label);
}

void FourOscProAudioProcessorEditor::styleToggle(juce::ToggleButton& button, const juce::String& text)
{
    button.setButtonText(text);
    addAndMakeVisible(button);
}

void FourOscProAudioProcessorEditor::styleCombo(juce::ComboBox& box)
{
    box.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(box);
}

void FourOscProAudioProcessorEditor::applySkin(int skinIndex)
{
    skinIndex = juce::jlimit(0, juce::jmax(0, skinBox.getNumItems() - 1), skinIndex);
    switch (skinIndex)
    {
        case 1: // Green
            kPanelBg = juce::Colour::fromRGB(8, 24, 18);
            kAccent = juce::Colour::fromRGB(46, 255, 170);
            kAccentSoft = juce::Colour::fromRGB(24, 190, 120);
            kText = juce::Colour::fromRGB(196, 255, 226);
            break;
        case 2: // Purple
            kPanelBg = juce::Colour::fromRGB(14, 12, 30);
            kAccent = juce::Colour::fromRGB(196, 92, 255);
            kAccentSoft = juce::Colour::fromRGB(126, 88, 255);
            kText = juce::Colour::fromRGB(236, 220, 255);
            break;
        case 3: // Amber
            kPanelBg = juce::Colour::fromRGB(30, 17, 8);
            kAccent = juce::Colour::fromRGB(255, 184, 48);
            kAccentSoft = juce::Colour::fromRGB(255, 122, 34);
            kText = juce::Colour::fromRGB(255, 236, 206);
            break;
        default: // Blue
            kPanelBg = juce::Colour::fromRGB(10, 16, 30);
            kAccent = juce::Colour::fromRGB(18, 216, 255);
            kAccentSoft = juce::Colour::fromRGB(60, 120, 255);
            kText = juce::Colour::fromRGB(190, 235, 255);
            break;
    }

    // Refresh LookAndFeel colours used by sliders/combos.
    neonLookAndFeel.setColour(juce::Slider::thumbColourId, kAccent);
    neonLookAndFeel.setColour(juce::Slider::rotarySliderFillColourId, kAccent);
    neonLookAndFeel.setColour(juce::Slider::rotarySliderOutlineColourId, kAccentSoft.withAlpha(0.42f));
    neonLookAndFeel.setColour(juce::Label::textColourId, kText);
    neonLookAndFeel.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(12, 24, 45));
    neonLookAndFeel.setColour(juce::ComboBox::outlineColourId, kAccentSoft.withAlpha(0.75f));
    neonLookAndFeel.setColour(juce::ComboBox::textColourId, kText);

    titleLabel.setColour(juce::Label::textColourId, kAccent);
    subtitleLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.85f));
    presetLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.9f));
    skinLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.9f));
    glowLabel.setColour(juce::Label::textColourId, kText.withAlpha(0.9f));
    for (auto* label : { &synthSectionLabel, &oscSectionLabel, &arpSectionLabel, &fxSectionLabel })
        label->setColour(juce::Label::textColourId, kAccent.withAlpha(0.98f));

    presetBox.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(12, 24, 45));
    presetBox.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(16, 32, 58));
    presetBox.setColour(juce::TextButton::textColourOffId, kText);
    presetBox.setColour(juce::TextButton::textColourOnId, kText);
    for (auto* b : { &presetPrevButton, &presetNextButton })
    {
        b->setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(12, 24, 45));
        b->setColour(juce::TextButton::textColourOffId, kText);
    }
    presetRandomButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    presetRandomButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    presetRandomButton.setColour(juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    presetRandomButton.setColour(juce::TextButton::textColourOnId, juce::Colours::transparentBlack);

    for (auto* s : { &masterGainSlider, &driveSlider, &cutoffSlider, &resonanceSlider, &madnezzSlider, &arpGateSlider, &subAmountSlider, &subFreqSlider,
                     &fxSpaceSlider, &delayMixSlider, &reverbMixSlider, &driftSlider, &bassCompSlider, &analogWarmSlider, &attackSlider,
                     &decaySlider, &sustainSlider, &releaseSlider, &unisonDetuneSlider, &stereoSpreadSlider, &monoWidthSlider, &vibratoDepthSlider,
                     &vibratoRateSlider, &pitchEnvTimeSlider,
                     &panSweepAmountSlider, &panSweepRateSlider, &chorusRateSlider, &chorusDepthSlider, &chorusMixSlider, &phaserMixSlider,
                     &delayTimeSlider, &delayFeedbackSlider, &reverbSizeSlider, &reverbDampingSlider, &postSatSlider, &limiterThresholdSlider,
                     &glowIntensitySlider })
    {
        s->setColour(juce::Slider::textBoxTextColourId, kText);
        s->setColour(juce::Slider::textBoxOutlineColourId, kAccentSoft.withAlpha(0.7f));
    }

    processor.getValueTreeState().state.setProperty(kUiSkinIndexStateKey, skinIndex, nullptr);
    repaint();
}

void FourOscProAudioProcessorEditor::buildPresetList()
{
    allPresetNames = {
        "INIT - Init",
        "BASS - Neon Bass",
        "BASS - Mono Punch Bass",
        "BASS - Acid Line",
        "BASS - Deep Sub Motion",
        "BASS - Rubber Sub",
        "BASS - Grit Reese",
        "BASS - Dark Analog",
        "BASS - Wide House Bass",
        "KEYS - Chiptune Keys",
        "KEYS - Wide Chorus Keys",
        "KEYS - Glass Keys",
        "KEYS - Electric Pluck Keys",
        "KEYS - Hollow Metallic",
        "PAD - Space Pad",
        "PAD - Soft Tape Pad",
        "PAD - Ambient Cloud",
        "PAD - Shimmer Blanket",
        "PAD - Warm Poly",
        "LEAD - SuperSaw Lead",
        "LEAD - Laser Mono",
        "LEAD - Soft Lead",
        "LEAD - Bright Sync",
        "PLUCK - Wide Pluck",
        "PLUCK - Glass Bell",
        "PLUCK - Pulse Stab",
        "PLUCK - Reso Pluck",
        "PLUCK - Nylon Hit",
        "ARP - Stereo Sweep Arp",
        "ARP - Trance Gate",
        "ARP - Random Motion",
        "ARP - Octave Runner",
        "MOTION - Pan Orbit",
        "DRONE - Dark Drone",
        "FX - Lo-Fi Poly",
        "FX - Frozen Space",
        "DRONE - Abyss Bloom",
        "DRONE - Magnetar Choir",
        "DRONE - Slow Glass Field",
        "DRONE - Cathedral Fog",
        "DRONE - Layered Orbit",
        "DRONE - Rusted Nebula",
        "DRONE - Hollow Machine Sky",
        "DRONE - Sub Polar Drift"
    };

    const juce::StringArray motionHookNames {
        "MOTION - Orbit Bloom",
        "MOTION - Neon Drift",
        "MOTION - Polar Sweep",
        "MOTION - Phase Ladder",
        "MOTION - Glass Rotor",
        "MOTION - Pan Lattice",
        "MOTION - Cosmic Gate",
        "MOTION - Slow Tension",
        "MOTION - Ribbon Pulse",
        "MOTION - Wide Haze",
        "MOTION - Noisy Halo",
        "MOTION - Blue Arc",
        "MOTION - Brown Current",
        "MOTION - Pink Shiver",
        "MOTION - White Mist",
        "MOTION - Spiral Drift",
        "MOTION - Deep Orbit",
        "MOTION - Air Current",
        "MOTION - Frozen Motion",
        "MOTION - Twilight Mod",
        "MOTION - Warp Echo",
        "MOTION - Pulse Matrix",
        "MOTION - Arc Runner",
        "MOTION - Stereo Tide",
        "MOTION - Layer Shift",
        "MOTION - Sub Orbit",
        "MOTION - Phase Tides",
        "MOTION - Mod Cascade",
        "MOTION - Wide Engine",
        "MOTION - Drift Tunnel",
        "MOTION - Motion Alloy",
        "MOTION - Flux Ribbon",
        "MOTION - Rain Field",
        "MOTION - Shadow Glide",
        "MOTION - Crystal Sweep",
        "MOTION - Neon Flare",
        "MOTION - Static Bloom",
        "MOTION - Hollow Axis",
        "MOTION - Motion Dusk",
        "MOTION - Velvet Arc",
        "MOTION - Signal Bloom",
        "MOTION - Phase Dust",
        "MOTION - Tilt Orbit",
        "MOTION - Drift Weave",
        "MOTION - Pan Reactor",
        "MOTION - Arc Prism",
        "MOTION - Aurora Gate",
        "MOTION - Wave Cartel",
        "MOTION - Mod Resonance",
        "MOTION - Endless Rotor"
    };
    allPresetNames.addArray(motionHookNames);

    const juce::StringArray generatedCategories { "BASS", "KEYS", "PAD", "LEAD", "PLUCK", "ARP", "MOTION", "FX" };
    const juce::StringArray adjectives {
        "Neon", "Shadow", "Crystal", "Velvet", "Solar", "Deep", "Bright", "Dusty",
        "Wide", "Cold", "Warm", "Liquid", "Analog", "Digital", "Twisted", "Hypno"
    };
    const juce::StringArray bassNouns   { "Sub", "Rumble", "Driver", "Reese", "Punch", "Growl", "Weight", "Pulse" };
    const juce::StringArray keysNouns   { "Keys", "Chord", "Tone", "Glock", "Comp", "Piano", "Bell", "EP" };
    const juce::StringArray padNouns    { "Pad", "Blanket", "Sky", "Cloud", "Wash", "Choir", "Float", "Aura" };
    const juce::StringArray leadNouns   { "Lead", "Blade", "Ray", "Wire", "Voice", "Line", "Sync", "Solo" };
    const juce::StringArray pluckNouns  { "Pluck", "Stab", "Pick", "Ping", "Click", "Snap", "Bell", "Nylon" };
    const juce::StringArray arpNouns    { "Arp", "Runner", "Stepper", "Gate", "Cascade", "Cycle", "Spiral", "Drive" };
    const juce::StringArray motionNouns { "Orbit", "Sweep", "Drift", "Flow", "Shift", "Spin", "Move", "Warp" };
    const juce::StringArray fxNouns     { "Space", "Texture", "Noise", "Freeze", "Field", "Wash", "Color", "Grain" };

    int generatedIdx = 1;
    while (allPresetNames.size() < 500)
    {
        const int catIndex = (generatedIdx - 1) % generatedCategories.size();
        const auto cat = generatedCategories[catIndex];
        const auto adj = adjectives[(generatedIdx - 1) % adjectives.size()];
        const juce::StringArray* nounSet = &fxNouns;
        switch (catIndex)
        {
            case 0: nounSet = &bassNouns; break;
            case 1: nounSet = &keysNouns; break;
            case 2: nounSet = &padNouns; break;
            case 3: nounSet = &leadNouns; break;
            case 4: nounSet = &pluckNouns; break;
            case 5: nounSet = &arpNouns; break;
            case 6: nounSet = &motionNouns; break;
            default: nounSet = &fxNouns; break;
        }
        const auto noun = (*nounSet)[(generatedIdx / 3) % nounSet->size()];
        const auto number = juce::String(generatedIdx).paddedLeft('0', 3);
        allPresetNames.add(cat + " - " + adj + " " + noun + " " + number);
        ++generatedIdx;
    }
}

void FourOscProAudioProcessorEditor::showPresetMenu()
{
    juce::PopupMenu root;
    root.addItem(1000, "INIT - Init");
    root.addSeparator();

    const juce::StringArray categories { "BASS", "KEYS", "PAD", "LEAD", "PLUCK", "ARP", "MOTION", "DRONE", "FX" };
    for (const auto& category : categories)
    {
        juce::PopupMenu sub;
        for (int i = 0; i < allPresetNames.size(); ++i)
        {
            const auto& name = allPresetNames[i];
            if (name.startsWith(category + " - "))
                sub.addItem(1000 + i, name);
        }
        root.addSubMenu(category + " >", sub, true);
    }

    juce::PopupMenu usersSub;
    usersSub.addItem(2001, "Save Current As...");
    usersSub.addSeparator();
    const auto userDir = getUserPresetDirectory();
    const auto userFiles = userDir.findChildFiles(juce::File::findFiles, false, "*" + juce::String(kUserPresetExtension));
    if (userFiles.isEmpty())
    {
        usersSub.addItem(2002, "(No user presets yet)", false, false);
    }
    else
    {
        for (int i = 0; i < userFiles.size(); ++i)
            usersSub.addItem(3000 + i, "USER - " + userFiles.getReference(i).getFileNameWithoutExtension());
    }
    root.addSubMenu("USERS >", usersSub, true);

    root.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&presetBox).withMinimumWidth(280),
                       [this, userFiles] (int result)
                       {
                           if (result == 2001)
                           {
                               saveCurrentAsUserPreset();
                               return;
                           }
                           if (result >= 3000)
                           {
                               const int idx = result - 3000;
                               if (idx >= 0 && idx < userFiles.size())
                                   loadUserPresetFromFile(userFiles.getReference(idx));
                               return;
                           }
                           if (result < 1000)
                               return;
                           const int presetIndex = result - 1000;
                           if (presetIndex >= 0 && presetIndex < allPresetNames.size())
                               selectPresetIndex(presetIndex);
                       });
}

void FourOscProAudioProcessorEditor::selectPresetIndex(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= allPresetNames.size())
        return;
    currentPresetIndex = presetIndex;
    applyFactoryPreset(presetIndex);
    applyMotionHookPresetConfig(presetIndex);
    presetBox.setButtonText(allPresetNames[presetIndex]);
    processor.getValueTreeState().state.setProperty(kUiPresetIndexStateKey, presetIndex, nullptr);
}

void FourOscProAudioProcessorEditor::stepPresetByCategory(int direction)
{
    if (allPresetNames.isEmpty())
        return;

    const juce::StringArray categoryOrder { "INIT", "BASS", "KEYS", "PAD", "LEAD", "PLUCK", "ARP", "MOTION", "DRONE", "FX" };
    auto getCategory = [] (const juce::String& name)
    {
        return name.upToFirstOccurrenceOf(" - ", false, false);
    };

    juce::Array<int> orderedIndices;
    for (const auto& cat : categoryOrder)
    {
        for (int i = 0; i < allPresetNames.size(); ++i)
        {
            if (getCategory(allPresetNames[i]) == cat)
                orderedIndices.add(i);
        }
    }

    // Any uncategorized entries are appended at the end.
    for (int i = 0; i < allPresetNames.size(); ++i)
    {
        if (! orderedIndices.contains(i))
            orderedIndices.add(i);
    }

    if (orderedIndices.isEmpty())
        return;

    int pos = orderedIndices.indexOf(currentPresetIndex);
    if (pos < 0)
        pos = 0;

    const int step = (direction >= 0) ? 1 : -1;
    const int count = orderedIndices.size();
    const int nextPos = (pos + step + count) % count;
    selectPresetIndex(orderedIndices.getReference(nextPos));
}

void FourOscProAudioProcessorEditor::selectRandomPreset()
{
    if (allPresetNames.isEmpty())
        return;

    constexpr int basePresetCount = 44;
    constexpr int motionPresetCount = 50;
    constexpr int motionEnd = basePresetCount + motionPresetCount;
    const int generatedCount = juce::jmax(0, allPresetNames.size() - motionEnd);

    if (generatedCount <= 0)
    {
        // Fallback: choose an existing preset if generated range is unavailable.
        const int current = juce::jlimit(0, allPresetNames.size() - 1, currentPresetIndex);
        int next = current;
        if (allPresetNames.size() > 1)
        {
            while (next == current)
                next = uiRandom.nextInt(allPresetNames.size());
        }
        selectPresetIndex(next);
        return;
    }

    int nextGenerated = uiRandom.nextInt(generatedCount);
    if (generatedCount > 1)
    {
        const int currentGenerated = juce::jlimit(0, generatedCount - 1, currentPresetIndex - motionEnd);
        while (nextGenerated == currentGenerated)
            nextGenerated = uiRandom.nextInt(generatedCount);
    }

    const int generatedPresetIndex = motionEnd + nextGenerated;
    applyFactoryPreset(generatedPresetIndex);
    clearArpHooks();
    currentPresetIndex = generatedPresetIndex;
    presetBox.setButtonText("RANDOM");
    processor.getValueTreeState().state.setProperty(kUiPresetIndexStateKey, -1, nullptr);
}

juce::File FourOscProAudioProcessorEditor::getUserPresetDirectory() const
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("P2PDAW")
                .getChildFile("Pulze")
                .getChildFile("UserPresets");
    dir.createDirectory();
    return dir;
}

void FourOscProAudioProcessorEditor::saveCurrentAsUserPreset()
{
    auto* alert = new juce::AlertWindow("Save User Preset",
                                        "Enter a name for this preset:",
                                        juce::AlertWindow::NoIcon);
    alert->addTextEditor("name", presetBox.getButtonText()
                                     .upToFirstOccurrenceOf(" - ", false, false)
                                     .replaceCharacters("\\/:*?\"<>|", "__________")
                                     .trim());
    alert->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    alert->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    alert->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, alert] (int result)
        {
            std::unique_ptr<juce::AlertWindow> holder(alert);
            if (result != 1)
                return;

            auto name = alert->getTextEditorContents("name").trim();
            if (name.isEmpty())
                name = "User Preset";
            name = name.replaceCharacters("\\/:*?\"<>|", "__________");
            const auto file = getUserPresetDirectory().getChildFile(name + juce::String(kUserPresetExtension));
            if (auto xml = processor.getValueTreeState().copyState().createXml())
                xml->writeTo(file);
        }), true);
}

bool FourOscProAudioProcessorEditor::loadUserPresetFromFile(const juce::File& presetFile)
{
    std::unique_ptr<juce::XmlElement> xmlState(juce::XmlDocument::parse(presetFile));
    if (xmlState == nullptr)
        return false;
    auto& vts = processor.getValueTreeState();
    if (! xmlState->hasTagName(vts.state.getType()))
        return false;
    vts.replaceState(juce::ValueTree::fromXml(*xmlState));
    clearArpHooks();
    currentPresetIndex = 0;
    presetBox.setButtonText("USER - " + presetFile.getFileNameWithoutExtension());
    processor.getValueTreeState().state.setProperty(kUiPresetIndexStateKey, 0, nullptr);
    return true;
}

void FourOscProAudioProcessorEditor::updateOscTabVisibility()
{
    for (int i = 0; i < 4; ++i)
    {
        oscTabButtons[static_cast<size_t>(i)].setVisible(false);
        oscHeadLabels[static_cast<size_t>(i)].setVisible(true);
        oscWaveLabels[static_cast<size_t>(i)].setVisible(true);
        oscOctaveLabels[static_cast<size_t>(i)].setVisible(true);
        oscSemitoneLabels[static_cast<size_t>(i)].setVisible(true);
        oscLevelLabels[static_cast<size_t>(i)].setVisible(true);
        oscTuneLabels[static_cast<size_t>(i)].setVisible(true);
        oscPanLabels[static_cast<size_t>(i)].setVisible(true);
        oscPhaseLabels[static_cast<size_t>(i)].setVisible(true);
        oscPitchEnvLabels[static_cast<size_t>(i)].setVisible(true);
        oscPitchShapeLabels[static_cast<size_t>(i)].setVisible(true);
        oscWaveBoxes[static_cast<size_t>(i)].setVisible(true);
        oscOctaveBoxes[static_cast<size_t>(i)].setVisible(true);
        oscPitchShapeBoxes[static_cast<size_t>(i)].setVisible(true);
        oscLevelSliders[static_cast<size_t>(i)].setVisible(true);
        oscSemitoneSliders[static_cast<size_t>(i)].setVisible(true);
        oscTuneSliders[static_cast<size_t>(i)].setVisible(true);
        oscPanSliders[static_cast<size_t>(i)].setVisible(true);
        oscPhaseSliders[static_cast<size_t>(i)].setVisible(true);
        oscPitchEnvSliders[static_cast<size_t>(i)].setVisible(true);

        const auto headColour = kAccent.withAlpha(1.0f);
        oscHeadLabels[static_cast<size_t>(i)].setColour(juce::Label::textColourId, headColour);
    }

    oscTabButtons[4].setVisible(false);
    synthSectionLabel.setVisible(true);
    retriggerButton.setVisible(true);
    masterGainLabel.setVisible(true);  masterGainSlider.setVisible(true);
    driveLabel.setVisible(true);       driveSlider.setVisible(true);
    cutoffLabel.setVisible(true);      cutoffSlider.setVisible(true);
    resonanceLabel.setVisible(true);   resonanceSlider.setVisible(true);
    madnezzLabel.setVisible(true);     madnezzSlider.setVisible(true);
    driftLabel.setVisible(true);       driftSlider.setVisible(true);
    bassCompLabel.setVisible(true);    bassCompSlider.setVisible(true);
    analogWarmLabel.setVisible(true);  analogWarmSlider.setVisible(true);
    attackLabel.setVisible(true);      attackSlider.setVisible(true);
    decayLabel.setVisible(true);       decaySlider.setVisible(true);
    sustainLabel.setVisible(true);     sustainSlider.setVisible(true);
    releaseLabel.setVisible(true);     releaseSlider.setVisible(true);
    unisonVoicesBox.setVisible(true);
    unisonDetuneLabel.setVisible(true); unisonDetuneSlider.setVisible(true);
    stereoSpreadLabel.setVisible(true); stereoSpreadSlider.setVisible(true);
    monoWidthLabel.setVisible(true);    monoWidthSlider.setVisible(true);
    vibratoDepthLabel.setVisible(true); vibratoDepthSlider.setVisible(true);
    vibratoRateLabel.setVisible(true);  vibratoRateSlider.setVisible(true);
    pitchEnvTimeLabel.setVisible(true); pitchEnvTimeSlider.setVisible(true);
    panSweepAmountLabel.setVisible(true); panSweepAmountSlider.setVisible(true);
    panSweepRateLabel.setVisible(true);   panSweepRateSlider.setVisible(true);
    arpSectionLabel.setVisible(true);
    arpEnabledButton.setVisible(true);
    arpLatchButton.setVisible(true);
    chorusSyncButton.setVisible(true);
    arpDivisionBox.setVisible(true);
    arpModeBox.setVisible(true);
    arpOctavesBox.setVisible(true);
    chorusDivisionBox.setVisible(true);
    arpGateLabel.setVisible(true);
    arpGateSlider.setVisible(true);
    subAmountLabel.setVisible(true);
    subAmountSlider.setVisible(true);
    subFreqLabel.setVisible(true);
    subFreqSlider.setVisible(true);
    fxSectionLabel.setVisible(true);
    chorusRateLabel.setVisible(true);    chorusRateSlider.setVisible(true);
    chorusDepthLabel.setVisible(true);   chorusDepthSlider.setVisible(true);
    chorusMixLabel.setVisible(true);     chorusMixSlider.setVisible(true);
    limiterOnButton.setVisible(true);
    delaySyncButton.setVisible(true);
    delayDivisionBox.setVisible(true);
    delayTimeLabel.setVisible(true);     delayTimeSlider.setVisible(true);
    delayFeedbackLabel.setVisible(true); delayFeedbackSlider.setVisible(true);
    fxSpaceLabel.setVisible(true);   fxSpaceSlider.setVisible(true);
    delayMixLabel.setVisible(true);  delayMixSlider.setVisible(true);
    reverbMixLabel.setVisible(true); reverbMixSlider.setVisible(true);
    phaserMixLabel.setVisible(true); phaserMixSlider.setVisible(true);
    reverbSizeLabel.setVisible(true);    reverbSizeSlider.setVisible(true);
    reverbDampingLabel.setVisible(true); reverbDampingSlider.setVisible(true);
    postSatLabel.setVisible(true);        postSatSlider.setVisible(true);
    limiterThresholdLabel.setVisible(true); limiterThresholdSlider.setVisible(true);
    oscSectionLabel.setText("OSCILLATORS", juce::dontSendNotification);
}

void FourOscProAudioProcessorEditor::applyFactoryPreset(int presetIndex)
{
    auto& state = processor.getValueTreeState();
    auto setParam = [&state] (const juce::String& id, float value)
    {
        if (auto* p = state.getParameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(value));
    };

    // Always start from a known base.
    setParam("masterGain", -3.0f);
    setParam("drive", 6.0f);
    setParam("cutoff", 8000.0f);
    setParam("resonance", 0.35f);
    setParam("madnezz", 0.0f);
    setParam("bassCompAmount", 0.6f);
    setParam("analogWarm", 0.35f);
    setParam("unisonVoices", 0.0f);
    setParam("unisonDetune", 8.0f);
    setParam("stereoSpread", 0.75f);
    setParam("monoWidth", 1.0f);
    setParam("vibratoDepthCents", 8.0f);
    setParam("vibratoRateHz", 5.2f);
    setParam("pitchEnvTimeMs", 220.0f);
    setParam("panSweepAmount", 0.0f);
    setParam("panSweepRateHz", 0.35f);
    setParam("chorusSync", 0.0f);
    setParam("chorusSyncDivision", 2.0f);
    setParam("chorusRate", 0.35f);
    setParam("chorusDepth", 0.35f);
    setParam("chorusMix", 0.2f);
    setParam("phaserMix", 0.0f);
    setParam("fxSpace", 0.45f);
    setParam("delaySync", 1.0f);
    setParam("delaySyncDivision", 3.0f);
    setParam("delayTimeMs", 340.0f);
    setParam("delayFeedback", 0.34f);
    setParam("delayMix", 0.18f);
    setParam("reverbSize", 0.55f);
    setParam("reverbDamping", 0.35f);
    setParam("reverbMix", 0.2f);
    setParam("postSaturation", 0.15f);
    setParam("limiterOn", 1.0f);
    setParam("limiterThresholdDb", -0.8f);
    setParam("arpEnabled", 0.0f);
    setParam("arpDivision", 4.0f);
    setParam("arpMode", 0.0f);
    setParam("arpOctaves", 0.0f);
    setParam("arpGate", 0.58f);
    setParam("subAmount", 0.0f);
    setParam("subFrequencyHz", 60.0f);
    setParam("osc1Wave", 1.0f); setParam("osc2Wave", 1.0f); setParam("osc3Wave", 1.0f); setParam("osc4Wave", 1.0f);
    setParam("osc1Octave", 2.0f); setParam("osc2Octave", 2.0f); setParam("osc3Octave", 2.0f); setParam("osc4Octave", 2.0f);
    setParam("osc1Level", 0.25f); setParam("osc2Level", 0.25f); setParam("osc3Level", 0.25f); setParam("osc4Level", 0.25f);
    setParam("osc1Semitone", 0.0f); setParam("osc2Semitone", 0.0f); setParam("osc3Semitone", 0.0f); setParam("osc4Semitone", 0.0f);
    setParam("osc1TuneCents", 0.0f); setParam("osc2TuneCents", 0.0f); setParam("osc3TuneCents", 0.0f); setParam("osc4TuneCents", 0.0f);
    setParam("osc1Pan", 0.0f); setParam("osc2Pan", 0.0f); setParam("osc3Pan", 0.0f); setParam("osc4Pan", 0.0f);
    setParam("osc1PhaseDeg", 0.0f); setParam("osc2PhaseDeg", 0.0f); setParam("osc3PhaseDeg", 0.0f); setParam("osc4PhaseDeg", 0.0f);
    setParam("osc1PitchEnvSemi", 0.0f); setParam("osc2PitchEnvSemi", 0.0f); setParam("osc3PitchEnvSemi", 0.0f); setParam("osc4PitchEnvSemi", 0.0f);
    setParam("osc1PitchEnvShape", 0.0f); setParam("osc2PitchEnvShape", 0.0f); setParam("osc3PitchEnvShape", 0.0f); setParam("osc4PitchEnvShape", 0.0f);

    constexpr int basePresetCount = 44;
    constexpr int motionPresetCount = 50;
    constexpr int motionStart = basePresetCount;
    constexpr int motionEnd = motionStart + motionPresetCount;

    if (presetIndex >= motionStart && presetIndex < motionEnd)
    {
        const int motion = presetIndex - motionStart;
        const float t = static_cast<float>(motion % 50) / 49.0f;
        const float m = static_cast<float>(motion);
        setParam("masterGain", -4.2f - 1.9f * t); // loudness trim for motion intensity
        setParam("arpEnabled", 1.0f);
        setParam("arpMode", static_cast<float>((motion % 6 == 0) ? 3 : (motion % 4)));
        setParam("arpDivision", static_cast<float>(3 + (motion % 5)));
        setParam("arpOctaves", static_cast<float>((motion / 7) % 3));
        setParam("arpGate", (motion % 5 == 0) ? (0.2f + 0.14f * (1.0f - t)) : (0.34f + 0.34f * (1.0f - t)));
        setParam("madnezz", 0.12f + 0.22f * t);
        setParam("phaserMix", 0.20f + 0.42f * t);
        setParam("panSweepAmount", 0.16f + 0.62f * t);
        setParam("panSweepRateHz", 0.12f + 0.95f * t);
        setParam("vibratoDepthCents", 14.0f + 24.0f * t);
        setParam("vibratoRateHz", 0.10f + 1.2f * t);
        setParam("pitchEnvTimeMs", 90.0f + 740.0f * (1.0f - t));
        setParam("delaySync", 1.0f);
        setParam("delaySyncDivision", static_cast<float>(2 + (motion % 4)));
        setParam("delayMix", 0.14f + 0.30f * t);
        setParam("reverbMix", 0.16f + 0.34f * t);
        setParam("fxSpace", 0.52f + 0.38f * t);
        setParam("drift", 2.2f + 5.0f * t);
        setParam("analogWarm", 0.42f + 0.42f * t);
        setParam("postSaturation", 0.10f + 0.22f * t);
        setParam("stereoSpread", 0.65f + 0.33f * t);
        setParam("monoWidth", 0.72f + 0.28f * (1.0f - t));
        // Spread osc pans with opposing offsets so pan modulation has width to move through.
        setParam("osc1Pan", -0.58f + 0.12f * std::sin(0.19f * m));
        setParam("osc2Pan",  0.58f + 0.10f * std::sin(0.31f * m + 1.2f));
        setParam("osc3Pan", -0.22f + 0.16f * std::sin(0.27f * m + 2.1f));
        setParam("osc4Pan",  0.22f + 0.18f * std::sin(0.23f * m + 0.8f));
        setParam("osc1PitchEnvSemi", -2.0f - 22.0f * std::abs(std::sin(0.17f * m + 0.4f)));
        setParam("osc2PitchEnvSemi",  1.5f + 18.0f * std::sin(0.13f * m + 1.1f));
        setParam("osc3PitchEnvSemi", -1.0f - 26.0f * std::abs(std::sin(0.21f * m + 2.0f)));
        setParam("osc4PitchEnvSemi",  0.8f + 20.0f * std::sin(0.19f * m + 2.7f));
        setParam("osc1PitchEnvShape", static_cast<float>(motion % 3));
        setParam("osc2PitchEnvShape", static_cast<float>((motion + 1) % 3));
        setParam("osc3PitchEnvShape", static_cast<float>((motion + 2) % 3));
        setParam("osc4PitchEnvShape", static_cast<float>((motion + 1) % 3));
        // Rotate in noise oscillators for stronger motion character.
        const int noiseMode = motion % 4;
        if (noiseMode == 1)
        {
            setParam("osc3Wave", 4.0f); // White
            setParam("osc4Wave", 5.0f); // Pink
        }
        else if (noiseMode == 2)
        {
            setParam("osc2Wave", 6.0f); // Brown
            setParam("osc4Wave", 7.0f); // Blue
        }
        else if (noiseMode == 3)
        {
            setParam("osc1Wave", 5.0f); // Pink
            setParam("osc3Wave", 7.0f); // Blue
        }
        return;
    }

    if (presetIndex >= motionEnd)
    {
        // Generated presets: categorized deterministic variations (high diversity).
        const int generated = presetIndex - motionEnd;
        const int category = generated % 8; // BASS, KEYS, PAD, LEAD, PLUCK, ARP, MOTION, FX
        const int tier = generated / 8;
        const float t = static_cast<float>(tier % 64) / 63.0f;
        const float t2 = static_cast<float>((tier / 3) % 64) / 63.0f;
        const float t3 = static_cast<float>((tier / 7) % 64) / 63.0f;

        // Deterministic pseudo-random source from preset index for uniqueness.
        juce::uint32 seed = 0x9e3779b9u ^ static_cast<juce::uint32>((presetIndex + 1) * 2654435761u);
        auto rand01 = [&seed]()
        {
            seed = seed * 1664525u + 1013904223u;
            return static_cast<float>((seed >> 8) & 0x00ffffffu) / 16777215.0f;
        };
        auto randRange = [&rand01] (float a, float b) { return a + (b - a) * rand01(); };

        // Give every generated preset a different oscillator setup baseline.
        for (int osc = 1; osc <= 4; ++osc)
        {
            const auto idx = juce::String(osc);
            setParam("osc" + idx + "Wave", std::floor(randRange(0.0f, 7.99f)));
            setParam("osc" + idx + "Octave", std::floor(randRange(1.2f, 3.8f)));
            setParam("osc" + idx + "Level", randRange(0.12f, 0.34f));
            setParam("osc" + idx + "Semitone", std::round(randRange(-7.9f, 7.9f)));
            setParam("osc" + idx + "TuneCents", randRange(-12.0f, 12.0f));
            setParam("osc" + idx + "Pan", randRange(-0.7f, 0.7f));
            setParam("osc" + idx + "PhaseDeg", randRange(-170.0f, 170.0f));
            setParam("osc" + idx + "PitchEnvSemi", randRange(-2.0f, 2.0f));
            setParam("osc" + idx + "PitchEnvShape", std::floor(randRange(0.0f, 2.99f)));
        }

        switch (category)
        {
            case 0: // BASS
                setParam("attackMs", 1.0f + 7.0f * t);
                setParam("decayMs", 90.0f + 220.0f * t2);
                setParam("sustain", 0.4f + 0.35f * (1.0f - t));
                setParam("releaseMs", 70.0f + 120.0f * t);
                setParam("drive", 8.0f + 9.0f * t3);
                setParam("cutoff", 500.0f + 1800.0f * t2);
                setParam("resonance", 0.35f + 0.6f * t);
                setParam("monoWidth", 0.0f + 0.2f * t2);
                setParam("analogWarm", 0.45f + 0.45f * t);
                setParam("postSaturation", 0.2f + 0.35f * t2);
                setParam("osc3Level", randRange(0.0f, 0.12f)); setParam("osc4Level", randRange(0.0f, 0.08f));
                break;
            case 1: // KEYS
                setParam("attackMs", 2.0f + 60.0f * t);
                setParam("decayMs", 160.0f + 620.0f * t2);
                setParam("sustain", 0.35f + 0.4f * t);
                setParam("releaseMs", 130.0f + 520.0f * t2);
                setParam("cutoff", 2600.0f + 3800.0f * t);
                setParam("chorusMix", 0.15f + 0.35f * t2);
                setParam("delayMix", 0.08f + 0.2f * t);
                setParam("reverbMix", 0.14f + 0.28f * t2);
                setParam("monoWidth", 0.8f + 0.2f * t);
                break;
            case 2: // PAD
                setParam("attackMs", 380.0f + 1700.0f * t);
                setParam("decayMs", 600.0f + 1500.0f * t2);
                setParam("sustain", 0.68f + 0.22f * t);
                setParam("releaseMs", 1300.0f + 3000.0f * t2);
                setParam("unisonVoices", 1.0f + (t > 0.55f ? 1.0f : 0.0f));
                setParam("unisonDetune", 9.0f + 10.0f * t);
                setParam("fxSpace", 0.65f + 0.35f * t2);
                setParam("chorusMix", 0.25f + 0.35f * t);
                setParam("reverbMix", 0.38f + 0.3f * t2);
                setParam("analogWarm", 0.3f + 0.5f * t);
                break;
            case 3: // LEAD
                setParam("attackMs", 1.0f + 30.0f * t);
                setParam("decayMs", 140.0f + 320.0f * t2);
                setParam("sustain", 0.55f + 0.2f * t);
                setParam("releaseMs", 90.0f + 260.0f * t2);
                setParam("unisonVoices", 2.0f + (t > 0.6f ? 1.0f : 0.0f));
                setParam("unisonDetune", 12.0f + 9.0f * t);
                setParam("drive", 7.0f + 6.0f * t2);
                setParam("cutoff", 2800.0f + 5200.0f * t);
                setParam("postSaturation", 0.15f + 0.25f * t2);
                break;
            case 4: // PLUCK
                setParam("attackMs", 1.0f + 6.0f * t);
                setParam("decayMs", 120.0f + 260.0f * t2);
                setParam("sustain", 0.04f + 0.2f * t);
                setParam("releaseMs", 70.0f + 180.0f * t2);
                setParam("cutoff", 1800.0f + 5000.0f * t);
                setParam("resonance", 0.45f + 0.8f * t2);
                setParam("delayMix", 0.05f + 0.22f * t);
                setParam("reverbMix", 0.08f + 0.24f * t2);
                break;
            case 5: // ARP
                setParam("arpEnabled", 1.0f);
                setParam("arpMode", static_cast<float>((tier / 7) % 4));
                setParam("arpDivision", static_cast<float>(3 + (tier % 4)));
                setParam("arpGate", 0.38f + 0.28f * t);
                setParam("arpOctaves", static_cast<float>((tier / 5) % 3));
                setParam("delaySync", 1.0f);
                setParam("delaySyncDivision", static_cast<float>(3 + (tier % 3)));
                setParam("delayMix", 0.12f + 0.2f * t2);
                setParam("panSweepAmount", 0.25f + 0.55f * t);
                setParam("panSweepRateHz", 0.3f + 2.0f * t2);
                break;
            case 6: // MOTION
                setParam("masterGain", -4.6f - 1.6f * t2);
                setParam("drift", 2.6f + 4.8f * t3);
                setParam("madnezz", 0.14f + 0.26f * t2);
                setParam("vibratoDepthCents", 12.0f + 24.0f * t);
                setParam("vibratoRateHz", 0.12f + 1.2f * t2);
                setParam("pitchEnvTimeMs", 70.0f + 820.0f * (1.0f - t2));
                setParam("osc1PitchEnvSemi", -3.0f - 24.0f * t);
                setParam("osc2PitchEnvSemi",  2.0f + 16.0f * t2);
                setParam("osc3PitchEnvSemi", -2.0f - 28.0f * t3);
                setParam("osc4PitchEnvSemi",  1.0f + 20.0f * t);
                setParam("osc1PitchEnvShape", 0.0f); // Exp
                setParam("osc2PitchEnvShape", 2.0f); // S
                setParam("osc3PitchEnvShape", 1.0f); // Linear
                setParam("osc4PitchEnvShape", 2.0f); // S
                setParam("panSweepAmount", 0.56f + 0.44f * t2);
                setParam("panSweepRateHz", 0.25f + 1.8f * t);
                setParam("chorusMix", 0.24f + 0.42f * t2);
                setParam("phaserMix", 0.18f + 0.36f * t);
                setParam("fxSpace", 0.52f + 0.42f * t);
                setParam("delayMix", 0.18f + 0.28f * t2);
                setParam("reverbMix", 0.24f + 0.34f * t);
                break;
            default: // FX
                setParam("masterGain", -4.3f - 1.5f * t);
                setParam("madnezz", 0.10f + 0.24f * t2);
                setParam("vibratoDepthCents", 8.0f + 18.0f * t);
                setParam("vibratoRateHz", 0.10f + 0.75f * t2);
                setParam("pitchEnvTimeMs", 80.0f + 620.0f * (1.0f - t));
                setParam("osc1PitchEnvSemi", -2.0f - 14.0f * t2);
                setParam("osc2PitchEnvSemi",  1.5f + 10.0f * t);
                setParam("osc3PitchEnvSemi", -1.0f - 16.0f * t3);
                setParam("osc4PitchEnvSemi",  0.8f + 12.0f * t2);
                setParam("osc1PitchEnvShape", 1.0f); // Linear
                setParam("osc2PitchEnvShape", 1.0f); // Linear
                setParam("osc3PitchEnvShape", 0.0f); // Exp
                setParam("osc4PitchEnvShape", 2.0f); // S
                setParam("panSweepAmount", 0.12f + 0.32f * t);
                setParam("panSweepRateHz", 0.08f + 0.62f * t2);
                setParam("postSaturation", 0.16f + 0.50f * t);
                setParam("analogWarm", 0.25f + 0.65f * t2);
                setParam("limiterOn", 1.0f);
                setParam("limiterThresholdDb", -0.6f - 5.0f * t);
                setParam("delayFeedback", 0.28f + 0.5f * t2);
                setParam("reverbSize", 0.35f + 0.6f * t);
                setParam("reverbDamping", 0.1f + 0.8f * t2);
                setParam("phaserMix", 0.16f + 0.34f * t);
                setParam("fxSpace", 0.55f + 0.4f * t);
                break;
        }
        return;
    }

    switch (presetIndex)
    {
        case 1: // BASS - Neon Bass
            setParam("attackMs", 2.0f); setParam("decayMs", 180.0f); setParam("sustain", 0.6f); setParam("releaseMs", 120.0f);
            setParam("monoWidth", 0.1f); setParam("cutoff", 1200.0f); setParam("resonance", 0.55f);
            setParam("drive", 12.0f); setParam("reverbMix", 0.05f); setParam("delayMix", 0.06f); setParam("analogWarm", 0.55f);
            setParam("osc3Level", 0.0f); setParam("osc4Level", 0.0f);
            break;
        case 2: // BASS - Mono Punch Bass
            setParam("drive", 14.0f); setParam("cutoff", 950.0f); setParam("resonance", 0.38f); setParam("monoWidth", 0.0f);
            setParam("attackMs", 1.0f); setParam("decayMs", 120.0f); setParam("sustain", 0.62f); setParam("releaseMs", 90.0f);
            setParam("analogWarm", 0.7f); setParam("postSaturation", 0.42f);
            break;
        case 3: // BASS - Acid Line
            setParam("cutoff", 900.0f); setParam("resonance", 1.05f); setParam("bassCompAmount", 0.9f);
            setParam("osc1Wave", 1.0f); setParam("osc2Level", 0.0f); setParam("osc3Level", 0.0f); setParam("osc4Level", 0.0f);
            setParam("arpEnabled", 1.0f); setParam("arpDivision", 6.0f); setParam("arpMode", 0.0f);
            break;
        case 4: // BASS - Deep Sub Motion
            setParam("osc1Octave", 0.0f); setParam("osc2Octave", 1.0f); setParam("osc3Level", 0.0f); setParam("osc4Level", 0.0f);
            setParam("monoWidth", 0.0f); setParam("cutoff", 720.0f); setParam("analogWarm", 0.82f);
            setParam("panSweepAmount", 0.2f); setParam("panSweepRateHz", 0.15f); setParam("postSaturation", 0.36f);
            break;
        case 5: // BASS - Rubber Sub
            setParam("osc1Wave", 0.0f); setParam("osc2Wave", 1.0f); setParam("osc2Octave", 1.0f);
            setParam("cutoff", 640.0f); setParam("resonance", 0.52f); setParam("drive", 9.0f);
            setParam("attackMs", 3.0f); setParam("decayMs", 150.0f); setParam("sustain", 0.68f); setParam("releaseMs", 130.0f);
            break;
        case 6: // BASS - Grit Reese
            setParam("unisonVoices", 2.0f); setParam("unisonDetune", 18.0f); setParam("stereoSpread", 0.9f);
            setParam("osc1Wave", 1.0f); setParam("osc2Wave", 1.0f); setParam("osc1TuneCents", -7.0f); setParam("osc2TuneCents", 7.0f);
            setParam("cutoff", 1800.0f); setParam("drive", 13.5f); setParam("postSaturation", 0.45f);
            break;
        case 7: // BASS - Dark Analog
            setParam("analogWarm", 0.9f); setParam("postSaturation", 0.2f); setParam("cutoff", 980.0f);
            setParam("resonance", 0.62f); setParam("monoWidth", 0.15f); setParam("fxSpace", 0.25f);
            break;
        case 8: // BASS - Wide House Bass
            setParam("unisonVoices", 1.0f); setParam("unisonDetune", 9.0f); setParam("stereoSpread", 0.75f);
            setParam("cutoff", 1600.0f); setParam("drive", 8.0f); setParam("chorusMix", 0.16f);
            setParam("attackMs", 6.0f); setParam("decayMs", 140.0f); setParam("sustain", 0.72f);
            break;
        case 9: // KEYS - Chiptune Keys
            setParam("osc1Wave", 2.0f); setParam("osc2Wave", 2.0f); setParam("osc3Wave", 2.0f); setParam("osc4Wave", 0.0f);
            setParam("attackMs", 1.0f); setParam("decayMs", 110.0f); setParam("sustain", 0.42f); setParam("releaseMs", 80.0f);
            setParam("monoWidth", 0.15f);
            break;
        case 10: // KEYS - Wide Chorus Keys
            setParam("chorusSync", 0.0f); setParam("chorusRate", 0.85f); setParam("chorusDepth", 0.5f); setParam("chorusMix", 0.48f);
            setParam("stereoSpread", 0.95f); setParam("monoWidth", 1.0f); setParam("delayMix", 0.14f); setParam("reverbMix", 0.24f);
            break;
        case 11: // KEYS - Glass Keys
            setParam("osc1Wave", 0.0f); setParam("osc2Wave", 3.0f); setParam("osc3Wave", 0.0f); setParam("osc4Wave", 3.0f);
            setParam("attackMs", 6.0f); setParam("decayMs", 900.0f); setParam("sustain", 0.2f); setParam("releaseMs", 520.0f);
            setParam("delayMix", 0.24f); setParam("reverbMix", 0.36f);
            break;
        case 12: // KEYS - Electric Pluck Keys
            setParam("attackMs", 1.0f); setParam("decayMs", 300.0f); setParam("sustain", 0.32f); setParam("releaseMs", 180.0f);
            setParam("cutoff", 4100.0f); setParam("resonance", 0.62f); setParam("chorusMix", 0.2f);
            break;
        case 13: // KEYS - Hollow Metallic
            setParam("osc1Wave", 3.0f); setParam("osc2Wave", 2.0f); setParam("osc3Wave", 3.0f); setParam("osc4Level", 0.15f);
            setParam("attackMs", 3.0f); setParam("decayMs", 700.0f); setParam("sustain", 0.28f); setParam("releaseMs", 420.0f);
            setParam("reverbMix", 0.32f); setParam("delayMix", 0.16f);
            break;
        case 14: // PAD - Space Pad
            setParam("attackMs", 680.0f); setParam("decayMs", 800.0f); setParam("sustain", 0.78f); setParam("releaseMs", 1600.0f);
            setParam("unisonVoices", 1.0f); setParam("unisonDetune", 11.0f); setParam("stereoSpread", 0.95f);
            setParam("fxSpace", 0.9f); setParam("reverbMix", 0.55f); setParam("delayMix", 0.38f);
            setParam("chorusMix", 0.38f); setParam("chorusDepth", 0.45f); setParam("analogWarm", 0.32f);
            break;
        case 15: // PAD - Soft Tape Pad
            setParam("analogWarm", 0.85f); setParam("postSaturation", 0.18f);
            setParam("cutoff", 4200.0f); setParam("resonance", 0.32f);
            setParam("attackMs", 900.0f); setParam("decayMs", 1000.0f); setParam("sustain", 0.82f); setParam("releaseMs", 2200.0f);
            setParam("fxSpace", 0.84f); setParam("reverbMix", 0.48f);
            break;
        case 16: // PAD - Ambient Cloud
            setParam("fxSpace", 1.0f); setParam("reverbMix", 0.68f); setParam("delayMix", 0.42f);
            setParam("chorusMix", 0.42f); setParam("chorusDepth", 0.52f);
            setParam("panSweepAmount", 0.34f); setParam("panSweepRateHz", 0.24f);
            setParam("attackMs", 1100.0f); setParam("releaseMs", 2800.0f);
            break;
        case 17: // PAD - Shimmer Blanket
            setParam("attackMs", 1300.0f); setParam("decayMs", 1400.0f); setParam("sustain", 0.86f); setParam("releaseMs", 3200.0f);
            setParam("delaySync", 1.0f); setParam("delaySyncDivision", 2.0f); setParam("delayMix", 0.36f);
            setParam("reverbMix", 0.62f); setParam("reverbSize", 0.9f);
            break;
        case 18: // PAD - Warm Poly
            setParam("analogWarm", 0.7f); setParam("cutoff", 3200.0f); setParam("attackMs", 220.0f);
            setParam("decayMs", 700.0f); setParam("sustain", 0.72f); setParam("releaseMs", 1400.0f);
            setParam("chorusMix", 0.26f); setParam("reverbMix", 0.3f);
            break;
        case 19: // LEAD - SuperSaw Lead
            setParam("unisonVoices", 3.0f); setParam("unisonDetune", 19.0f); setParam("stereoSpread", 1.0f);
            setParam("osc1Wave", 1.0f); setParam("osc2Wave", 1.0f); setParam("osc3Wave", 1.0f); setParam("osc4Wave", 1.0f);
            setParam("attackMs", 8.0f); setParam("decayMs", 260.0f); setParam("sustain", 0.66f);
            setParam("chorusMix", 0.33f); setParam("postSaturation", 0.26f);
            break;
        case 20: // LEAD - Laser Mono
            setParam("monoWidth", 0.0f); setParam("drive", 11.0f); setParam("cutoff", 4600.0f); setParam("resonance", 0.78f);
            setParam("attackMs", 1.0f); setParam("decayMs", 150.0f); setParam("sustain", 0.58f); setParam("releaseMs", 120.0f);
            break;
        case 21: // LEAD - Soft Lead
            setParam("attackMs", 40.0f); setParam("decayMs", 260.0f); setParam("sustain", 0.72f); setParam("releaseMs", 300.0f);
            setParam("chorusMix", 0.22f); setParam("delayMix", 0.12f); setParam("reverbMix", 0.18f);
            break;
        case 22: // LEAD - Bright Sync
            setParam("osc1Wave", 1.0f); setParam("osc2Wave", 2.0f); setParam("osc2Semitone", 12.0f);
            setParam("cutoff", 6800.0f); setParam("resonance", 0.52f); setParam("drive", 9.5f);
            setParam("postSaturation", 0.22f);
            break;
        case 23: // PLUCK - Wide Pluck
            setParam("attackMs", 5.0f); setParam("decayMs", 220.0f); setParam("sustain", 0.25f); setParam("releaseMs", 150.0f);
            setParam("unisonVoices", 2.0f); setParam("unisonDetune", 14.0f); setParam("stereoSpread", 0.95f);
            setParam("cutoff", 6200.0f); setParam("resonance", 0.45f); setParam("postSaturation", 0.2f);
            setParam("osc1TuneCents", -4.0f); setParam("osc2TuneCents", 4.0f);
            break;
        case 24: // PLUCK - Glass Bell
            setParam("osc1Wave", 0.0f); setParam("osc2Wave", 3.0f); setParam("osc3Wave", 0.0f); setParam("osc4Wave", 3.0f);
            setParam("attackMs", 4.0f); setParam("decayMs", 1300.0f); setParam("sustain", 0.0f); setParam("releaseMs", 900.0f);
            setParam("delayMix", 0.28f); setParam("reverbMix", 0.44f); setParam("chorusMix", 0.12f);
            break;
        case 25: // PLUCK - Pulse Stab
            setParam("osc1Wave", 2.0f); setParam("osc2Wave", 2.0f);
            setParam("attackMs", 1.0f); setParam("decayMs", 130.0f); setParam("sustain", 0.0f); setParam("releaseMs", 95.0f);
            setParam("drive", 10.5f); setParam("postSaturation", 0.32f); setParam("reverbMix", 0.08f);
            break;
        case 26: // PLUCK - Reso Pluck
            setParam("attackMs", 2.0f); setParam("decayMs", 160.0f); setParam("sustain", 0.08f); setParam("releaseMs", 100.0f);
            setParam("cutoff", 2200.0f); setParam("resonance", 1.15f); setParam("bassCompAmount", 0.72f);
            setParam("drive", 8.5f); setParam("postSaturation", 0.24f);
            break;
        case 27: // PLUCK - Nylon Hit
            setParam("osc1Wave", 3.0f); setParam("osc2Wave", 0.0f);
            setParam("attackMs", 3.0f); setParam("decayMs", 240.0f); setParam("sustain", 0.16f); setParam("releaseMs", 180.0f);
            setParam("cutoff", 3600.0f); setParam("reverbMix", 0.12f);
            break;
        case 28: // ARP - Stereo Sweep Arp
            setParam("arpEnabled", 1.0f); setParam("arpDivision", 3.0f); setParam("arpMode", 0.0f);
            setParam("panSweepAmount", 0.72f); setParam("panSweepRateHz", 1.8f);
            setParam("delaySync", 1.0f); setParam("delaySyncDivision", 4.0f); setParam("delayMix", 0.25f);
            break;
        case 29: // ARP - Trance Gate
            setParam("arpEnabled", 1.0f); setParam("arpDivision", 4.0f); setParam("arpGate", 0.42f); setParam("arpMode", 0.0f);
            setParam("unisonVoices", 2.0f); setParam("delaySync", 1.0f); setParam("delaySyncDivision", 3.0f);
            setParam("delayMix", 0.24f); setParam("chorusMix", 0.18f);
            break;
        case 30: // ARP - Random Motion
            setParam("drift", 5.0f); setParam("chorusSync", 1.0f); setParam("delaySync", 1.0f);
            setParam("arpEnabled", 1.0f); setParam("arpMode", 3.0f); setParam("arpDivision", 4.0f);
            setParam("fxSpace", 0.62f); setParam("delayMix", 0.32f); setParam("reverbMix", 0.3f);
            setParam("panSweepAmount", 0.5f); setParam("panSweepRateHz", 0.7f);
            setParam("osc1Wave", 2.0f); setParam("osc2Wave", 1.0f); setParam("osc3Wave", 3.0f); setParam("osc4Wave", 0.0f);
            break;
        case 31: // ARP - Octave Runner
            setParam("arpEnabled", 1.0f); setParam("arpDivision", 5.0f); setParam("arpMode", 0.0f); setParam("arpOctaves", 2.0f);
            setParam("arpGate", 0.48f); setParam("delaySync", 1.0f); setParam("delayMix", 0.18f);
            break;
        case 32: // MOTION - Pan Orbit
            setParam("masterGain", -5.0f);
            setParam("panSweepAmount", 0.95f); setParam("panSweepRateHz", 0.55f);
            setParam("stereoSpread", 1.0f); setParam("fxSpace", 0.72f); setParam("reverbMix", 0.32f);
            setParam("madnezz", 0.24f); setParam("drift", 4.4f); setParam("phaserMix", 0.34f); setParam("delayMix", 0.26f);
            setParam("vibratoDepthCents", 18.0f); setParam("vibratoRateHz", 0.42f);
            setParam("pitchEnvTimeMs", 220.0f);
            setParam("osc1PitchEnvSemi", -18.0f); setParam("osc2PitchEnvSemi", 9.0f); setParam("osc3PitchEnvSemi", -24.0f); setParam("osc4PitchEnvSemi", 14.0f);
            setParam("osc1PitchEnvShape", 0.0f); setParam("osc2PitchEnvShape", 2.0f); setParam("osc3PitchEnvShape", 1.0f); setParam("osc4PitchEnvShape", 2.0f);
            break;
        case 33: // DRONE - Dark Drone
            setParam("masterGain", -4.8f);
            setParam("osc1Octave", 1.0f); setParam("osc2Octave", 1.0f); setParam("osc3Octave", 0.0f); setParam("osc4Octave", 0.0f);
            setParam("cutoff", 520.0f); setParam("resonance", 0.72f); setParam("analogWarm", 0.78f);
            setParam("attackMs", 1400.0f); setParam("releaseMs", 2600.0f); setParam("fxSpace", 0.78f); setParam("madnezz", 0.10f);
            setParam("pitchEnvTimeMs", 900.0f);
            setParam("osc1PitchEnvSemi", -6.0f); setParam("osc2PitchEnvSemi", 3.0f); setParam("osc3PitchEnvSemi", -8.0f); setParam("osc4PitchEnvSemi", 2.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 0.0f); setParam("osc3PitchEnvShape", 2.0f); setParam("osc4PitchEnvShape", 0.0f);
            break;
        case 34: // FX - Lo-Fi Poly
            setParam("masterGain", -4.9f);
            setParam("osc3Wave", 4.0f); setParam("osc4Wave", 5.0f);
            setParam("postSaturation", 0.52f); setParam("analogWarm", 0.67f); setParam("limiterThresholdDb", -3.5f);
            setParam("cutoff", 3000.0f); setParam("reverbMix", 0.30f); setParam("monoWidth", 0.8f);
            setParam("madnezz", 0.18f); setParam("drift", 3.4f); setParam("panSweepAmount", 0.24f); setParam("panSweepRateHz", 0.18f);
            setParam("vibratoDepthCents", 12.0f); setParam("vibratoRateHz", 0.35f); setParam("phaserMix", 0.22f); setParam("delayMix", 0.20f);
            setParam("pitchEnvTimeMs", 140.0f);
            setParam("osc1PitchEnvSemi", -14.0f); setParam("osc2PitchEnvSemi", 8.0f); setParam("osc3PitchEnvSemi", -20.0f); setParam("osc4PitchEnvSemi", 12.0f);
            setParam("osc1PitchEnvShape", 1.0f); setParam("osc2PitchEnvShape", 1.0f); setParam("osc3PitchEnvShape", 0.0f); setParam("osc4PitchEnvShape", 2.0f);
            break;
        case 35: // FX - Frozen Space
            setParam("masterGain", -5.3f);
            setParam("osc2Wave", 5.0f); setParam("osc3Wave", 6.0f); setParam("osc4Wave", 7.0f);
            setParam("fxSpace", 1.0f); setParam("reverbSize", 0.98f); setParam("reverbDamping", 0.05f); setParam("reverbMix", 0.72f);
            setParam("delayMix", 0.36f); setParam("delayFeedback", 0.82f); setParam("analogWarm", 0.5f);
            setParam("madnezz", 0.24f); setParam("drift", 2.8f); setParam("panSweepAmount", 0.28f); setParam("panSweepRateHz", 0.12f);
            setParam("vibratoDepthCents", 16.0f); setParam("vibratoRateHz", 0.22f); setParam("phaserMix", 0.32f);
            setParam("attackMs", 600.0f); setParam("releaseMs", 2200.0f);
            setParam("pitchEnvTimeMs", 380.0f);
            setParam("osc1PitchEnvSemi", -10.0f); setParam("osc2PitchEnvSemi", 5.0f); setParam("osc3PitchEnvSemi", -16.0f); setParam("osc4PitchEnvSemi", 9.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 1.0f); setParam("osc3PitchEnvShape", 0.0f); setParam("osc4PitchEnvShape", 2.0f);
            break;
        case 36: // DRONE - Abyss Bloom
            setParam("masterGain", -4.9f);
            setParam("osc1Wave", 0.0f); setParam("osc2Wave", 1.0f); setParam("osc3Wave", 3.0f); setParam("osc4Wave", 0.0f);
            setParam("osc1Octave", 1.0f); setParam("osc2Octave", 0.0f); setParam("osc3Octave", 1.0f); setParam("osc4Octave", 0.0f);
            setParam("attackMs", 1800.0f); setParam("decayMs", 1200.0f); setParam("sustain", 0.86f); setParam("releaseMs", 4200.0f);
            setParam("unisonVoices", 2.0f); setParam("unisonDetune", 18.0f); setParam("stereoSpread", 1.0f); setParam("monoWidth", 1.0f);
            setParam("vibratoDepthCents", 18.0f); setParam("vibratoRateHz", 0.23f); setParam("panSweepAmount", 0.18f); setParam("panSweepRateHz", 0.09f);
            setParam("madnezz", 0.12f);
            setParam("pitchEnvTimeMs", 980.0f);
            setParam("osc1PitchEnvSemi", -7.0f); setParam("osc2PitchEnvSemi", 4.0f); setParam("osc3PitchEnvSemi", -9.0f); setParam("osc4PitchEnvSemi", 3.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 0.0f); setParam("osc3PitchEnvShape", 2.0f); setParam("osc4PitchEnvShape", 0.0f);
            setParam("cutoff", 1500.0f); setParam("resonance", 0.44f); setParam("analogWarm", 0.72f);
            setParam("chorusMix", 0.42f); setParam("phaserMix", 0.22f); setParam("fxSpace", 0.85f); setParam("reverbMix", 0.54f); setParam("delayMix", 0.20f);
            break;
        case 37: // DRONE - Magnetar Choir
            setParam("masterGain", -5.1f);
            setParam("osc1Wave", 1.0f); setParam("osc2Wave", 0.0f); setParam("osc3Wave", 1.0f); setParam("osc4Wave", 3.0f);
            setParam("osc1TuneCents", -5.0f); setParam("osc2TuneCents", 4.0f); setParam("osc3TuneCents", -9.0f); setParam("osc4TuneCents", 7.0f);
            setParam("attackMs", 2400.0f); setParam("decayMs", 1400.0f); setParam("sustain", 0.88f); setParam("releaseMs", 5000.0f);
            setParam("unisonVoices", 3.0f); setParam("unisonDetune", 23.0f); setParam("stereoSpread", 1.0f);
            setParam("vibratoDepthCents", 24.0f); setParam("vibratoRateHz", 0.18f);
            setParam("madnezz", 0.16f);
            setParam("pitchEnvTimeMs", 1100.0f);
            setParam("osc1PitchEnvSemi", -8.0f); setParam("osc2PitchEnvSemi", 6.0f); setParam("osc3PitchEnvSemi", -12.0f); setParam("osc4PitchEnvSemi", 5.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 2.0f); setParam("osc3PitchEnvShape", 0.0f); setParam("osc4PitchEnvShape", 2.0f);
            setParam("cutoff", 980.0f); setParam("resonance", 0.62f); setParam("analogWarm", 0.82f); setParam("postSaturation", 0.24f);
            setParam("chorusMix", 0.52f); setParam("phaserMix", 0.28f); setParam("fxSpace", 0.92f); setParam("reverbMix", 0.62f); setParam("delayMix", 0.24f);
            break;
        case 38: // DRONE - Slow Glass Field
            setParam("masterGain", -4.7f);
            setParam("osc1Wave", 0.0f); setParam("osc2Wave", 3.0f); setParam("osc3Wave", 0.0f); setParam("osc4Wave", 3.0f);
            setParam("attackMs", 2600.0f); setParam("decayMs", 1600.0f); setParam("sustain", 0.83f); setParam("releaseMs", 5600.0f);
            setParam("unisonVoices", 1.0f); setParam("unisonDetune", 16.0f); setParam("stereoSpread", 0.95f); setParam("monoWidth", 1.0f);
            setParam("vibratoDepthCents", 15.0f); setParam("vibratoRateHz", 0.14f); setParam("panSweepAmount", 0.12f); setParam("panSweepRateHz", 0.06f);
            setParam("madnezz", 0.08f);
            setParam("pitchEnvTimeMs", 850.0f);
            setParam("osc1PitchEnvSemi", -5.0f); setParam("osc2PitchEnvSemi", 3.0f); setParam("osc3PitchEnvSemi", -7.0f); setParam("osc4PitchEnvSemi", 2.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 2.0f); setParam("osc3PitchEnvShape", 0.0f); setParam("osc4PitchEnvShape", 2.0f);
            setParam("cutoff", 2300.0f); setParam("resonance", 0.36f); setParam("analogWarm", 0.38f);
            setParam("chorusMix", 0.34f); setParam("phaserMix", 0.16f); setParam("fxSpace", 0.88f); setParam("reverbMix", 0.58f); setParam("delayMix", 0.18f);
            break;
        case 39: // DRONE - Cathedral Fog
            setParam("masterGain", -5.2f);
            setParam("osc1Wave", 1.0f); setParam("osc2Wave", 1.0f); setParam("osc3Wave", 0.0f); setParam("osc4Wave", 1.0f);
            setParam("osc1Octave", 0.0f); setParam("osc2Octave", 1.0f); setParam("osc3Octave", 0.0f); setParam("osc4Octave", 1.0f);
            setParam("attackMs", 2200.0f); setParam("decayMs", 1700.0f); setParam("sustain", 0.9f); setParam("releaseMs", 6200.0f);
            setParam("unisonVoices", 2.0f); setParam("unisonDetune", 20.0f); setParam("stereoSpread", 1.0f);
            setParam("vibratoDepthCents", 22.0f); setParam("vibratoRateHz", 0.20f);
            setParam("madnezz", 0.14f);
            setParam("pitchEnvTimeMs", 1200.0f);
            setParam("osc1PitchEnvSemi", -9.0f); setParam("osc2PitchEnvSemi", 5.0f); setParam("osc3PitchEnvSemi", -11.0f); setParam("osc4PitchEnvSemi", 4.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 0.0f); setParam("osc3PitchEnvShape", 2.0f); setParam("osc4PitchEnvShape", 2.0f);
            setParam("cutoff", 880.0f); setParam("resonance", 0.58f); setParam("analogWarm", 0.88f); setParam("postSaturation", 0.30f);
            setParam("chorusMix", 0.46f); setParam("phaserMix", 0.26f); setParam("fxSpace", 1.0f); setParam("reverbMix", 0.7f); setParam("delayMix", 0.22f);
            break;
        case 40: // DRONE - Layered Orbit
            setParam("masterGain", -4.8f);
            setParam("osc1Wave", 1.0f); setParam("osc2Wave", 3.0f); setParam("osc3Wave", 1.0f); setParam("osc4Wave", 0.0f);
            setParam("osc1Pan", -0.35f); setParam("osc2Pan", 0.35f); setParam("osc3Pan", -0.6f); setParam("osc4Pan", 0.6f);
            setParam("attackMs", 1400.0f); setParam("decayMs", 1000.0f); setParam("sustain", 0.84f); setParam("releaseMs", 3800.0f);
            setParam("unisonVoices", 2.0f); setParam("unisonDetune", 17.0f); setParam("stereoSpread", 0.95f);
            setParam("vibratoDepthCents", 20.0f); setParam("vibratoRateHz", 0.27f); setParam("panSweepAmount", 0.42f); setParam("panSweepRateHz", 0.12f);
            setParam("madnezz", 0.18f);
            setParam("pitchEnvTimeMs", 760.0f);
            setParam("osc1PitchEnvSemi", -6.0f); setParam("osc2PitchEnvSemi", 7.0f); setParam("osc3PitchEnvSemi", -10.0f); setParam("osc4PitchEnvSemi", 8.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 1.0f); setParam("osc3PitchEnvShape", 0.0f); setParam("osc4PitchEnvShape", 2.0f);
            setParam("cutoff", 1800.0f); setParam("resonance", 0.42f); setParam("analogWarm", 0.62f);
            setParam("chorusMix", 0.4f); setParam("phaserMix", 0.24f); setParam("fxSpace", 0.82f); setParam("reverbMix", 0.46f); setParam("delayMix", 0.16f);
            break;
        case 41: // DRONE - Rusted Nebula
            setParam("masterGain", -5.4f);
            setParam("osc1Wave", 2.0f); setParam("osc2Wave", 1.0f); setParam("osc3Wave", 3.0f); setParam("osc4Wave", 1.0f);
            setParam("attackMs", 1600.0f); setParam("decayMs", 1300.0f); setParam("sustain", 0.79f); setParam("releaseMs", 4600.0f);
            setParam("unisonVoices", 1.0f); setParam("unisonDetune", 14.0f); setParam("stereoSpread", 0.9f);
            setParam("vibratoDepthCents", 14.0f); setParam("vibratoRateHz", 0.16f);
            setParam("madnezz", 0.20f);
            setParam("pitchEnvTimeMs", 680.0f);
            setParam("osc1PitchEnvSemi", -12.0f); setParam("osc2PitchEnvSemi", 6.0f); setParam("osc3PitchEnvSemi", -18.0f); setParam("osc4PitchEnvSemi", 10.0f);
            setParam("osc1PitchEnvShape", 1.0f); setParam("osc2PitchEnvShape", 0.0f); setParam("osc3PitchEnvShape", 1.0f); setParam("osc4PitchEnvShape", 2.0f);
            setParam("cutoff", 1050.0f); setParam("resonance", 0.76f); setParam("analogWarm", 0.92f); setParam("drive", 9.5f); setParam("postSaturation", 0.42f);
            setParam("chorusMix", 0.26f); setParam("phaserMix", 0.34f); setParam("fxSpace", 0.74f); setParam("reverbMix", 0.44f); setParam("delayMix", 0.14f);
            break;
        case 42: // DRONE - Hollow Machine Sky
            setParam("masterGain", -5.1f);
            setParam("osc1Wave", 3.0f); setParam("osc2Wave", 0.0f); setParam("osc3Wave", 2.0f); setParam("osc4Wave", 3.0f);
            setParam("osc1Semitone", -12.0f); setParam("osc3Semitone", 7.0f); setParam("osc4Semitone", 12.0f);
            setParam("attackMs", 2000.0f); setParam("decayMs", 1500.0f); setParam("sustain", 0.81f); setParam("releaseMs", 5200.0f);
            setParam("unisonVoices", 2.0f); setParam("unisonDetune", 21.0f); setParam("stereoSpread", 1.0f);
            setParam("vibratoDepthCents", 28.0f); setParam("vibratoRateHz", 0.12f);
            setParam("madnezz", 0.15f);
            setParam("pitchEnvTimeMs", 940.0f);
            setParam("osc1PitchEnvSemi", -8.0f); setParam("osc2PitchEnvSemi", 4.0f); setParam("osc3PitchEnvSemi", -13.0f); setParam("osc4PitchEnvSemi", 9.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 2.0f); setParam("osc3PitchEnvShape", 0.0f); setParam("osc4PitchEnvShape", 2.0f);
            setParam("cutoff", 1450.0f); setParam("resonance", 0.54f); setParam("analogWarm", 0.56f);
            setParam("chorusMix", 0.48f); setParam("phaserMix", 0.30f); setParam("fxSpace", 0.9f); setParam("reverbMix", 0.6f); setParam("delayMix", 0.21f);
            break;
        case 43: // DRONE - Sub Polar Drift
            setParam("masterGain", -4.9f);
            setParam("osc1Octave", 0.0f); setParam("osc2Octave", 0.0f); setParam("osc3Octave", 1.0f); setParam("osc4Octave", 1.0f);
            setParam("osc1Level", 0.30f); setParam("osc2Level", 0.28f); setParam("osc3Level", 0.2f); setParam("osc4Level", 0.14f);
            setParam("attackMs", 1500.0f); setParam("decayMs", 1100.0f); setParam("sustain", 0.88f); setParam("releaseMs", 4800.0f);
            setParam("unisonVoices", 1.0f); setParam("unisonDetune", 15.0f); setParam("stereoSpread", 0.92f); setParam("monoWidth", 0.96f);
            setParam("vibratoDepthCents", 12.0f); setParam("vibratoRateHz", 0.10f); setParam("panSweepAmount", 0.16f); setParam("panSweepRateHz", 0.05f);
            setParam("madnezz", 0.10f);
            setParam("pitchEnvTimeMs", 1020.0f);
            setParam("osc1PitchEnvSemi", -5.0f); setParam("osc2PitchEnvSemi", 2.0f); setParam("osc3PitchEnvSemi", -8.0f); setParam("osc4PitchEnvSemi", 3.0f);
            setParam("osc1PitchEnvShape", 2.0f); setParam("osc2PitchEnvShape", 0.0f); setParam("osc3PitchEnvShape", 2.0f); setParam("osc4PitchEnvShape", 2.0f);
            setParam("cutoff", 700.0f); setParam("resonance", 0.46f); setParam("analogWarm", 0.8f);
            setParam("chorusMix", 0.31f); setParam("phaserMix", 0.18f); setParam("fxSpace", 0.8f); setParam("reverbMix", 0.52f); setParam("delayMix", 0.17f);
            break;
        default: // INIT - Init
            setParam("attackMs", 8.0f); setParam("decayMs", 180.0f); setParam("sustain", 0.7f); setParam("releaseMs", 280.0f);
            break;
    }
}

juce::Rectangle<int> FourOscProAudioProcessorEditor::getArpMeterBounds() const
{
    // Keep ARP status on the top-right.
    const int y = 18;
    const int h = 24;
    const int rightMargin = 16;
    const int width = 206;
    const int x = juce::jmax(20, getWidth() - width - rightMargin);
    return { x, y, width, h };
}

juce::Colour FourOscProAudioProcessorEditor::getArpModeColor(int arpMode) const
{
    switch (arpMode)
    {
        case 1: return juce::Colour::fromRGB(0, 255, 170);
        case 2: return juce::Colour::fromRGB(170, 125, 255);
        case 3: return juce::Colour::fromRGB(255, 120, 235);
        default: return kAccent;
    }
}

void FourOscProAudioProcessorEditor::spawnSparkBurst(int step, juce::Colour modeColor)
{
    auto meter = getArpMeterBounds();
    auto segments = meter.reduced(8, 7);
    const int segW = juce::jmax(4, segments.getWidth() / 16 - 2);
    const float x = static_cast<float>(segments.getX() + step * (segW + 2) + segW / 2);
    const float y = static_cast<float>(segments.getY() + segments.getHeight() / 2);

    int emitted = 0;
    for (auto& spark : sparks)
    {
        if (spark.life > 0.0f)
            continue;

        const float speed = 0.8f + uiRandom.nextFloat() * 2.4f;
        const float angle = (uiRandom.nextFloat() * juce::MathConstants<float>::pi) - juce::MathConstants<float>::halfPi;
        spark.pos = { x, y };
        spark.vel = { std::cos(angle) * speed, std::sin(angle) * speed };
        spark.color = modeColor;
        spark.life = 1.0f;

        if (++emitted >= 8)
            break;
    }
}

void FourOscProAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (isShowing() && ! isTimerRunning())
        startTimerHz(60);

    const float audioGlowBoost = 1.0f + audioGlowVisual * 1.4f;
    const float glow = juce::jlimit(0.4f, 2.2f, glowIntensity * audioGlowBoost);
    auto glowA = [glow] (float alpha)
    {
        return juce::jlimit(0.0f, 1.0f, alpha * glow);
    };

    juce::ColourGradient gradient(kPanelBg.brighter(0.2f), 0.0f, 0.0f, kPanelBg, 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(kAccentSoft.withAlpha(glowA(0.12f)));
    for (int x = 0; x < getWidth(); x += 28)
        g.drawVerticalLine(x, 0.0f, static_cast<float>(getHeight()));
    for (int y = 0; y < getHeight(); y += 28)
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));

    auto frameGlow = getLocalBounds().toFloat().reduced(8.0f);
    const float breathAlpha = 0.14f + panelBreath * 0.22f;
    g.setColour(kAccent.withAlpha(glowA(breathAlpha)));
    g.drawRoundedRectangle(frameGlow, 16.0f, 2.0f);
    g.setColour(kAccent.withAlpha(glowA(0.05f + panelBreath * 0.10f)));
    g.drawRoundedRectangle(frameGlow.reduced(5.0f), 13.0f, 1.2f);

    if (oscTabButtons[0].isVisible())
    {
        // Connected segmented frame around top tabs (1 2 3 4 FX).
        auto tabUnion = oscTabButtons[0].getBounds();
        for (int i = 1; i < 5; ++i)
            tabUnion = tabUnion.getUnion(oscTabButtons[static_cast<size_t>(i)].getBounds());
        tabUnion = tabUnion.expanded(6, 5);
        g.setColour(juce::Colour::fromRGB(7, 16, 31).withAlpha(0.9f));
        g.fillRoundedRectangle(tabUnion.toFloat(), 6.0f);
        g.setColour(kAccent.withAlpha(glowA(0.52f)));
        g.drawRoundedRectangle(tabUnion.toFloat(), 6.0f, 1.45f);
        g.setColour(kAccent.withAlpha(glowA(0.10f + panelBreath * 0.12f)));
        g.drawRoundedRectangle(tabUnion.toFloat().expanded(1.8f), 7.0f, 1.1f);

        for (int i = 0; i < 4; ++i)
        {
            const auto& b = oscTabButtons[static_cast<size_t>(i)].getBounds();
            const int x = b.getRight() + 3;
            g.setColour(kAccentSoft.withAlpha(glowA(0.2f)));
            g.drawLine(static_cast<float>(x), static_cast<float>(tabUnion.getY() + 5),
                       static_cast<float>(x), static_cast<float>(tabUnion.getBottom() - 5), 1.0f);
        }
    }

    const auto phase = juce::jlimit(0.0f, 1.0f, processor.getUiArpPhase());
    const auto step = juce::jlimit(0, 15, processor.getUiArpStep());
    const bool active = processor.isUiArpActive();
    const float bpm = processor.getUiHostBpm();
    const int arpMode = static_cast<int>(processor.getValueTreeState().getRawParameterValue("arpMode")->load());
    const char* modeText = "UP";
    juce::Colour modeColor = getArpModeColor(arpMode);
    switch (arpMode)
    {
        case 1:
            modeText = "DOWN";
            break;
        case 2:
            modeText = "UPDOWN";
            break;
        case 3:
            modeText = "RANDOM";
            break;
        default:
            break;
    }

    auto meter = getArpMeterBounds();
    g.setColour(juce::Colour::fromRGB(12, 24, 45));
    g.fillRoundedRectangle(meter.toFloat(), 7.0f);
    g.setColour(modeColor.withAlpha(glowA(0.6f)));
    g.drawRoundedRectangle(meter.toFloat(), 7.0f, 1.2f);

    auto segments = meter.reduced(8, 7);
    const int segW = juce::jmax(4, segments.getWidth() / 16 - 2);
    for (int i = 0; i < 16; ++i)
    {
        const int x = segments.getX() + i * (segW + 2);
        auto seg = juce::Rectangle<int>(x, segments.getY(), segW, segments.getHeight());
        juce::Colour c = kAccentSoft.withAlpha(0.15f);
        if (active)
        {
            // Draw a small mode-dependent trail so motion style differs per arp mode.
            int dist = std::abs(i - step);
            if (arpMode == 0)         // Up
                dist = (i <= step) ? (step - i) : 99;
            else if (arpMode == 1)    // Down
                dist = (i >= step) ? (i - step) : 99;
            else if (arpMode == 3)    // Random
                dist = std::min(dist, 8);

            if (dist == 0)
                c = modeColor.withAlpha(glowA(0.38f + 0.62f * phase));
            else if (dist < 4)
                c = modeColor.withAlpha(glowA((0.22f - 0.045f * static_cast<float>(dist)) * (0.35f + 0.65f * phase)));
        }
        g.setColour(c);
        g.fillRoundedRectangle(seg.toFloat(), 2.5f);
    }

    const auto meterInfo = getArpMeterBounds();
    juce::Rectangle<int> infoBounds(meterInfo.getX(), meterInfo.getBottom() + 2, meterInfo.getWidth(), 16);
    g.setColour(kText.withAlpha(0.82f + panelBreath * 0.16f));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawFittedText("ARP " + juce::String(active ? "SYNC" : "IDLE") + "  " + juce::String(modeText) + "  "
                        + juce::String(bpm, 1) + " BPM",
                     infoBounds.reduced(2, 0), juce::Justification::centredRight, 1);

    for (const auto& spark : sparks)
    {
        if (spark.life <= 0.0f)
            continue;

        const float alpha = juce::jlimit(0.0f, 1.0f, spark.life);
        const float size = 1.6f + (1.0f - alpha) * 2.3f;
        g.setColour(spark.color.withAlpha(glowA(0.18f * alpha)));
        g.fillEllipse(spark.pos.x - size * 1.8f, spark.pos.y - size * 1.8f, size * 3.6f, size * 3.6f);
        g.setColour(spark.color.withAlpha(glowA(0.85f * alpha)));
        g.fillEllipse(spark.pos.x - size, spark.pos.y - size, size * 2.0f, size * 2.0f);
    }

    if (! bootSequenceDone)
    {
        const float progress = juce::jlimit(0.0f, 1.0f, bootSequenceT / 0.95f);
        const float fade = 1.0f - juce::jlimit(0.0f, 1.0f, (bootSequenceT - 1.0f) / 0.25f);

        g.setColour(juce::Colours::black.withAlpha(0.68f * fade));
        g.fillRect(getLocalBounds());

        // Moving scanlines for a retro terminal boot feel.
        g.setColour(kAccentSoft.withAlpha(0.08f * fade));
        const int scanOffset = static_cast<int>(bootSequenceT * 180.0f) % 14;
        for (int y = -14 + scanOffset; y < getHeight(); y += 14)
            g.fillRect(0, y, getWidth(), 2);

        auto panel = getLocalBounds().reduced(getWidth() / 5, getHeight() / 3);
        g.setColour(juce::Colour::fromRGB(10, 22, 44).withAlpha(0.96f * fade));
        g.fillRoundedRectangle(panel.toFloat(), 12.0f);
        g.setColour(kAccent.withAlpha(0.85f * fade));
        g.drawRoundedRectangle(panel.toFloat(), 12.0f, 1.4f);

        auto textArea = panel.reduced(18);
        g.setColour(kAccent.withAlpha(0.96f * fade));
        g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
        g.drawFittedText("INITIALIZING FOUROSC PRO", textArea.removeFromTop(32), juce::Justification::centred, 1);

        g.setColour(kText.withAlpha(0.8f * fade));
        g.setFont(juce::FontOptions(13.0f));
        g.drawFittedText("DSP core, sync engine, neon interface...", textArea.removeFromTop(24), juce::Justification::centred, 1);
        textArea.removeFromTop(8);

        auto bar = textArea.removeFromTop(20);
        g.setColour(kAccentSoft.withAlpha(0.35f * fade));
        g.fillRoundedRectangle(bar.toFloat(), 4.0f);
        auto filled = bar.withWidth(static_cast<int>(std::round(bar.getWidth() * progress)));
        g.setColour(kAccent.withAlpha(0.9f * fade));
        g.fillRoundedRectangle(filled.toFloat(), 4.0f);

        g.setColour(kText.withAlpha(0.9f * fade));
        g.drawFittedText("BOOT " + juce::String(static_cast<int>(std::round(progress * 100.0f))) + "%", textArea.removeFromTop(28),
                         juce::Justification::centred, 1);
    }
}

void FourOscProAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Standalone dice icon with click shrink feedback.
    auto b = presetRandomButton.getBounds().toFloat().reduced(3.0f);
    const float press = juce::jlimit(0.0f, 1.0f, randomDicePressAnim);
    const float s = 1.0f - 0.12f * press;
    b = b.withSizeKeepingCentre(b.getWidth() * s, b.getHeight() * s);

    if (diceIconImage.isValid())
    {
        const auto target = b.toNearestInt();
        g.setOpacity(0.98f);
        g.drawImageWithin(diceIconImage, target.getX(), target.getY(), target.getWidth(), target.getHeight(),
                          juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, false);
        g.setOpacity(1.0f);
        return;
    }

    // Fallback if the file is missing.
    const auto front = b.reduced(6.0f);
    g.setColour(kText.withAlpha(0.95f));
    g.fillRoundedRectangle(front, 3.2f);
    g.setColour(juce::Colour::fromRGB(18, 22, 28));
    const float r = juce::jmax(1.4f, front.getWidth() * 0.07f);
    g.fillEllipse(front.getCentreX() - r, front.getCentreY() - r, r * 2.0f, r * 2.0f);
}

void FourOscProAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(14);
    auto header = area.removeFromTop(52);
    auto headerTop = header.removeFromTop(32);

    const int skinLabelW = 44;
    const int skinBoxW = 120;
    const int skinGroupW = skinLabelW + skinBoxW;
    const int presetLabelW = 72;
    const int presetNavW = 28;
    const int presetRandomW = 30;
    const int presetBoxW = 280;
    const int presetGroupW = presetLabelW + presetNavW + 6 + presetBoxW + 6 + presetNavW + 6 + presetRandomW;
    const int presetGroupX = juce::jmax(0, (getWidth() - presetGroupW) / 2);
    const int skinGroupX = juce::jmax(14, presetGroupX - skinGroupW - 16);

    auto titleArea = headerTop;
    titleArea.setRight(skinGroupX - 10);
    titleLabel.setBounds(titleArea);

    juce::Rectangle<int> skinArea(skinGroupX, headerTop.getY(), skinGroupW, headerTop.getHeight());
    skinLabel.setBounds(skinArea.removeFromLeft(skinLabelW));
    skinBox.setBounds(skinArea.removeFromLeft(skinBoxW).reduced(2, 4));

    juce::Rectangle<int> presetArea(presetGroupX, headerTop.getY(), presetGroupW, headerTop.getHeight());
    presetLabel.setBounds(presetArea.removeFromLeft(presetLabelW));
    presetPrevButton.setBounds(presetArea.removeFromLeft(presetNavW).reduced(2, 4));
    presetArea.removeFromLeft(6);
    presetBox.setBounds(presetArea.removeFromLeft(presetBoxW).reduced(2, 4));
    presetArea.removeFromLeft(6);
    presetNextButton.setBounds(presetArea.removeFromLeft(presetNavW).reduced(2, 4));
    presetArea.removeFromLeft(6);
    presetRandomButton.setBounds(presetArea.removeFromLeft(presetRandomW).reduced(2, 4));

    auto headerBottom = header.removeFromTop(14);
    subtitleLabel.setBounds(headerBottom.reduced(0, 1));
    area.removeFromTop(1);

    auto pageTop = area.removeFromTop(18);
    oscSectionLabel.setBounds(pageTop.removeFromLeft(220));
    auto topRow = area.removeFromTop(350);
    area.removeFromTop(4);
    auto bottomRow = area;

    auto synthArea = topRow.removeFromLeft(static_cast<int>(topRow.getWidth() * 0.60f)).reduced(8);
    auto arpArea = topRow.reduced(8);
    auto oscArea = bottomRow.removeFromLeft(static_cast<int>(bottomRow.getWidth() * 0.70f)).reduced(8);
    auto fxArea = bottomRow.reduced(8);

    synthSectionLabel.setBounds(synthArea.removeFromTop(20));
    synthArea.removeFromTop(4);
    auto synthKnobRow = synthArea.removeFromTop(130);
    const int synthCellW = synthKnobRow.getWidth() / 4;
    auto layoutKnobCell = [] (juce::Rectangle<int> cell, juce::Label& label, juce::Slider& slider)
    {
        cell = cell.reduced(4);
        label.setBounds(cell.removeFromTop(16));
        slider.setBounds(cell);
    };
    auto synthCell1 = synthKnobRow.removeFromLeft(synthCellW);
    auto synthCell2 = synthKnobRow.removeFromLeft(synthCellW);
    auto synthCell3 = synthKnobRow.removeFromLeft(synthCellW);
    auto synthCell4 = synthKnobRow;
    layoutKnobCell(synthCell1, masterGainLabel, masterGainSlider);
    layoutKnobCell(synthCell2, driveLabel, driveSlider);
    layoutKnobCell(synthCell3, cutoffLabel, cutoffSlider);
    layoutKnobCell(synthCell4, resonanceLabel, resonanceSlider);

    synthArea.removeFromTop(4);
    retriggerButton.setBounds(synthArea.removeFromTop(28).removeFromLeft(146));
    synthArea.removeFromTop(2);
    unisonVoicesBox.setBounds(synthArea.removeFromTop(28).removeFromLeft(200));
    synthArea.removeFromTop(10);

    auto layoutSmallRow = [] (juce::Rectangle<int> row, juce::Label& l1, juce::Slider& s1,
                               juce::Label& l2, juce::Slider& s2, juce::Label& l3, juce::Slider& s3)
    {
        auto c1 = row.removeFromLeft(row.getWidth() / 3).reduced(2, 0);
        auto c2 = row.removeFromLeft(row.getWidth() / 2).reduced(2, 0);
        auto c3 = row.reduced(2, 0);
        l1.setBounds(c1.removeFromLeft(74)); s1.setBounds(c1);
        l2.setBounds(c2.removeFromLeft(86)); s2.setBounds(c2);
        l3.setBounds(c3.removeFromLeft(86)); s3.setBounds(c3);
    };

    auto rowA = synthArea.removeFromTop(20);
    layoutSmallRow(rowA, driftLabel, driftSlider, analogWarmLabel, analogWarmSlider, bassCompLabel, bassCompSlider);
    synthArea.removeFromTop(2);
    auto rowB = synthArea.removeFromTop(20);
    layoutSmallRow(rowB, attackLabel, attackSlider, decayLabel, decaySlider, unisonDetuneLabel, unisonDetuneSlider);
    synthArea.removeFromTop(2);
    auto rowC = synthArea.removeFromTop(20);
    layoutSmallRow(rowC, sustainLabel, sustainSlider, releaseLabel, releaseSlider, stereoSpreadLabel, stereoSpreadSlider);
    synthArea.removeFromTop(2);
    auto rowD = synthArea.removeFromTop(20);
    auto d1 = rowD.removeFromLeft(rowD.getWidth() / 3).reduced(2, 0);
    auto d2 = rowD.removeFromLeft(rowD.getWidth() / 2).reduced(2, 0);
    auto d3 = rowD.reduced(2, 0);
    monoWidthLabel.setBounds(d1.removeFromLeft(74));
    monoWidthSlider.setBounds(d1);
    panSweepAmountLabel.setBounds(d2.removeFromLeft(86));
    panSweepAmountSlider.setBounds(d2);
    panSweepRateLabel.setBounds(d3.removeFromLeft(86));
    panSweepRateSlider.setBounds(d3);
    synthArea.removeFromTop(2);
    auto rowE = synthArea.removeFromTop(20);
    auto e1 = rowE.removeFromLeft(rowE.getWidth() / 3).reduced(2, 0);
    auto e2 = rowE.removeFromLeft(rowE.getWidth() / 2).reduced(2, 0);
    auto e3 = rowE.reduced(2, 0);
    vibratoDepthLabel.setBounds(e1.removeFromLeft(86));
    vibratoDepthSlider.setBounds(e1);
    vibratoRateLabel.setBounds(e2.removeFromLeft(86));
    vibratoRateSlider.setBounds(e2);
    pitchEnvTimeLabel.setBounds(e3.removeFromLeft(88));
    pitchEnvTimeSlider.setBounds(e3);

    arpSectionLabel.setBounds(arpArea.removeFromTop(20));
    arpArea.removeFromTop(4);
    auto arpToggles = arpArea.removeFromTop(32);
    arpEnabledButton.setBounds(arpToggles.removeFromLeft(128));
    arpToggles.removeFromLeft(8);
    arpLatchButton.setBounds(arpToggles.removeFromLeft(108));
    arpArea.removeFromTop(4);
    arpDivisionBox.setBounds(arpArea.removeFromTop(28).removeFromLeft(180));
    arpArea.removeFromTop(2);
    arpModeBox.setBounds(arpArea.removeFromTop(28).removeFromLeft(180));
    arpArea.removeFromTop(2);
    arpOctavesBox.setBounds(arpArea.removeFromTop(28).removeFromLeft(180));
    arpArea.removeFromTop(2);
    chorusDivisionBox.setBounds(arpArea.removeFromTop(28).removeFromLeft(180));
    arpArea.removeFromTop(6);
    auto arpKnobRow = arpArea.removeFromTop(150);
    auto arpCellW = arpKnobRow.getWidth() / 4;
    auto gateCell = arpKnobRow.removeFromLeft(arpCellW).reduced(4);
    auto subAmtCell = arpKnobRow.removeFromLeft(arpCellW).reduced(4);
    auto subFreqCell = arpKnobRow.removeFromLeft(arpCellW).reduced(4);
    auto madCell = arpKnobRow.reduced(4);
    layoutKnobCell(gateCell, arpGateLabel, arpGateSlider);
    layoutKnobCell(subAmtCell, subAmountLabel, subAmountSlider);
    layoutKnobCell(subFreqCell, subFreqLabel, subFreqSlider);
    layoutKnobCell(madCell, madnezzLabel, madnezzSlider);

    fxSectionLabel.setBounds(fxArea.removeFromTop(20));
    fxArea.removeFromTop(4);
    auto fxTop = fxArea.removeFromTop(32);
    delaySyncButton.setBounds(fxTop.removeFromLeft(160));
    fxTop.removeFromLeft(8);
    chorusSyncButton.setBounds(fxTop.removeFromLeft(160));
    fxTop.removeFromLeft(8);
    limiterOnButton.setBounds(fxTop.removeFromLeft(140));
    fxArea.removeFromTop(4);
    delayDivisionBox.setBounds(fxArea.removeFromTop(28).removeFromLeft(196));
    fxArea.removeFromTop(6);
    auto fxKnobRow = fxArea.removeFromTop(170);
    const int fxCellW = fxKnobRow.getWidth() / 4;
    auto fxCell1 = fxKnobRow.removeFromLeft(fxCellW);
    auto fxCell2 = fxKnobRow.removeFromLeft(fxCellW);
    auto fxCell3 = fxKnobRow.removeFromLeft(fxCellW);
    auto fxCell4 = fxKnobRow;
    layoutKnobCell(fxCell1, fxSpaceLabel, fxSpaceSlider);
    layoutKnobCell(fxCell2, delayMixLabel, delayMixSlider);
    layoutKnobCell(fxCell3, reverbMixLabel, reverbMixSlider);
    layoutKnobCell(fxCell4, phaserMixLabel, phaserMixSlider);

    fxArea.removeFromTop(4);
    auto fxRow1 = fxArea.removeFromTop(22);
    layoutSmallRow(fxRow1, chorusRateLabel, chorusRateSlider, chorusDepthLabel, chorusDepthSlider, chorusMixLabel, chorusMixSlider);
    fxArea.removeFromTop(2);
    auto fxRow2 = fxArea.removeFromTop(22);
    layoutSmallRow(fxRow2, delayTimeLabel, delayTimeSlider, delayFeedbackLabel, delayFeedbackSlider, reverbSizeLabel, reverbSizeSlider);
    fxArea.removeFromTop(2);
    auto fxRow3 = fxArea.removeFromTop(22);
    reverbDampingLabel.setBounds(fxRow3.removeFromLeft(94));
    reverbDampingSlider.setBounds(fxRow3.removeFromLeft(230));
    fxRow3.removeFromLeft(8);
    postSatLabel.setBounds(fxRow3.removeFromLeft(84));
    postSatSlider.setBounds(fxRow3.removeFromLeft(180));
    fxRow3.removeFromLeft(8);
    limiterThresholdLabel.setBounds(fxRow3.removeFromLeft(76));
    limiterThresholdSlider.setBounds(fxRow3.removeFromLeft(180));

    // Bottom oscillator bank (all 4 oscillators on one page).
    auto oscTitle = oscArea.removeFromTop(20);
    oscSectionLabel.setBounds(oscTitle);
    oscArea.removeFromTop(2);
    for (int i = 0; i < 4; ++i)
    {
        auto row = oscArea.removeFromTop(58).reduced(2, 1);
        oscHeadLabels[static_cast<size_t>(i)].setBounds(row.removeFromTop(16));
        row.removeFromTop(2);

        auto line1 = row.removeFromTop(20);
        oscWaveLabels[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(46));
        oscWaveBoxes[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(116));
        line1.removeFromLeft(6);
        oscOctaveLabels[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(36));
        oscOctaveBoxes[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(78));
        line1.removeFromLeft(6);
        oscLevelLabels[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(44));
        oscLevelSliders[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(146));
        line1.removeFromLeft(6);
        oscSemitoneLabels[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(46));
        oscSemitoneSliders[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(126));
        line1.removeFromLeft(6);
        oscPitchShapeLabels[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(42));
        oscPitchShapeBoxes[static_cast<size_t>(i)].setBounds(line1.removeFromLeft(94));

        row.removeFromTop(2);
        auto line2 = row.removeFromTop(20);
        oscTuneLabels[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(44));
        oscTuneSliders[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(144));
        line2.removeFromLeft(6);
        oscPanLabels[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(34));
        oscPanSliders[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(126));
        line2.removeFromLeft(6);
        oscPhaseLabels[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(48));
        oscPhaseSliders[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(144));
        line2.removeFromLeft(6);
        oscPitchEnvLabels[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(46));
        oscPitchEnvSliders[static_cast<size_t>(i)].setBounds(line2.removeFromLeft(144));

        oscArea.removeFromTop(2);
    }
    updateOscTabVisibility();
}

void FourOscProAudioProcessorEditor::timerCallback()
{
    const int step = juce::jlimit(0, 15, processor.getUiArpStep());
    const bool active = processor.isUiArpActive();
    const int arpMode = static_cast<int>(processor.getValueTreeState().getRawParameterValue("arpMode")->load());
    const auto modeColor = getArpModeColor(arpMode);
    const float bpm = juce::jmax(20.0f, processor.getUiHostBpm());
    const float audioLevel = juce::jlimit(0.0f, 1.0f, processor.getUiOutputLevel());
    const float gatedLevel = juce::jmax(0.0f, audioLevel - 0.015f);
    const float target = juce::jlimit(0.0f, 1.0f, gatedLevel * 1.2f);
    const float dt = 1.0f / 60.0f;

    // Stage 1: fast follower for quick in/out response.
    const float rise = 0.82f;
    const float fall = 0.56f;
    const float coeff = (target > audioGlowEnv) ? rise : fall;
    audioGlowEnv += (target - audioGlowEnv) * coeff;

    // Stage 2: small visual smoothing to prevent flicker while staying snappy.
    audioGlowVisual += (audioGlowEnv - audioGlowVisual) * 0.42f;

    // Pause ambient pulse while audio is active; resume after brief silence.
    if (audioGlowVisual > 0.045f)
        audioGlowHoldSeconds = 0.28f;
    else
        audioGlowHoldSeconds = juce::jmax(0.0f, audioGlowHoldSeconds - dt);
    const bool audioDrivingGlow = (audioGlowHoldSeconds > 0.0f);

    if (active && (! lastVisualActive || step != lastVisualStep))
        spawnSparkBurst(step, modeColor);

    lastVisualStep = step;
    lastVisualActive = active;

    for (auto& spark : sparks)
    {
        if (spark.life <= 0.0f)
            continue;

        spark.pos += spark.vel;
        spark.vel.x *= 0.985f;
        spark.vel.y = spark.vel.y * 0.985f + 0.035f;
        spark.life -= 0.055f;
        if (spark.life < 0.0f)
            spark.life = 0.0f;
    }

    // Ambient pulse is intentionally slow, and disabled while audio drives glow.
    const float breathsPerSecond = audioDrivingGlow ? 0.0f : (active ? ((bpm / 60.0f) * 0.22f) : 0.08f);
    panelBreathPhase += juce::MathConstants<float>::twoPi * breathsPerSecond * dt;
    if (panelBreathPhase > juce::MathConstants<float>::twoPi)
        panelBreathPhase -= juce::MathConstants<float>::twoPi;
    const float targetBreath = audioDrivingGlow ? (0.18f + 0.35f * audioGlowVisual)
                                                : (0.5f + 0.5f * std::sin(panelBreathPhase));
    panelBreath += (targetBreath - panelBreath) * (audioDrivingGlow ? 0.45f : 0.12f);

    if (! bootSequenceDone)
    {
        bootSequenceT += 1.0f / 30.0f;
        if (bootSequenceT >= 1.25f)
            bootSequenceDone = true;
    }

    if (randomDicePressAnim > 0.0f)
    {
        randomDicePressAnim *= 0.74f;
        if (randomDicePressAnim < 0.01f)
            randomDicePressAnim = 0.0f;
    }

    repaint();
}
