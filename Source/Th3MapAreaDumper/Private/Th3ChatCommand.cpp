/* SPDX-License-Identifier: MPL-2.0 */

#include "Th3ChatCommand.h"
#include "FGMapArea.h"
#include "FGMapAreaTexture.h"
#include "FGMinimapCaptureActor.h"
#include "FGGameState.h"
#include "Misc/Base64.h"
#include "Components/SceneCaptureComponent2D.h"

DEFINE_LOG_CATEGORY_STATIC(LogTh3ChatCommand, Log, All);

namespace Th3
{
	template <typename T>
	FORCEINLINE FString GetPathSafe(const T* Obj, const FString& Fallback)
	{
		return Obj ? Obj->GetPathName() : FString::Printf(TEXT("NULL %s"), *Fallback);
	}

	template <typename T>
	FORCEINLINE FString GetPathSafe(const T* Obj)
	{
		return GetPathSafe(Obj, *T::StaticClass()->GetName());
	}

	template <typename T>
	FORCEINLINE FString GetPathSafe(const TSubclassOf<T> Class)
	{
		return GetPathSafe(Class.Get(), *T::StaticClass()->GetName());
	}
};

template<typename T>
static FORCEINLINE TSharedRef<FJsonValueArray> SerVecXY(const T& X, const T& Y)
{
	TArray<TSharedPtr<FJsonValue>> JVec;
	JVec.Add(MakeShared<FJsonValueNumber>(X));
	JVec.Add(MakeShared<FJsonValueNumber>(Y));
	return MakeShared<FJsonValueArray>(JVec);
}

static FORCEINLINE TSharedRef<FJsonValueArray> SerVec(const FVector2D& Vec)
{
	TArray<TSharedPtr<FJsonValue>> JVec;
	JVec.Add(MakeShared<FJsonValueNumber>(Vec.X));
	JVec.Add(MakeShared<FJsonValueNumber>(Vec.Y));
	return MakeShared<FJsonValueArray>(JVec);
}

static FORCEINLINE TSharedRef<FJsonValueArray> SerVec(const FVector& Vec)
{
	TArray<TSharedPtr<FJsonValue>> JVec;
	JVec.Add(MakeShared<FJsonValueNumber>(Vec.X));
	JVec.Add(MakeShared<FJsonValueNumber>(Vec.Y));
	JVec.Add(MakeShared<FJsonValueNumber>(Vec.Z));
	return MakeShared<FJsonValueArray>(JVec);
}

static FORCEINLINE TSharedRef<FJsonValueArray> SerRot(const FRotator& Rot)
{
	TArray<TSharedPtr<FJsonValue>> JRot;
	JRot.Add(MakeShared<FJsonValueNumber>(Rot.Roll));
	JRot.Add(MakeShared<FJsonValueNumber>(Rot.Pitch));
	JRot.Add(MakeShared<FJsonValueNumber>(Rot.Yaw));
	return MakeShared<FJsonValueArray>(JRot);
}

static FORCEINLINE TSharedRef<FJsonValueArray> SerQuat(const FQuat& Quat)
{
	TArray<TSharedPtr<FJsonValue>> JQuat;
	JQuat.Add(MakeShared<FJsonValueNumber>(Quat.W));
	JQuat.Add(MakeShared<FJsonValueNumber>(Quat.X));
	JQuat.Add(MakeShared<FJsonValueNumber>(Quat.Y));
	JQuat.Add(MakeShared<FJsonValueNumber>(Quat.Z));
	return MakeShared<FJsonValueArray>(JQuat);
}

