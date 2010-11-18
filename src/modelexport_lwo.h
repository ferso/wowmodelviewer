#ifndef MODELEXPORT_LWO_H
#define MODELEXPORT_LWO_H

#include <wx/wfstream.h>

#include "globalvars.h"
#include "modelexport.h"
#include "modelcanvas.h"

#define MAX_POINTS_PER_POLYGON 1023
#define FRAMES_PER_SECOND 30

// -----------------------------------------
// New LW Header Stuff
// -----------------------------------------

int i32;
uint32 u32;
float f32;
uint16 u16;
unsigned char ub;
struct LWSurface;

// Counting system for Scene Items
struct CountSystem{
	uint32 Value;

	void Reset(){
		Value = 0;
	}
	uint32 GetPlus(){
		return Value;
		Value++;
	}
};

CountSystem LW_ObjCount;
CountSystem LW_LightCount;
CountSystem LW_BoneCount;
CountSystem LW_CamCount;

// --== Lightwave Type Numbers ==--
// These numbers identify object, light and bone types in the Lightwave Program.
enum LWItemType {
	LW_ITEMTYPE_NOPARENT = -1,
	LW_ITEMTYPE_OBJECT = 1,
	LW_ITEMTYPE_LIGHT,
	LW_ITEMTYPE_CAMERA,
	LW_ITEMTYPE_BONE,
};
enum LWLightType {
	LW_LIGHTTYPE_DISTANT = 0,
	LW_LIGHTTYPE_POINT,
	LW_LIGHTTYPE_SPOTLIGHT,
	LW_LIGHTTYPE_LINEAR,
	LW_LIGHTTYPE_AREA,

	LW_LIGHTTYPE_PLUGIN = 100,
};
enum LWBoneType {
	LW_BONETYPE_BONE = 0,
	LW_BONETYPE_JOINT,
};
enum LWTextureAxisType{
	LW_TEXTUREAXIS_PLANAR = 0,
	LW_TEXTUREAXIS_UV,
};

// Polygon Chunk
struct PolyChunk {
	uint16 numVerts;		// The Number of Indices that make up the poly.
	uint32 indice[3];		// The IDs of the 3 Indices that make up each poly. In reality, this should be indice[MAX_POINTS_PER_POLYGON], but WoW will never go above 3.
};

// Polygon Normal
struct PolyNormal {
	wxString NormalMapName;
	uint32 indice[3];
	uint32 polygon;
	Vec3D direction[3];		// This is the direction the polygon's normal should point towards.	
};

struct AnimVector{
	std::vector<float> Value;
	std::vector<float> Time;
	std::vector<uint16> Spline;

	// Push values into the arrays.
	void Push(float value, float time, uint16 spline = 0){
		Value.push_back(value);
		Time.push_back(time);
		Spline.push_back(spline);
	}

	// Get the number of keyframes for this channel.
	size_t Size(){
		return Time.size();
	}

	// Generate, and push the first key.
	AnimVector() {}
	AnimVector(float value, float time, uint16 spline = 0){
		Push(value,time,spline);
	}
};

struct AnimVec3D{
	AnimVector x, y, z;

	AnimVec3D() {}
	AnimVec3D(AnimVector cx, AnimVector cy, AnimVector cz){
		x = cx;
		y = cy;
		z = cz;
	}
};

// Animation Data
struct AnimationData {
	AnimVec3D Position;
	AnimVec3D Rotation;
	AnimVec3D Scale;

	// Returns the number of keyframes in this animation.
	size_t Size(){
		float time = 0.0f;

		// Check Position Timing
		for (size_t i=0;i<Position.x.Size();i++){
			if (Position.x.Time[i] > time)
				time = Position.x.Time[i];
			if (Position.y.Time[i] > time)
				time = Position.y.Time[i];
			if (Position.z.Time[i] > time)
				time = Position.z.Time[i];
		}
		// Check Rotation Timing
		for (size_t i=0;i<Rotation.x.Size();i++){
			if (Rotation.x.Time[i] > time)
				time = Rotation.x.Time[i];
			if (Rotation.y.Time[i] > time)
				time = Rotation.y.Time[i];
			if (Rotation.z.Time[i] > time)
				time = Rotation.z.Time[i];
		}
		// Check Scale Timing
		for (size_t i=0;i<Scale.x.Size();i++){
			if (Scale.x.Time[i] > time)
				time = Scale.x.Time[i];
			if (Scale.y.Time[i] > time)
				time = Scale.y.Time[i];
			if (Scale.z.Time[i] > time)
				time = Scale.z.Time[i];
		}

		return time;
	}

