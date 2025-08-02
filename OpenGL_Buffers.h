#pragma once

struct opengl_vertex_buffer
{
	u32 Handle;
	u32 Size;
};

// Holds u32 indices
struct opengl_index_buffer
{
	u32 Handle;
	u32 Count;
};

internal void OpenGL_IndexBuffer_Create(opengl_index_buffer* IndexBuffer, const u32* Indices, u32 IndicesCount)
{
	IndexBuffer->Count = IndicesCount;

	glCreateBuffers(1, &IndexBuffer->Handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer->Handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer->Count * sizeof(u32), Indices, GL_STATIC_DRAW);
}

internal void OpenGL_IndexBuffer_Destroy(opengl_index_buffer* IndexBuffer)
{
	glDeleteBuffers(1, &IndexBuffer->Handle);
	IndexBuffer->Handle = 0; 
	IndexBuffer->Count = 0;
}