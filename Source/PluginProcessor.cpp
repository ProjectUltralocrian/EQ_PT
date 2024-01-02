/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
EqPTAudioProcessor::EqPTAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), 
        m_TreeState{*this, nullptr, "PARAMS", createLayout()}

#endif
{
    
    using namespace Params;
    using params = Params::Parameters;
   
    for (int i = static_cast<int>(params::HPF_FREQ); i <= static_cast<int>(params::HPF_BYPASS); i++) {

        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.addParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<HPF>());
        }
    }
    for (int i = static_cast<int>(params::LPF_FREQ); i <= static_cast<int>(params::LPF_BYPASS); i++) {

        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.addParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<LPF>());
        }
    }

    for (int i = static_cast<int>(params::LOW_MID_FREQ); i <= static_cast<int>(params::HIGH_MID_BYPASS); i++) {
        for (auto& filter : getAllParametricFilters()) {
            m_TreeState.addParameterListener(ParameterNames[static_cast<params>(i)], filter);

        }
    }

    for (int i = static_cast<int>(params::LOW_SHELF_FREQ); i <= static_cast<int>(params::LOW_SHELF_BYPASS); i++) {

        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.addParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<LF>());
        }
    }
    for (int i = static_cast<int>(params::HIGH_SHELF_FREQ); i <= static_cast<int>(params::HIGH_SHELF_BYPASS); i++) {
        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.addParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<HF>());
        }
    }

    for (int i = 0; i < m_MonoChains.size(); ++i) {
        m_MonoChains[i].get<HPF>().isHPF = true;
        m_MonoChains[i].get<LPF>().isHPF = false;

        m_MonoChains[i].get<HPF>().freq = m_TreeState.getRawParameterValue(ParameterNames[params::HPF_FREQ])->load();  //20.f
        m_MonoChains[i].get<LPF>().freq = m_TreeState.getRawParameterValue(ParameterNames[params::LPF_FREQ])->load();
        
        m_MonoChains[i].get<LF>().freq = m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_FREQ])->load();
        m_MonoChains[i].get<HF>().freq = m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_FREQ])->load();  //20.f

        m_MonoChains[i].get<LMF>().freq = m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_FREQ])->load();
        m_MonoChains[i].get<MF>().freq = m_TreeState.getRawParameterValue(ParameterNames[params::MID_FREQ])->load();
        m_MonoChains[i].get<HMF>().freq = m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_FREQ])->load();
        m_MonoChains[i].get<LMF>().filterType = LMF;
        m_MonoChains[i].get<MF>().filterType = MF;
        m_MonoChains[i].get<HMF>().filterType = HMF;

        m_MonoChains[i].get<LF>().isLowShelf = true;
        m_MonoChains[i].get<HF>().isLowShelf = false;
      
    }

}

EqPTAudioProcessor::~EqPTAudioProcessor()
{
    using namespace Params;
    using params = Params::Parameters;
    for (int i = static_cast<int>(params::HPF_FREQ); i <= static_cast<int>(params::HPF_BYPASS); i++) {
        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.removeParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<HPF>());
        }
    }
    for (int i = static_cast<int>(params::LPF_FREQ); i <= static_cast<int>(params::LPF_BYPASS); i++) {
        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.removeParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<LPF>());
        }
    }

    for (int i = static_cast<int>(params::LOW_MID_FREQ); i <= static_cast<int>(params::HIGH_MID_BYPASS); i++) {
        for (auto& filter : getAllParametricFilters()) {
            m_TreeState.addParameterListener(ParameterNames[static_cast<params>(i)], filter);
        }
    }
    for (int i = static_cast<int>(params::LOW_SHELF_FREQ); i <= static_cast<int>(params::LOW_SHELF_BYPASS); i++) {
        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.removeParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<LF>());
        }

    }
    for (int i = static_cast<int>(params::HIGH_SHELF_FREQ); i <= static_cast<int>(params::HIGH_SHELF_BYPASS); i++) {
        for (int c = 0; c < m_MonoChains.size(); ++c) {
            m_TreeState.removeParameterListener(ParameterNames[static_cast<params>(i)], &m_MonoChains[c].get<HF>());
        }
    }
}

//==============================================================================
const juce::String EqPTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EqPTAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EqPTAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EqPTAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EqPTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EqPTAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EqPTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EqPTAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EqPTAudioProcessor::getProgramName (int index)
{
    return {};
}

void EqPTAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EqPTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.numChannels = getNumOutputChannels();
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    m_MonoChains[0].prepare(spec);
    m_MonoChains[1].prepare(spec);
}

void EqPTAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EqPTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EqPTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getNumChannels() > 1 ? block.getSingleChannelBlock(0) : block;
    auto rightBlock = block.getNumChannels() > 1 ? block.getSingleChannelBlock(1) : block;
    
	updateFilters();

    m_MonoChains[0].process(juce::dsp::ProcessContextReplacing<float>(leftBlock));
    m_MonoChains[1].process(juce::dsp::ProcessContextReplacing<float>(rightBlock));
    
    if (m_TreeState.getRawParameterValue(Params::ParameterNames[Params::Parameters::POLARITY_FLIP])->load() == true) {
        buffer.applyGain(-1.f);
    }

	buffer.applyGain(juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(Params::ParameterNames[Params::Parameters::OUT_GAIN])->load()));
    


}

//==============================================================================
bool EqPTAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EqPTAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
    //return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void EqPTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    m_TreeState.state.writeToStream(mos);
}

void EqPTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, sizeInBytes, true);
    auto state = juce::ValueTree::readFromStream(stream);
    if (state.isValid())
        m_TreeState.replaceState(state);
}

juce::AudioProcessorValueTreeState::ParameterLayout EqPTAudioProcessor::createLayout()
{   
    using namespace Params;
    using params = Params::Parameters;
    using floatRange = juce::NormalisableRange<float>;
   
    auto layout = juce::AudioProcessorValueTreeState::ParameterLayout();
    auto addFloatParam = [&layout](params p, floatRange nr, float defVal) {layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[p], ParameterNames[p], nr, defVal)); };
    auto addBoolParam = [&layout](params p, bool defVal) {layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[p], ParameterNames[p], defVal)); };
    auto addChoiceParam = [&layout](params p, juce::StringArray sa, int defVal) {layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterNames[p], ParameterNames[p], sa, defVal)); };
    
    addFloatParam(params::OUT_GAIN, floatRange(-60.f, 12.f, 0.5f, 1.5f), 0.f);
    addBoolParam(params::POLARITY_FLIP, false);
    addFloatParam(params::HPF_FREQ, floatRange(20.f, 20000.f, 1.f, 0.25f), 20.f);
    addChoiceParam(params::HPF_SLOPE, juce::StringArray{ "12 db/oct", "24 db/oct", "36 db/oct" }, 1);
    addBoolParam(params::HPF_BYPASS, false);
    addFloatParam(params::LOW_SHELF_FREQ, floatRange(20.f, 20000.f, 1.f, 0.25f), 100.f);
    addFloatParam(params::LOW_SHELF_GAIN, floatRange(-24.f, 24.f, 0.5f, 1.f), 0.f);
    addFloatParam(params::LOW_SHELF_Q, floatRange(0.1f, 5.f, 0.1f, 1.f), 1.f);
    addBoolParam(params::LOW_SHELF_BYPASS, false);
    addFloatParam(params::LOW_MID_FREQ, floatRange(20.f, 20000.f, 1.f, 0.25f), 200.f);
    addFloatParam(params::LOW_MID_GAIN, floatRange(-24.f, 24.f, 0.5f, 1.f), 0.f);
    addFloatParam(params::LOW_MID_Q, floatRange(0.1f, 5.f, 0.1f, 1.f), 1.f);
    addBoolParam(params::LOW_MID_BYPASS, false);
    addFloatParam(params::MID_FREQ, floatRange(20.f, 20000.f, 1.f, 0.25f), 1000.f);
    addFloatParam(params::MID_GAIN, floatRange(-24.f, 24.f, 0.5f, 1.f), 0.f);
    addFloatParam(params::MID_Q, floatRange(0.1f, 5.f, 0.1f, 1.f), 1.f);
    addBoolParam(params::MID_BYPASS, false);
    addFloatParam(params::HIGH_MID_FREQ, floatRange(20.f, 20000.f, 1.f, 0.25f), 4000.f);
    addFloatParam(params::HIGH_MID_GAIN, floatRange(-24.f, 24.f, 0.5f, 1.f), 0.f);
    addFloatParam(params::HIGH_MID_Q, floatRange(0.1f, 5.f, 0.1f, 1.f), 1.f);
    addBoolParam(params::HIGH_MID_BYPASS, false);
    addFloatParam(params::HIGH_SHELF_FREQ, floatRange(20.f, 20000.f, 1.f, 0.25f), 10000.f);
    addFloatParam(params::HIGH_SHELF_GAIN, floatRange(-24.f, 24.f, 0.5f, 1.f), 0.f);
    addFloatParam(params::HIGH_SHELF_Q, floatRange(0.1f, 5.f, 0.1f, 1.f), 1.f);
    addBoolParam(params::HIGH_SHELF_BYPASS, false);
    addFloatParam(params::LPF_FREQ, floatRange(20.f, 20000.f, 1.f, 0.25f), 20000.f);
    addChoiceParam(params::LPF_SLOPE, juce::StringArray{ "12 db/oct", "24 db/oct", "36 db/oct" }, 1);
    addBoolParam(params::LPF_BYPASS, false);
    return layout;
}

