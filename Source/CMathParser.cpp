#ifndef _CMathParser_CPP
#define _CMathParser_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <StdIO.H>
#include <StdLib.H>
#include <Math.H>
#include <Float.H>
#include <Limits.H>

#include "CMathParser.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//using namespace NSWFL::String;

#ifndef _CVTBUFSIZE
#define _CVTBUFSIZE (309+40) /* Number of digits in maximum double precision value + slop */
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* sNativeMethods[] =
{
	"ACOS",
	"ASIN",
	"ATAN",
	"ATAN2",
	"LDEXP",
	"SINH",
	"COSH",
	"TANH",
	"LOG",
	"LOG10",
	"EXP",
	"MODPOW",
	"SQRT",
	"POW",
	"FLOOR",
	"CEIL",
	"NOT",
	"AVG",
	"SUM",
	"TAN",
	"ATAN",
	"SIN",
	"COS",
	"ABS",
	NULL
};

const char *sPreOrder[] =
{
	"!",  //Logical NOT
	NULL
};

const char *sFirstOrder[] =
{
	"~",  //Bitwise NOT
	"*",  //Multiplication
	"/",  //Division
	"%",  //Modulation
	NULL
};

const char *sSecondOrder[] =
{
	"+",  //Addition
	"-",  //Subtraction
	NULL
};

