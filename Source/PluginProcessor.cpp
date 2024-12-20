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
    ), waveviewer(1)





#endif
{

    waveviewer.setRepaintRate(30);
    waveviewer.setBufferSize(256);


    mFormatManager.registerBasicFormats();
    for (int i = 0; i < mNumVoices; i++) {

        mSampler.addVoice(new juce::SamplerVoice());

    }



    Py_Initialize();
    currentPath = juce::File::getCurrentWorkingDirectory();
    root = currentPath;

    pyEmbedder = std::make_unique<Pyembedder>();



    addParameter(reverbRoomSize = new juce::AudioParameterFloat("reverbRoomSize", "Reverb Room Size", 0.0f, 1.0f, 0.5f));
    addParameter(reverbDamping = new juce::AudioParameterFloat("reverbDamping", "Reverb Damping", 0.0f, 1.0f, 0.5f));
    addParameter(reverbWet = new juce::AudioParameterFloat("reverbWet", "Reverb Dry", 0.0f, 1.0, 0.33f));
    addParameter(reverbDry = new juce::AudioParameterFloat("reverbDry", "Reverb Dry", 0.0f, 1.0f, 0.4f));
    addParameter(reverbWidth = new juce::AudioParameterFloat("reverbWidth", "Reverb Width", 0.0f, 1.0f, 1.0f));
    addParameter(reverbEnabled = new juce::AudioParameterBool("reverbEnabled", "Reverb Enabled", false));

    addParameter(delayTime = new juce::AudioParameterFloat("delayTime", "Delay Time", 0.0f, 2.0f, 0.5f));
    addParameter(feedback = new juce::AudioParameterFloat("feedback", "Feedback", 0.0f, 2.0f, 0.5f));
    addParameter(mix = new juce::AudioParameterFloat("mix", "Mix", 0.0f, 1.0f, 0.5f));
    addParameter(delayEnabled = new juce::AudioParameterBool("delayEnabled", "Delay Enabled", false));

    addParameter(gainS = new juce::AudioParameterFloat("gainS", "GainS", 0.0f, 10.0f, 1.0f));
    addParameter(filterFreq = new juce::AudioParameterInt("filterFreq", "Filter Frequency", 0, 5000, 20));
    addParameter(filterQ = new juce::AudioParameterFloat("filterQ", "FilterQ", 0.0f, 5.0f, 0.1f));
    addParameter(attack = new juce::AudioParameterFloat("attack", "Attack", 0.0f, 5.0f, 0.1f));
    addParameter(decay = new juce::AudioParameterFloat("decay", "Decay", 0.0f, 5.0f, 0.1f));
    addParameter(sustain = new juce::AudioParameterFloat("sustain", "Sustain", 0.0f, 1.0f, 1.0f));
    addParameter(release = new juce::AudioParameterFloat("release", "Release", 0.0f, 5.0f, 2.0f));

    addParameter(filterEnabled = new juce::AudioParameterBool("filterEnabled", "Filter Enabled", false));
    addParameter(filterOne = new juce::AudioParameterBool("filterOne", "Filter One", true));
    addParameter(filterTwo = new juce::AudioParameterBool("filterTwo", "Filter Two", false));
    addParameter(filterThree = new juce::AudioParameterBool("filterThree", "Filter Three", false));
    addParameter(filterFour = new juce::AudioParameterBool("filterFour", "Filter Four", false));

    addParameter(isfilterOne = new juce::AudioParameterBool("isfilterOne", "isFilter One", false));
    addParameter(isfilterTwo = new juce::AudioParameterBool("isfilterTwo", "isFilter Two", false));
    addParameter(isfilterThree = new juce::AudioParameterBool("isfilterThree", "isFilter Three", false));
    addParameter(isfilterFour = new juce::AudioParameterBool("isfilterFour", "isFilter Four", false));

    juce::StringArray choices = { "LowPass", "HighPass", "BandPass", "Notch" };
    addParameter(filterType = new juce::AudioParameterChoice("filterType", "Filter Type", choices, 0));

    addParameter(kernel = new juce::AudioParameterInt("kernel", "kernel", 0, 50, 25));
    addParameter(stepSize = new juce::AudioParameterInt("stepSize", "Step Size", 0, 50, 10));
    addParameter(level = new juce::AudioParameterFloat("level", "Level", 0, 10, 1));
    addParameter(duration = new juce::AudioParameterInt("duration", "Duration", 0, 30, 10));

    addParameter(filter1Type = new juce::AudioParameterInt("filter1Type", "Filter1 Type", 0, 4, 0));
    addParameter(filter2Type = new juce::AudioParameterInt("filter2Type", "Filter2 Type", 0, 4, 0));
    addParameter(filter3Type = new juce::AudioParameterInt("filter3Type", "Filter3 Type", 0, 4, 0));
    addParameter(filter4Type = new juce::AudioParameterInt("filter4Type", "Filter4 Type", 0, 4, 0));

    addParameter(filter1Freq = new juce::AudioParameterInt("filter1Freq", "Filter1 Frequency", 0, 5000, 20));
    addParameter(filter2Freq = new juce::AudioParameterInt("filter2Freq", "Filter2 Frequency", 0, 5000, 20));
    addParameter(filter3Freq = new juce::AudioParameterInt("filter3Freq", "Filter3 Frequency", 0, 5000, 20));
    addParameter(filter4Freq = new juce::AudioParameterInt("filter4Freq", "Filter4 Frequency", 0, 5000, 20));

    addParameter(filter1Q = new juce::AudioParameterFloat("filter1Q", "Filter1Q", 0.0f, 5.0f, 0.1f));
    addParameter(filter2Q = new juce::AudioParameterFloat("filter2Q", "Filter2Q", 0.0f, 5.0f, 0.1f));
    addParameter(filter3Q = new juce::AudioParameterFloat("filter3Q", "Filter3Q", 0.0f, 5.0f, 0.1f));
    addParameter(filter4Q = new juce::AudioParameterFloat("filter4Q", "Filter4Q", 0.0f, 5.0f, 0.1f));

    addParameter(modulationIntensity = new juce::AudioParameterFloat("modulationIntensity", "Modulation Intensity", 0.0f, 1.0f, 0.8f));
    addParameter(modulationEnvelopeIntensity = new juce::AudioParameterFloat("modulationEnvelopeIntensity", "Modulation Envelope Intensity", 0.0f, 1.0f, 0.2f));
    addParameter(modulationDuration = new juce::AudioParameterFloat("modulationDuration", "Modulation Duration", -20.0f, 20.0f, 6.0f));
    addParameter(lfoScalarFreq = new juce::AudioParameterFloat("lfoScalarFreq", "Lfo Scalar Freq", 0.0f, 50.0f, 0.5f));
    addParameter(lfoScalarAmplitude = new juce::AudioParameterFloat("lfoScalarAmplitude", "Lfo Scalar Amplitude", 0.0f, 1.0f, 0.6f));
    addParameter(lfoIntensity = new juce::AudioParameterFloat("lfoIntensity", "Lfo Intensity", 0.0f, 1.0f, 0.6f));
    addParameter(overtoneNumScalar = new juce::AudioParameterFloat("overtoneNumScalar", "Overtone Num Scalar", 0.0f, 4.0f, 1.0f));
    addParameter(lfoAmountScalar = new juce::AudioParameterFloat("lfoAmountScalar", "Lfo Amount Scalar", 0.0f, 4.0f, 1.0f));

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

void ImagineAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String ImagineAudioProcessor::getProgramName(int index)
{
    return {};
}

void ImagineAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}


