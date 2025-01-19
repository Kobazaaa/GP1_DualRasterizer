#pragma once
#include <fstream>
#include "Math.h"
#include "Renderer.h"
#include "RenderStates.h"

namespace dae
{
	inline bool HaveSameSign(float val1, float val2, float val3)
	{
		return std::signbit(val1) == std::signbit(val2) && std::signbit(val2) == std::signbit(val3);
	}

	template<typename AttributeType>
	inline AttributeType InterpolateAttribute(const AttributeType& data0, const AttributeType& data1, const AttributeType& data2,
		float Z0, float Z1, float Z2, float interpolatedDepth,
		const Vector3& weights)
	{
		return (data0 * weights.x * Z1 * Z2 + data1 * weights.y * Z0 * Z2 + data2 * weights.z * Z0 * Z1)
			/ (Z0 * Z1 * Z2) * interpolatedDepth;
	}

	inline float InterpolateDepth(float Z0, float Z1, float Z2, const Vector3& weights)
	{
		return (Z0 * Z1 * Z2)
			/ (weights.x * Z1 * Z2 + weights.y * Z0 * Z2 + weights.z * Z0 * Z1);
	}

	// Both pixel and the triangle vertices must be in SCREEN SPACE
	inline Vector3 CalculateBarycentricCoordinates(const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& p, float invArea)
	{
		// Calculate the barycentric coordinates
		// the if-checks and early return statements are to prevent further computing once one of the weights is outside the -1 - 1 range
		// Since at that point, our pixel is 100% going to be outside the triangle
		// If this is the case, we return the barycentric {0, 0, 0}, which will immediately get discarded by the AreBarycentricValid function
		float u = Vector2::Cross(v1 - p, v2 - v1) * invArea;
		if (u < -1 or u > 1) return { 1,-1,1 }; // return invalid weights
		float v = Vector2::Cross(v2 - p, v0 - v2) * invArea;
		if (v < -1 or v > 1) return { 1,-1,1 }; // return invalid weights
		float w = Vector2::Cross(v0 - p, v1 - v0) * invArea;
		if (w < -1 or w > 1) return { 1,-1,1 }; // return invalid weights

		return { u, v, w };
	}
	inline bool AreBarycentricValid(Vector3& barycentric)
	{
		// If they differ from sign, already return false
		if (!HaveSameSign(barycentric.x, barycentric.y, barycentric.z)) return false;

		// Create simpler aliases
		const float X = barycentric.x = abs(barycentric.x);
		const float Y = barycentric.y = abs(barycentric.y);
		const float Z = barycentric.z = abs(barycentric.z);

		// Do a quick check to immediately discard if the sum doesn't equal 1
		if (!AreEqual(X + Y + Z, 1, 0.0001f)) return false;

		return true;


		// Below code not needed anymore as I already check if the weights have the same sign and their absolutes add up to 1.
		// Mathematically speaking, this must mean their absolute values must be between 0-1.
		// Check if they are within the valid range of 0-1
		//if (X < 0.f or X > 1.f) return false;
		//if (Y < 0.f or Y > 1.f) return false;
		//if (Z < 0.f or Z > 1.f) return false;

	}

	inline bool IsNDCTriangleInFrustum(const Vertex& vertex)
	{
		const Vector3& positionNDC = vertex.position;

		if (positionNDC.x < -1 or positionNDC.x > 1) return false;
		if (positionNDC.y < -1 or positionNDC.y > 1) return false;
		if (positionNDC.z < 0 or positionNDC.z > 1) return false;
		return true;
	}
	inline bool IsNDCTriangleInFrustum(const VertexOut& vertex)
	{
		Vertex temp;
		temp.position = vertex.position.GetXYZ();
		temp.color = vertex.color;
		temp.uv = vertex.uv;

		return IsNDCTriangleInFrustum(temp);
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 vertices, add them to the vertex array
					//add three indices to the index array
					//add the material index as attribute to the attribute array
					//
					// Faces or triangles
					Vertex vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.uv = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(vertices.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding) 
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[size_t(i) + 1];
				uint32_t index2 = indices[size_t(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].uv;
				const Vector2& uv1 = vertices[index1].uv;
				const Vector2& uv2 = vertices[index2].uv;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Create the Tangents (reject)
			for (auto& v : vertices)
			{
				v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

				if(flipAxisAndWinding)
				{
					v.position.z *= -1.f;
					v.normal.z *= -1.f;
					v.tangent.z *= -1.f;
				}

			}

			return true;
		}
#pragma warning(pop)
	}
}