void EqPTAudioProcessor::updateFilters()
{
    for (int i = 0; i < m_MonoChains.size(); ++i) {
        updateCutFilter(m_MonoChains[i].get<HPF>());
        updateCutFilter(m_MonoChains[i].get<LPF>());
    }
    updateShelfFilters();
    updateParametricFilters();
}

void EqPTAudioProcessor::updateCutFilter(CutFilter& filter)
{
    if (!filter.paramsChanged) {
        return;
    }
    filter.setBypassed<0>(true);
    filter.setBypassed<1>(true);
    filter.setBypassed<2>(true);
    
    if (!filter.isBypassed)
    {
        switch (filter.slope) {
        case 2: {
            filter.get<2>().coefficients =
                filter.isHPF
                ? juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), filter.freq)
                : juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), filter.freq);
            filter.setBypassed<2>(false);

        }
        case 1: {
            filter.get<1>().coefficients =
                filter.isHPF
                ? juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), filter.freq)
                : juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), filter.freq);
            filter.setBypassed<1>(false);
        }
        case 0: {
            filter.get<0>().coefficients =
                filter.isHPF
                ? juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), filter.freq)
                : juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), filter.freq);
            filter.setBypassed<0>(false);
            break;
        }

        default: jassert(false);
        }
    }
    filter.paramsChanged = false;
}

juce::Array<PeakFilter*> EqPTAudioProcessor::getAllParametricFilters()
{
    juce::Array<PeakFilter*> output;
    for (int i = 0; i < m_MonoChains.size(); ++i) {
        output.add(&m_MonoChains[i].get<LMF>());
        output.add(&m_MonoChains[i].get<MF>());
        output.add(&m_MonoChains[i].get<HMF>());
    }
    return output;

}

juce::Array<ShelfFilter*> EqPTAudioProcessor::getAllShelfFilters()
{
    juce::Array<ShelfFilter*> output;
    for (int i = 0; i < m_MonoChains.size(); ++i) {
        output.add(&m_MonoChains[i].get<LF>());
        output.add(&m_MonoChains[i].get<HF>());
    }

    return output;
}


void EqPTAudioProcessor::updateParametricFilters()
{
    auto filters = getAllParametricFilters();
    for (auto& filter : filters) {
        if (!filter->paramsChanged) {
            continue;
        }
        switch (filter->filterType) {
        case LMF: {
            m_MonoChains[0].setBypassed<LMF>(filter->isBypassed);
            m_MonoChains[1].setBypassed<LMF>(filter->isBypassed);
            break;
        }
        case MF: {
            m_MonoChains[0].setBypassed<MF>(filter->isBypassed);
            m_MonoChains[1].setBypassed<MF>(filter->isBypassed);
            break;
        }
        case HMF: {
            m_MonoChains[0].setBypassed<HMF>(filter->isBypassed);
            m_MonoChains[1].setBypassed<HMF>(filter->isBypassed);
            break;
        }
        }
        filter->coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), filter->freq, filter->q, juce::Decibels::decibelsToGain(filter->gain));
        filter->paramsChanged = false;
    }
}


void EqPTAudioProcessor::updateShelfFilters()
{
    auto filters = getAllShelfFilters();
    for (auto& filter : filters) {
        if (!filter->paramsChanged) {
            continue;
        }
        if (filter->isLowShelf) {
            m_MonoChains[0].setBypassed<LF>(filter->isBypassed);
            m_MonoChains[1].setBypassed<LF>(filter->isBypassed);
        }
        else {
            m_MonoChains[0].setBypassed<HF>(filter->isBypassed);
            m_MonoChains[1].setBypassed<HF>(filter->isBypassed);
        }
        filter->coefficients =
            filter->isLowShelf
            ? juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), filter->freq, filter->q, juce::Decibels::decibelsToGain(filter->gain))
            : juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), filter->freq, filter->q, juce::Decibels::decibelsToGain(filter->gain));
        filter->paramsChanged = false;
    }
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EqPTAudioProcessor();
}