const char *sThirdOrder[] =
{
	"<>", //Logical Not Equal
	"|=", //Bitwise Or Equal
	"&=", //Bitwise And Equal
	"^=", //Bitwise XOR Equal
	"<=", //Logical Less or Equal
	">=", //Logical Greater or Equal
	"!=", //Logical Not Equal

	"<<", //Bitwise Left Shift
	">>", //Bitwise Right Shift

	"=",  //Logical Equals
	">",  //Logical Greater Than
	"<",  //Logical Less Than

	"&&", //Logical AND
	"||", //Logical OR

	"|",  //Bitwise OR
	"&",  //Bitwise AND
	"^",  //Exclusive OR

	NULL
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Lets us know whether the identifier is a built-in function.
/// </summary>
/// <param name="sName"></param>
/// <returns></returns>
bool CMathParser::IsNativeMethod(const char* sName)
{
	for (int i = 0; sNativeMethods[i] != NULL; i++)
	{
		if (_strcmpi(sNativeMethods[i], sName) == 0)
		{
			return true;
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CMathParser::TrailingChars(const char *sVal, int iStartPos, const char cChar)
{
	for (int i = iStartPos; i > 0; i--)
	{
		if (sVal[i] != cChar)
		{
			return iStartPos - i;
		}
	}

	return iStartPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CMathParser::SmartRound(double dValue, char *sOut, int iMaxOutSz)
{
	char sVal[_CVTBUFSIZE * 2];
	char sFormat[32];
	int iValLen = this->DoubleToChar(dValue, sVal, sizeof(sVal));
	if (iValLen < 0)
	{
		return -1;
	}
	int iDecPos = InStr(".", sVal, iValLen, 0);

	if (iDecPos < 0)
	{
		return sprintf_s(sOut, iMaxOutSz, "%.0f", dValue);
	}

	//Detect long trail before last number (ex: 3.14000000000000000001).
	if (sVal[iValLen - 1] != sVal[iValLen - 2])
	{
		int iTrailPos = iValLen - 2;
		int iTrailAfterLastNumber = 0;
		while (sVal[iTrailPos] == sVal[iValLen - 2])
		{
			iTrailPos--;
			iTrailAfterLastNumber++;
		}
		if (iTrailAfterLastNumber >= 3)
		{
			sVal[iValLen - 1] = '\0'; //Truncate last number;
		}
	}

	//Detect trailing 9's (ex: 3.99999999999999999).
	int iNines = this->TrailingChars(sVal, iValLen - 1, '9');
	if (iNines > 8)
	{
		int iRoundTo = (iValLen - iNines) - (iDecPos + 1);
		sprintf_s(sFormat, sizeof(sFormat), "%%.%df", iRoundTo);
		sprintf_s(sOut, iMaxOutSz, sFormat, dValue);
	}
	else {
		//Detect trailing 0's (ex: 3.0000000000000).
		int iZeros = TrailingChars(sVal, iValLen - 2, '0');
		if (iZeros > 4)
		{
			int iRoundTo = (iValLen - iZeros) - (iDecPos + 2);
			sprintf_s(sFormat, sizeof(sFormat), "%%.%df", iRoundTo);
			sprintf_s(sOut, iMaxOutSz, sFormat, dValue);
		}
		else {
			strcpy_s(sOut, iMaxOutSz, sVal);
		}
	}

	int iSz = (int)strlen(sOut);

	//Remove any remaining 0's after conversion.
	if (sOut[iSz - 1] == '0')
	{
		iSz--;
		while (sOut[iSz] == '0')
		{
			iSz--;
		}

		if (sOut[iSz] == '.')
		{
			iSz--;
		}

		iSz++;
	}

	sOut[iSz] = '\0';

	return iSz;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Converts a double floating point to a string.
/// </summary>
/// <param name="dVal"></param>
/// <param name="sOut"></param>
/// <param name="iMaxOutSz"></param>
/// <returns></returns>
int CMathParser::DoubleToChar(double dVal, char *sOut, int iMaxOutSz)
{
	int iSigned = 0;
	int iDecPos = 0;
	int iWPos = 0;
	int iValLen = 0;

	if (dVal == 0)
	{
		strcpy_s(sOut, iMaxOutSz, "0");
		return 1;
	}

	char sVal[_CVTBUFSIZE * 2];
	if (_fcvt_s(sVal, sizeof(sVal), dVal, this->ciPrecision, &iDecPos, &iSigned) != 0)
	{
		return -1;
	}

	iValLen = (int)strlen(sVal);

	if (iSigned)
	{
		sOut[iWPos++] = '-';
	}

	if (iDecPos <= 0)
	{
		sOut[iWPos++] = '0';
		sOut[iWPos++] = '.';
		while (iDecPos < 0)
		{
			sOut[iWPos++] = '0';
			iDecPos++;
		}
		iDecPos = -1;
	}

	for (int iRPos = 0; iRPos < iValLen; iRPos++)
	{
		if (iRPos == iDecPos)
		{
			sOut[iWPos++] = '.';
		}
		sOut[iWPos++] = sVal[iRPos];
	}

	if (sOut[iWPos - 1] == '0')
	{
		iWPos--;
		while (iWPos > 2 && sOut[iWPos] == '0' && sOut[iWPos - 1] != '.')
		{
			iWPos--;
		}

		if (sOut[iWPos] == '.')
		{
			iWPos--;
		}

		iWPos++;
	}

	sOut[iWPos] = '\0';

	return iWPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Gets the number to the left of an operator.
/// </summary>
/// <param name="pExp"></param>
/// <param name="iStartPos"></param>
/// <param name="sOutVal"></param>
/// <param name="iMaxSz"></param>
/// <param name="iOutSz"></param>
/// <param name="iBegin"></param>
/// <returns></returns>
CMathParser::MathResult CMathParser::GetLeftNumber(MATHEXPRESSION *pExp, int iStartPos, char *sOutVal, int iMaxSz, int *iOutSz, int *iBegin)
{
	int iRPos = (iStartPos - 1);
	int iWPos = 0;

	iMaxSz--;

	while (iRPos >= 0 && iWPos < iMaxSz)
	{
		if (this->IsMathChar(pExp->Text[iRPos]))
		{
			if ((pExp->Text[iRPos] == '-' || pExp->Text[iRPos] == '+'))
			{
				if (iRPos > 0)
				{
					if (!this->IsMathChar(pExp->Text[iRPos - 1]))
					{
						break;
					}
				}
				else {
					//Negative or Positive number is explicitly defined.
				}
			}
			else {
				break;
			}
		}
		else if (!IsNumeric(pExp->Text[iRPos]) && pExp->Text[iRPos] != '.')
		{
			return this->SetError(ResultInvalidToken, "Token is invalid: %c", pExp->Text[iRPos]);
		}

		sOutVal[iWPos++] = pExp->Text[iRPos--];
	}

	sOutVal[iWPos] = '\0';

	ReverseString(sOutVal, iWPos);

	*iOutSz = iWPos;
	*iBegin = (iRPos + 1);

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Gets the number to the right of an operator.
/// </summary>
/// <param name="pExp"></param>
/// <param name="iStartPos"></param>
/// <param name="sOutVal"></param>
/// <param name="iMaxSz"></param>
/// <param name="iOutSz"></param>
/// <param name="iEnd"></param>
/// <returns></returns>
CMathParser::MathResult CMathParser::GetRightNumber(MATHEXPRESSION *pExp, int iStartPos, char *sOutVal, int iMaxSz, int *iOutSz, int *iEnd)
{
	int iRPos = iStartPos;
	int iWPos = 0;

	iMaxSz--;

	while (iRPos < pExp->Length && iWPos < iMaxSz)
	{
		if (this->IsMathChar(pExp->Text[iRPos]))
		{
			if ((pExp->Text[iRPos] == '-' || pExp->Text[iRPos] == '+') && iRPos < pExp->Length)
			{
				if (!this->IsMathChar(pExp->Text[iRPos - 1]))
				{
					break;
				}
			}
			else {
				break;
			}
		}
		else if (!IsNumeric(pExp->Text[iRPos]) && pExp->Text[iRPos] != '.')
		{
			return this->SetError(ResultInvalidToken, "Token is invalid: %c", pExp->Text[iRPos]);
		}

		sOutVal[iWPos++] = pExp->Text[iRPos++];
	}

	sOutVal[iWPos] = '\0';

	*iOutSz = iWPos;
	*iEnd = iRPos;

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Parses a single operation.
/// </summary>
/// <param name="pInst"></param>
/// <param name="pExp"></param>
/// <param name="sOp"></param>
/// <param name="iOpPos"></param>
/// <param name="iOpSz"></param>
/// <returns></returns>
CMathParser::MathResult CMathParser::ParseOperator(MATHINSTANCE *pInst, MATHEXPRESSION *pExp, const char *sOp, int iOpPos, int iOpSz)
{
	char sVal[_CVTBUFSIZE];
	char sVal1[_CVTBUFSIZE];
	char sVal2[_CVTBUFSIZE];
	int iValSz = 0;

	MathResult ErrorCode = ResultOk;

	int iBegin = 0;
	int iEnd = 0;

	if ((ErrorCode = this->GetLeftNumber(pExp, iOpPos, sVal1, sizeof(sVal1), &iValSz, &iBegin)) != ResultOk)
	{
		return ErrorCode;
	}
	if (iValSz > 0)
	{
		if ((ErrorCode = this->GetRightNumber(pExp, iOpPos + iOpSz, sVal2, sizeof(sVal2), &iValSz, &iEnd)) != ResultOk)
		{
			return ErrorCode;
		}
		if (iValSz > 0)
		{
			if (pInst->ForceIntegerMath)
			{
				double dLeft = atof(sVal1);
				double dRight = atof(sVal2);

				if(pInst->ForceUnsignedMath)
				{
					if (dLeft < 0 || dRight < 0)
					{
						return this->SetError(ResultMissingOperator, "Unsigned integer underflow.");
					}
					else if (dLeft > UINT_MAX || dRight > UINT_MAX)
					{
						return this->SetError(ResultMissingOperator, "unsigned integer overflow.");
					}
				}
				else
				{
					if (dLeft < INT_MIN || dRight < INT_MIN)
					{
						return this->SetError(ResultIntegerunderflow, "Integer underflow.");
					}
					else if (dLeft > INT_MAX || dRight > INT_MAX)
					{
						return this->SetError(ResultIntegerOverflow, "Integer overflow.");
					}
				}

				if ((ErrorCode = this->PerformDoubleOperation(pInst, atol(sVal1), sOp, atol(sVal2))) != ResultOk)
				{
					return ErrorCode;
				}

				if (pInst->RunningTotal < INT_MIN)
				{
					return this->SetError(ResultIntegerunderflow, "Integer underflow.");
				}
				else if (pInst->RunningTotal > INT_MAX)
				{
					return this->SetError(ResultIntegerOverflow, "Integer overflow.");
				}

				if (_itoa_s((int)pInst->RunningTotal, sVal, sizeof(sVal), 10) != 0)
				{
					return this->SetError(ResultIntegerTextConversionFailed, "Integer->Text converion failed.");
				}
			}
			else {
				if ((ErrorCode = this->PerformDoubleOperation(pInst, atof(sVal1), sOp, atof(sVal2))) != ResultOk)
				{
					return ErrorCode;
				}

				if (this->DoubleToChar(pInst->RunningTotal, sVal, sizeof(sVal)) <= 0)
				{
					return this->SetError(ResultDoubleTextConversionFailed, "Text->Double converion failed.");
				}
			}

			iValSz = (int)strlen(sVal);

			if (sVal[iValSz - 1] == '.')
			{
				iValSz--;
				sVal[iValSz] = '\0';
			}

			if (this->cbDebugMode)
			{
				char sDebugMath[1024 + (_CVTBUFSIZE * 2)];

				if (pInst->ForceIntegerMath)
				{
					sprintf_s(sDebugMath, sizeof(sDebugMath), "\t(%s %s %s) = %s\n", sVal1, sOp, sVal2, sVal);

					if (this->pDebugProc)
					{
						this->pDebugProc(this, sDebugMath);
					}
					else {
						printf("%s", sDebugMath);
					}
				}
				else {
					sprintf_s(sDebugMath, sizeof(sDebugMath), "\t(%.4f %s %.4f) = %.4f\n", atof(sVal1), sOp, atof(sVal2), atof(sVal));

					if (this->pDebugProc)
					{
						this->pDebugProc(this, sDebugMath);
					}
					else {
						printf("%s", sDebugMath);
					}
				}
			}

			if ((ErrorCode = this->ReplaceValue(pExp, iBegin, iEnd, sVal, iValSz)) != ResultOk)
			{
				return ErrorCode;
			}
		}
		else {
			return this->SetError(ResultRightValueFailed, "Value to the right of operator is missing or invalid.");
		}
	}
	else {
		if (strcmp(sOp, "!") == 0 || strcmp(sOp, "~") == 0)
		{
			if ((ErrorCode = this->GetRightNumber(pExp, iOpPos + iOpSz, sVal2, sizeof(sVal2), &iValSz, &iEnd)) != ResultOk)
			{
				return ErrorCode;
			}

			if (iValSz > 0)
			{
				if (sOp[0] == '!')
				{
					if ((ErrorCode = this->PerformBooleanOperation(pInst, atol(sVal2), sOp)) != ResultOk)
					{
						return ErrorCode;
					}
				}
				else if (sOp[0] == '~')
				{
					if ((ErrorCode = this->PerformIntOperation(pInst, atol(sVal2), sOp, NULL)) != ResultOk)
					{
						return ErrorCode;
					}
				}
				else {
					return this->SetError(ResultInvalidOperator, "Invalid operator: %s.", sOp);
				}

				if (_itoa_s((int)pInst->RunningTotal, sVal, sizeof(sVal), 10) != 0)
				{
					return this->SetError(ResultIntegerTextConversionFailed, "Integer to text conversion failed.");
				}

				iValSz = (int)strlen(sVal);

				if (this->cbDebugMode)
				{
					char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
					sprintf_s(sDebugMath, sizeof(sDebugMath), "\t%s%s%s = %s\n", sVal1, sOp, sVal2, sVal);

					if (this->pDebugProc)
					{
						this->pDebugProc(this, sDebugMath);
					}
					else {
						printf("%s", sDebugMath);
					}
				}

				if ((ErrorCode = this->ReplaceValue(pExp, iBegin, iEnd, sVal, iValSz)) != ResultOk)
				{
					return ErrorCode;
				}
			}
			else {
				return this->SetError(ResultRightValueFailed, "Value to the right of operator is missing or invalid.");
			}
		}
		else if (strcmp(sOp, "-") == 0)
		{
			return ResultFoundNegative;
		}
		else {
			return this->SetError(ResultInvalidToken, "Invalid token: %s", sOp);
		}
	}

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Returns the position of a free-standing not operator.
/// </summary>
/// <param name="pExp"></param>
/// <returns></returns>
int CMathParser::GetFreestandingNotOperation(MATHEXPRESSION *pExp) //Pre order.
{
	for (int iRPos = 0; iRPos < pExp->Length; iRPos++)
	{
		//Make sure we have a "!' and not a "!=", these two have to be handled in different places.
		if (pExp->Text[iRPos] == '!' && (iRPos + 1 < pExp->Length) && pExp->Text[iRPos + 1] != '=')
		{
			/*
			//This is used to enforce the order of NOT expressions.
			for(int iTmpRPos = iRPos + 1; iRPos < pExp->Length; iTmpRPos++)
			{
				if(!IsNumeric(pExp->Text[iTmpRPos]))
				{
					if(pExp->Text[iTmpRPos] != '(')
					{
						iRPos = iTmpRPos;
						break;
					}
				}
			}

			if(pExp->Text[iRPos] == '~' || pExp->Text[iRPos] == '!')
			{
				return iRPos;
			}
			*/
			return iRPos;
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Returns the first position of a first-order operation.
/// </summary>
/// <param name="pExp"></param>
/// <returns></returns>
int CMathParser::GetFirstOrderOperation(MATHEXPRESSION *pExp) //First order.
{
	for (int iRPos = 1; iRPos < pExp->Length; iRPos++)
	{
		if (pExp->Text[iRPos] == '*' || pExp->Text[iRPos] == '/' || pExp->Text[iRPos] == '%' || pExp->Text[iRPos] == '~')
		{

			return iRPos;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Returns the first position of a second-order operation.
/// </summary>
/// <param name="pExp"></param>
/// <param name="iStartPos"></param>
/// <returns></returns>
int CMathParser::GetSecondOrderOperation(MATHEXPRESSION *pExp, int iStartPos) //Second order.
{
	for (int iRPos = iStartPos; iRPos < pExp->Length; iRPos++)
	{
		if (pExp->Text[iRPos] == '-' || pExp->Text[iRPos] == '+')
		{
			return iRPos;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::CalculateSimpleExpression(MATHINSTANCE *pInst, MATHEXPRESSION *pSubExp)
{
	MathResult ErrorCode = ResultOk;

	int iDivPos = 0;
	int iOpPos = 0;
	int iOpSz = 0;

	char *sOp = NULL;

	int iRPos = 0;
	int iWPos = 0;

	int iBegin = 0;
	int iEnd = 0;

	int iOperator = 0;

	pInst->RunningTotal = 0;

	//Pre Order.
	while ((iOpPos = this->GetFreestandingNotOperation(pSubExp)) >= 0)
	{
		iOpSz = 1;

		if (pSubExp->Text[iOpPos] == '!')
		{
			sOp = "!";
		}
		else {
			return this->SetError(ResultInvalidOperator, "Invalid operator: %c.", pSubExp->Text[iOpPos]);
		}

		if ((ErrorCode = this->ParseOperator(pInst, pSubExp, sOp, iOpPos, iOpSz)) != ResultOk)
		{
			return ErrorCode;
		}
	}

	//First Order.
	while ((iOpPos = this->GetFirstOrderOperation(pSubExp)) > 0)
	{
		iOpSz = 1;

		if (pSubExp->Text[iOpPos] == '*') {
			sOp = "*";
		}
		else if (pSubExp->Text[iOpPos] == '/') {
			sOp = "/";
		}
		else if (pSubExp->Text[iOpPos] == '%') {
			sOp = "%";
		}
		else if (pSubExp->Text[iOpPos] == '~') {
			sOp = "~";
		}
		/*
		else if(pSubExp->Text[iOpPos] == '!'){
			sOp = "!";
		}
		*/
		else {
			return this->SetError(ResultInvalidOperator, "Invalid operator: %c.", pSubExp->Text[iOpPos]);
		}

		if ((ErrorCode = this->ParseOperator(pInst, pSubExp, sOp, iOpPos, iOpSz)) != ResultOk)
		{
			return ErrorCode;
		}
	}

	int iStartPos = 1;

	//Second Order.
	while ((iOpPos = this->GetSecondOrderOperation(pSubExp, iStartPos)) > 0)
	{
		iOpSz = 1;

		if (pSubExp->Text[iOpPos] == '-') {
			sOp = "-";
		}
		else if (pSubExp->Text[iOpPos] == '+') {
			sOp = "+";
		}
		else {
			return this->SetError(ResultInvalidOperator, "Invalid operator: %c.", pSubExp->Text[iOpPos]);
		}

		if ((ErrorCode = this->ParseOperator(pInst, pSubExp, sOp, iOpPos, iOpSz)) != ResultOk)
		{
			if (ErrorCode == ResultFoundNegative)
			{
				iStartPos = iOpPos + 1;
			}
			else {
				return ErrorCode;
			}
		}
	}

	for (iOperator = 0; sThirdOrder[iOperator]; iOperator++)
	{
		iOpSz = (int)strlen(sThirdOrder[iOperator]);

		while ((iOpPos = InStr(sThirdOrder[iOperator], pSubExp->Text, pSubExp->Length, 0)) > 0)
		{
			if ((ErrorCode = this->ParseOperator(pInst, pSubExp, sThirdOrder[iOperator], iOpPos, iOpSz)) != ResultOk)
			{
				return ErrorCode;
			}
		}
	}

	if (IsNumeric(pSubExp->Text))
	{
		pInst->RunningTotal = atof(pSubExp->Text);

		if (pInst->ForceIntegerMath)
		{
			if(pInst->ForceUnsignedMath)
			{
				if (pInst->RunningTotal < 0)
				{
					return this->SetError(ResultMissingOperator, "Unsigned integer underflow.");
				}
				else if (pInst->RunningTotal > UINT_MAX)
				{
					return this->SetError(ResultMissingOperator, "unsigned integer overflow.");
				}
				pInst->RunningTotal = (unsigned int)pInst->RunningTotal;
			}
			else
			{
				if (pInst->RunningTotal < INT_MIN)
				{
					return this->SetError(ResultMissingOperator, "Signed integer underflow.");
				}
				else if (pInst->RunningTotal > INT_MAX)
				{
					return this->SetError(ResultMissingOperator, "Signed integer overflow.");
				}
				pInst->RunningTotal = (int)pInst->RunningTotal;
			}
		}
	}
	else if (pSubExp->Text[0] == '!' && IsNumeric(pSubExp->Text + 1))
	{
		if (this->cbDebugMode)
		{
			char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
			sprintf_s(sDebugMath, sizeof(sDebugMath), "\t%s = %.4f\n", pSubExp->Text, pInst->RunningTotal);

			if (this->pDebugProc)
			{
				this->pDebugProc(this, sDebugMath);
			}
			else {
				printf("%s", sDebugMath);
			}
		}

		if ((ErrorCode = this->PerformBooleanOperation(pInst, atol(pSubExp->Text + 1), "!")) != ResultOk)
		{
			return ErrorCode;
		}
	}
	else if (pSubExp->Text[0] == '~' && IsNumeric(pSubExp->Text + 1))
	{
		if (this->cbDebugMode)
		{
			char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
			sprintf_s(sDebugMath, sizeof(sDebugMath), "\t%s = %.4f\n", pSubExp->Text, pInst->RunningTotal);

			if (this->pDebugProc)
			{
				this->pDebugProc(this, sDebugMath);
			}
			else {
				printf("%s", sDebugMath);
			}
		}

		if ((ErrorCode = this->PerformIntOperation(pInst, atol(pSubExp->Text + 1), "~", NULL)) != ResultOk)
		{
			return ErrorCode;
		}
	}
	else {
		return this->SetError(ResultInvalidToken, "Invalid token: %c", pSubExp->Text[0]);
	}

	return ErrorCode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Verifies that all parentheses have matches.
/// </summary>
/// <param name="sExpression"></param>
/// <param name="iExpressionSz"></param>
/// <returns></returns>
int CMathParser::MatchParentheses(const char *sExpression, const int iExpressionSz)
{
	int iRPos = 0;
	int iScope = 0;

	while (iRPos < iExpressionSz)
	{
		if (sExpression[iRPos] == '(') {
			iScope++;
		}
		else if (sExpression[iRPos] == ')') {
			iScope--;
		}
		iRPos++;
	}

	return iScope;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::AllocateExpression(MATHEXPRESSION* pExp, const char* sSource, int iSourceSz)
{
	pExp->Allocated = (int)iSourceSz + 1;

	pExp->Text = (char*)calloc(sizeof(char), pExp->Allocated);
	if (!pExp->Text)
	{
		return this->SetError(ResultMemoryAllocationError, "Memory allocation error.");
	}

	short LastChar = 0; // -1=Other 0=Not-Set 1=Numeric
	bool bAfterWhiteSpace = false;

	for (int iRPos = 0; iRPos < iSourceSz; iRPos++)
	{
		if (!IsWhiteSpace(sSource[iRPos]))
		{
			if (IsNumeric(sSource[iRPos]))
			{
				if (bAfterWhiteSpace)
				{
					if (LastChar != -1 && LastChar != 0)
					{
						return this->SetError(ResultInvalidToken, "Token is invalid: %c", sSource[iRPos]);
					}
				}
				LastChar = 1;
			}
			else if (this->IsValidChar(sSource[iRPos]))
			{
				LastChar = -1;
			}
			else if (this->IsValidVariableChar(sSource[iRPos]))
			{
				//Parse variable name.
				char sVarName[CMATHPARSER_MAX_VAR_LENGTH + 1];

				int iVarWPos = 0;

				while (this->IsValidVariableChar(sSource[iRPos]))
				{
					sVarName[iVarWPos++] = sSource[iRPos++];
					if (iVarWPos >= CMATHPARSER_MAX_VAR_LENGTH)
					{
						return this->SetError(ResultInvalidToken, "Variable length limit exceeded.");
					}
				}

				sVarName[iVarWPos] = '\0';

				//Skip whitespaces.
				while( iRPos < iSourceSz && IsWhiteSpace(sSource[iRPos]))
				{
					iRPos++;
				}

				if (sSource[iRPos] == '(')
				{
					double* pOutParameters = NULL;
					int iParameterCount = 0;

					double dProcValue = 0;
					MathResult result = ResultOk;

					if ((result = ParseMethodParameters(sSource, iSourceSz, &iRPos, &pOutParameters, &iParameterCount)) != ResultOk)
					{
						return result;
					}

					if (IsNativeMethod(sVarName))
					{
						if ((result = ExecuteNativeMethod(sVarName, pOutParameters, iParameterCount, &dProcValue)) != ResultOk)
						{
							return result;
						}
					}
					else if (this->pMethodProc != NULL && this->pMethodProc(this, sVarName, pOutParameters, iParameterCount, &dProcValue))
					{
						//Non-native method executed successfully.
					}
					else
					{
						return this->SetError(ResultInvalidToken, "Undeclared identifier: %s.", sVarName);
					}

					char sVarValue[64];
					//Convert double to string (must be a faster way, but this is just super safe and doesn't create infinite repeating patterns).
					//TODO: Fixed percision of 8 on variables seems inflexible.
					int iVarValLength = sprintf_s(sVarValue, sizeof(sVarValue), "%.8f", dProcValue);

					if (pExp->Length + iVarValLength + iSourceSz >= pExp->Allocated)
					{
						//Adding [iSourceSz] so that the remainder of the expression can be stored without allocating again (unless we encounter more vars).
						pExp->Allocated = (pExp->Length + iVarValLength) + iSourceSz + 1;
						pExp->Text = (char*)realloc(pExp->Text, sizeof(char) * pExp->Allocated);
						if (!pExp->Text)
						{
							return this->SetError(ResultMemoryAllocationError, "Memory allocation error.");
						}
					}

					//Copy the resulting variable value to the expression for further processing.
					strcpy_s(pExp->Text + pExp->Length, pExp->Allocated - pExp->Length, sVarValue);
					pExp->Length += strlen(sVarValue);
				}
				else
				{
					double dVarValue = 0;
					//Get variable value...
					if (!this->pVariableSetProc(this, sVarName, &dVarValue))
					{
						return this->SetError(ResultInvalidToken, "Variable was not defined: %s.", sVarName);
					}

					char sVarValue[64];
					//Convert double to string (must be a faster way, but this is just super safe and doesn't create infinite repeating patterns).
					//TODO: Fixed percision of 8 on variables seems inflexible.
					int iVarValLength = sprintf_s(sVarValue, sizeof(sVarValue), "%.8f", dVarValue);

					if (pExp->Length + iVarValLength + iSourceSz >= pExp->Allocated)
					{
						//Adding [iSourceSz] so that the remainder of the expression can be stored without allocating again (unless we encounter more vars).
						pExp->Allocated = (pExp->Length + iVarValLength) + iSourceSz + 1;
						pExp->Text = (char*)realloc(pExp->Text, sizeof(char) * pExp->Allocated);
						if (!pExp->Text)
						{
							return this->SetError(ResultMemoryAllocationError, "Memory allocation error.");
						}
					}

					//Copy the resulting variable value to the expression for further processing.
					strcpy_s(pExp->Text + pExp->Length, pExp->Allocated - pExp->Length, sVarValue);
					pExp->Length += strlen(sVarValue);
				}

				iRPos--; //We need to let the outer loop determine if we are done yet.
				continue;
			}
			else {
				return this->SetError(ResultInvalidToken, "Token is invalid: %c", sSource[iRPos]);
			}

			pExp->Text[pExp->Length++] = sSource[iRPos];

			bAfterWhiteSpace = false;
		}
		else {
			bAfterWhiteSpace = true;
		}
	}

	pExp->Text[pExp->Length] = '\0';

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// Takes a method name and its parameters, apples logic to the result of a single floating point value.
/// </summary>
/// <param name="sMethodName"></param>
/// <param name="dParameters"></param>
/// <param name="iParamCount"></param>
/// <param name="pOutResult"></param>
/// <returns></returns>
CMathParser::MathResult CMathParser::ExecuteNativeMethod(
	const char* sMethodName, double* dParameters, int iParamCount, double* pOutResult)
{
	if (_strcmpi(sMethodName, "NOT") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = !((long long)dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "ACOS") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = acos(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "ASIN") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = asin(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "ATAN") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = atan(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "ATAN2") == 0)
	{
		if (iParamCount != 2)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = atan2(dParameters[0], dParameters[1]);
	}
	else if (_strcmpi(sMethodName, "LDEXP") == 0)
	{
		if (iParamCount != 2)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = ldexp(dParameters[0], (int)dParameters[1]);
	}
	else if (_strcmpi(sMethodName, "TAN") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = tan(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "SIN") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = sin(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "COS") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = cos(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "ATAN") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = atan(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "ABS") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = fabs(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "SQRT") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = sqrt(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "POW") == 0)
	{
		if (iParamCount != 2)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = pow(dParameters[0], dParameters[1]);
	}
	else if (_strcmpi(sMethodName, "MODPOW") == 0)
	{
		if (iParamCount != 3)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = this->ModPow((long long)dParameters[0], (long long)dParameters[1], (int)dParameters[2]);
	}
	else if (_strcmpi(sMethodName, "SINH") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = sinh(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "COSH") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = cosh(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "TANH") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = tanh(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "LOG") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = log(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "LOG10") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = log10(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "EXP") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = exp(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "FLOOR") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = floor(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "CEIL") == 0)
	{
		if (iParamCount != 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		*pOutResult = ceil(dParameters[0]);
	}
	else if (_strcmpi(sMethodName, "SUM") == 0)
	{
		if (iParamCount < 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		double dSum = 0;

		for (int i = 0; i < iParamCount; i++)
		{
			dSum += dParameters[i];
		}

		*pOutResult = dSum;
	}
	else if (_strcmpi(sMethodName, "AVG") == 0)
	{
		if (iParamCount < 1)
		{
			return this->SetError(ResultInvalidToken, "Invalid number of parameters passed to method: %s", sMethodName);
		}

		double dSum = 0;

		for (int i = 0; i < iParamCount; i++)
		{
			dSum += dParameters[i];
		}

		dSum /= iParamCount;

		*pOutResult = dSum;
	}

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::ParseMethodParameters(
	const char* sSource, int iSourceSz, int* piRPos, double** pOutParameters, int *piOutParamCount)
{
	int iRPos = *piRPos;
	int iWPos = 0;

	int iParenNestLevel = 0;
	int iParameters = 0;

	char sBuf[1024];

	if (sSource[iRPos] == '(')
	{
		iRPos++; //We skip the first paren because it encloses the currently parsing method.
		iParenNestLevel++;
	}

	for (; iRPos < iSourceSz; iRPos++)
	{
		if (IsWhiteSpace(sSource[iRPos]))
		{
			continue;
		}

		if (sSource[iRPos] == '(')
		{
			if (IsNativeMethod(sBuf))
			{
				double* pOutParameters = NULL;
				int iParameterCount = 0;

				double dProcValue = 0;
				MathResult result = ResultOk;

				if ((result = ParseMethodParameters(sSource, iSourceSz, &iRPos, &pOutParameters, &iParameterCount)) != ResultOk)
				{
					return result;
				}

				if ((result = ExecuteNativeMethod(sBuf, pOutParameters, iParameterCount, &dProcValue)) != ResultOk)
				{
					return result;
				}

				iRPos--; //Let this loop determine the next action.

				memset(sBuf, 0, sizeof(sBuf));

				iWPos = sprintf_s(sBuf, sizeof(sBuf), "%.8f", dProcValue);

				continue;
			}

			iParenNestLevel++;
		}
		else if (sSource[iRPos] == ')')
		{
			iParenNestLevel--;
		}

		if (sSource[iRPos] == ',')
		{

		}
		else if(iParenNestLevel > 0)
		{
			sBuf[iWPos++] = sSource[iRPos];
		}

		if (iParenNestLevel == 0 || sSource[iRPos] == ',')
		{
			*pOutParameters = (double*)realloc(*pOutParameters, sizeof(double) * (iParameters + 1));

			sBuf[iWPos] = '\0';
			double dResult = 0;
			this->Calculate(sBuf, &dResult);

			(*pOutParameters)[iParameters] = dResult;

			iParameters++;

			if (sSource[iRPos] == ',')
			{
				iWPos = 0; //Reset for the next parameter.
				memset(sBuf, 0, sizeof(sBuf));
			}
			else if (iParenNestLevel == 0)
			{
				iRPos++;
				break;
			}
		}
	}

	if (iParenNestLevel != 0)
	{
		return this->SetError(ResultParenthesesMismatch, "Parentheses mismatch.");
	}

	*piOutParamCount = iParameters;
	*piRPos = iRPos;

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::CalculateComplexExpression(MATHINSTANCE *pInst)
{
	char sVal[_CVTBUFSIZE];

	int iBegin = 0;
	int iEnd = 0;
	int iValSz = 0;

	MathResult ErrorCode = ResultOk;

	MATHEXPRESSION SubExpr;
	memset(&SubExpr, 0, sizeof(SubExpr));

	//Check braces to see if they each have a match.
	if (this->MatchParentheses(pInst->Expression.Text, pInst->Expression.Length) != 0)
	{
		return this->SetError(ResultParenthesesMismatch, "Parentheses mismatch.");
	}

	SubExpr.Allocated = pInst->Expression.Length;;
	SubExpr.Text = (char *)calloc(sizeof(char), SubExpr.Allocated);
	if (!SubExpr.Text)
	{
		free(pInst->Expression.Text);
		return this->SetError(ResultMemoryAllocationError, "Memory allocation error.");
	}

	while ((ErrorCode = this->GetSubExpression(pInst, &iBegin, &iEnd)) == ResultOk)
	{
		if (iEnd > iBegin)
		{
			SubExpr.Length = ((iEnd - iBegin) - 2);

			if (SubExpr.Length >= SubExpr.Allocated)
			{
				SubExpr.Allocated = ((iEnd - iBegin) - 2) + 1;
				SubExpr.Text = (char *)realloc(SubExpr.Text, SubExpr.Allocated);
			}

			memcpy_s(SubExpr.Text, SubExpr.Allocated, pInst->Expression.Text + (iBegin + 1), SubExpr.Length);
			SubExpr.Text[SubExpr.Length] = '\0';

			if ((ErrorCode = this->CalculateSimpleExpression(pInst, &SubExpr)) != ResultOk)
			{
				break;
			}

			if (pInst->ForceIntegerMath)
			{
				if (pInst->RunningTotal < INT_MIN)
				{
					ErrorCode = this->SetError(ResultMissingOperator, "Integer underflow.");
					break;
				}
				else if (pInst->RunningTotal > INT_MAX)
				{
					ErrorCode = this->SetError(ResultMissingOperator, "Integer overflow.");
					break;
				}

				if (_itoa_s((int)pInst->RunningTotal, sVal, sizeof(sVal), 10) != 0)
				{
					ErrorCode = this->SetError(ResultMissingOperator, "Integer->Text converion failed.");
					break;
				}
			}
			else {
				sprintf_s(sVal, sizeof(sVal), "%.*g", this->ciPrecision, pInst->RunningTotal);
				/*
				if(_gcvt_s(sVal, sizeof(sVal), pInst->RunningTotal, this->ciPrecision) != 0)
				{
					ErrorCode = this->SetError(ResultMissingOperator, "Double->Text converion failed.");
					break;
				}
				*/
			}

			iValSz = (int)strlen(sVal);

			if (sVal[iValSz - 1] == '.')
			{
				iValSz--;
				sVal[iValSz] = '\0';
			}

			if ((ErrorCode = this->ReplaceValue(&pInst->Expression, iBegin, iEnd, sVal, iValSz)) != ResultOk)
			{
				break;
			}
		}
		else {
			ErrorCode = this->CalculateSimpleExpression(pInst, &pInst->Expression);
			break;
		}
	}

	free(SubExpr.Text);

	return ErrorCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::GetSubExpression(MATHINSTANCE *pInst, int *iBegin, int *iEnd)
{
	int iIn = 0;
	int iOut = 0;

	for (int iRPos = 0; iRPos < pInst->Expression.Length; iRPos++)
	{
		if (pInst->Expression.Text[iRPos] == ')')
		{
			*iEnd = iRPos + 1;
			iOut++;
			for (iRPos--; iRPos >= 0; iRPos--)
			{
				if (pInst->Expression.Text[iRPos] == '(')
				{
					iIn++;
				}
				else if (pInst->Expression.Text[iRPos] == ')')
				{
					iOut++;
				}

				if (iIn == iOut)
				{
					*iBegin = iRPos;

					if (((int)*iBegin) > 0)
					{
						if (!this->IsMathChar(pInst->Expression.Text[((int)*iBegin) - 1]) && pInst->Expression.Text[((int)*iBegin) - 1] != '(')
						{
							return this->SetError(ResultMissingOperator, "Missing mathematical operator near: (.");
						}
					}

					if (((int)*iEnd) < pInst->Expression.Length)
					{
						if (!this->IsMathChar(pInst->Expression.Text[((int)*iEnd)]) && pInst->Expression.Text[((int)*iEnd)] != ')')
						{
							return this->SetError(ResultMissingOperator, "Missing mathematical operator near: ).");
						}
					}

					return ResultOk;
				}
			}
		}
	}

	*iBegin = pInst->Expression.Length;
	*iEnd = -pInst->Expression.Length;

	return ResultOk; //This is not an error.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::ReplaceValue(MATHEXPRESSION *pExp, int iBegin, int iEnd, const char *sWith, int iWithSz)
{
	int iPos = 0;
	int iNewSz = pExp->Length + (iWithSz - (iEnd - iBegin));
	int iGapSize = (pExp->Length - iNewSz);

	pExp->Length = iNewSz;

	if (iNewSz >= pExp->Allocated)
	{
		pExp->Allocated = iNewSz + 1;
		pExp->Text = (char *)realloc(pExp->Text, pExp->Allocated);
	}

	if (iGapSize < 0)
	{
		iGapSize = abs(iGapSize);

		for (iPos = iNewSz; iPos > iEnd && (iPos - iGapSize) >= 0; iPos--)
		{
			pExp->Text[iPos] = pExp->Text[iPos - iGapSize];
		}
		memcpy_s(pExp->Text + iBegin, pExp->Allocated - iBegin, sWith, iWithSz);
	}
	else if (iGapSize > 0)
	{
		memcpy_s(pExp->Text + iBegin, pExp->Allocated - iBegin, sWith, iWithSz);

		for (iPos = iBegin + iWithSz; iPos < iNewSz; iPos++)
		{
			pExp->Text[iPos] = pExp->Text[iPos + iGapSize];
		}

		pExp->Text[iPos] = '\0';
	}
	else {
		memcpy(pExp->Text + iBegin, sWith, iWithSz);
	}

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsIntegerExclusive(const char *sOperator)
{
	if (strcmp(sOperator, "&") == 0) {
		return true;
	}
	else if (strcmp(sOperator, "|") == 0) {
		return true;
	}
	else if (strcmp(sOperator, "^") == 0) {
		return true;
	}
	else if (strcmp(sOperator, "&=") == 0) {
		return true;
	}
	else if (strcmp(sOperator, "|=") == 0) {
		return true;
	}
	else if (strcmp(sOperator, "^=") == 0) {
		return true;
	}
	else if (strcmp(sOperator, "<<") == 0) {
		return true;
	}
	else if (strcmp(sOperator, ">>") == 0) {
		return true;
	}
	else return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsMathChar(const char cChar)
{
	return(cChar == '*' || cChar == '/'
		|| cChar == '+' || cChar == '-'
		|| cChar == '>' || cChar == '<'
		|| cChar == '!' || cChar == '='
		|| cChar == '&' || cChar == '|'
		|| cChar == '^' || cChar == '%'
		|| cChar == '~');
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsValidChar(const char cChar)
{
	return IsNumeric(cChar) || this->IsMathChar(cChar) || cChar == '.' || cChar == '(' || cChar == ')';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsValidVariableChar(const char cChar)
{
	return IsNumeric(cChar)
		|| (cChar >= 'a' && cChar <= 'z')
		|| (cChar >= 'A' && cChar <= 'Z')
		|| cChar == '_';
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::PerformBooleanOperation(MATHINSTANCE *pInst, int iVal, const char *sOpr)
{
	if (strcmp(sOpr, "!") == 0)
	{
		pInst->RunningTotal = (!iVal);
	}
	else
	{
		return this->SetError(ResultInvalidOperator, "Invalid operator: %s.", sOpr);
	}
	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::PerformIntOperation(MATHINSTANCE *pInst, int iVal1, const char *sOpr, int iVal2)
{
	if (strcmp(sOpr, "&&") == 0)
		pInst->RunningTotal = (iVal1 && iVal2);
	else if (strcmp(sOpr, "||") == 0)
		pInst->RunningTotal = (iVal1 || iVal2);
	else if (strcmp(sOpr, "&") == 0)
		pInst->RunningTotal = (iVal1 & iVal2);
	else if (strcmp(sOpr, "|") == 0)
		pInst->RunningTotal = (iVal1 | iVal2);
	else if (strcmp(sOpr, "!") == 0)
		pInst->RunningTotal = (iVal1 != iVal2);
	else if (strcmp(sOpr, "~") == 0)
		pInst->RunningTotal = ~iVal1;
	else if (strcmp(sOpr, "^") == 0)
		pInst->RunningTotal = (iVal1 ^ iVal2);
	else if (strcmp(sOpr, "&=") == 0)
		pInst->RunningTotal = (iVal1 &= iVal2);
	else if (strcmp(sOpr, "|=") == 0)
		pInst->RunningTotal = (iVal1 |= iVal2);
	else if (strcmp(sOpr, "^=") == 0)
		pInst->RunningTotal = (iVal1 ^= iVal2);
	else if (strcmp(sOpr, "<<") == 0)
		pInst->RunningTotal = (iVal1 << iVal2);
	else if (strcmp(sOpr, ">>") == 0)
		pInst->RunningTotal = (iVal1 >> iVal2);
	else {
		return this->SetError(ResultInvalidOperator, "Invalid operator: %s.", sOpr);
	}

	return ResultOk;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::PerformDoubleOperation(MATHINSTANCE *pInst, double dVal1, const char *sOpr, double dVal2)
{
	double dResult = 0;

	if (this->IsIntegerExclusive(sOpr))
		return this->PerformIntOperation(pInst, (int)dVal1, sOpr, (int)dVal2);
	else if (strcmp(sOpr, "*") == 0)
		dResult = (dVal1 * dVal2);
	else if (strcmp(sOpr, "/") == 0)
	{
		if (dVal2 == 0)
		{
			return this->SetError(ResultInvalidOperator, "Divide by zero.");
		}
		dResult = (dVal1 / dVal2);
	}
	else if (strcmp(sOpr, "+") == 0)
		dResult = (dVal1 + dVal2);
	else if (strcmp(sOpr, "&&") == 0)
		dResult = (dVal1 && dVal2);
	else if (strcmp(sOpr, "||") == 0)
		dResult = (dVal1 || dVal2);
	else if (strcmp(sOpr, "-") == 0)
		dResult = (dVal1 - dVal2);
	else if (strcmp(sOpr, "=") == 0)
		dResult = (dVal1 == dVal2);
	else if (strcmp(sOpr, ">") == 0)
		dResult = (dVal1 > dVal2);
	else if (strcmp(sOpr, "<") == 0)
		dResult = (dVal1 < dVal2);
	else if (strcmp(sOpr, ">=") == 0)
		dResult = (dVal1 >= dVal2);
	else if (strcmp(sOpr, "<=") == 0)
		dResult = (dVal1 <= dVal2);
	else if (strcmp(sOpr, "<>") == 0)
		dResult = (dVal1 != dVal2);
	else if (strcmp(sOpr, "!") == 0)
		dResult = (dVal1 != dVal2);
	else if (strcmp(sOpr, "!=") == 0)
		dResult = (dVal1 != dVal2);
	else if (strcmp(sOpr, "%") == 0)
	{
		if (dVal2 == 0)
		{
			return this->SetError(ResultInvalidOperator, "Mod by zero.");
		}
		dResult = fmod(dVal1, dVal2);
	}
	else {
		return this->SetError(ResultInvalidOperator, "Invalid operator: %s.", sOpr);
	}

	if (_finite(dResult) || !_isnan(dResult))
	{
		pInst->RunningTotal = dResult;
		return ResultOk;
	}
	else {
		return this->SetError(ResultInfiniteOrNotANumber, "Result of %s is infinite or not a number.", sOpr);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::Calculate(const char *sExpression, int iExpressionSz, double *dResult)
{
	if (this->cbDebugMode)
	{
		if (this->pDebugProc)
		{
			char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
			sprintf_s(sDebugMath, sizeof(sDebugMath), "\"%s\" = {\n", sExpression);
			this->pDebugProc(this, sDebugMath);
		}
		else {
			printf("(%s) = {\n", sExpression);
		}
	}

	MATHINSTANCE Inst;
	memset(&Inst, 0, sizeof(Inst));
	MathResult ErrorCode = ResultOk;

	Inst.ForceIntegerMath = false;
	if ((ErrorCode = this->AllocateExpression(&Inst.Expression, sExpression, iExpressionSz)) != ResultOk)
	{
		return ErrorCode;
	}

	if ((ErrorCode = this->CalculateComplexExpression(&Inst)) == ResultOk)
	{
		*dResult = Inst.RunningTotal;
	}

	free(Inst.Expression.Text);

	if (this->cbDebugMode)
	{
		char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
		sprintf_s(sDebugMath, sizeof(sDebugMath), "} = %.4f\n", *dResult);

		if (this->pDebugProc)
		{
			this->pDebugProc(this, sDebugMath);
		}
		else {
			printf("%s", sDebugMath);
		}
	}

	return ErrorCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::Calculate(const char *sExpression, double *dResult)
{
	return this->Calculate(sExpression, (int)strlen(sExpression), dResult);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::Calculate(const char *sExpression, int iExpressionSz, unsigned int *iResult)
{
	if (this->cbDebugMode)
	{
		if (this->pDebugProc)
		{
			char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
			sprintf_s(sDebugMath, sizeof(sDebugMath), "\"%s\" = {\n", sExpression);
			this->pDebugProc(this, sDebugMath);
		}
		else {
			printf("(%s) = {\n", sExpression);
		}
	}

	MATHINSTANCE Inst;
	memset(&Inst, 0, sizeof(Inst));
	MathResult ErrorCode = ResultOk;

	Inst.ForceIntegerMath = true;
	Inst.ForceUnsignedMath = true;
	if ((ErrorCode = this->AllocateExpression(&Inst.Expression, sExpression, iExpressionSz)) != ResultOk)
	{
		return ErrorCode;
	}

	if ((ErrorCode = this->CalculateComplexExpression(&Inst)) == ResultOk)
	{
		*iResult = (unsigned int)Inst.RunningTotal;
	}

	free(Inst.Expression.Text);

	if (this->cbDebugMode)
	{
		char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
		sprintf_s(sDebugMath, sizeof(sDebugMath), "} = %d\n", *iResult);

		if (this->pDebugProc)
		{
			this->pDebugProc(this, sDebugMath);
		}
		else {
			printf("%s", sDebugMath);
		}
	}

	return ErrorCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::Calculate(const char *sExpression, unsigned int *iResult)
{
	return this->Calculate(sExpression, (int)strlen(sExpression), iResult);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::Calculate(const char *sExpression, int iExpressionSz, int *iResult)
{
	if (this->cbDebugMode)
	{
		if (this->pDebugProc)
		{
			char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
			sprintf_s(sDebugMath, sizeof(sDebugMath), "\"%s\" = {\n", sExpression);
			this->pDebugProc(this, sDebugMath);
		}
		else {
			printf("(%s) = {\n", sExpression);
		}
	}

	MATHINSTANCE Inst;
	memset(&Inst, 0, sizeof(Inst));
	MathResult ErrorCode = ResultOk;

	Inst.ForceIntegerMath = true;
	if ((ErrorCode = this->AllocateExpression(&Inst.Expression, sExpression, iExpressionSz)) != ResultOk)
	{
		return ErrorCode;
	}

	if ((ErrorCode = this->CalculateComplexExpression(&Inst)) == ResultOk)
	{
		*iResult = (int)Inst.RunningTotal;
	}

	free(Inst.Expression.Text);

	if (this->cbDebugMode)
	{
		char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
		sprintf_s(sDebugMath, sizeof(sDebugMath), "} = %d\n", *iResult);

		if (this->pDebugProc)
		{
			this->pDebugProc(this, sDebugMath);
		}
		else {
			printf("%s", sDebugMath);
		}
	}

	return ErrorCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::Calculate(const char *sExpression, int *iResult)
{
	return this->Calculate(sExpression, (int)strlen(sExpression), iResult);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::CMathParser(short iPrecision)
{
	memset(&this->LastErrorInfo, 0, sizeof(this->LastErrorInfo));
	this->Precision(iPrecision);
	this->pDebugProc = NULL;
	this->cbDebugMode = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::CMathParser(void)
{
	memset(&this->LastErrorInfo, 0, sizeof(this->LastErrorInfo));
	this->Precision(CMATHPARSER_DEFAULT_PRECISION);
	this->pDebugProc = NULL;
	this->cbDebugMode = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::~CMathParser(void)
{
	if (this->LastErrorInfo.Text)
	{
		free(this->LastErrorInfo.Text);
	}
	memset(&this->LastErrorInfo, 0, sizeof(this->LastErrorInfo));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

short CMathParser::Precision(short iPrecision)
{
	if (iPrecision > CMATHPARSER_MAX_PRECISION)
	{
		iPrecision = CMATHPARSER_MAX_PRECISION;
	}
	else if (iPrecision < CMATHPARSER_MIN_PRECISION)
	{
		iPrecision = CMATHPARSER_MIN_PRECISION;
	}

	short iOldPrecision = this->ciPrecision;
	this->ciPrecision = iPrecision;
	return iOldPrecision;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

short CMathParser::Precision(void)
{
	return this->ciPrecision;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::DebugMode(bool bDebugMode)
{
	bool bOldDebugMode = this->cbDebugMode;
	this->cbDebugMode = bDebugMode;
	return bOldDebugMode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::TMethodCallback CMathParser::GetMethodCallback(void)
{
	return this->pMethodProc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::TMethodCallback CMathParser::SetMethodCallback(TMethodCallback procPtr)
{
	TMethodCallback oldProcPtr = this->pMethodProc;
	this->pMethodProc = procPtr;
	return oldProcPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::TVariableSetCallback CMathParser::GetVariableSetCallback(void)
{
	return this->pVariableSetProc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::TVariableSetCallback CMathParser::SetVariableSetCallback(TVariableSetCallback procPtr)
{
	TVariableSetCallback oldProcPtr = this->pVariableSetProc;
	this->pVariableSetProc = procPtr;
	return oldProcPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::TDebugTextCallback CMathParser::GetDebugCallback(void)
{
	return this->pDebugProc;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::TDebugTextCallback CMathParser::SetDebugCallback(TDebugTextCallback procPtr)
{
	TDebugTextCallback oldProcPtr = this->pDebugProc;
	this->pDebugProc = procPtr;
	return oldProcPtr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::DebugMode(void)
{
	return this->cbDebugMode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MATHERRORINFO *CMathParser::LastError(void)
{
	return &this->LastErrorInfo;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMathParser::MathResult CMathParser::SetError(MathResult ErrorCode, const char *sFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, sFormat);

	this->LastErrorInfo.Error = ErrorCode;

	int iMemoryRequired = _vscprintf(sFormat, ArgList) + 1;

	if (this->LastErrorInfo.Text)
	{
		free(this->LastErrorInfo.Text);
		this->LastErrorInfo.Text = NULL;
		this->LastErrorInfo.Error = ResultOk;
	}

	this->LastErrorInfo.Text = (char *)calloc(sizeof(char), iMemoryRequired);

	_vsprintf_s_l(this->LastErrorInfo.Text, iMemoryRequired, sFormat, NULL, ArgList);
	va_end(ArgList);

	if (this->cbDebugMode)
	{
		char sDebugMath[1024 + (_CVTBUFSIZE * 2)];
		sprintf_s(sDebugMath, sizeof(sDebugMath), "\t%s\n", this->LastErrorInfo.Text);

		if (this->pDebugProc)
		{
			this->pDebugProc(this, sDebugMath);
		}
		else {
			printf("%s", sDebugMath);
		}
	}
	return ErrorCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::ReverseString(char *sBuf, int iBufSz)
{
	char *String1 = NULL;
	char *String2 = NULL;

	if (!sBuf || !*sBuf)
	{
		return false;
	}

	for (String1 = sBuf, String2 = sBuf + iBufSz - 1; String2 > String1; ++String1, --String2)
	{
		*String1 ^= *String2;
		*String2 ^= *String1;
		*String1 ^= *String2;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsWhiteSpace(const char cChar)
{
	if (cChar == ' ' || cChar == '\r' || cChar == '\n' || cChar == '\t' || cChar == '\0')
	{
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsNumeric(const char cIn)
{
	return (cIn >= 48 && cIn <= 57);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsNumeric(const char *sText, int iLength)
{
	int iRPos = 0;
	bool bHasDecimal = false;

	if (iLength == 0)
	{
		return false;
	}

	if (sText[iRPos] == '-' || sText[iRPos] == '+')
	{
		iRPos++;
	}

	for (; iRPos < iLength; iRPos++)
	{
		if (!IsNumeric(sText[iRPos]))
		{
			if (sText[iRPos] == '.')
			{
				if (iRPos == iLength - 1) //Decimal cannot be the last character.
				{
					return false;
				}
				if (iRPos == 0 || (iRPos == 1 && sText[0] == '-')) //Decimal cannot be the first character.
				{
					return false;
				}


				if (bHasDecimal)
				{
					return false;
				}
				bHasDecimal = true;
			}
			else {
				return false;
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CMathParser::IsNumeric(const char *sText)
{
	return IsNumeric(sText, (int)strlen(sText));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CMathParser::InStr(const char *sSearchFor, const char *sInBuf, const int iBufSz, const int iStartPos)
{
	int iLookingLoop = 0;

	if (iStartPos > iBufSz)
	{
		return -2;
	}
	int sSearchForSz = (int)strlen(sSearchFor);
	if (sSearchForSz > iBufSz)
	{
		return -2;
	}

	for (int iControlLoop = iStartPos; iControlLoop <= (iBufSz - sSearchForSz); iControlLoop++)
	{
		if (sInBuf[iControlLoop] == sSearchFor[iLookingLoop])
		{
			while (iLookingLoop <= sSearchForSz)
			{
				if (sSearchFor[iLookingLoop] == sInBuf[iLookingLoop + iControlLoop])
				{
					iLookingLoop++;
				}
				else iLookingLoop = sSearchForSz + 1;

				if (iLookingLoop == sSearchForSz)
				{
					return iControlLoop;
				}
			}
		}
		iLookingLoop = 0;
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CMathParser::ModPow(long long base, long long exponent, int modulus)
{
	long long result = 1;
	while (exponent > 0)
	{
		if (exponent % 2 == 1)
		{
			result = (result * base) % modulus;
		}
		exponent = exponent >> 1;
		base = (base * base) % modulus;
	}
	return (int)result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
