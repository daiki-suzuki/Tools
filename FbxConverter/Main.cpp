#include <fbxsdk.h>
#pragma comment( lib, "libfbxsdk-mt.lib" )
#pragma comment( lib, "libxml2-mt.lib" )
#pragma comment( lib, "zlib-mt.lib" )
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>

using namespace std;


struct Vertex
{
	float position[3];
	float normal[3];
	float textureCoord[2];
};

struct Subset
{
	int mat_index;
	int vertexCount;
	int vertexStart;
};

struct Color
{
	float color[3];
};

struct Material
{
	int id;
	float diffuse[3];
	float alpha;
	float ambient[3];
	float specular[3];
	float power;
	float emmisive[3];

	string texture_name;
};

struct Mesh
{
	vector<int> indexArray;
	vector<Vertex> verteices;
	vector<Subset> subset;
	vector<Material> material;
};

Mesh g_mesh;

void ChangeTriangle(FbxScene* scene,FbxNode* node);
void PrintNode(FbxNode* node,int tab);
const char* GetNodeAttributeName( FbxNodeAttribute::EType type );
void PrintVertex(FbxMesh* mesh);
void PrintNormal(FbxMesh* mesh);
int GetNormalIndex(FbxGeometryElementNormal* layer,int index);
void PrintUV(FbxMesh* mesh);
void PrintMaterial(FbxSurfaceMaterial* material);
Color PrintMaterialProperty(FbxSurfaceMaterial* material,const char* prop,const char* propFactory,string* textureName);

void SaveMesh(const char* name);

int main(int argc, char** args)
{
	if(argc <  2)
	{
		cout << "引数が足りません。ファイルを指定してください" << endl;
		return -1;
	}

	FbxManager* fbxManager = FbxManager::Create();

	FbxIOSettings* ioSetting = FbxIOSettings::Create(fbxManager,IOSROOT);
	fbxManager->SetIOSettings(ioSetting);

	//FbxString fileName("boxTexture.fbx");
	//FbxString fileName("humanoid.fbx");
	//FbxString fileName("model1.fbx");
	//FbxString fileName("sadface.fbx");
	//FbxString fileName("Normals.fbx");
	//FbxString fileName("SD_unitychan_humanoid.fbx");
	FbxString fileName(args[1]);
	auto fbxImporter = FbxImporter::Create(fbxManager,"imp");
	if(!fbxImporter->Initialize(fileName.Buffer(),-1,fbxManager->GetIOSettings()))
	{
		cout << "インポーターの初期化に失敗しました。" << endl;
		return -1;
	}

	auto fbxScene = FbxScene::Create(fbxManager,"fbxscene");

	fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();

	FbxAxisSystem axis = FbxAxisSystem::DirectX;

	// DirectX系
	FbxAxisSystem currentAxis = fbxScene->GetGlobalSettings().GetAxisSystem();
	if(currentAxis != axis)
	{
		FbxAxisSystem::DirectX.ConvertScene(fbxScene);
	}

	FbxSystemUnit unit = fbxScene->GetGlobalSettings().GetSystemUnit();
	if( unit.GetScaleFactor() != 1.0 )
	{
		// センチメーター単位にコンバートする
		FbxSystemUnit::cm.ConvertScene( fbxScene );
	}
	//ChangeTriangle(fbxScene,fbxScene->GetRootNode());
	FbxGeometryConverter converter(fbxManager);
	converter.Triangulate(fbxScene,true);

	int numStatcks = fbxScene->GetSrcObjectCount();
	cout << "anim_stack:" << numStatcks << endl;
	for(int i = 0;i < numStatcks;i++)
	{
		FbxAnimStack* fbxAnimStack = FbxCast<FbxAnimStack>(fbxScene->GetSrcObject(i));
		if(fbxAnimStack)
		{
			cout << "layer_num" << fbxAnimStack->GetMemberCount() << endl;

		}
	}

	FbxNode* rootNode = fbxScene->GetRootNode();
	if( rootNode )
	{
		PrintNode(rootNode,0);
	}

	fbxManager->Destroy();

	SaveMesh(args[1]);

	system("pause");

	return 0;
}

