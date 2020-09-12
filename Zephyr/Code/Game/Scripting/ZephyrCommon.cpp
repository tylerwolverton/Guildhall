#include "Game/Scripting/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
std::string ToString( eTokenType type )
{
	switch ( type )
	{
		case eTokenType::BRACE_LEFT:	return "BRACE_LEFT";
		case eTokenType::BRACE_RIGHT:	return "BRACE_RIGHT";
		case eTokenType::STATE_MACHINE: return "STATE_MACHINE";
		case eTokenType::STATE:			return "STATE";
		case eTokenType::NUMBER:		return "NUMBER";
		case eTokenType::IDENTIFIER:	return "IDENTIFIER";
		case eTokenType::CONSTANT:		return "CONSTANT";
		case eTokenType::PLUS:			return "PLUS";
		case eTokenType::MINUS:			return "MINUS";
		case eTokenType::STAR:			return "STAR";
		case eTokenType::SLASH:			return "SLASH";
		case eTokenType::SEMICOLON:		return "SEMICOLON";
		case eTokenType::END_OF_FILE:	return "END_OF_FILE";
		default:						return "UNKNOWN";
	}
}

