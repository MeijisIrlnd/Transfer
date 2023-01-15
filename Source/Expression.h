/*
  ==============================================================================

    Expression.h
    Created: 25 Jun 2021 4:06:01am
    Author:  Syl

  ==============================================================================
*/

#pragma once
//#include <mathpresso.h>
#include <atmsp.h>
#include "exprtk.hpp"
#include <string>
#include <functional>
#include <SDSP/SDSP.h>
#include <Utils/ErrorReporter.h>
template <typename T>
struct Expression
{
    Expression<T>(std::string initialExpr, T coeff, T cz, ErrorReporter& errorReporter) : distortionCoefficient(coeff), z(cz), m_errorReporter(errorReporter)
    {

        symbolTable.add_variable("x", currentIp);
        symbolTable.add_variable("d", distortionCoefficient);
        symbolTable.add_variable("y", y);
        symbolTable.add_variable("z", z);
        symbolTable.add_vector("gpr", gpr.data(), gpr.size());
        symbolTable.add_variable("prev", previous);
        symbolTable.add_constants();
        expression.register_symbol_table(symbolTable);
        parser.settings().enable_all_control_structures();
        setExpr(initialExpr);
    }

    void setExpr(std::string newExpr)
    {
        std::lock_guard<std::mutex> m_lock(m_mutex);
        zeroRegisters();
        if (parser.compile(newExpr, expression)) {
            transferFunction = [this](float x) {
                try {
                    std::lock_guard<std::mutex> m_lock(m_mutex);
                    currentIp = x;
                    auto res = expression.value();
                    res = std::isnan(res) || std::isinf(res) ? 0 : res;
                    previous = x;
                    return res;
                }
                catch (std::exception& e) {
                    return 0.0f;
                }
            };
            if (m_errorReporter.hasError()) {
                m_errorReporter.clearErrors();
            }
        }
        else {
            std::vector<error_t> errors(parser.error_count());
            
            for (auto i = 0; i < parser.error_count(); i++) {
                errors[i] = parser.get_error(i);
            }
            m_errorReporter.report(errors);
            transferFunction = [this](float x) { return x; };
        }
    }

    std::function<float(float)>& getTransferFunction() { return transferFunction; }

    SDSP_INLINE void setDistortionCoefficient(T newCoeff) { 
        distortionCoefficient = newCoeff;
    }

    SDSP_INLINE void setY(T newY) { y = newY; }

    SDSP_INLINE void setZ(T newZ) { z = newZ; }

    SDSP_INLINE void zeroRegisters() {
        std::fill(gpr.begin(), gpr.end(), static_cast<T>(0));
    }

private: 

    ErrorReporter& m_errorReporter;
    T currentIp = 0;
    exprtk::symbol_table<T> symbolTable;
    exprtk::expression<T> expression;
    exprtk::parser<T> parser;
    T distortionCoefficient = 1;
    T y{ 0 }, z{ 0 };
    T previous = 0;
    std::array<T, 4> gpr = { 0, 0, 0, 0 };
    std::function<float(float)> transferFunction;
    std::mutex m_mutex;
};