void ChangeTriangle(FbxScene* scene,FbxNode* node)
{
	FbxNodeAttribute* lNodeAttribute = node->GetNodeAttribute();

	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
		{
			FbxGeometryConverter lConverter(node->GetFbxManager());
			// これでどんな形状も三角形化
			lConverter.Triangulate( scene, true );
		}
	}

	const int lChildCount = node->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		// 子ノードを探索
		ChangeTriangle(scene,node->GetChild(lChildIndex));
	}
}

void PrintNode(FbxNode* node,int tab)
{
	for(int i = 0;i < tab;i++)
	{
		//cout << " ";
	}
	//cout << node->GetName() << " ";

	for(int i = 0;i < node->GetNodeAttributeCount();i++)
	{
		FbxNodeAttribute* nodeAttribte = node->GetNodeAttribute();
		auto type = nodeAttribte->GetAttributeType();
		int materialCount = node->GetMaterialCount();
		//cout << nodeAttribte->GetName() << " " << GetNodeAttributeName(type) << ",MaterialCount:" << materialCount;
		for(int k = 0;k < materialCount;k++)
		{
			cout << "Material";
			PrintMaterial(node->GetMaterial(k));
		}
		if(type == FbxNodeAttribute::eMesh)
		{
			FbxMesh* mesh = node->GetMesh();
			if(mesh)
			{
				PrintVertex(mesh);
			}
		}
	}

	cout << endl;

	for(int i = 0;i < node->GetChildCount();i++)
	{
		PrintNode(node->GetChild(i), tab+1);
	}
}

const char* GetNodeAttributeName( FbxNodeAttribute::EType type )
{
	switch(type)
	{
	case FbxNodeAttribute::eUnknown:
		return "Unknown";
	case FbxNodeAttribute::eNull:
		return "Null";
	case FbxNodeAttribute::eMarker:
		return "Marker";
	case FbxNodeAttribute::eSkeleton:
		return "Skeleton";
	case FbxNodeAttribute::eMesh:
		return "Mesh";
	case FbxNodeAttribute::eNurbs:
		return "Nurbs";
	case FbxNodeAttribute::ePatch:
		return "Patch";
	case FbxNodeAttribute::eCamera:
		return "Camera";
	case FbxNodeAttribute::eCameraStereo:
		return "CameraStereo";
	case FbxNodeAttribute::eCameraSwitcher:
		return "CameraSwitcher";
	case FbxNodeAttribute::eLight:
		return "Light";
	case FbxNodeAttribute::eOpticalReference:
		return "OpticalReference";
	case FbxNodeAttribute::eOpticalMarker:
		return "OpticalMarker";
	case FbxNodeAttribute::eNurbsCurve:
		return "NurbsCurve";
	case FbxNodeAttribute::eTrimNurbsSurface:
		return "TrimNurbsSurface";
	case FbxNodeAttribute::eBoundary:
		return "Boundary";
	case FbxNodeAttribute::eNurbsSurface:
		return "NurbsSurface";
	case FbxNodeAttribute::eShape:
		return "Shape";
	case FbxNodeAttribute::eLODGroup:
		return "LODGroup";
	case FbxNodeAttribute::eSubDiv:
		return "SubDiv";
	case FbxNodeAttribute::eCachedEffect:
		return "CachedEffect";
	case FbxNodeAttribute::eLine:
		return "Line";		
	};

	return "Null";
}