	AnimationData() {}
	AnimationData(AnimVec3D position, AnimVec3D rotation, AnimVec3D scale){
		Position = position;
		Rotation = rotation;
		Scale = scale;
	}
};

// --== Scene Formats ==--
struct LWBones{
	uint32 BoneID;
	Vec3D RestPos;
	Vec3D RestRot;
	AnimationData AnimData;
	wxString Name;
	bool Active;
	uint16 BoneType;
	uint32 ParentID;
	uint16 ParentType;
	float Length;
	wxString WeightMap_Name;
	bool WeightMap_Only;
	bool WeightMap_Normalize;

	LWBones(uint32 BoneNum){
		BoneID = BoneNum;
		BoneType = LW_BONETYPE_JOINT;
		Active = true;
		WeightMap_Name = wxEmptyString;
		WeightMap_Only = true;
		WeightMap_Normalize = true;
		Length = 0.25f;
		RestPos = Vec3D(0,0,0);
		RestRot = Vec3D(0,0,0);
	}
};

// Scene Object
struct LWSceneObj{
	uint32 ObjectID;
	int32 LayerID;
	AnimationData AnimData;
	wxString Name;
	wxString ObjectTags;
	std::vector<LWBones> Bones;
	bool isNull;
	uint32 ParentID;
	int16 ParentType;	// -1 or LW_ITEMTYPE_NOPARENT for No Parent

	LWSceneObj(wxString name, uint32 id, int32 parentID, int16 parentType = LW_ITEMTYPE_NOPARENT, bool IsNull = false, int32 layerID = 1){
		Name = name;
		ObjectID = id;
		ParentType = parentType;
		ParentID = parentID;
		isNull = IsNull;
		LayerID = layerID;
		ObjectTags = wxEmptyString;
	}
};

struct LWLight{
	uint32 LightID;
	AnimationData AnimData;
	wxString Name;
	Vec3D Color;
	float Intensity;
	uint16 LightType;
	float FalloffRadius;
	uint32 ParentID;
	int16 ParentType;
	bool UseAttenuation;

	LWLight(){
		Intensity = 1.0f;
		LightType = LW_LIGHTTYPE_POINT;
		ParentType = LW_ITEMTYPE_NOPARENT;
		FalloffRadius = 2.5f;
	}
};

struct LWCamera{
	uint32 CameraID;
	AnimationData AnimData;
	wxString Name;
	float FieldOfView;
	int32 TargetObjectID;
	int32 ParentID;
	uint8 ParentType;
};

// Master Scene
struct LWScene{
	std::vector<LWSceneObj> Objects;
	std::vector<LWLight> Lights;
	std::vector<LWCamera> Cameras;

	wxString FileName;
	wxString FilePath;
	float AmbientIntensity;
	float FirstFrame;
	float LastFrame;

	LWScene(wxString file, wxString path, float ambint = 0.5, float FrameFirst = 0.0f, float FrameLast = 1.0f){
		FileName = file;
		FilePath = path + SLASH;
		FirstFrame = FrameFirst;
		LastFrame = FrameLast;

		AmbientIntensity = ambint;
	}

	~LWScene(){
		Objects.clear();
		Lights.clear();
		Cameras.clear();
		FileName.Clear();
		FilePath.Clear();
		//free(&AmbientIntensity);
	}
};

// --== Object Formats ==--
// Weight Data
struct LWWeightInfo{
	uint32 PointID;
	float Value;
};

struct LWWeightMap{
	wxString WeightMapName;
	std::vector<LWWeightInfo> PData;
	uint32 BoneID;

	LWWeightMap(wxString MapName=_T("Weight"), uint32 Bone_ID = 0){
		WeightMapName = MapName;
		BoneID = Bone_ID;
	}
};

// Vertex Color Data
struct LWVertexColor{
	uint8 r, g, b, a;

	LWVertexColor(uint8 R=255, uint8 G=255, uint8 B=255, uint8 A=255){
		r = R;
		g = G;
		b = B;
		a = A;
	}
};

