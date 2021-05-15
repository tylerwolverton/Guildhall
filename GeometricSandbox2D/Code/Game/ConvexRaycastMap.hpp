#pragma once
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
class BufferParser;
class BufferWriter;
class SymmetricQuadTree;


//-----------------------------------------------------------------------------------------------
struct Ray
{
	Vec2 startPos = Vec2::ZERO;
	Vec2 forwardNormal = Vec2::ZERO;
	float maxDist = 0.f;
};


//-----------------------------------------------------------------------------------------------
class ConvexRaycastMap : public Map
{
public:
	ConvexRaycastMap( const std::string& name );
	~ConvexRaycastMap();

	virtual void Reset() override;

	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;
	virtual void DebugRender() const override;

	virtual void AddEntitySet( int numNewEntites ) override;
	virtual void RemoveEntitySet( int numEntitesToRemove ) override;

	virtual int GetObjectCount() const override;
	virtual int GetRaycastCount() const override;

	void SaveConvexSceneToFile( const std::string& fileName );
	void LoadConvexSceneFromFile( const std::string& fileName );

private:
	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateHighlightedEntity();
	void UpdateDraggedEntity();
	void UpdateEntities( float deltaSeconds );
	void UpdateVisibleRaycast();
	void UpdateInvisibleRaycasts();
	void PerformInvisibleRaycasts();
	void PerformVisibleRaycast();

	void RenderVisibleRaycast( std::vector<Vertex_PCU>& vertices ) const;

	Entity* GetTopEntityAtMousePosition();
	Entity* GetTopEntityAtPosition( const Vec2& position ) const;

	void SpawnNewEntitySet( int numNewEntites );
	void AddInvisibleRaycasts( int numNewRaycasts );
	void RemoveInvisibleRaycasts( int numRaycastsToRemove );

	virtual RaycastResult Raycast( const Vec2& startPos, const Vec2& forwardNormal, float maxDist ) override;
	RaycastResult RaycastWithQuadTree( const Vec2& startPos, const Vec2& forwardNormal, float maxDist );

	// File save/load
	bool WriteFileHeaderToBuffer( BufferWriter& bufferWriter );
	bool WriteConvexPolysChunk( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer );
	bool WriteSceneInfoChunk( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer );
	bool WriteBoundingDiscsChunk( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer );
	bool WriteTableOfContentsToBuffer( BufferWriter& bufferWriter, std::vector<byte>& tocBuffer );

	bool ReadFileHeaderFromBuffer( BufferParser& bufferParser, uint32_t& out_offsetToTOC );
	bool ReadChunksFromBuffer( BufferParser& bufferParser, uint32_t offsetToTOC );
	bool ReadChunk( BufferParser& bufferParser, byte expectedChunkType, uint32_t expectedChunkDataSize );
	bool ReadConvexPolysChunk( BufferParser& bufferParser );
	bool ReadSceneInfoChunk( BufferParser& bufferParser );
	bool ReadBoundingDiscsChunk( BufferParser& bufferParser );

private:
	Entity* m_draggedEntity = nullptr;
	Entity* m_hoverEntity = nullptr;
	Vec2 m_mouseOffsetFromCenterOfDraggedEntity = Vec2::ZERO;

	bool m_isRaycastStartMoving = false;
	bool m_isRaycastEndMoving = false;
	Vec2 m_visibleRaycastStartPos = Vec2::ZERO;
	Vec2 m_visibleRaycastEndPos = Vec2( 1.f, 0.f );
	Vec2 m_visibleRaycastForwardVector = Vec2( 1.f, 0.f );
	float m_visibleRaycastMaxDist = 0.f;
	RaycastResult m_visibleRaycastResult;

	int m_numTotalEntities = 0;
	int m_numActiveEntities = 0;
	int m_numInvisibleRaycasts = 1024;
	int m_curRayNum = 0;
	float m_dummyVal = 0.f; // make sure invisible raycast isn't optimized away

	SymmetricQuadTree* m_symmetricQuadTree;

	std::vector<Ray> m_invisibleRays;
};
