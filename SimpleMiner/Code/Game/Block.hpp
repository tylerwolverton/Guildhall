#pragma once
//-----------------------------------------------------------------------------------------------
class Block
{
public:
	void SetType( unsigned char type )		{ m_type = type; }

private:
	unsigned char m_type = 0;
};
