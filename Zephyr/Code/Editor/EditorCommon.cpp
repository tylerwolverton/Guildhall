#include "Editor/EditorCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
App* g_app = nullptr;						// Owned by Main_Windows.cpp
Window* g_window = nullptr;					// Owned by Main_Windows.cpp
RenderContext* g_renderer = nullptr;		// Owned by the App
InputSystem* g_inputSystem = nullptr;		// Owned by the App
Editor* g_editor = nullptr;					// Owned by the App


//-----------------------------------------------------------------------------------------------
void PopulateEditorConfig()
{
	XmlDocument doc;
	XmlError loadError = doc.LoadFile( "Data/EditorConfig.xml" );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		return;
	}

	XmlElement* root = doc.RootElement();
	g_editorConfigBlackboard.PopulateFromXmlElementAttributes( *root );
}

