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
        HPF_BYPASS,
        LOW_SHELF_FREQ,
        LOW_SHELF_GAIN,
        LOW_SHELF_Q,
        LOW_SHELF_BYPASS,
        LOW_MID_FREQ,
        LOW_MID_GAIN,
        LOW_MID_Q,
        LOW_MID_BYPASS,
        MID_FREQ,
        MID_GAIN,
        MID_Q,
        MID_BYPASS,
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
        LPF_BYPASS,
    };

    std::map<Parameters, juce::String> ParameterNames
    {
        {Parameters::OUT_GAIN, "Out Gain"},
        {Parameters::POLARITY_FLIP, "Polarity"},
        {Parameters::HPF_FREQ, "HPF Freq"},
        {Parameters::HPF_SLOPE, "HPF Slope"},
        {Parameters::HPF_BYPASS, "HPF Bypass"},
        {Parameters::LOW_SHELF_FREQ, "Low Shelf Freq"},
        {Parameters::LOW_SHELF_GAIN, "Low Shelf Gain"},
        {Parameters::LOW_SHELF_Q, "Low Shelf Q"},
        {Parameters::LOW_SHELF_BYPASS, "Low Shelf Bypass"},
        {Parameters::LOW_MID_FREQ, "Low-Mid Freq"},
        {Parameters::LOW_MID_GAIN, "Low-Mid Gain"},
        {Parameters::LOW_MID_Q, "Low-Mid Q"},
        {Parameters::LOW_MID_BYPASS, "Low-Mid Bypass"},
        {Parameters::MID_FREQ, "Mid Freq"},
        {Parameters::MID_GAIN, "Mid Gain"},
        {Parameters::MID_Q, "Mid Q"},
        {Parameters::MID_BYPASS, "Mid Bypass"},
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
        {Parameters::LPF_BYPASS, "LPF Bypass"},
    };
}

enum CutSlope {
    Slope_12 = 0,
    Slope_24,
    Slope_36,
};


enum Filters
{
	HPF, LF, LMF, MF, HMF, HF, LPF,
};

struct PeakFilter : public juce::dsp::IIR::Filter<float>, public juce::AudioProcessorValueTreeState::Listener
{
    float freq;
    float gain{ 1.f };
    float q{ 1.f };
    Filters filterType;
    bool isBypassed{ false };
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    bool paramsChanged{ false };

};
struct CutFilter : public juce::dsp::ProcessorChain<PeakFilter, PeakFilter, PeakFilter>, public juce::AudioProcessorValueTreeState::Listener
{
    CutSlope slope{ Slope_24 };
    bool isHPF;
    bool isBypassed{ false };
    float freq;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    bool paramsChanged{ false };
};
struct ShelfFilter : public juce::dsp::IIR::Filter<float>, public juce::AudioProcessorValueTreeState::Listener
{
    float freq;
    float gain{ 1.f };
    float q{ 1.f };
    bool isLowShelf;
    bool isBypassed{ false };
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    bool paramsChanged{ false };

};


class EqPTAudioProcessor  : public juce::AudioProcessor, juce::ChangeBroadcaster
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    EqPTAudioProcessor();
    ~EqPTAudioProcessor() override;

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


    juce::AudioProcessorValueTreeState m_TreeState;
private:
    //==============================================================================

    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, ShelfFilter, PeakFilter, PeakFilter, PeakFilter, ShelfFilter, CutFilter>;
    std::array<MonoChain, 2> m_MonoChains;
    void updateFilters();
    void updateCutFilter(CutFilter& filter);
    void updateCutFilterParams(CutFilter& filter);
    void updatePeakFilter(PeakFilter& filter, int filterNo);
    void updateShelfFilters();
    juce::Array<PeakFilter*> getAllParametricFilters();
    juce::Array<ShelfFilter*> getAllShelfFilters();
    juce::Array<PeakFilter*> getAllFiltersFromCutFilter(const CutFilter&);
    void updateParametricFilters();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqPTAudioProcessor)
};
