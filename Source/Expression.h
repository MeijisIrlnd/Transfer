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
#include <string>
#include <functional>

template <typename T>
struct Expression
{
    Expression<T>(std::string initialExpr, T coeff, T cz) : distortionCoefficient(coeff), z(cz)
    {
        setExpr(initialExpr);
    }

    void setExpr(std::string newExpr)
    {
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
    }

    std::function<float(float)>& getTransferFunction() { return transferFunction; }

    void setDistortionCoefficient(T newCoeff) { 
        distortionCoefficient = newCoeff;
    
    }

    void setZ(T newZ) { z = newZ; }

private: 
    ATMSP<T> parser;
    ATMSB<T> byteCode;
    T distortionCoefficient = 1;
    T z = 0;
    std::function<float(float)> transferFunction;
};
