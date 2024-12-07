/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
ImagineAudioProcessorEditor::ImagineAudioProcessorEditor (ImagineAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), thumbnailCache(5),  // Initialize the thumbnail cache with a cache size of 5
    thumbnail(512, formatManager, thumbnailCache)
{   
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize (1280, 720);
    state = (Stopping);

    //addAndMakeVisible(&playButton);
    //playButton.onClick = [this] { playButtonClicked(); };
    //playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);



    //addAndMakeVisible(&stopButton);
    //stopButton.onClick = [this] { stopButtonClicked(); };
    //stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
       
    //Audio thumbnail init


    // Optionally, add more items to mainFlexBox as needed


    formatManager.registerBasicFormats();
    deviceManager.initialiseWithDefaultDevices(0, 2);
    deviceManager.addAudioCallback(&audioSourcePlayer);
    audioSourcePlayer.setSource(&transportSource);

    slider_window = std::make_unique<juce::DocumentWindow>("Generation Parameters", juce::Colours::lightgrey, juce::DocumentWindow::closeButton | juce::DocumentWindow::minimiseButton);
    windowComponent = new SliderWindow(this);
    slider_window->setContentOwned(windowComponent, true);
    slider_window->setVisible(false);
    slider_window->setBounds(0, 0, 400, 950);

}

ImagineAudioProcessorEditor::~ImagineAudioProcessorEditor()
{
    audioSourcePlayer.setSource(nullptr);
    deviceManager.removeAudioCallback(&audioSourcePlayer);
}


void ImagineAudioProcessorEditor::addSlider(juce::Slider& slider, juce::Label& label, const juce::String& name, double min, double max, double default)
{
    slider.setRange(min, max);
    slider.setValue(default);
    slider.setTextValueSuffix(name);
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);
    addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.attachToComponent(&slider, true);
    addAndMakeVisible(label);
}

void ImagineAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{

}

//==============================================================================
void ImagineAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour::fromString(app_colors[1]));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    if (thumbnail.getTotalLength() > 0.0)
    {
        auto area = getLocalBounds().reduced(10); // Set drawing area with padding
        thumbnail.drawChannels(g, area, 0.0, thumbnail.getTotalLength(), 1.0f);
    }
    else
    {
        g.drawText(imgstate, getLocalBounds(), juce::Justification::centred, true);
    }
}


void ImagineAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //playButton.setBounds(10, 10, 100, 30);
    //stopButton.setBounds(120, 10, 100, 30);

}

bool ImagineAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Accept only image files
    for (auto& file : files)
    {
        if (file.endsWithIgnoreCase(".jpg") || file.endsWithIgnoreCase(".png"))
            return true;
    }
    return false;
}

void ImagineAudioProcessorEditor::deleteFiles(const juce::File& folder)
{
    juce::DirectoryIterator iter(folder, false, "*", juce::File::findFiles);

    while (iter.next())
    {
        juce::File file = iter.getFile();
        if (file.existsAsFile())
        {
            file.deleteFile();
        }
    }
}

void ImagineAudioProcessorEditor::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    imgstate = "Path Loaded. Change parameters and Generate sound";
    thumbnail.clear();
    repaint();  // Repaint the editor to show the cleared thumbnail
    audioProcessor.mSampler.clearSounds();

//Default values
    windowComponent->getKernelSlider().setValue(25);
    windowComponent->getStepSlider().setValue(10);
    windowComponent->getSoundLevelSlider().setValue(1);
    windowComponent->getSoundDurationSlider().setValue(10);
    windowComponent->getModulationIntensitySlider().setValue(.8);
    windowComponent->getModulationEnvelopeIntensitySlider().setValue(.2);
    windowComponent->getModulationDurationSlider().setValue(6);
    windowComponent->getLfoScalarFreqSlider().setValue(.5);
    windowComponent->getLfoScalarAmplitudeSlider().setValue(1);
    windowComponent->getOvertoneNumScalarSlider().setValue(1);
    windowComponent->getLfoAmountScalarSlider().setValue(1);


    documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    mainFolder = createFolderIfNotExists(documentsDir, "Imagine");
    outputFolder = createFolderIfNotExists(mainFolder, "output");
    outputPath = outputFolder.getFullPathName().toStdString() + "\\";
    
    for (const auto& file : files)
    {
        juce::File imageFile(file);
        imagePath = imageFile.getFullPathName().toStdString();
    }
    slider_window->setVisible(true);

}

juce::File ImagineAudioProcessorEditor::createFolderIfNotExists(const juce::File& parentFolder, const std::string& folderName)
{
    juce::File folder = parentFolder.getChildFile(folderName);

    if (!folder.exists())
    {
        bool success = folder.createDirectory();
        if (success)
        {
            DBG(folderName + " folder created successfully.");
        }
        else
        {
            DBG("Failed to create " + folderName + " folder.");
        }
    }
    else
    {
        DBG(folderName + " folder already exists.");
    }

    return folder;
}

//void ImagineAudioProcessorEditor::playButtonClicked()
//{
//    changeState(Starting);
//}
//
//void ImagineAudioProcessorEditor::stopButtonClicked()
//{
//    changeState(Stopping);
//}

//void ImagineAudioProcessorEditor::playWavFile()
//{
//    juce::File wavFile = audioProcessor.outputpath;
//
//    if (wavFile.existsAsFile())
//    {
//        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(wavFile));
//
//        if (reader != nullptr)
//        {
//            std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader.release(), true));
//            transportSource.setSource(newSource.get(), 0, nullptr, 44800);
//            readerSource.reset(newSource.release());
//            transportSource.start();
//        }
//    }
//}

void ImagineAudioProcessorEditor::changeState(TransportState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
        case Starting:
            //playWavFile();
            transportSource.start();
            break;

        case Stopping:
            transportSource.stop();
            break;
        }
    }
}

void ImagineAudioProcessorEditor::generateSound()
{
    
    audioProcessor.mSampler.clearSounds();
    audioProcessor.callPythonFunction(imagePath,
        outputPath,
        (int) windowComponent-> getKernelSlider().getValue(),
        (int) windowComponent->getStepSlider().getValue(),
        windowComponent->getSoundLevelSlider().getValue(),
        windowComponent->getSoundDurationSlider().getValue(),
        windowComponent->getModulationDurationSlider().getValue(),
        windowComponent->getModulationIntensitySlider().getValue(),
        windowComponent->getModulationEnvelopeIntensitySlider().getValue(),
        windowComponent->getOvertoneNumScalarSlider().getValue(),
        windowComponent->getLfoScalarFreqSlider().getValue(),
        windowComponent->getLfoScalarAmplitudeSlider().getValue(),
        windowComponent->getLfoIntensitySlider().getValue(),
        windowComponent->getLfoAmountScalarSlider().getValue());
    
    //slider_window->setVisible(false);
    audioProcessor.loadSound(audioProcessor.outputpath);
    thumbnail.setSource(new juce::FileInputSource(audioProcessor.outputpath));
    repaint();
}

