/*
  ==============================================================================

    ErrorReporter.h
    Created: 9 Jan 2023 2:23:57am
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <mutex>
#include <exprtk.hpp>
using error_t = exprtk::parser_error::type;
class TransferAudioProcessorEditor;
class ErrorReporter
{
public:
    ErrorReporter();
    ~ErrorReporter();
    ErrorReporter(const ErrorReporter& other) = delete;
    ErrorReporter(ErrorReporter&& other) noexcept = delete;
    ErrorReporter& operator=(const ErrorReporter& other) = delete;
    ErrorReporter& operator=(ErrorReporter&& other) noexcept = delete;
    void setEditor(TransferAudioProcessorEditor* editor) { m_editor = editor; }
    void removeEditor() { m_editor = nullptr; }
    void report(const std::vector<error_t>& toReport);
    const bool hasError() const { return m_hasError; }
    void clearErrors();
    std::vector<error_t>& getErrors() { return m_storedErrors; }
private:
    TransferAudioProcessorEditor* m_editor{ nullptr };
    bool m_hasError{ false };
    std::vector<error_t> m_storedErrors;
};
