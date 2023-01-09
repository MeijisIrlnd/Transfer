/*
  ==============================================================================

    ErrorReporter.cpp
    Created: 9 Jan 2023 2:23:57am
    Author:  Syl

  ==============================================================================
*/

#include "ErrorReporter.h"
#include <PluginEditor.h>

std::mutex ErrorReporter::m_mutex;
ErrorReporter* ErrorReporter::m_instance{ nullptr };

ErrorReporter::ErrorReporter()
{
}

ErrorReporter::~ErrorReporter()
{
}

void ErrorReporter::init()
{
    getInstance();
}

ErrorReporter* ErrorReporter::getInstance()
{
    std::scoped_lock<std::mutex> sl(m_mutex);
    if (m_instance == nullptr)
    {
        m_instance = new ErrorReporter();
    }
    return m_instance;
}

void ErrorReporter::destroy()
{
    std::scoped_lock<std::mutex> sl(m_mutex);
    delete m_instance;
}

void ErrorReporter::report(const std::vector<error_t>& errors)
{
    m_storedErrors = errors;
    m_hasError = true;
    if (m_editor == nullptr) return;
    m_editor->showError(errors);

}

void ErrorReporter::clearErrors()
{
    if (m_editor == nullptr) return;
    m_hasError = false;
    m_editor->clearErrors();
}