//==============================================================================
void ImagineAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mSampler.setCurrentPlaybackSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    filter1.prepare(spec);
    filter2.prepare(spec);
    filter3.prepare(spec);
    filter4.prepare(spec);

    // Configure ADSR with sample rate
    adsr.setSampleRate(sampleRate);

    // Default ADSR settings (optional)
    adsrParams.attack = 0.0f;
    adsrParams.decay = 0.0f;
    adsrParams.sustain = 1.0f;
    adsrParams.release = 0.0f;
    adsr.setParameters(adsrParams);
    adsr.reset();

    delayLine.reset();
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples((int)(2.0 * sampleRate));
}

void ImagineAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ImagineAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void ImagineAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear output channels if they exceed input channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Check MIDI buffer for note-on and note-off events to control ADSR
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            adsr.noteOn();
        }
        else if (message.isNoteOff())
        {
            adsr.noteOff();
        }
    }

    // Render the sample block and apply gain
    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    auto& gain = effectsChain.get<0>();
    gain.setGainLinear(currentGain);
    juce::dsp::AudioBlock<float> block(buffer);
    gain.process(juce::dsp::ProcessContextReplacing<float>(block));


    if (delayEnabled->get())
    {
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        const int delayInSample = (int)(delayTime->get() * getSampleRate());

        delayLine.setDelay(delayInSample);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);

            auto delaySample = delayLine.popSample(channel);
            delayLine.pushSample(channel, channelData[0] + feedback->get() * delaySample);
            for (int i = 0; i < numSamples; ++i)
            {
                float delayedSample = delayLine.popSample(channel);
                delayLine.pushSample(channel, channelData[i] + feedback->get() * delayedSample);
                channelData[i] = channelData[i] * (1.0f - mix->get()) + delayedSample * mix->get();
            }
        }
    }


    // Apply reverb if enabled
    if (reverbEnabled->get())
    {
        auto& reverb = effectsChain.get<1>();
        reverb.setParameters({
            reverbRoomSize->get(),
            reverbDamping->get(),
            reverbWet->get(),
            reverbDry->get(),
            reverbWidth->get(),
            0.0f
            });
        reverb.process(juce::dsp::ProcessContextReplacing<float>(block));
    }

    // Apply filters
    if (filter1_enabled) filter1.process(juce::dsp::ProcessContextReplacing<float>(block));
    if (filter2_enabled) filter2.process(juce::dsp::ProcessContextReplacing<float>(block));
    if (filter3_enabled) filter3.process(juce::dsp::ProcessContextReplacing<float>(block));
    if (filter4_enabled) filter4.process(juce::dsp::ProcessContextReplacing<float>(block));

    // Apply ADSR envelope to the buffer
    adsr.applyEnvelopeToBuffer(buffer, 0, buffer.getNumSamples());

    // Push buffer to visualizer
    waveviewer.pushBuffer(buffer);
}

