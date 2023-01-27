cmake_minimum_required(VERSION 3.7.2)
project(Transfer VERSION 0.0.2)
find_package(JUCE CONFIG REQUIRED)
set(CMAKE_CXX_STANDARD 20)
juce_add_plugin(Transfer
COMPANY_NAME Syl Morrison
PLUGIN_MANUFACTURER_CODE SAPI
PLUGIN_CODE SM01
FORMATS Standalone VST3 AU
PRODUCT_NAME "Transfer"
)
#juce_add_module(${CMAKE_CURRENT_SOURCE_DIR}/Modules/SUX)
#juce_add_module(${CMAKE_CURRENT_SOURCE_DIR}/Modules/KMUI)
#add_subdirectory(Include/DspFilters)
#add_subdirectory(Source/Cascade-Core)
#add_subdirectory(Modules/KMUI)
#ßadd_subdirectory(Source)
#juce_add_bundle_resources_directory(AmpSim ${CMAKE_CURRENT_SOURCE_DIR}/Convolutions)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Source)
juce_generate_juce_header(Transfer)
#file(GLOB BETA_LEAD_BUNDLE_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Data/Convolutions/SunnBetaLead/*.wav)
target_sources(Transfer PUBLIC 
    ${SOURCE}
)
#set_source_files_properties(${BETA_LEAD_BUNDLE_RESOURCES} PROPERTIES
  MACOSX_PACKAGE_LOCATION "Resources/Convolutions/SunnBetaLead"
#)


target_include_directories(Transfer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Include 

    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Cascade-Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/DSPCommon 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include/DspFilters
    ${CMAKE_CURRENT_SOURCE_DIR}/Modules
)
target_compile_definitions(AmpSim
    PUBLIC 

    JUCE_WEB_BROWSER=0
    JUCE_USER_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_MODAL_LOOPS_PERMITTED=1
)

#file(GLOB BINARY_DATA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.png)
#juce_add_binary_data(BinaryData SOURCES ${BINARY_DATA_SOURCE})

target_link_libraries(AmpSim PRIVATE 
    #BinaryData
    KMUI
    juce::juce_gui_basics
    juce::juce_audio_utils 
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

