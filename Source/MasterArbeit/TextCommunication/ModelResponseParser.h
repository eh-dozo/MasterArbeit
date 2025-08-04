#pragma once
#include "CoreMinimal.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "ModelResponseParser.generated.h"

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
struct FVerbalInteraction
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Verbal Interaction")
	EVerbalResponseType ResponseType = EVerbalResponseType::Invalid;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Verbal Interaction")
	FString Content;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Verbal Interaction")
	EEmotionalTone EmotionalTone = EEmotionalTone::Invalid;

	FVerbalInteraction()
	{
		ResponseType = EVerbalResponseType::Invalid;
		Content = "";
		EmotionalTone = EEmotionalTone::Invalid;
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
	static bool ParseVerbalInteractionFromJson(const FJsonObjectWrapper& JsonObjectWrapper,
	                                           FVerbalInteraction& OutVerbalInteraction,
	                                           FString& OutErrorMessage);

private:
	static EVerbalResponseType StringToResponseType(const FString& TypeString);

	static EEmotionalTone StringToEmotionalTone(const FString& ToneString);
};