//==============================================================================
bool ImagineAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ImagineAudioProcessor::createEditor()
{
    return new ImagineAudioProcessorEditor(*this);
}

//==============================================================================
void ImagineAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ImagineAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

class ImagineSynth : public juce::SynthesiserSound {
public:
    bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel(int /*midiChannel*/) override { return true; }
};

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

void ImagineAudioProcessor::onBlockChange(int start, int end)
{
    int blockSize = end - start;
    if (blockSize > 0)
    {
        selectedBlock = std::make_unique<juce::AudioBuffer<float>>(mainbuffer->getNumChannels(), blockSize);

        for (int channel = 0; channel < mainbuffer->getNumChannels(); ++channel)
        {
            selectedBlock->copyFrom(channel, 0, *mainbuffer, channel, start, blockSize);
        }

        int firstBlockSize = start;
        firstBlock = std::make_unique<juce::AudioBuffer<float>>(mainbuffer->getNumChannels(), firstBlockSize);
        for (int channel = 0; channel < mainbuffer->getNumChannels(); ++channel)
        {
            firstBlock->copyFrom(channel, 0, *mainbuffer, channel, 0, firstBlockSize);
        }
        int lastBlockSize = mainbuffer->getNumSamples() - end;
        lastBlock = std::make_unique<juce::AudioBuffer<float>>(mainbuffer->getNumChannels(), lastBlockSize);
        for (int channel = 0; channel < mainbuffer->getNumChannels(); ++channel)
        {
            lastBlock->copyFrom(channel, 0, *mainbuffer, channel, end, lastBlockSize);
        }


        juce::File outputFile = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("Imagine")
            .getChildFile("temp")
            .getChildFile("block.wav");
        outputFile.deleteFile();
        // Make sure the directory exists, if not create it
        if (!outputFile.getParentDirectory().exists())
        {
            if (!outputFile.getParentDirectory().createDirectory())
            {
                DBG("Error: Failed to create directory: " << outputFile.getParentDirectory().getFullPathName());
                return;
            }
        }

        juce::WavAudioFormat wavFormat;
        std::unique_ptr<juce::FileOutputStream> outputStream(outputFile.createOutputStream());
        std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(outputStream.get(),
            getSampleRate(),
            selectedBlock->getNumChannels(),
            16,
            {},
            0));

        if (writer == nullptr)
        {
            DBG("Error Creating WAV File Writer!");
            return;
        }

        bool writeResult = writer->writeFromAudioSampleBuffer(*selectedBlock, 0, selectedBlock->getNumSamples());
        if (!writeResult)
        {
            DBG("Error writing to WAV file");
            return;
        }


        DBG("WAV file saved successfully!");
        writer->flush();
        outputStream->flush();
        outputStream.release();
        mFormatReader = mFormatManager.createReaderFor(outputFile);

        std::unique_ptr<juce::AudioFormatReader> reader(mFormatManager.createReaderFor(outputFile));
        if (reader == nullptr)
        {
            selectedBlock = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, reader->lengthInSamples);
            reader->read(selectedBlock.get(), 0, reader->lengthInSamples, 0, true, true);
            return;
        }

        juce::BigInteger selectedRange;
        selectedRange.setRange(0, 128, true);
        mSampler.clearSounds();
        mSampler.addSound(new juce::SamplerSound("Selected Block", *reader, selectedRange, 60, 0.1, 0.1, 10));
    }
}


