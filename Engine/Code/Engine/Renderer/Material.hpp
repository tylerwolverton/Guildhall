#pragma once
#include "Engine/Core/XmlUtils.hpp"

class RenderContext;


class Material
{
public:
	Material( RenderContext* context, const char* filename, const XmlElement& materialElem );

private:

};
