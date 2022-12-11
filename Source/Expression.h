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

template <typename T>
struct Expression
{
    Expression<T>(std::string initialExpr, T coeff, T cz) : distortionCoefficient(coeff), z(cz)
    {
#if defined USE_EXPRTK
        symbolTable.add_variable("x", currentIp);
        symbolTable.add_variable("d", distortionCoefficient);
        symbolTable.add_variable("z", z);
        symbolTable.add_variable("prev", previous);
        symbolTable.add_constants();
        expression.register_symbol_table(symbolTable);
        parser.settings().disable_all_control_structures();
#endif
        setExpr(initialExpr);
    }

    void setExpr(std::string newExpr)
    {


#ifdef USE_EXPRTK
        std::lock_guard<std::mutex> m_lock(m_mutex);
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
        }
        else {
            transferFunction = [this](float x) { return x; };
        }
#else 
        size_t err = parser.parse(byteCode, newExpr, "x, d, z");
        if (err) {
            // y = x;
            transferFunction = [this](float x) { return x; };
        }
        else {
            transferFunction = [this](float x) {
                byteCode.var[0] = x;
                byteCode.var[1] = distortionCoefficient;
                byteCode.var[2] = z;
                auto res = byteCode.run();
                if (byteCode.fltErr) return x;
                return res;
            };
        }
#endif
    }

    std::function<float(float)>& getTransferFunction() { return transferFunction; }

    void setDistortionCoefficient(T newCoeff) { 
        distortionCoefficient = newCoeff;
    }

    void setZ(T newZ) { z = newZ; }

private: 

    T currentIp = 0;
#if defined USE_EXPRTK
    exprtk::symbol_table<T> symbolTable;
    exprtk::expression<T> expression;
    exprtk::parser<T> parser;
#else 
    ATMSP<T> parser;
    ATMSB<T> byteCode;
#endif
    T distortionCoefficient = 1;
    T z = 0;
    T previous = 0;
    std::function<float(float)> transferFunction;
    std::mutex m_mutex;
};
