/*
  ==============================================================================

    Expression.h
    Created: 25 Jun 2021 4:06:01am
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <mathpresso.h>
#include <string>
#include <functional>

struct Expression
{
    Expression(mathpresso::Context& c, std::string expr, float coeff, float cz) : ctx(c), distortionCoefficient(coeff), z(cz)
    {
        try {
            ctx.addVariable("x", 0 * sizeof(double));
            ctx.addVariable("d", 1 * sizeof(double));
            ctx.addVariable("z", 2 * sizeof(double));
            
            mathpresso::Error err = exp.compile(ctx, expr.c_str(), mathpresso::kNoOptions);
            if (err != mathpresso::kErrorOk) {
                // Function should return x..
                transferFunction = [this](float x) { 
                    return x; 
                };
            }
            else {
                transferFunction = [this](float x) {
                    double data[] = { x, distortionCoefficient, z };
                    return exp.evaluate(data);
                };
            }
        }
        catch (std::exception& e) {
            transferFunction = [this](float x) { 
                return x; 
            };
        }
    }

    std::function<float(float)>& getTransferFunction() { return transferFunction;  }

    mathpresso::Expression* getExpr() { return &exp; }

    void setExpr(std::string expr)
    {
        mathpresso::Error err = exp.compile(ctx, expr.c_str(), mathpresso::kNoOptions);
        if (err != mathpresso::kErrorOk) {
            // Function should return x..
            transferFunction = [this](float x) {
                return x;
            };
        }
        else {
            transferFunction = [this](float x) {
                double data[] = { x, distortionCoefficient, z };
                return exp.evaluate(data);
            };
        }
    }

    void setDistortionCoefficient(float newDistortionCoefficient) {
        distortionCoefficient = newDistortionCoefficient;
    }

    void setZ(float newZ) {
        z = newZ;
    }

private:
    mathpresso::Context& ctx;
    mathpresso::Expression exp;
    std::function<float(float)> transferFunction;
    juce::CriticalSection cs;
    float distortionCoefficient = 1;
    float z = 0;
};