/*
  ==============================================================================

    ErrorReporter.cpp
    Created: 9 Jan 2023 2:23:57am
    Author:  Syl

  ==============================================================================
*/

#include "ErrorReporter.h"
#include <PluginEditor.h>

namespace Transfer 
{
  ErrorReporter::ErrorReporter()
  {
  }
  
  ErrorReporter::~ErrorReporter()
  {
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
}