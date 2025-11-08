#pragma once
#include "CoreMinimal.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "ModelResponseParser.generated.h"

UENUM(BlueprintType)
enum class EMovementPrimitive : uint8
{
	Seek UMETA(DisplayName = "Seek"),
	Flee UMETA(DisplayName = "Flee"),
	Wander UMETA(DisplayName = "Wander"),
	Orbit UMETA(DisplayName = "Orbit"),
	AvoidObstacle UMETA(DisplayName = "AvoidObstacle"),
	Hide UMETA(DisplayName = "Hide"),
	Stay UMETA(DisplayName = "Stay"),
	Invalid UMETA(DisplayName = "Invalid")
};

//TODO: must become dynamic 
UENUM(BlueprintType)
enum class EMovementTarget : uint8
{
	Mercenary UMETA(DisplayName = "Mercenary"),
	Road UMETA(DisplayName = "Road"),
	Forest UMETA(DisplayName = "Forest"),
	Invalid UMETA(DisplayName = "Invalid")
};

UENUM(BlueprintType)
enum class EMovementDistance : uint8
{
	Adjacent UMETA(DisplayName = "Adjacent"),
	Near UMETA(DisplayName = "Near"),
	Moderate UMETA(DisplayName = "Moderate"),
	Far UMETA(DisplayName = "Far"),
	Invalid UMETA(DisplayName = "Invalid")
};

UENUM(BlueprintType)
enum class EStayOrientation : uint8
{
	Maintain UMETA(DisplayName = "Maintain"),
	Spin UMETA(DisplayName = "Spin"),
	Invalid UMETA(DisplayName = "Invalid")
};

UENUM(BlueprintType)
enum class EVerbalResponseType : uint8
{
	Speech UMETA(DisplayName = "Speech"),
	Silence UMETA(DisplayName = "Silence"),
	Invalid UMETA(DisplayName = "Invalid")
};

UENUM(BlueprintType)
enum class EEmotionalTone : uint8
{
	Enthusiasm UMETA(DisplayName = "Enthusiasm"),
	Conservatism UMETA(DisplayName = "Conservatism"),
	Boredom UMETA(DisplayName = "Boredom"),
	Antagonism UMETA(DisplayName = "Antagonism"),
	Pain UMETA(DisplayName = "Pain"),
	Anger UMETA(DisplayName = "Anger"),
	NoSympathy UMETA(DisplayName = "No Sympathy"),
	Fear UMETA(DisplayName = "Fear"),
	Sympathy UMETA(DisplayName = "Sympathy"),
	Grief UMETA(DisplayName = "Grief"),
	Apathy UMETA(DisplayName = "Apathy"),
	Invalid UMETA(DisplayName = "Invalid")
};

USTRUCT(BlueprintType)
struct FModelResponse
{
	GENERATED_BODY()

	// ---
	// Verbal Interaction

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Verbal Interaction")
	EVerbalResponseType ResponseType = EVerbalResponseType::Invalid;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Verbal Interaction")
	FString Content;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Verbal Interaction")
	EEmotionalTone EmotionalTone = EEmotionalTone::Invalid;

	// ---
	// Movement

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	EMovementPrimitive MovementPrimitive = EMovementPrimitive::Invalid;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	EMovementTarget MovementTarget = EMovementTarget::Invalid;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	EMovementDistance MovementDistance = EMovementDistance::Invalid;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement")
	EStayOrientation StayOrientation = EStayOrientation::Invalid;

	FModelResponse()
	{
		ResponseType = EVerbalResponseType::Invalid;
		Content = "";
		EmotionalTone = EEmotionalTone::Invalid;
		MovementPrimitive = EMovementPrimitive::Invalid;
		MovementTarget = EMovementTarget::Invalid;
		MovementDistance = EMovementDistance::Invalid;
		StayOrientation = EStayOrientation::Invalid;
	}
};

UCLASS(BlueprintType)
class MASTERARBEIT_API UModelResponseParser : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "JSON Parser")
	static bool ParseJsonFromString(const FString& JsonString,
	                                FJsonObjectWrapper& OutJsonObject,
	                                FString& OutErrorMessage);

	UFUNCTION(BlueprintCallable, Category = "JSON Parser")
	static bool ParseModelResponseFromJson(const FJsonObjectWrapper& JsonObjectWrapper,
	                                       FModelResponse& OutModelResponse,
	                                       FString& OutErrorMessage);

private:
	static EVerbalResponseType StringToResponseType(const FString& TypeString);
	static EEmotionalTone StringToEmotionalTone(const FString& ToneString);
	static EMovementPrimitive StringToMovementPrimitive(const FString& PrimitiveString);
	static EMovementTarget StringToMovementTarget(const FString& TargetString);
	static EMovementDistance StringToMovementDistance(const FString& DistanceString);
	static EStayOrientation StringToStayOrientation(const FString& OrientationString);
};
