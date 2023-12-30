/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

namespace Params {

    enum class Parameters
    {
        OUT_GAIN,
        POLARITY_FLIP,
        HPF_FREQ,
        HPF_SLOPE,
        HPF_RESONANCE,
        HPF_BYPASS,
        LOW_SHELF_FREQ,
        LOW_SHELF_GAIN,
        LOW_SHELF_Q,
        LOW_SHELF_BYPASS,
        LOW_MID_FREQ,
        LOW_MID_GAIN,
        LOW_MID_Q,
        LOW_MID_BYPASS,
        HIGH_MID_FREQ,
        HIGH_MID_GAIN,
        HIGH_MID_Q,
        HIGH_MID_BYPASS,
        HIGH_SHELF_FREQ,
        HIGH_SHELF_GAIN,
        HIGH_SHELF_Q,
        HIGH_SHELF_BYPASS,
        LPF_FREQ,
        LPF_SLOPE,
        LPF_RESONANCE,
        LPF_BYPASS,
    };

    std::map<Parameters, juce::String> ParameterNames
    {
        {Parameters::OUT_GAIN, "Out Gain"},
        {Parameters::POLARITY_FLIP, "Polarity"},
        {Parameters::HPF_FREQ, "HPF Freq"},
        {Parameters::HPF_SLOPE, "HPF Slope"},
        {Parameters::HPF_RESONANCE, "HPF Resonance"},
        {Parameters::LOW_SHELF_FREQ, "Low Shelf Freq"},
        {Parameters::LOW_SHELF_GAIN, "Low Shelf Gain"},
        {Parameters::LOW_SHELF_Q, "Low Shelf Q"},
        {Parameters::LOW_SHELF_BYPASS, "Low Shelf Bypass"},
        {Parameters::LOW_MID_FREQ, "Low-Mid Freq"},
        {Parameters::LOW_MID_GAIN, "Low-Mid Gain"},
        {Parameters::LOW_MID_Q, "Low-Mid Q"},
        {Parameters::LOW_MID_BYPASS, "Low-Mid Bypass"},
        {Parameters::HIGH_MID_FREQ, "High-Mid Freq"},
        {Parameters::HIGH_MID_GAIN, "High-Mid Gain"},
        {Parameters::HIGH_MID_Q, "High-Mid Q"},
        {Parameters::HIGH_MID_BYPASS, "High-Mid Bypass"},
        {Parameters::HIGH_SHELF_FREQ, "High Shelf Freq"},
        {Parameters::HIGH_SHELF_GAIN, "High Shelf Gain"},
        {Parameters::HIGH_SHELF_Q, "High Shelf Q"},
        {Parameters::HIGH_SHELF_BYPASS, "High Shelf Bypass"},
        {Parameters::LPF_FREQ, "LPF Freq"},
        {Parameters::LPF_SLOPE, "LPF Slope"},
        {Parameters::LPF_RESONANCE, "LPF Resonance"},
    };
}


class NewProjectAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    enum Filters
    {
        HPF, LF, LMF, HMF, HF, LPF,
    };

private:
    //==============================================================================

    juce::AudioProcessorValueTreeState m_TreeState;
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    using Filter = juce::dsp::IIR::Filter<float>;
    //Filter hpf, lf, lmf, mf, hmf, hf, lpf;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, Filter, Filter, Filter, CutFilter>;
    MonoChain m_LeftChain, m_RightChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