// Point Chunk Data
struct LWPoint {
	Vec3D PointData;
	Vec2D UVData;
	LWVertexColor VertexColors;		// In reality, this should be a std::vector, but WoW doesn't use more than 1.
};

// Poly Chunk Data
struct LWPoly {
	PolyChunk PolyData;
	PolyNormal Normals;
	uint32 PartTagID;
	uint32 SurfTagID;
};

// -= Lightwave Chunk Structures =-

// Layer Data
struct LWLayer {
	// Layer Data
	wxString Name;						// Name of the Layer, Optional
	int ParentLayer;					// 0-based number of parent layer. -1 or omitted for no parent.

	// Points Block
	std::vector<LWPoint>Points;			// Various Point Blocks used by this layer.
	std::vector<LWWeightMap>Weights;	// Weight Map Data
	bool HasVectorColors;				// Is True if the layer has a Vector Color map
	Vec3D BoundingBox1;					// First Corner of the Layer's Bounding Box
	Vec3D BoundingBox2;					// Second Corner of the Layer's Bounding Box

	// Poly Block
	std::vector<LWPoly> Polys;

	LWLayer(){
		Name = _T("(unnamed)");
		HasVectorColors = false;
		ParentLayer = -1;
		BoundingBox1 = Vec3D();
		BoundingBox2 = Vec3D();
	};
};

// Clip Data
struct LWClip {
	wxString Filename;		// The Path & Filename of the image to be used in Lightwave
	wxString Source;		// The Source Path & Filename, as used in WoW.
	uint32 TagID;			// = Number of Parts + Texture number
};

struct LWSurf_Image {
	uint32 ID;				// Tag ID for the Image
	uint16 Axis;			// LWTextureAxisType value
	float UVRate_U;			// Rate to move the U with UV Animation
	float UVRate_V;			// Rate to move the V with UV Animation

	// Contructor
	LWSurf_Image(uint32 idtag=-1, uint16 axis=LW_TEXTUREAXIS_UV, float UVAnimRate_U=0.0f, float UVAnimRate_V=0.0f){
		ID = idtag;
		Axis = axis;
		UVRate_U = UVAnimRate_U;
		UVRate_V = UVAnimRate_V;
	}
};

// Surface Chunk Data
struct LWSurface {
	wxString Name;			// The Surface's Name
	wxString Comment;		// Comment for the surface.

	bool isDoubleSided;		// Should it show the same surface on both sides of a poly.
	bool hasVertColors;

	// Base Attributes
	Vec3D Surf_Color;			// Surface Color as floats
	float Surf_Diff;			// Diffusion Amount
	float Surf_Lum;				// Luminosity Amount
	float Surf_Spec;			// Specularity Amount
	float Surf_Reflect;			// Reflection Amount
	float Surf_Trans;			// Transparancy Amount

	// Images
	LWSurf_Image Image_Color;	// Color Image data
	LWSurf_Image Image_Spec;	// Specular Image data
	LWSurf_Image Image_Trans;	// Transparancy Image data

	// Constructors
	LWSurface(wxString name, wxString comment, LWSurf_Image ColorID, LWSurf_Image SpecID, LWSurf_Image TransID, Vec3D SurfColor=Vec3D(0.75,0.75,0.75), float Diffusion=1.0f, float Luminosity = 0.0f, bool doublesided = false, bool hasVC = false){
		Name = name;
		Comment = comment;

		Surf_Color = SurfColor;
		Surf_Diff = Diffusion;
		Surf_Lum = Luminosity;
		Surf_Spec = 0.0f;
		Surf_Reflect = 0.0f;
		Surf_Trans = 0.0f;

		Image_Color = ColorID;
		Image_Spec = SpecID;
		Image_Trans = TransID;

		isDoubleSided = doublesided;
		hasVertColors = hasVC;
	}
};

// The Master Structure for each individual LWO file.
struct LWObject {
	wxArrayString PartNames;			// List of names for all the Parts;
	std::vector<LWLayer> Layers;		// List of Layers (usually 1) that make up the Geometery.
	std::vector<LWClip> Images;			// List of all the Unique Images used in the model.
	std::vector<LWSurface> Surfaces;	// List of the Surfaces used in the model.

	wxString SourceType;				// M2, WMO or ADT

	LWObject(){
		SourceType = wxEmptyString;
	}