void CutFilter::parameterChanged(const juce::String& parameterID, float newValue)
{
    using namespace Params;
    using params = Params::Parameters;
    
    if (parameterID == ParameterNames[params::HPF_FREQ] && isHPF) {
        freq = newValue;
    }
    if (parameterID == ParameterNames[params::HPF_SLOPE] && isHPF) {
        slope = (CutSlope)newValue;
    }
    if (parameterID == ParameterNames[params::HPF_BYPASS] && isHPF) {
        isBypassed = newValue;
    }

    if (parameterID == ParameterNames[params::LPF_FREQ] && !isHPF) {
        freq = newValue;
    }
    if (parameterID == ParameterNames[params::LPF_SLOPE] && !isHPF) {
        slope = (CutSlope)newValue;
    }
    if (parameterID == ParameterNames[params::LPF_BYPASS] && !isHPF) {
        isBypassed = newValue;
    }
    paramsChanged = true;
}

void PeakFilter::parameterChanged(const juce::String& parameterID, float newValue)
{
    using namespace Params;
    using params = Params::Parameters;
    
    if (parameterID == ParameterNames[params::LOW_MID_FREQ] && filterType == LMF) {
        freq = newValue;
    }
    if (parameterID == ParameterNames[params::LOW_MID_GAIN] && filterType == LMF) {
        gain = newValue;
    }
    if (parameterID == ParameterNames[params::LOW_MID_Q] && filterType == LMF) {
        q = newValue;
    }
    if (parameterID == ParameterNames[params::LOW_MID_BYPASS] && filterType == LMF) {
        isBypassed = newValue;
    }
    if (parameterID == ParameterNames[params::MID_FREQ] && filterType == MF) {
        freq = newValue;
    }
    if (parameterID == ParameterNames[params::MID_GAIN] && filterType == MF) {
        gain = newValue;
    }
    if (parameterID == ParameterNames[params::MID_Q] && filterType == MF) {
        q = newValue;
    }
    if (parameterID == ParameterNames[params::MID_BYPASS] && filterType == MF) {
        isBypassed = newValue;
    }
    if (parameterID == ParameterNames[params::HIGH_MID_FREQ] && filterType == HMF) {
        freq = newValue;
    }
    if (parameterID == ParameterNames[params::HIGH_MID_GAIN] && filterType == HMF) {
        gain = newValue;
    }
    if (parameterID == ParameterNames[params::HIGH_MID_Q] && filterType == HMF) {
        q = newValue;
    }
    if (parameterID == ParameterNames[params::HIGH_MID_BYPASS] && filterType == HMF) {
        isBypassed = newValue;
    }
    DBG(filterType << ", " << freq << ", " << gain);
    paramsChanged = true;
}

void ShelfFilter::parameterChanged(const juce::String& parameterID, float newValue)
{
    using namespace Params;
    using params = Params::Parameters;

    if (parameterID == ParameterNames[params::LOW_SHELF_FREQ] && isLowShelf) {
        freq = newValue;
    }
    if (parameterID == ParameterNames[params::LOW_SHELF_GAIN] && isLowShelf) {
        gain = newValue;
    }
    if (parameterID == ParameterNames[params::LOW_SHELF_Q] && isLowShelf) {
        q = newValue;
    }
    if (parameterID == ParameterNames[params::LOW_SHELF_BYPASS] && isLowShelf) {
        isBypassed = newValue;
    }

    if (parameterID == ParameterNames[params::HIGH_SHELF_FREQ] && !isLowShelf) {
        freq = newValue;
    }
    if (parameterID == ParameterNames[params::HIGH_SHELF_GAIN] && !isLowShelf) {
        gain = newValue;
    }
    if (parameterID == ParameterNames[params::HIGH_SHELF_Q] && !isLowShelf) {
        q = newValue;
    }
    if (parameterID == ParameterNames[params::HIGH_SHELF_BYPASS] && !isLowShelf) {
        isBypassed = newValue;
    }

    paramsChanged = true;
}
