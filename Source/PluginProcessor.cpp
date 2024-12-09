/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

//==============================================================================
ImagineAudioProcessor::ImagineAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations


    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )





#endif
{
    
    mFormatManager.registerBasicFormats();
    for (int i = 0; i < mNumVoices; i++) {

        mSampler.addVoice(new juce::SamplerVoice());

    }
    


    Py_Initialize();
    currentPath = juce::File::getCurrentWorkingDirectory();
    root = currentPath;

    pyEmbedder = std::make_unique<Pyembedder>();
}
ImagineAudioProcessor::~ImagineAudioProcessor()
{
    Py_Finalize();
    this->mFormatReader = nullptr;
}

//==============================================================================
const juce::String ImagineAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ImagineAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ImagineAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ImagineAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ImagineAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ImagineAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ImagineAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ImagineAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ImagineAudioProcessor::getProgramName (int index)
{
    return {};
}

void ImagineAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ImagineAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
}

void ImagineAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ImagineAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ImagineAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool ImagineAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ImagineAudioProcessor::createEditor()
{
    return new ImagineAudioProcessorEditor (*this);
}

//==============================================================================
void ImagineAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ImagineAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ImagineAudioProcessor();
}


void ImagineAudioProcessor::callPythonFunction(const std::string& img_path,
    const std::string& out_path,
    int kernel_size,
    int step_size,
    int sound_level,
    int sound_duration,
    int modulation_duration,
    float modulation_intensity,
    float modulation_envelope_intensity,
    float overtone_num_scalar,
    float lfo_scalar_freq,
    float lfo_scalar_amplitude,
    float lfo_intensity,
    float lfo_amount_scalar) {
    // Run the embedded Python code
    std::string fullPythonCode = std::string(pyEmbedder->Pythoncode1) +
        std::string(pyEmbedder->Pythoncode2) +
        std::string(pyEmbedder->Pythoncode3) +
        std::string(pyEmbedder->Pythoncode4) +
        std::string(pyEmbedder->Pythoncode5) +
        std::string(pyEmbedder->Pythoncode6) +
        std::string(pyEmbedder->Pythoncode7) +
        std::string(pyEmbedder->Pythoncode8) +
        std::string(pyEmbedder->Pythoncode9) +
        std::string(pyEmbedder->Pythoncode10) +
        std::string(pyEmbedder->Pythoncode11);

    // Use fullPythonCode where needed to execute or pass to the Python interpreter.
    PyRun_SimpleString(fullPythonCode.c_str());

    // Access the "__main__" module to retrieve the embedded function
    PyObject* mainModule = PyImport_AddModule("__main__");
    PyObject* globalDict = PyModule_GetDict(mainModule);
    PyObject* pFunc = PyDict_GetItemString(globalDict, "main_generation_handler");

    if (pFunc && PyCallable_Check(pFunc)) {
        // Create arguments for main_generation_handler
        PyObject* pArgs = PyTuple_Pack(15,
            PyUnicode_FromString(img_path.c_str()),
            PyUnicode_FromString(out_path.c_str()),
            PyLong_FromLong(kernel_size),
            PyLong_FromLong(step_size),
            PyLong_FromLong(sound_level),
            PyLong_FromLong(getSampleRate()),
            PyLong_FromLong(sound_duration),
            PyLong_FromLong(modulation_duration),
            PyFloat_FromDouble(modulation_intensity),
            PyFloat_FromDouble(modulation_envelope_intensity),
            PyFloat_FromDouble(overtone_num_scalar),
            PyFloat_FromDouble(lfo_scalar_freq),
            PyFloat_FromDouble(lfo_scalar_amplitude),
            PyFloat_FromDouble(lfo_intensity),
            PyFloat_FromDouble(lfo_amount_scalar)
        );

        // Call the Python function
        PyObject* pValue = PyObject_CallObject(pFunc, pArgs);

        if (pValue != nullptr) {
            if (PyUnicode_Check(pValue)) { // Ensure the return type is a string
                const char* resultCStr = PyUnicode_AsUTF8(pValue);
                std::string resultStr = resultCStr;  // Convert to std::string if needed
                juce::Logger::outputDebugString("Returned string from Python: " + resultStr);
                this->outputpath = resultCStr; // Set output path from result
            }
            else {
                juce::Logger::outputDebugString("Error: Python function did not return a string.");
            }

            Py_DECREF(pValue); // Release the Python object
        }
        else {
            PyErr_Print();
            juce::Logger::outputDebugString("Call to Python function failed");
        }

        Py_DECREF(pArgs);
    }
    else {
        if (PyErr_Occurred())
            PyErr_Print();
        juce::Logger::outputDebugString("Cannot find function main_generation_handler");
    }

}


void ImagineAudioProcessor::loadSound(juce::File& filepath) {
    juce::BigInteger range;
    range.setRange(0, 128, true);
    mFormatReader = mFormatManager.createReaderFor(filepath);

    int numChannels = static_cast<int>(mFormatReader->numChannels);
    int numSamples = static_cast<int>(mFormatReader->lengthInSamples);


    auto sampleLength = static_cast<int>(mFormatReader->lengthInSamples);
    std::unique_ptr<juce::AudioFormatReader> reader(mFormatManager.createReaderFor(filepath));

    if (reader != nullptr) {

        mainbuffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, reader->lengthInSamples);
        reader->read(mainbuffer.get(), 0, reader->lengthInSamples, 0, true, true);
    }

    mSampler.addSound(new juce::SamplerSound("Sample", *mFormatReader, range, 60, 0.1, 0.1, 10));
    

}


