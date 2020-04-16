#include <fbxsdk.h>
#pragma comment( lib, "libfbxsdk-mt.lib" )
#pragma comment( lib, "libxml2-mt.lib" )
#pragma comment( lib, "zlib-mt.lib" )
#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;


struct Vertex
{
	float position[3];
	float normal[3];
	//float textureCoord[2];
};

struct Mesh
{
	int vartexCount;
	int* indexArray;
	Vertex* verteices;
};

struct Model
{
};

void ChangeTriangle(FbxScene* scene,FbxNode* node);
void PrintNode(FbxNode* node,int tab);
const char* GetNodeAttributeName( FbxNodeAttribute::EType type );
void PrintVertex(FbxMesh* mesh);
void PrintNormal(FbxMesh* mesh);
int GetNormalIndex(FbxGeometryElementNormal* layer,int index);
void PrintUV(FbxMesh* mesh);
void PrintMaterial(FbxSurfaceMaterial* material);
void PrintMaterialProperty(FbxSurfaceMaterial* material,const char* prop,const char* propFactory);

int main()
{
	FbxManager* fbxManager = FbxManager::Create();

	FbxIOSettings* ioSetting = FbxIOSettings::Create(fbxManager,IOSROOT);
	fbxManager->SetIOSettings(ioSetting);

	//FbxString fileName("boxTexture.fbx");
	//FbxString fileName("humanoid.fbx");
	//FbxString fileName("model1.fbx");
	//FbxString fileName("sadface.fbx");
	//FbxString fileName("Normals.fbx");
	FbxString fileName("SD_unitychan_humanoid.fbx");
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
		cout << " ";
	}
	cout << node->GetName() << " ";

	for(int i = 0;i < node->GetNodeAttributeCount();i++)
	{
		FbxNodeAttribute* nodeAttribte = node->GetNodeAttribute();
		auto type = nodeAttribte->GetAttributeType();
		int materialCount = node->GetMaterialCount();
		cout << nodeAttribte->GetName() << " " << GetNodeAttributeName(type) << ",MaterialCount:" << materialCount;
		for(int k = 0;k < materialCount;k++)
		{
			PrintMaterial(node->GetMaterial(k));
		}
		if(type == FbxNodeAttribute::eMesh)
		{
			FbxMesh* mesh = node->GetMesh();
			if(mesh)
			{
				PrintVertex(mesh);
				/*int count = mesh->GetControlPointsCount();
				cout << "\nControlPointsCount:" << count << endl;
				for(int k = 0;k < count;k++)
				{
					auto pos = mesh->GetControlPointAt(k);
					cout << k << ":(" << pos[0] << "," << pos[1] << "," << pos[2] << ")" << endl;
				}
				count = mesh->GetPolygonVertexCount();
				cout << "PolygonVertexCount:" << count << endl;
				auto vertices = mesh->GetPolygonVertices();
				for(int k = 0;k < count;k++)
				{
					cout << k << ":" << vertices[k]  << endl;
				}
				PrintNormal(mesh);
				PrintUV(mesh);*/
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

	Mesh meshData;

	meshData.vartexCount = mesh->GetPolygonVertexCount();
	meshData.verteices = new Vertex[meshData.vartexCount];
	meshData.indexArray = new int[meshData.vartexCount];
	int count = 0;
	for(int i = 0;i < polygonCount;i++)
	{
		int polygonSize = mesh->GetPolygonSize(i);
		if( polygonSize != 3 )
		{
			cout << "polygonSize:" << polygonSize << endl;
		}
		for(int k = 0;k < polygonSize;k++)
		{
			int index = mesh->GetPolygonVertex(i,k);
			meshData.indexArray[count] = count;
			auto pos = mesh->GetControlPointAt(index);
			FbxVector4 normal;
			mesh->GetPolygonVertexNormal(i,k,normal);

			/*cout << "Vertex Data "  << i << "," << k << ":";
			cout << "Pos:(";*/
			for(int m = 0;m < 3;m++)
			{
				meshData.verteices[count].position[m] = pos[m];
				//cout << pos[m] << ",";
			}
			//cout << ") ";

			//cout << "Normal:(";
			for(int m = 0;m < 3;m++)
			{
				meshData.verteices[count].normal[m] = normal[m];
				//cout << normal[m] << ",";
			}
			//cout << ") ";

			/*for(int n = 0;n < uvSetCount;n++)
			{
				const char* setName = uvSetName.GetStringAt(n);
				
				FbxVector2 textureCoord;
				mesh->GetPolygonVertexUV(i,k,setName,textureCoord,unmaped);
				cout << "UV" << n << ":(";
				for(int m = 0;m < 2;m++)
				{
					meshData.verteices[count].textureCoord[m] = textureCoord[m];
					cout << textureCoord[m] << ",";
				}
				cout << ") ";
			}*/

			//cout << endl;
			
			count++;
		}
	}

	ofstream file("model.gmb",ios::binary);

	if(!file.is_open())
	{
		return;
	}
	file.write(reinterpret_cast<const char*>(&meshData.vartexCount),sizeof(int));
	for(int i = 0;i < meshData.vartexCount;i++)
	{
		file.write(reinterpret_cast<const char*>(meshData.verteices[i].position),sizeof(float) * 3);
	}
	for(int i = 0;i < meshData.vartexCount;i++)
	{
		file.write(reinterpret_cast<const char*>(meshData.verteices[i].normal),sizeof(float) * 3);
	}
	/*for(int i = 0;i < meshData.vartexCount;i++)
	{
		file.write(reinterpret_cast<const char*>(meshData.verteices[i].textureCoord),sizeof(float) * 2);
	}*/
	file.write(reinterpret_cast<const char*>(meshData.indexArray),sizeof(int) * meshData.vartexCount);
	file.close();

	ofstream dfile("model.gmd");

	if(!dfile.is_open())
	{
		return;
	}
	dfile << "VertexCount:" << meshData.vartexCount << endl;
	
	dfile << "Position:" << endl;
	for(int i = 0;i < meshData.vartexCount;i++)
	{
		dfile << "(" << meshData.verteices[i].position[0] << "," << meshData.verteices[i].position[1] << "," << meshData.verteices[i].position[2] << ")" << endl;
	}
	dfile << "Normal:" << endl;
	for(int i = 0;i < meshData.vartexCount;i++)
	{
		dfile << "(" << meshData.verteices[i].normal[0] << "," << meshData.verteices[i].normal[1] << "," << meshData.verteices[i].normal[2] << ")" << endl;
	}
	/*dfile << "TextureCoord:" << endl;
	for(int i = 0;i < meshData.vartexCount;i++)
	{
		dfile << "(" << meshData.verteices[i].textureCoord[0] << "," << meshData.verteices[i].textureCoord[1] << ")" << endl;
	}*/
	dfile << "indexArray:" << endl;
	for(int i = 0;i < meshData.vartexCount;i++)
	{
		dfile << meshData.indexArray[i] << ",";
	}
	dfile << endl;
	dfile.close();
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
	cout << material->GetClassId().GetName();
	if(material->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		cout << " Lambert";
	}
	else if(material->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		cout << " Phong";
	}
	cout << endl;
	PrintMaterialProperty(material,FbxSurfaceMaterial::sDiffuse,FbxSurfaceMaterial::sDiffuseFactor);
	PrintMaterialProperty(material,FbxSurfaceMaterial::sAmbient,FbxSurfaceMaterial::sAmbientFactor);
	PrintMaterialProperty(material,FbxSurfaceMaterial::sEmissive,FbxSurfaceMaterial::sEmissiveFactor);
	PrintMaterialProperty(material,FbxSurfaceMaterial::sSpecular,FbxSurfaceMaterial::sSpecularFactor);

	FbxProperty transparency = material->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if(transparency.IsValid())
	{
		cout << FbxSurfaceMaterial::sTransparencyFactor << ":" << transparency.Get<FbxDouble>() << endl;
	}
	FbxProperty shininess = material->FindProperty(FbxSurfaceMaterial::sShininess);
	if(shininess.IsValid())
	{
		cout << FbxSurfaceMaterial::sShininess << ":" << shininess.Get<FbxDouble>() << endl;
	}
}

void PrintMaterialProperty(FbxSurfaceMaterial* material,const char* prop,const char* propFactory)
{
	FbxProperty fProp = material->FindProperty(prop);
	FbxProperty fPropFactory = material->FindProperty(propFactory);
	if(fProp.IsValid()&&fPropFactory.IsValid())
	{
		FbxDouble3 color = fProp.Get<FbxDouble3>();
		double k = fPropFactory.Get<FbxDouble>();
		cout << " " << prop << ":(" << color[0] << "," << color[1] << "," << color[2] << ") " << propFactory << ":" << k << endl;
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
			}
		}
	}
}