	void Plus(LWObject o, int LayerNum=0,wxString PartNamePrefix = _T("")){
		//wxLogMessage(_T("Running LW Plus Function, Num Layers: %i, into Layer %i."),o.Layers.size(),LayerNum);
		// Add layers if nessicary...
		while (Layers.size() < (size_t)LayerNum+1){
			LWLayer a;
			Layers.push_back(a);
		}

		uint32 OldPartNum = (uint32)PartNames.size();
		uint32 OldSurfNum = (uint32)Surfaces.size();
		uint32 OldTagNum =  OldPartNum + OldSurfNum;
		std::vector<uint32> OldPointNum;

		// Parts
		for (size_t i=0;i<o.PartNames.size();i++){
			wxString a = PartNamePrefix + o.PartNames[i];
			PartNames.push_back(a);
		}
		// Surfaces
		for (size_t i=0;i<o.Surfaces.size();i++){
			LWSurface s = o.Surfaces[i];
			s.Image_Color.ID += OldTagNum;
			Surfaces.push_back(s);
		}
		//Images
		for (size_t i=0;i<o.Images.size();i++){
			LWClip a = o.Images[i];
			a.TagID += OldTagNum;
			Images.push_back(a);
		}

		// Parts Difference
		uint32 PartDiff = (uint32)PartNames.size() - OldPartNum;

		// --== Layers ==--
		// Process Old Layers
		for (size_t i=0;i<Layers.size();i++){
			OldPointNum.push_back((uint32)Layers[i].Points.size());
			for (size_t x=0;x<Layers[i].Polys.size();x++){
				Layers[i].Polys[x].SurfTagID += PartDiff;
			}
		}
		// Proccess New Layers
		for (size_t i=0;i<o.Layers.size();i++){
			LWLayer a = o.Layers[i];

			// Vector Colors
			if (a.HasVectorColors == true)
				Layers[LayerNum].HasVectorColors = true;

			// Bounding Box
			if (a.BoundingBox1.x > Layers[LayerNum].BoundingBox1.x)
				Layers[LayerNum].BoundingBox1.x = a.BoundingBox1.x;
			if (a.BoundingBox1.y > Layers[LayerNum].BoundingBox1.y)
				Layers[LayerNum].BoundingBox1.y = a.BoundingBox1.y;
			if (a.BoundingBox1.z > Layers[LayerNum].BoundingBox1.z)
				Layers[LayerNum].BoundingBox1.z = a.BoundingBox1.z;
			if (a.BoundingBox2.x < Layers[LayerNum].BoundingBox2.x)
				Layers[LayerNum].BoundingBox2.x = a.BoundingBox2.x;
			if (a.BoundingBox2.y < Layers[LayerNum].BoundingBox2.y)
				Layers[LayerNum].BoundingBox2.y = a.BoundingBox2.y;
			if (a.BoundingBox2.z < Layers[LayerNum].BoundingBox2.z)
				Layers[LayerNum].BoundingBox2.z = a.BoundingBox2.z;

			// Points
			for (size_t x=0;x<a.Points.size();x++){
				Layers[LayerNum].Points.push_back(a.Points[x]);
			}
			// Polys
			for (size_t x=0;x<a.Polys.size();x++){
				for (uint16 j=0;j<a.Polys[x].PolyData.numVerts;j++){
					a.Polys[x].PolyData.indice[j] += OldPointNum[LayerNum];
				}
				a.Polys[x].PartTagID += OldPartNum;
				a.Polys[x].SurfTagID += OldTagNum;
				Layers[LayerNum].Polys.push_back(a.Polys[x]);
			}
		}
	}
	LWObject operator= (LWObject o){
		PartNames = o.PartNames;
		Layers = o.Layers;
		Images = o.Images;
		Surfaces = o.Surfaces;

		SourceType = o.SourceType;

		// return LWObject?
		return o;
	}
	~LWObject(){
		PartNames.Clear();
		Layers.clear();
		Images.clear();
		Surfaces.clear();
		SourceType.Clear();
	}
};