void PrintVertex(FbxMesh* mesh)
{
	FbxStringList uvSetName;
	mesh->GetUVSetNames(uvSetName);

	int polygonCount = mesh->GetPolygonCount();
	//cout << "PolygonCount:" << polygonCount << endl;
	int uvSetCount = uvSetName.GetCount();
	bool unmaped = false;
	/*for(int n = 0;n < uvSetCount;n++)
	{
		setName = uvSetName.GetStringAt(n);
		for(int i = 0;i < polygonCount;i++)
		{
			int polygonSize = mesh->GetPolygonSize(i);
			for(int k = 0;k < polygonSize;k++)
			{
				FbxVector2 textureCoord;
				mesh->GetPolygonVertexUV(i,k,setName,textureCoord,unmaped);
			}
		}
	}*/

	FbxLayerElementArrayTemplate<int>* materialArray;
	int materialCount = 0;
	if(mesh->GetMaterialIndices(&materialArray))
	{
		materialCount = materialArray->GetCount();
		/*for(int i = 0;i < polygonCount;i++)
		{
			cout << materialArray->GetAt(i) << endl;
		}*/
	}


	int vartexCount = mesh->GetPolygonVertexCount();
	g_mesh.verteices.reserve( g_mesh.verteices.size() + vartexCount );
	g_mesh.indexArray.reserve( g_mesh.indexArray.size() + vartexCount );

	int indexStart = g_mesh.indexArray.size();

	int matCount = mesh->GetNode()->GetMaterialCount();
	if( matCount <= 0 )
	{
		matCount = 1;
	}
	vector<int>* indexArray = new vector<int>[matCount];

	int count = 0;
	int indexPos = 0;
	int startArray = g_mesh.indexArray.size();
	Vertex vertex;
	for(int i = 0;i < polygonCount;i++)
	{
		if( i < materialCount)
		{	
			indexPos = materialArray->GetAt(i);
			//cout << indexPos << endl;
		}
		int polygonSize = mesh->GetPolygonSize(i);
		for(int k = 0;k < polygonSize;k++)
		{
			int index = mesh->GetPolygonVertex(i,k);
			indexArray[indexPos].push_back( startArray + count );
			//g_mesh.indexArray.push_back( count );
			auto pos = mesh->GetControlPointAt(index);
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(i,k,normal);

			for(int m = 0;m < 3;m++)
			{
				vertex.position[m] = pos[m];
				vertex.normal[m] = normal[m];
			}

			for(int n = 0;n < uvSetCount;n++)
			{
				const char* setName = uvSetName.GetStringAt(n);
				
				FbxVector2 textureCoord;
				mesh->GetPolygonVertexUV(i,k,setName,textureCoord,unmaped);
				for(int m = 0;m < 2;m++)
				{
					vertex.textureCoord[m] = textureCoord[m];
				}
			}

			g_mesh.verteices.push_back(vertex);

			count++;
		}
	}
	
	//cout << "array" << endl;
	for(int i = 0;i < matCount;i++)
	{
		Subset subset;
		subset.mat_index = -1;
		auto mat = mesh->GetNode()->GetMaterial(i);
		if( mat != nullptr )
		{
			int id = mat->GetUniqueID();
			for(int k = 0;k < g_mesh.material.size();k++)
			{
				if(g_mesh.material[k].id == id)
				{
					subset.mat_index = k;
				}
			}
		}

		subset.vertexStart = g_mesh.indexArray.size();
		subset.vertexCount = indexArray[i].size();
		g_mesh.subset.push_back(subset);
		for(int k = 0;k < subset.vertexCount;k++)
		{
			//cout << i << ",";
			g_mesh.indexArray.push_back(indexArray[i][k]);
		}
		//cout << endl;
	}

	delete[] indexArray;
}

void PrintNormal(FbxMesh* mesh)
{
	int count = mesh->GetElementNormalCount();
	cout << "LayerCount:" << count << endl;
	for(int i = 0;i < count;i++)
	{
		auto layer = mesh->GetElementNormal(i);
		switch(layer->GetMappingMode())
		{
		case FbxLayerElement::eByControlPoint:
			{
				int pointCount = mesh->GetControlPointsCount();
				cout << "ControlPointsCount:" << pointCount << endl;
				for(int k = 0;k < pointCount;k++)
				{
					int index = GetNormalIndex(layer,k);
					auto normal = layer->GetDirectArray().GetAt(index);
					cout << k << ":(" << normal[0] << "," << normal[1] << "," << normal[2] << ")" << endl;
				}
			}
			break;
		case FbxLayerElement::eByPolygonVertex:
			{
				int count = 0;
				int polygonCount = mesh->GetPolygonCount();
				cout << "PolygonCount:" << polygonCount << endl;
				for(int k = 0;k < polygonCount;k++)
				{
					int size = mesh->GetPolygonSize(k);
					for(int n = 0;n < size;n++)
					{
						int index = GetNormalIndex(layer,count);
						auto normal = layer->GetDirectArray().GetAt(index);
						cout << k << "," << n << ":(" << normal[0] << "," << normal[1] << "," << normal[2] << ")" << endl;
						count++;
					}
				}
			}
			break;
		default:
			break;
		}
	}
}

