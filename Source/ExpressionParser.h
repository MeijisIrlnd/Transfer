/*
  ==============================================================================

    ExpressionParser.h
    Created: 24 Jun 2021 3:32:38pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <CMathParser.h>

struct ExpressionContext
{
	CMathParser parser;
	std::string expression;
	std::string exprCopy;
	std::function<float(float)> processFunction;
	float distortionCoefficient = 1;
	float z;

	ExpressionContext(const std::string& expr, double coeff, double zv) : expression(expr), distortionCoefficient(coeff), z(zv)
	{
		processFunction = [this](float x) {
			try {
				replaceVariables(x);
				double res = x;
				auto mRes = parser.Calculate(exprCopy.c_str(), &res);
				if (mRes != CMathParser::MathResult::ResultOk) throw InvalidExpressionException();
				return (float)res;
			}
			catch (InvalidExpressionException& e) {
				DBG(e.what());
				return x;
			}
		};
	}

	void replaceVariables(float val)
	{
		exprCopy = expression;
		std::stringstream vs;
		vs << val;
		size_t pos = 0;
		while ((pos = exprCopy.find("x")) != std::string::npos) {
			exprCopy.replace(pos, std::string("x").length(), std::to_string(val));
		}
		std::stringstream dCs;
		dCs << distortionCoefficient;
		while ((pos = exprCopy.find("d")) != std::string::npos) {
			exprCopy.replace(pos, std::string("d").length(), dCs.str());
		}
		while ((pos = exprCopy.find("z")) != std::string::npos) {
			exprCopy.replace(pos, std::string("z").length(), std::to_string(z));
		}
		while ((pos = exprCopy.find("pow")) != std::string::npos) {
			std::string powSubExpr = exprCopy.substr(pos, exprCopy.length());
			size_t endBrace = powSubExpr.find_first_of(")");
			if (endBrace == std::string::npos) { 
				throw InvalidExpressionException(); 
				break;
			}
			powSubExpr = powSubExpr.substr(0, endBrace + std::string(")").length());
			double powRes;
			std::stringstream powS;
			parser.Calculate(powSubExpr.c_str(), &powRes);
			powS << powRes;
			exprCopy.replace(pos, powSubExpr.length(), std::to_string(powRes));
		}
	}

	void setDistortionCoefficient(float newDistortionCoefficient)
	{
		distortionCoefficient = newDistortionCoefficient;
	}

	void setZ(float newZ) { z = newZ; }

	struct InvalidExpressionException : public std::exception
	{
		const char* what() const noexcept override
		{
			return "Invalid expression, unable to deduce..";
		}
	};
};