// --== Writing Functions ==--
// VX is Lightwave Shorthand for any Point Number, because Lightwave stores points differently if they're over a certain threshold.
void LW_WriteVX(wxFFileOutputStream &f, uint32 p, uint32 &Size){
	if (p <= 0xFF00){
		uint16 indice = MSB2(p & 0x0000FFFF);
		f.Write(reinterpret_cast<char *>(&indice),2);
		Size += 2;
	}else{
		uint32 indice = MSB4<uint32>(p + 0xFF000000);
		f.Write(reinterpret_cast<char *>(&indice), 4);
		Size += 4;
	}
}

// Keyframe Writing Functions
// Writes a single Key for an envelope.
void WriteLWSceneEnvKey(ofstream &fs, uint32 Chan, float value, float time, uint16 spline = 0)
{
	fs << _T("  Key ");				// Announces the start of a Key
	fs << value;					// The Key's Value;
	fs << _T(" " << time);			// Time, in seconds, a float. This can be negative, zero or positive. Keys are listed in the envelope in increasing time order.
	fs << _T(" " << spline);		// The curve type, an integer: 0 - TCB, 1 - Hermite, 2 - 1D Bezier (obsolete, equivalent to Hermite), 3 - Linear, 4 - Stepped, 5 - 2D Bezier
	fs << _T(" 0 0 0 0 0 0 \n");	// Curve Data 1-6, all 0s for now.
}

// Used for writing the keyframes of an animation.
// Single Keyframe use. Use WriteLWSceneEnvArray for writing animations.
void WriteLWSceneEnvChannel(ofstream &fs, uint32 ChanNum, float value, float time, uint16 spline = 0)
{
	fs << _T("Channel " << ChanNum << "\n");	// Channel Number
	fs << _T("{ Envelope\n");
	fs << _T("  1\n");							// Number of Keys in this envelope.
	WriteLWSceneEnvKey(fs,ChanNum,value,time,spline);
	fs << _T("  Behaviors 1 1\n");				// Pre/Post Behaviors. Defaults to 1 - Constant.
	fs << _T("}\n");
}

// Multiple-frame use.
void WriteLWSceneEnvArray(ofstream &fs, uint32 ChanNum, AnimVector value)
{
	fs << _T("Channel " << ChanNum << "\n");
	fs << _T("{ Envelope\n");
	fs << _T("  " << value.Time.size() << "\n");
	for (uint32 n=0;n<value.Time.size();n++){
		float time = (value.Time[n]/FRAMES_PER_SECOND)/FRAMES_PER_SECOND;	// Convert from WoW Frame Number into a Per-Ssecond Float

		WriteLWSceneEnvKey(fs,ChanNum,value.Value[n],time,value.Spline[n]);
	}

	fs << _T("  Behaviors 1 1\n");
	fs << _T("}\n");
}

// Writes the "Plugin" information for a scene object, light, camera &/or bones.
void WriteLWScenePlugin(ofstream &fs, wxString type, uint32 PluginCount, wxString PluginName, wxString Data = wxEmptyString)
{
	fs << _T("Plugin " << type << " " << PluginCount << " " << PluginName << "\n" << Data << "EndPlugin\n");
}

// Write Keyframe Motion Array
void LW_WriteMotionArray(ofstream &fs,AnimationData AnimData){
	// Position
	WriteLWSceneEnvArray(fs,0,AnimData.Position.x);
	WriteLWSceneEnvArray(fs,1,AnimData.Position.y);
	WriteLWSceneEnvArray(fs,2,AnimData.Position.z);
	// Rotation
	WriteLWSceneEnvArray(fs,3,AnimData.Rotation.x);
	WriteLWSceneEnvArray(fs,4,AnimData.Rotation.y);
	WriteLWSceneEnvArray(fs,5,AnimData.Rotation.z);
	// Scale
	WriteLWSceneEnvArray(fs,6,AnimData.Scale.x);
	WriteLWSceneEnvArray(fs,7,AnimData.Scale.y);
	WriteLWSceneEnvArray(fs,8,AnimData.Scale.z);
}

// Gather Functions
LWObject GatherM2forLWO(Attachment *att, Model *m, bool init, const char *fn, LWScene &scene, bool announce = true);
LWObject GatherWMOforLWO(WMO *m, const char *fn, LWScene &scene);

AnimVec3D animValue0 = AnimVec3D(AnimVector(0,0),AnimVector(0,0),AnimVector(0,0));
AnimVec3D animValue1 = AnimVec3D(AnimVector(1,0),AnimVector(1,0),AnimVector(1,0));

#endif