int GetNormalIndex(FbxGeometryElementNormal* layer,int index)
{
	switch (layer->GetReferenceMode())
	{
	case FbxLayerElement::eDirect:
		return index;
	case FbxLayerElement::eIndexToDirect:
		return layer->GetIndexArray().GetAt(index);
	default:
		return -1;
	}
	return -1;
}

void PrintUV(FbxMesh* mesh)
{
	FbxStringList uvSetName;
	mesh->GetUVSetNames(uvSetName);

	int uvSetCount = uvSetName.GetCount();
	for(int i = 0;i < uvSetCount;i++)
	{
		const char* name = uvSetName.GetStringAt(i);
		FbxGeometryElementUV* elementUV =  mesh->GetElementUV(name);
		cout << "UVSetName:" << name << endl;

		if(!elementUV)
		{
			continue;
		}
		int polygonCount = mesh->GetPolygonCount();
		auto mappingMode = elementUV->GetMappingMode();
		auto referenceMode = elementUV->GetReferenceMode();
		bool useIndex = referenceMode != FbxGeometryElement::eDirect;
		int indexCount = useIndex?elementUV->GetIndexArray().GetCount():0;
		if(mappingMode == FbxGeometryElement::eByControlPoint)
		{
			for(int k = 0;k < polygonCount;k++)
			{
				int polygonSize = mesh->GetPolygonSize(k);
				for(int n = 0;n < polygonSize;n++)
				{
					int polygonIndex = mesh->GetPolygonVertex(k,n);
					int uvIndex = useIndex?elementUV->GetIndexArray().GetAt(polygonIndex):polygonIndex;
					auto uv = elementUV->GetDirectArray().GetAt(uvIndex);
					cout << "polygonIndex:" << polygonIndex << " (" << uv[0] << "," << uv[1] << ")" << endl;
				}
			}
		}
		else if(mappingMode == FbxGeometryElement::eByPolygonVertex)
		{
			int polygonIndexCount = 0;
			for(int k = 0;k < polygonCount;k++)
			{
				int polygonSize = mesh->GetPolygonSize(k);
				for(int n = 0;n < polygonSize;n++)
				{
					if(polygonIndexCount < indexCount)
					{
						int uvIndex = useIndex?elementUV->GetIndexArray().GetAt(polygonIndexCount):polygonIndexCount;
						auto uv = elementUV->GetDirectArray().GetAt(uvIndex);
						cout << "polygonIndexCount:" << polygonIndexCount << " (" << uv[0] << "," << uv[1] << ")" << endl;
						polygonIndexCount++;
					}
				}
			}
		}
	}
}

