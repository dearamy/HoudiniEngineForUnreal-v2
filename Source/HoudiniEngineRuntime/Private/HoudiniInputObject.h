/*
* Copyright (c) <2018> Side Effects Software Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. The name of Side Effects Software may not be used to endorse or
*    promote products derived from this software without specific prior
*    written permission.
*
* THIS SOFTWARE IS PROVIDED BY SIDE EFFECTS SOFTWARE "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
* NO EVENT SHALL SIDE EFFECTS SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <functional>

#include "HoudiniSplineComponent.h"
#include "HoudiniGeoPartObject.h"

#include "CoreTypes.h"
#include "Materials/MaterialInterface.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

#include "Engine/Brush.h"
#include "Engine/Polys.h"
#include "UObject/SoftObjectPtr.h"

#include "HoudiniInputObject.generated.h"

class UStaticMesh;
class USkeletalMesh;
class USceneComponent;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class USplineComponent;
class UHoudiniAssetComponent;
class AActor;
class ALandscapeProxy;
class ABrush;
class UHoudiniInput;
class ALandscapeProxy;
class UModel;
class UHoudiniInput;
class UCameraComponent;

UENUM()
enum class EHoudiniInputObjectType : uint8
{
	Invalid,

	Object,
	StaticMesh,
	SkeletalMesh,
	SceneComponent,
	StaticMeshComponent,
	InstancedStaticMeshComponent,
	SplineComponent,
	HoudiniSplineComponent,
	HoudiniAssetComponent,
	Actor,
	Landscape,
	Brush,
	CameraComponent,
	DataTable,
	HoudiniAssetActor,
};

//-----------------------------------------------------------------------------------------------------------------------------
// UObjects input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	// Create the proper input object
	static UHoudiniInputObject * CreateTypedInputObject(UObject * InObject, UObject* InOuter, const FString& InParamName);

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	// Check whether two input objects match
	virtual bool Matches(const UHoudiniInputObject& Other) const;

	//
	static EHoudiniInputObjectType GetInputObjectTypeFromObject(UObject* InObject);

	//
	virtual void Update(UObject * InObject);

	// Invalidate and ask for the deletion of this input object's node
	virtual void InvalidateData();

	// UObject accessor
	virtual UObject* GetObject() const;

	// Indicates if this input has changed and should be updated
	virtual bool HasChanged() const { return bHasChanged; };

	// Indicates if this input has changed and should be updated
	virtual bool HasTransformChanged() const { return bTransformChanged; };

	// Indicates if this input needs to trigger an update
	virtual bool NeedsToTriggerUpdate() const { return bNeedsToTriggerUpdate; };

	virtual void MarkChanged(const bool& bInChanged) { bHasChanged = bInChanged; SetNeedsToTriggerUpdate(bInChanged); };
	void MarkTransformChanged(const bool& bInChanged) { bTransformChanged = bInChanged; SetNeedsToTriggerUpdate(bInChanged); };
	virtual void SetNeedsToTriggerUpdate(const bool& bInTriggersUpdate) { bNeedsToTriggerUpdate = bInTriggersUpdate; };

	void SetImportAsReference(const bool& bInImportAsRef) { bImportAsReference = bInImportAsRef; };
	bool GetImportAsReference() const { return bImportAsReference; };

#if WITH_EDITOR
	void SwitchUniformScaleLock() { bUniformScaleLocked = !bUniformScaleLocked; };
	bool IsUniformScaleLocked() const { return bUniformScaleLocked; };

	void PostEditUndo() override;
#endif

	virtual UHoudiniInputObject* DuplicateAndCopyState(UObject* DestOuter);
	virtual void CopyStateFrom(UHoudiniInputObject* InInput, bool bCopyAllProperties);

	// Set whether this object can delete Houdini nodes.
	virtual void SetCanDeleteHoudiniNodes(bool bInCanDeleteNodes);
	bool CanDeleteHoudiniNodes() const { return bCanDeleteHoudiniNodes; }

	FGuid GetInputGuid() const { return Guid; }


protected:

	virtual void BeginDestroy() override;

public:

	// The object referenced by this input
	// This property should be protected. Don't access this directly. Use GetObject() / Update() instead.
	UPROPERTY()
	TSoftObjectPtr<UObject> InputObject;

	// The object's transform/transform offset
	UPROPERTY()
	FTransform Transform;

	// The type of Object this input refers to
	UPROPERTY()
	EHoudiniInputObjectType Type;

	// This input object's "main" (SOP) NodeId
	UPROPERTY(Transient, DuplicateTransient, NonTransactional)
	int32 InputNodeId;

	// This input object's "container" (OBJ) NodeId
	UPROPERTY(Transient, DuplicateTransient, NonTransactional)
	int32 InputObjectNodeId;

	// Guid that uniquely identifies this input object.
	// Also useful to correlate inputs between blueprint component templates and instances.
	UPROPERTY(DuplicateTransient)
	FGuid Guid;

protected:

	// Indicates this input object has changed
	UPROPERTY(DuplicateTransient)
	bool bHasChanged;

	// Indicates this input object should trigger an input update/cook
	UPROPERTY(DuplicateTransient)
	bool bNeedsToTriggerUpdate;

	// Indicates that this input transform should be updated
	UPROPERTY(DuplicateTransient)
	bool bTransformChanged;

	UPROPERTY()
	bool bImportAsReference;

	// Indicates if change the scale of Transfrom Offset of this object uniformly
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient, DuplicateTransient, NonTransactional)
	bool bUniformScaleLocked;
#endif

	UPROPERTY()
	bool bCanDeleteHoudiniNodes;
};


//-----------------------------------------------------------------------------------------------------------------------------
// UStaticMesh input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputStaticMesh : public UHoudiniInputObject
{
	GENERATED_UCLASS_BODY()
		
public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	// UHoudiniInputObject overrides
	
	// virtual void DuplicateAndCopyState(UObject* DestOuter, UHoudiniInputObject*& OutNewObject) override;
	virtual void CopyStateFrom(UHoudiniInputObject* InInput, bool bCopyAllProperties) override;
	virtual void SetCanDeleteHoudiniNodes(bool bInCanDeleteNodes) override;
	virtual void InvalidateData() override;

	//
	virtual void Update(UObject * InObject) override;

	// Nothing to add for Static Meshes?

	// StaticMesh accessor
	class UStaticMesh* GetStaticMesh();

	// Blueprint accessor
	class UBlueprint* GetBlueprint();

	// Check if this SM Input object is passed in as a BP
	bool bIsBlueprint() const;

	// The Blueprint's Static Meshe Components that can be sent as inputs
	UPROPERTY()
	TArray<UHoudiniInputStaticMesh*> BlueprintStaticMeshes;
};



//-----------------------------------------------------------------------------------------------------------------------------
// USkeletalMesh input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputSkeletalMesh : public UHoudiniInputObject
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	// Nothing to add for SkeletalMesh Meshes?

	// StaticMesh accessor
	class USkeletalMesh* GetSkeletalMesh();
};



//-----------------------------------------------------------------------------------------------------------------------------
// USceneComponent input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputSceneComponent : public UHoudiniInputObject
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;
	
	// SceneComponent accessor
	class USceneComponent* GetSceneComponent();

	// Returns true if the attached actor's (parent) transform has been modified
	virtual bool HasActorTransformChanged() const;

	// Returns true if the attached component's transform has been modified
	virtual bool HasComponentTransformChanged() const;

	// Return true if the component itself has been modified
	virtual bool HasComponentChanged() const;

public:

	// This component's parent Actor transform
	UPROPERTY()
	FTransform ActorTransform = FTransform::Identity;
};



//-----------------------------------------------------------------------------------------------------------------------------
// UStaticMeshComponent input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputMeshComponent : public UHoudiniInputSceneComponent
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	// StaticMeshComponent accessor
	UStaticMeshComponent* GetStaticMeshComponent();

	// Get the referenced StaticMesh
	UStaticMesh* GetStaticMesh();

	// Returns true if the attached component's materials have been modified
	bool HasComponentMaterialsChanged() const;

	// Return true if SMC's static mesh has been modified
	virtual bool HasComponentChanged() const override;

public:

	// Keep track of the selected Static Mesh
	UPROPERTY()
	TSoftObjectPtr<class UStaticMesh> StaticMesh = nullptr;

	// Path to the materials assigned on the SMC
	UPROPERTY()
	TArray<FString> MeshComponentsMaterials;
};




//-----------------------------------------------------------------------------------------------------------------------------
// UInstancedStaticMeshComponent input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputInstancedMeshComponent : public UHoudiniInputMeshComponent
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	// InstancedStaticMeshComponent accessor
	UInstancedStaticMeshComponent* GetInstancedStaticMeshComponent();

	// Returns true if the instances have changed
	bool HasInstancesChanged() const;

	// Returns true if the attached component's transform has been modified
	virtual bool HasComponentTransformChanged() const override;
	
public:

	// Array of transform for this ISMC's instances
	UPROPERTY()
	TArray<FTransform> InstanceTransforms;	
};




//-----------------------------------------------------------------------------------------------------------------------------
// USplineComponent input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputSplineComponent : public UHoudiniInputSceneComponent
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	// USplineComponent accessor
	USplineComponent* GetSplineComponent();

	// Returns true if the attached spline component has been modified
	bool HasSplineComponentChanged(float fCurrentSplineResolution) const;

	// Returns true if the attached actor's (parent) transform has been modified
	virtual bool HasActorTransformChanged() const;

	// Returns true if the attached component's transform has been modified
	virtual bool HasComponentTransformChanged() const;

	// Return true if the component itself has been modified
	virtual bool HasComponentChanged() const;

public:

	// Number of CVs used by the spline component, used to detect modification
	UPROPERTY()
	int32 NumberOfSplineControlPoints = -1;

	// Spline Length, used for fast detection of modifications of the spline..
	UPROPERTY()
	float SplineLength = -1.0f;

	// Spline resolution used to generate the asset, used to detect setting modification
	UPROPERTY()
	float SplineResolution = -1.0f;

	// Is the spline closed?
	UPROPERTY()
	bool SplineClosed = false;

	// Transforms of each of the spline's control points
	UPROPERTY()
	TArray<FTransform> SplineControlPoints;
};



//-----------------------------------------------------------------------------------------------------------------------------
// UHoudiniSplineComponent input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputHoudiniSplineComponent : public UHoudiniInputObject
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	virtual void Update(UObject * InObject) override;

	virtual UObject* GetObject() const override;

	virtual void MarkChanged(const bool& bInChanged) override;

	virtual void SetNeedsToTriggerUpdate(const bool& bInTriggersUpdate) override;

	// Indicates if this input has changed and should be updated
	virtual bool HasChanged() const override;

	// Indicates if this input needs to trigger an update
	virtual bool NeedsToTriggerUpdate() const override;

	// UHoudiniSplineComponent accessor
	UHoudiniSplineComponent* GetCurveComponent() const;

public:

	// The type of curve (polygon, NURBS, bezier)
	UPROPERTY()
	EHoudiniCurveType CurveType = EHoudiniCurveType::Polygon;

	// The curve's method (CVs, Breakpoint, Freehand)
	UPROPERTY()
	EHoudiniCurveMethod CurveMethod = EHoudiniCurveMethod::CVs;

	UPROPERTY()
	bool Reversed = false;
	// [AMY]
	virtual void CopyStateFrom(UHoudiniInputObject* InInput, bool bCopyAllProperties) override;

protected:
	
	// NOTE: We are using this reference to the component since the component, for now,
	// lives on the same actor as this input object. If we use a Soft Object Reference instead the editor
	// will complain about breaking references everytime we try to delete the actor.
	UPROPERTY(Instanced)
	UHoudiniSplineComponent* CachedComponent;
};



//-----------------------------------------------------------------------------------------------------------------------------
// UCameraComponent input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputCameraComponent : public UHoudiniInputSceneComponent
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	// UCameraComponent accessor
	UCameraComponent* GetCameraComponent();

	// Return true if SMC's static mesh has been modified
	virtual bool HasComponentChanged() const override;

public:

	float FOV;
	float AspectRatio;

	//TEnumAsByte<ECameraProjectionMode::Type> ProjectionType;
	bool bIsOrthographic;
	
	float OrthoWidth;
	float OrthoNearClipPlane;
	float OrthoFarClipPlane;
	
};


//-----------------------------------------------------------------------------------------------------------------------------
// UHoudiniAssetComponent input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputHoudiniAsset : public UHoudiniInputObject
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	// UHoudiniAssetComponent accessor
	UHoudiniAssetComponent* GetHoudiniAssetComponent();
public:

	// The output index of the node that we want to use as input
	UPROPERTY()
	int32 AssetOutputIndex;
};



//-----------------------------------------------------------------------------------------------------------------------------
// AActor input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputActor : public UHoudiniInputObject
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	// 
	virtual bool HasActorTransformChanged();

	// Return true if any content of this actor has possibly changed (for example geometry edits on a 
	// Brush or changes on procedurally generated content).
	// NOTE: This is more generally applicable and could be moved to the HoudiniInputObject class.
	virtual bool HasContentChanged() const;

	// AActor accessor
	AActor* GetActor();

public:

	// The actor's components that can be sent as inputs
	UPROPERTY()
	TArray<UHoudiniInputSceneComponent*> ActorComponents;
};



//-----------------------------------------------------------------------------------------------------------------------------
// ALandscapeProxy input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputLandscape : public UHoudiniInputActor
{
	GENERATED_UCLASS_BODY()

public:

	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	//
	virtual void Update(UObject * InObject) override;

	virtual bool HasActorTransformChanged() override;

	// ALandscapeProxy accessor
	ALandscapeProxy* GetLandscapeProxy();

	void SetLandscapeProxy(UObject* InLandscapeProxy);

	// Used to restore an input landscape's transform to its original state
	UPROPERTY()
	FTransform CachedInputLandscapeTraqnsform;
};



//-----------------------------------------------------------------------------------------------------------------------------
// ABrush input
//-----------------------------------------------------------------------------------------------------------------------------
// Cache info for a brush in order to determine whether it has changed.

#define BRUSH_HASH_SURFACE_PROPERTIES 0

//USTRUCT()
//struct FHoudiniBrushSurfaceInfo {
//	GENERATED_BODY()
//
//	FVector Base;
//	FVector Normal;
//	FVector TextureU;
//	FVector TextureV;
//	TSoftObjectPtr<UMaterialInterface> Material;
//
//	FHoudiniBrushSurfaceInfo(const FVector& InBase, const FVector& InNormal, const FVector& InTextureU, const FVector& InTextureV, UMaterialInterface* InMaterial)
//		: Base(InBase)
//		, Normal(InNormal)
//		, TextureU(InTextureU)
//		, TextureV(InTextureV)
//		, Material(InMaterial)
//	{ }
//
//	inline bool operator==(const FHoudiniBrushSurfaceInfo& Other) {
//		return Base.Equals(Other.Base)
//			&& Normal.Equals(Other.Normal)
//			&& TextureU.Equals(Other.TextureU)
//			&& TextureV.Equals(Other.TextureV)
//			&& Material.Get() == Other.Material.Get();
//	}
//
//	inline bool operator==(const FPoly& Poly) {
//		return Base.Equals(Poly.Base)
//			&& Normal.Equals(Poly.Normal)
//			&& TextureU.Equals(Poly.TextureU)
//			&& TextureV.Equals(Poly.TextureV)
//			&& Material.Get() == Poly.Material;
//	}
//};

USTRUCT()
struct FHoudiniBrushInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<ABrush> BrushActor;
	UPROPERTY()
	FTransform CachedTransform;
	UPROPERTY()
	FVector CachedOrigin;
	UPROPERTY()
	FVector CachedExtent;
	UPROPERTY()
	TEnumAsByte<EBrushType> CachedBrushType;
	
	UPROPERTY()
	uint64 CachedSurfaceHash;

	bool HasChanged() const;

	static int32 GetNumVertexIndicesFromModel(const UModel* Model);

	FHoudiniBrushInfo();
	FHoudiniBrushInfo(ABrush* InBrushActor);

	template <class T>
	inline void HashCombine(uint64& s, const T & v) const
	{
	  std::hash<T> h;
	  s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
	}

	inline void HashCombine(uint64& s, const FVector & V) const
	{
		HashCombine(s, V.X);
		HashCombine(s, V.Y);
		HashCombine(s, V.Z);
	}

	inline void CombinePolyHash(uint64& Hash, const FPoly& Poly) const
	{
		HashCombine(Hash, Poly.Base);
		HashCombine(Hash, Poly.TextureU);
		HashCombine(Hash, Poly.TextureV);
		HashCombine(Hash, Poly.Normal);
		HashCombine(Hash, (uint64)(Poly.Material));
	}
};

UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputBrush : public UHoudiniInputActor
{
	GENERATED_BODY()

public:

	UHoudiniInputBrush();

	// Factory function
	static UHoudiniInputBrush* Create(UObject* InObject, UObject* InOuter, const FString& InName);

	//----------------------------------------------------------------------
	// UHoudiniInputActor Interface - Begin
	//----------------------------------------------------------------------

	virtual void Update(UObject * InObject) override;

	// Indicates if this input has changed and should be updated
	virtual bool HasContentChanged() const override;

	// Indicates if this input has changed and should be updated
	virtual bool HasChanged() const override { return (!bIgnoreInputObject) && bHasChanged; };

	// Indicates if this input has changed and should be updated
	virtual bool HasTransformChanged() const override { return (!bIgnoreInputObject) && bTransformChanged; };

	virtual bool HasActorTransformChanged() override;

	virtual bool NeedsToTriggerUpdate() const override { return (!bIgnoreInputObject) && bNeedsToTriggerUpdate; };

	//----------------------------------------------------------------------
	// UHoudiniInputActor Interface - End
	//----------------------------------------------------------------------


	// ABrush accessor
	ABrush* GetBrush() const;

	UModel* GetCachedModel() const;

	// Check whether any of the brushes, or their transforms, used to generate this model have changed.
	bool HasBrushesChanged(const TArray<ABrush*>& InBrushes) const;

	// Cache the combined model as well as the input brushes.
	void UpdateCachedData(UModel* InCombinedModel, const TArray<ABrush*>& InBrushes);

	// Returns whether this input object should be ignored when uploading objects to Houdini.
	// This mechanism could be implemented on UHoudiniInputObject.
	bool ShouldIgnoreThisInput();


	// Find only the subtractive brush actors that intersect with the InputObject (Brush actor) bounding box but
	// excluding any selector bounds actors.
	static bool FindIntersectingSubtractiveBrushes(const UHoudiniInputBrush* InputBrush, TArray<ABrush*>& OutBrushes);

protected:
	UPROPERTY()
	TArray<FHoudiniBrushInfo> BrushesInfo;
	
	UPROPERTY(Transient, DuplicateTransient)
	UModel* CombinedModel;

	UPROPERTY()
	bool bIgnoreInputObject;

	UPROPERTY()
	TEnumAsByte<EBrushType> CachedInputBrushType;
};


//-----------------------------------------------------------------------------------------------------------------------------
// UDataTable input
//-----------------------------------------------------------------------------------------------------------------------------
UCLASS()
class HOUDINIENGINERUNTIME_API UHoudiniInputDataTable : public UHoudiniInputObject
{
	GENERATED_UCLASS_BODY()

public:
	
	//
	static UHoudiniInputObject* Create(UObject * InObject, UObject* InOuter, const FString& InName);

	// DataTable accessor
	class UDataTable* GetDataTable() const;
};