EExecutionStatus ATh3ChatCommand::ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label)
{
	UE_LOG(LogTh3ChatCommand, Warning, TEXT("Dumping map area textures..."));
	
	const FString Timestamp = FDateTime::UtcNow().ToFormattedString(TEXT("%Y.%m.%d-%H.%M.%S"));

	TArray<TSharedPtr<FJsonValue>> J;

	for (TObjectIterator<UFGMapAreaTexture> Itr; Itr; ++Itr) {
		UFGMapAreaTexture* Mat = *Itr;
		UE_LOG(LogTh3ChatCommand, Warning, TEXT("Dumping %s"), *Th3::GetPathSafe(Mat));
		if (not Mat) {
			continue;
		}

		TArray<TSharedPtr<FJsonValue>> JColorPalette;
		TArray<TSharedPtr<FJsonValue>> JColorToArea;
		TSharedRef<FJsonObject> JMinimap = MakeShared<FJsonObject>();

		for (const FColor& Color : Mat->mColorPalette) {
			JColorPalette.Add(MakeShared<FJsonValueString>(Color.ToHex()));
		}
		for (const FColorMapAreaPair& Pair : Mat->mColorToArea) {
			TSharedRef<FJsonObject> JObj = MakeShared<FJsonObject>();

			JObj->Values.Add("MapArea", MakeShared<FJsonValueString>(Th3::GetPathSafe(Pair.MapArea)));
			JObj->Values.Add("Min", SerVecXY(Pair.MinX, Pair.MinY));
			JObj->Values.Add("Max", SerVecXY(Pair.MaxX, Pair.MaxY));

			JColorToArea.Add(MakeShared<FJsonValueObject>(JObj));
		}

		JMinimap->Values.Add("Path", MakeShared<FJsonValueString>(Th3::GetPathSafe(Mat->mCaptureActor)));
		if (AFGMinimapCaptureActor* CaptureActor = Mat->mCaptureActor) {
			JMinimap->Values.Add("Location", SerVec(CaptureActor->GetActorLocation()));
			JMinimap->Values.Add("Rotation", SerRot(CaptureActor->GetActorRotation()));
			JMinimap->Values.Add("Quat", SerQuat(CaptureActor->GetActorQuat()));
			JMinimap->Values.Add("VecFwd", SerVec(CaptureActor->GetActorForwardVector()));
			JMinimap->Values.Add("VecRight", SerVec(CaptureActor->GetActorRightVector()));
			JMinimap->Values.Add("VecUp", SerVec(CaptureActor->GetActorUpVector()));

			if (USceneCaptureComponent2D* CaptureComp = CaptureActor->GetCaptureComponent2D()) {
				TSharedRef<FJsonObject> JCapture = MakeShared<FJsonObject>();

				JCapture->Values.Add("Location", SerVec(CaptureComp->GetComponentLocation()));
				JCapture->Values.Add("RelativeLocation", SerVec(CaptureComp->GetRelativeLocation()));
				JCapture->Values.Add("Rotation", SerRot(CaptureComp->GetComponentRotation()));
				JCapture->Values.Add("RelativeRotation", SerRot(CaptureComp->GetRelativeRotation()));
				JCapture->Values.Add("Quat", SerQuat(CaptureComp->GetComponentQuat()));
				JCapture->Values.Add("VecFwd", SerVec(CaptureComp->GetForwardVector()));
				JCapture->Values.Add("VecRight", SerVec(CaptureComp->GetRightVector()));
				JCapture->Values.Add("VecUp", SerVec(CaptureComp->GetUpVector()));

				JMinimap->Values.Add("Component", MakeShared<FJsonValueObject>(JCapture));
			}
		}

		TSharedPtr<FJsonObject> JMat = MakeShared<FJsonObject>();
		JMat->Values.Add("mFogOfWarTexture", MakeShared<FJsonValueString>(Th3::GetPathSafe(Mat->mFogOfWarTexture)));
		JMat->Values.Add("mAreaData", MakeShared<FJsonValueString>(FBase64::Encode(Mat->mAreaData)));
		JMat->Values.Add("mColorPalette", MakeShared<FJsonValueArray>(JColorPalette));
		JMat->Values.Add("mColorToArea", MakeShared<FJsonValueArray>(JColorToArea));
		JMat->Values.Add("mCaptureActor", MakeShared<FJsonValueObject>(JMinimap));
		JMat->Values.Add("mDataWidth", MakeShared<FJsonValueNumber>(Mat->mDataWidth));
		JMat->Values.Add("mUpperLeftWorld", SerVec(Mat->mUpperLeftWorld));
		JMat->Values.Add("mWorldToMapScale", MakeShared<FJsonValueNumber>(Mat->mWorldToMapScale));
		JMat->Values.Add("mCachedGameState", MakeShared<FJsonValueString>(Th3::GetPathSafe(Mat->mCachedGameState)));

		J.Add(MakeShared<FJsonValueObject>(JMat));
	}

	FString Data;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Data);
	FJsonSerializer::Serialize(J, JsonWriter);

	FFileHelper::SaveStringToFile(Data, *FString::Printf(TEXT("%s/MapAreaTextures/DUMP_%s.json"), *FPaths::ProjectSavedDir(), *Timestamp));

	return EExecutionStatus::COMPLETED;
}