void PrintMaterial(FbxSurfaceMaterial* material)
{
	int id = material->GetUniqueID();
	for(int i = 0;i < g_mesh.material.size();i++)
	{
		if(g_mesh.material[i].id == id)
		{
			return;
		}
	}
	Material mat;
	mat.id = id;
	
	
	cout << material->GetName() << endl;
	cout << material->GetClassId().GetName();
	if(material->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		cout << " Phong";
	}
	else if(material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		cout << " Lambert";
	}

	cout << endl;
	string textureName;
	Color color = PrintMaterialProperty(material,FbxSurfaceMaterial::sDiffuse,FbxSurfaceMaterial::sDiffuseFactor,&textureName);
	for(int i = 0;i < 3;i++)
	{
		mat.diffuse[i] = color.color[i];
	}
	color = PrintMaterialProperty(material,FbxSurfaceMaterial::sAmbient,FbxSurfaceMaterial::sAmbientFactor,&textureName);
	for(int i = 0;i < 3;i++)
	{
		mat.ambient[i] = color.color[i];
	}
	color = PrintMaterialProperty(material,FbxSurfaceMaterial::sEmissive,FbxSurfaceMaterial::sEmissiveFactor,&textureName);
	for(int i = 0;i < 3;i++)
	{
		mat.emmisive[i] = color.color[i];
	}
	color = PrintMaterialProperty(material,FbxSurfaceMaterial::sSpecular,FbxSurfaceMaterial::sSpecularFactor,&textureName);
	for(int i = 0;i < 3;i++)
	{
		mat.specular[i] = color.color[i];
	}
	if( !textureName.empty() )
	{
		auto pos = textureName.rfind("\\");
		if(pos != string::npos)
		{
			textureName.erase(0,pos+1);
		}
		mat.texture_name = textureName.c_str();
		cout << "Texture Name:" << textureName.c_str() << endl;
	}

	FbxProperty transparency = material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	mat.alpha = 1.0f;
	if(transparency.IsValid())
	{
		mat.alpha = 1.0f - static_cast<float>(transparency.Get<FbxDouble>());
		cout << FbxSurfaceMaterial::sTransparencyFactor << ":" << transparency.Get<FbxDouble>() << " " << mat.alpha << endl;
	}
	FbxProperty shininess = material->FindProperty(FbxSurfaceMaterial::sShininess);
	mat.power = 0.0f;
	if(shininess.IsValid())
	{
		mat.power = static_cast<float>(shininess.Get<FbxDouble>());
		cout << FbxSurfaceMaterial::sShininess << ":" << shininess.Get<FbxDouble>() << endl;
	}

	g_mesh.material.push_back(mat);
}

Color PrintMaterialProperty(FbxSurfaceMaterial* material,const char* prop,const char* propFactory,string* textureName)
{
	Color result = {0.0f,0.0f,0.0f};
	FbxProperty fProp = material->FindProperty(prop);
	FbxProperty fPropFactory = material->FindProperty(propFactory);
	if(fProp.IsValid()&&fPropFactory.IsValid())
	{
		FbxDouble3 color = fProp.Get<FbxDouble3>();
		double k = fPropFactory.Get<FbxDouble>();
		cout << " " << prop << ":(" << color[0] << "," << color[1] << "," << color[2] << ") " << propFactory << ":" << k << endl;
		for(int i = 0;i < 3;i++)
		{
			result.color[i] = color[i] * k;
		}
	}

	if( fProp.IsValid() )
	{
		int textureCount = fProp.GetSrcObjectCount<FbxFileTexture>();
		for(int i = 0;i < textureCount;i++)
		{
			FbxFileTexture* texture = fProp.GetSrcObject<FbxFileTexture>(i);
			if(!texture)
			{
				continue;
			}
			FbxString uvsetName = texture->UVSet.Get();
			cout <<"texture:" << i << " " << uvsetName.Buffer() << "," << texture->GetFileName() << endl;
			*textureName = texture->GetFileName();
		}

		textureCount = fProp.GetSrcObjectCount<FbxLayeredTexture>();
		for(int i = 0;i < textureCount;i++)
		{
			FbxLayeredTexture* layeredTexture = fProp.GetSrcObject<FbxLayeredTexture>(i);
			int layeredTextureCount = layeredTexture->GetSrcObjectCount<FbxFileTexture>();
			for(int k = 0;k < layeredTextureCount;k++)
			{
				FbxFileTexture* texture = layeredTexture->GetSrcObject<FbxFileTexture>(i);
				if(!texture)
				{
					continue;
				}
				FbxString uvsetName = texture->UVSet.Get();
				cout <<"LayeredTexture:" << i << " " << uvsetName.Buffer() << "," << texture->GetFileName() << endl;
				*textureName = texture->GetFileName();
			}
		}
	}

	return result;
}