void ImagineAudioProcessor::saveSound(const juce::File& file)
{
    if (file.hasFileExtension("imag"))
    {
        std::unique_ptr<juce::XmlElement> rootElement = juce::XmlDocument::parse(file);
        if (rootElement == nullptr)
        {
            rootElement = std::make_unique<juce::XmlElement>("Root");
        }

        if (auto* existingAudioData = rootElement->getChildByName("AudioData"))
        {
            rootElement->removeChildElement(existingAudioData, true);
        }

        juce::XmlElement* audioData = new juce::XmlElement("AudioData");
        int sampleRate = getSampleRate();
        int numChannels = mainbuffer->getNumChannels();
        int numSamples = mainbuffer->getNumSamples();
        audioData->setAttribute("SampleRate", sampleRate);
        audioData->setAttribute("NumChannels", numChannels);
        juce::MemoryOutputStream memoryStream;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                memoryStream.writeFloat(mainbuffer->getSample(channel, sample));
            }
        }
        juce::String base64Audio = juce::Base64::toBase64(memoryStream.getData(), memoryStream.getDataSize());

        audioData->setAttribute("AudioData", base64Audio);

        rootElement->addChildElement(audioData);

        if (!rootElement->writeToFile(file, {}))
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                "Save Error",
                "Could not save parameters to file.");
        }
    }
}


juce::File ImagineAudioProcessor::loadFileSound(const juce::File& file)
{
    if (file.existsAsFile())
    {
        juce::XmlDocument doc(file);
        std::unique_ptr<juce::XmlElement> rootElement(doc.getDocumentElement());

        if (rootElement == nullptr || !rootElement->hasTagName("Root"))
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                "Load Error",
                "Could not load audio data from file.");
            return file;
        }

        auto* audioData = rootElement->getChildByName("AudioData");
        if (audioData != nullptr)
        {
            int sampleRate = audioData->getIntAttribute("SampleRate", 44100);
            int numChannels = audioData->getIntAttribute("NumChannels", 2);
            juce::String base64Audio = audioData->getStringAttribute("AudioData");

            juce::MemoryOutputStream decodedAudioStream;
            if (juce::Base64::convertFromBase64(decodedAudioStream, base64Audio))
            {
                juce::MemoryInputStream memoryStream(decodedAudioStream.getData(), decodedAudioStream.getDataSize(), false);
                int numSamples = memoryStream.getDataSize() / (numChannels * sizeof(float));
                juce::AudioBuffer<float> buffer(numChannels, numSamples);

                for (int channel = 0; channel < numChannels; ++channel)
                {
                    for (int sample = 0; sample < numSamples; ++sample)
                    {
                        buffer.setSample(channel, sample, memoryStream.readFloat());
                    }
                }

                juce::File tempWavFile = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory).getChildFile("temp.wav");
                tempWavFile.deleteFile();
                juce::WavAudioFormat wavFormat;
                std::unique_ptr<juce::FileOutputStream> tempOutputStream(tempWavFile.createOutputStream());
                std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(tempOutputStream.get(),
                    sampleRate,
                    numChannels,
                    24,
                    {}, 0));

                if (writer != nullptr)
                {
                    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
                    writer->flush();
                    tempOutputStream->flush();
                    tempOutputStream.release();
                }

                std::unique_ptr<juce::AudioFormatReader> reader(mFormatManager.createReaderFor(tempWavFile));
                if (reader != nullptr)
                {
                    mainbuffer = std::make_unique<juce::AudioBuffer<float>>(reader->numChannels, reader->lengthInSamples);
                    reader->read(mainbuffer.get(), 0, reader->lengthInSamples, 0, true, true);

                    juce::BigInteger range;
                    range.setRange(0, 128, true);
                    mSampler.clearSounds();
                    mSampler.addSound(new juce::SamplerSound("Sample", *reader, range, 60, 0.1, 0.1, 10));
                }
                return tempWavFile;
            }
        }
    }
}

void ImagineAudioProcessor::setFilter(int filterIndex, std::string type, int frequency, float qFactor)
{
    auto newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), frequency, qFactor);


    if (type == "LowPass") {
        newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), frequency, qFactor);
    }
    else if (type == "HighPass") {
        newCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), frequency, qFactor);
    }
    else if (type == "BandPass") {
        newCoefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(), frequency, qFactor);
    }

    else if (type == "Notch") {
        newCoefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(getSampleRate(), frequency, qFactor);
    }

    if (newCoefficients != nullptr)
    {
        switch (filterIndex)
        {
        case 1:
            *filter1.state = *newCoefficients;
            break;
        case 2:
            *filter2.state = *newCoefficients;
            break;
        case 3:
            *filter3.state = *newCoefficients;
            break;
        case 4:
            *filter4.state = *newCoefficients;
            break;
        default:
            break;
        }
    }
}

void ImagineAudioProcessor::updateADSRParameters(float attack, float decay, float sustain, float release)
{
    adsrParams.attack = attack;
    adsrParams.decay = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;
    adsr.setParameters(adsrParams);
}




