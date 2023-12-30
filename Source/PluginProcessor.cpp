/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"



//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
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
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.numChannels = getNumOutputChannels();
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    m_LeftChain.prepare(spec);
    m_RightChain.prepare(spec);
}

void NewProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        //auto* channelData = buffer.getWritePointer (channel);

    }

    juce::dsp::AudioBlock<float> block(buffer);
    auto leftBlock = block.getNumChannels() > 1 ? block.getSingleChannelBlock(0) : block;
    auto rightBlock = block.getNumChannels() > 1 ? block.getSingleChannelBlock(1) : block;
    
    using namespace Params;
    using params = Params::Parameters;

    m_LeftChain.get<HPF>().setBypassed<0>(true);
    m_RightChain.get<HPF>().setBypassed<0>(true);
    m_LeftChain.get<HPF>().setBypassed<1>(true);
    m_RightChain.get<HPF>().setBypassed<1>(true);
    m_LeftChain.get<HPF>().setBypassed<2>(true);
    m_RightChain.get<HPF>().setBypassed<2>(true);

    auto slope = m_TreeState.getParameterAsValue(ParameterNames[params::HPF_SLOPE]).getValue().toString().substring(0, 2).getIntValue();
    
    if (m_TreeState.getRawParameterValue(ParameterNames[params::HPF_BYPASS])->load() == false)
    {
        switch (slope) {
        case 2: {
            m_LeftChain.get<HPF>().get<2>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HPF_FREQ])->load());
            m_RightChain.get<HPF>().get<2>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HPF_FREQ])->load());
            m_LeftChain.get<HPF>().setBypassed<2>(false);
            m_RightChain.get<HPF>().setBypassed<2>(false);
        }
        case 1: {
            m_LeftChain.get<HPF>().get<1>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HPF_FREQ])->load());
            m_RightChain.get<HPF>().get<1>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HPF_FREQ])->load());
            m_LeftChain.get<HPF>().setBypassed<1>(false);
            m_RightChain.get<HPF>().setBypassed<1>(false);
        }
        case 0: {
            m_LeftChain.get<HPF>().get<0>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HPF_FREQ])->load());
            m_RightChain.get<HPF>().get<0>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HPF_FREQ])->load());
            m_LeftChain.get<HPF>().setBypassed<0>(false);
            m_RightChain.get<HPF>().setBypassed<0>(false);
            break;
        }

        default: jassert(false);

        }
    }

    m_LeftChain.get<LPF>().setBypassed<0>(true);
    m_RightChain.get<LPF>().setBypassed<0>(true);
    m_LeftChain.get<LPF>().setBypassed<1>(true);
    m_RightChain.get<LPF>().setBypassed<1>(true);
    m_LeftChain.get<LPF>().setBypassed<2>(true);
    m_RightChain.get<LPF>().setBypassed<2>(true);
    
    slope = m_TreeState.getParameterAsValue(ParameterNames[params::LPF_SLOPE]).getValue().toString().substring(0, 2).getIntValue();
    
    
    if (m_TreeState.getRawParameterValue(ParameterNames[params::LPF_BYPASS])->load() == false)
    {
        switch (slope) {
        case 2: {
            m_LeftChain.get<LPF>().get<2>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LPF_FREQ])->load());
            m_RightChain.get<LPF>().get<2>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LPF_FREQ])->load());
            m_LeftChain.get<LPF>().setBypassed<2>(false);
            m_RightChain.get<LPF>().setBypassed<2>(false);
        }
        case 1: {
            m_LeftChain.get<LPF>().get<1>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LPF_FREQ])->load());
            m_RightChain.get<LPF>().get<1>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LPF_FREQ])->load());
            m_LeftChain.get<LPF>().setBypassed<1>(false);
            m_RightChain.get<LPF>().setBypassed<1>(false);
        }
        case 0: {
            m_LeftChain.get<LPF>().get<0>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LPF_FREQ])->load());
            m_RightChain.get<LPF>().get<0>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LPF_FREQ])->load());
            m_LeftChain.get<LPF>().setBypassed<0>(false);
            m_RightChain.get<LPF>().setBypassed<0>(false);
            break;
        }

        default: jassert(false);

        }

    }

    if (m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_BYPASS])->load() == false)
    {
		m_LeftChain.setBypassed<LF>(false);
		m_RightChain.setBypassed<LF>(false);
		m_LeftChain.get<LF>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_GAIN])->load()));
		m_RightChain.get<LF>().coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::LOW_SHELF_GAIN])->load()));
    }
    else
    {
		m_LeftChain.setBypassed<LF>(true);
		m_RightChain.setBypassed<LF>(true);
    }



    if (m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_BYPASS])->load() == false)
    {
		m_LeftChain.setBypassed<LMF>(false);
		m_RightChain.setBypassed<LMF>(false);
		m_LeftChain.get<LMF>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_GAIN])->load()));
		m_RightChain.get<LMF>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::LOW_MID_GAIN])->load()));
    }
    else
    {
		m_LeftChain.setBypassed<LMF>(true);
		m_RightChain.setBypassed<LMF>(true);
    }
    
    if (m_TreeState.getRawParameterValue(ParameterNames[params::MID_BYPASS])->load() == false)
    {
		m_LeftChain.setBypassed<MF>(false);
		m_RightChain.setBypassed<MF>(false);
		m_LeftChain.get<MF>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::MID_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::MID_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::MID_GAIN])->load()));
		m_RightChain.get<MF>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::MID_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::MID_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::MID_GAIN])->load()));
    }
    else
    {
		m_LeftChain.setBypassed<MF>(true);
		m_RightChain.setBypassed<MF>(true);
    }


    if (m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_BYPASS])->load() == false)
    {
		m_LeftChain.setBypassed<HMF>(false);
		m_RightChain.setBypassed<HMF>(false);
		m_LeftChain.get<HMF>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_GAIN])->load()));
		m_RightChain.get<HMF>().coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_MID_GAIN])->load()));
    }
    else
    {
		m_LeftChain.setBypassed<HMF>(true);
		m_RightChain.setBypassed<HMF>(true);
    }


    if (m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_BYPASS])->load() == false)
    {
		m_LeftChain.setBypassed<HF>(false);
		m_RightChain.setBypassed<HF>(false);
		m_LeftChain.get<HF>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_GAIN])->load()));
		m_RightChain.get<HF>().coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(getSampleRate(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_FREQ])->load(), m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_Q])->load(), juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::HIGH_SHELF_GAIN])->load()));
    }
    else
    {
		m_LeftChain.setBypassed<HF>(true);
		m_RightChain.setBypassed<HF>(true);
    }

    m_LeftChain.process(juce::dsp::ProcessContextReplacing<float>(leftBlock));
    m_RightChain.process(juce::dsp::ProcessContextReplacing<float>(rightBlock));
    
    if (m_TreeState.getRawParameterValue(ParameterNames[params::POLARITY_FLIP])->load() == true) {
        buffer.applyGain(-1.f);
    }

	buffer.applyGain(juce::Decibels::decibelsToGain(m_TreeState.getRawParameterValue(ParameterNames[params::OUT_GAIN])->load()));
    


}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
    //return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createLayout()
{   
    using namespace Params;
    using params = Params::Parameters;
    auto layout = juce::AudioProcessorValueTreeState::ParameterLayout();
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::OUT_GAIN], ParameterNames[params::OUT_GAIN], juce::NormalisableRange<float>(-60.f, 12.f, 0.5f, 1.5f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::POLARITY_FLIP], ParameterNames[params::POLARITY_FLIP],false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::HPF_FREQ], ParameterNames[params::HPF_FREQ], juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterNames[params::HPF_SLOPE], ParameterNames[params::HPF_SLOPE], juce::StringArray{ "12 db/oct", "24 db/oct", "36 db/oct" }, 1));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::HPF_BYPASS], ParameterNames[params::HPF_BYPASS], false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::LOW_SHELF_FREQ], ParameterNames[params::LOW_SHELF_FREQ], juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 100.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::LOW_SHELF_GAIN], ParameterNames[params::LOW_SHELF_GAIN], juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::LOW_SHELF_Q], ParameterNames[params::LOW_SHELF_Q], juce::NormalisableRange<float>(0.1f, 5.f, 0.1f, 1.f), 1.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::LOW_SHELF_BYPASS], ParameterNames[params::LOW_SHELF_BYPASS], false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::LOW_MID_FREQ], ParameterNames[params::LOW_MID_FREQ], juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 200.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::LOW_MID_GAIN], ParameterNames[params::LOW_MID_GAIN], juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::LOW_MID_Q], ParameterNames[params::LOW_MID_Q], juce::NormalisableRange<float>(0.1f, 5.f, 0.1f, 1.f), 1.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::LOW_MID_BYPASS], ParameterNames[params::LOW_MID_BYPASS], false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::MID_FREQ], ParameterNames[params::MID_FREQ], juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 1000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::MID_GAIN], ParameterNames[params::MID_GAIN], juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::MID_Q], ParameterNames[params::MID_Q], juce::NormalisableRange<float>(0.1f, 5.f, 0.1f, 1.f), 1.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::MID_BYPASS], ParameterNames[params::MID_BYPASS], false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::HIGH_MID_FREQ], ParameterNames[params::HIGH_MID_FREQ], juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 4000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::HIGH_MID_GAIN], ParameterNames[params::HIGH_MID_GAIN], juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::HIGH_MID_Q], ParameterNames[params::HIGH_MID_Q], juce::NormalisableRange<float>(0.1f, 5.f, 0.1f, 1.f), 1.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::HIGH_MID_BYPASS], ParameterNames[params::HIGH_MID_BYPASS], false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::HIGH_SHELF_FREQ], ParameterNames[params::HIGH_SHELF_FREQ], juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 10000.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::HIGH_SHELF_GAIN], ParameterNames[params::HIGH_SHELF_GAIN], juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::HIGH_SHELF_Q], ParameterNames[params::HIGH_SHELF_Q], juce::NormalisableRange<float>(0.1f, 5.f, 0.1f, 1.f), 1.f));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::HIGH_SHELF_BYPASS], ParameterNames[params::HIGH_SHELF_BYPASS], false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(ParameterNames[params::LPF_FREQ], ParameterNames[params::LPF_FREQ], juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20000.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterNames[params::LPF_SLOPE], ParameterNames[params::LPF_SLOPE], juce::StringArray{ "12 db/oct", "24 db/oct", "36 db/oct" }, 1));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParameterNames[params::LPF_BYPASS], ParameterNames[params::LPF_BYPASS], false));
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