void SaveMesh(const char* name)
{
	
	string fileName(name);
	auto pos = fileName.rfind("\\");
	if(pos != string::npos)
	{
		fileName.erase(0,pos+1);
	}
	pos = fileName.rfind(".");
	if(pos != string::npos)
	{
		fileName.erase(pos,4);
	}
	cout << fileName.c_str();
	ofstream file(fileName + ".gmb", ios::out|ios::binary);

	if(!file.is_open())
	{
		return;
	}
	int vertexCount = g_mesh.verteices.size();
	int indexCount = g_mesh.indexArray.size();
	int subsetCount = g_mesh.subset.size();
	int materialCount = g_mesh.material.size();
	file.write(reinterpret_cast<const char*>(&vertexCount),sizeof(int));
	file.write(reinterpret_cast<const char*>(&indexCount),sizeof(int));
	file.write(reinterpret_cast<const char*>(&subsetCount),sizeof(int));
	file.write(reinterpret_cast<const char*>(&materialCount),sizeof(int));
	file.write(reinterpret_cast<const char*>(&g_mesh.verteices[0]),sizeof(Vertex) * vertexCount);
	file.write(reinterpret_cast<const char*>(&g_mesh.indexArray[0]),sizeof(int) * indexCount);
	file.write(reinterpret_cast<const char*>(&g_mesh.subset[0]),sizeof(Subset) * subsetCount);
	for(int i = 0;i < materialCount;i++)
	{
		file.write(reinterpret_cast<const char*>(g_mesh.material[i].diffuse),sizeof(float) * 3);
		file.write(reinterpret_cast<const char*>(&g_mesh.material[i].alpha),sizeof(float));
		file.write(reinterpret_cast<const char*>(g_mesh.material[i].ambient),sizeof(float) * 3);
		file.write(reinterpret_cast<const char*>(g_mesh.material[i].specular),sizeof(float) * 3);
		file.write(reinterpret_cast<const char*>(&g_mesh.material[i].power),sizeof(float));
		file.write(reinterpret_cast<const char*>(g_mesh.material[i].emmisive),sizeof(float) * 3);
		int nameLength = g_mesh.material[i].texture_name.size() + 1;
		file.write(reinterpret_cast<const char*>(&nameLength),sizeof(int));
		file.write(g_mesh.material[i].texture_name.c_str(),nameLength);
	}


	file.close();

	ofstream dfile(fileName + ".gmd");

	if(!dfile.is_open())
	{
		return;
	}
	dfile << "Vertex Count:" << vertexCount << endl;
	for(int k = 0;k < vertexCount;k++)
	{
		dfile << "(" << g_mesh.verteices[k].position[0] << "," << g_mesh.verteices[k].position[1] << "," << g_mesh.verteices[k].position[2] << ") ";
		dfile << "(" << g_mesh.verteices[k].normal[0] << "," << g_mesh.verteices[k].normal[1] << "," << g_mesh.verteices[k].normal[2] << ") ";
		dfile << "(" << g_mesh.verteices[k].textureCoord[0] << "," << g_mesh.verteices[k].textureCoord[1] << ") " << endl;
	}

	dfile << "Index Count:" << indexCount << endl;
	for(int k = 0;k < indexCount;k++)
	{
		dfile << g_mesh.indexArray[k] << ",";
	}
	dfile << endl;

	dfile << "Subset Count:" << subsetCount << endl;
	for(int k = 0;k < subsetCount;k++)
	{
		dfile << "Material Index:" << g_mesh.subset[k].mat_index << " Vertex Start:" << g_mesh.subset[k].vertexStart << " Vertex Count:" << g_mesh.subset[k].vertexCount << endl;
	}

	dfile << "Material Count:" << materialCount << endl;
	for(int k = 0;k < materialCount;k++)
	{
		Material& mat = g_mesh.material[k];
		dfile << "Material" << k << endl;
		dfile << "Diffuse Color(" << mat.diffuse[0] << "," << mat.diffuse[1] << "," << mat.diffuse[2] << ")" << endl;
		dfile << "Alpah:" << mat.alpha << endl;
		dfile << "Ambient Color(" << mat.ambient[0] << "," << mat.ambient[1] << "," << mat.ambient[2] << ")" << endl;
		dfile << "Specular Color(" << mat.specular[0] << "," << mat.specular[1] << "," << mat.specular[2] << ")" << endl;
		dfile << "Power:" << mat.power << endl;
		dfile << "Emmisive Color(" << mat.emmisive[0] << "," << mat.emmisive[1] << "," << mat.emmisive[2] << ")" << endl;
		dfile << "Texture Name:" << mat.texture_name.c_str() << endl;
		
	}

	dfile.close();
}
