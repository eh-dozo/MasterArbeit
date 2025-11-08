#include "ModelResponseParser.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "JsonObjectWrapper.h"
#include "Serialization/JsonSerializer.h"
#include "LlamaCppSubsystem.h"

bool UModelResponseParser::ParseJsonFromString(const FString& JsonString,
									FJsonObjectWrapper& OutJsonObject,
									FString& OutErrorMessage)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, OutJsonObject.JsonObject) || !OutJsonObject.JsonObject.IsValid())
	{
		OutErrorMessage = "Failed to parse JSON string";
		return false;
	}

	return true;
}

EVerbalResponseType UModelResponseParser::StringToResponseType(const FString& TypeString)
{
	static const TMap<FString, EVerbalResponseType> ResponseTypeMap = {
		{TEXT("speech"), EVerbalResponseType::Speech},
		{TEXT("silence"), EVerbalResponseType::Silence}
	};

	const EVerbalResponseType* Found = ResponseTypeMap.Find(TypeString.ToLower());
	return Found ? *Found : EVerbalResponseType::Invalid;
}

EEmotionalTone UModelResponseParser::StringToEmotionalTone(const FString& ToneString)
{
	static const TMap<FString, EEmotionalTone> EmotionalToneMap = {
		{TEXT("enthusiasm"), EEmotionalTone::Enthusiasm},
		{TEXT("conservatism"), EEmotionalTone::Conservatism},
		{TEXT("boredom"), EEmotionalTone::Boredom},
		{TEXT("antagonism"), EEmotionalTone::Antagonism},
		{TEXT("pain"), EEmotionalTone::Pain},
		{TEXT("anger"), EEmotionalTone::Anger},
		{TEXT("no-sympathy"), EEmotionalTone::NoSympathy},
		{TEXT("fear"), EEmotionalTone::Fear},
		{TEXT("sympathy"), EEmotionalTone::Sympathy},
		{TEXT("grief"), EEmotionalTone::Grief},
		{TEXT("apathy"), EEmotionalTone::Apathy}
	};

	const EEmotionalTone* Found = EmotionalToneMap.Find(ToneString.ToLower());
	return Found ? *Found : EEmotionalTone::Invalid;
}

EMovementPrimitive UModelResponseParser::StringToMovementPrimitive(const FString& PrimitiveString)
{
	static const TMap<FString, EMovementPrimitive> PrimitiveMap = {
		{TEXT("seek"), EMovementPrimitive::Seek},
		{TEXT("flee"), EMovementPrimitive::Flee},
		{TEXT("wander"), EMovementPrimitive::Wander},
		{TEXT("orbit"), EMovementPrimitive::Orbit},
		{TEXT("avoid-obstacles"), EMovementPrimitive::AvoidObstacle},
		{TEXT("hide"), EMovementPrimitive::Hide},
		{TEXT("stay"), EMovementPrimitive::Stay}
	};

	const EMovementPrimitive* Found = PrimitiveMap.Find(PrimitiveString.ToLower());
	return Found ? *Found : EMovementPrimitive::Invalid;
}

EMovementTarget UModelResponseParser::StringToMovementTarget(const FString& TargetString)
{
	static const TMap<FString, EMovementTarget> TargetMap = {
		{TEXT("mercenary"), EMovementTarget::Mercenary},
		{TEXT("road"), EMovementTarget::Road},
		{TEXT("forest"), EMovementTarget::Forest}
	};

	const EMovementTarget* Found = TargetMap.Find(TargetString.ToLower());
	return Found ? *Found : EMovementTarget::Invalid;
}

EMovementDistance UModelResponseParser::StringToMovementDistance(const FString& DistanceString)
{
	static const TMap<FString, EMovementDistance> DistanceMap = {
		{TEXT("adjacent"), EMovementDistance::Adjacent},
		{TEXT("near"), EMovementDistance::Near},
		{TEXT("moderate"), EMovementDistance::Moderate},
		{TEXT("far"), EMovementDistance::Far}
	};

	const EMovementDistance* Found = DistanceMap.Find(DistanceString.ToLower());
	return Found ? *Found : EMovementDistance::Invalid;
}

EStayOrientation UModelResponseParser::StringToStayOrientation(const FString& OrientationString)
{
	static const TMap<FString, EStayOrientation> OrientationMap = {
		{TEXT("maintain"), EStayOrientation::Maintain},
		{TEXT("spin"), EStayOrientation::Spin}
	};

	const EStayOrientation* Found = OrientationMap.Find(OrientationString.ToLower());
	return Found ? *Found : EStayOrientation::Invalid;
}

bool UModelResponseParser::ParseModelResponseFromJson(const FJsonObjectWrapper& JsonObjectWrapper,
                                                       FModelResponse& OutModelResponse,
                                                       FString& OutErrorMessage)
{
	// --- LOGS
	FString JsonObjectAsString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonObjectAsString);
	FJsonSerializer::Serialize(JsonObjectWrapper.JsonObject.ToSharedRef(), Writer);
	UE_LOG(LogLlamaRunner, Display, TEXT("%s"), *JsonObjectAsString);
	// --- END LOGS

	if (!JsonObjectWrapper.JsonObject.IsValid())
	{
		OutErrorMessage = "Invalid JSON object";
		UE_LOG(LogLlamaRunner, Error, TEXT("Parsing failed: %s"), *OutErrorMessage);
		return false;
	}

	const TSharedPtr<FJsonObject>* VerbalInteractionObject;
	if (!JsonObjectWrapper.JsonObject->TryGetObjectField(TEXT("verbal-interaction"), VerbalInteractionObject))
	{
		OutErrorMessage = "verbal-interaction field not found";
		return false;
	}

	FString ResponseTypeString;
	if (!(*VerbalInteractionObject)->TryGetStringField(TEXT("response-type"), ResponseTypeString))
	{
		OutErrorMessage = "response-type field not found";
		return false;
	}
	OutModelResponse.ResponseType = StringToResponseType(ResponseTypeString);
	if (OutModelResponse.ResponseType == EVerbalResponseType::Invalid)
	{
		OutErrorMessage = "Invalid response-type value: " + ResponseTypeString;
		return false;
	}

	if (!(*VerbalInteractionObject)->TryGetStringField(TEXT("content"), OutModelResponse.Content))
	{
		OutErrorMessage = "content field not found";
		return false;
	}

	FString EmotionalToneString;
	if (!(*VerbalInteractionObject)->TryGetStringField(TEXT("emotional_tone"), EmotionalToneString))
	{
		OutErrorMessage = "emotional_tone field not found";
		return false;
	}
	OutModelResponse.EmotionalTone = StringToEmotionalTone(EmotionalToneString);
	if (OutModelResponse.EmotionalTone == EEmotionalTone::Invalid)
	{
		OutErrorMessage = "Invalid emotional_tone value: " + EmotionalToneString;
		return false;
	}

	const TSharedPtr<FJsonObject>* ActionsTakenObject;
	if (!JsonObjectWrapper.JsonObject->TryGetObjectField(TEXT("actions-taken"), ActionsTakenObject))
	{
		OutErrorMessage = "actions-taken field not found";
		return false;
	}

	const TSharedPtr<FJsonObject>* MovementActionObject;
	if ((*ActionsTakenObject)->TryGetObjectField(TEXT("movement-action"), MovementActionObject))
	{
		FString PrimitiveString;
		if (!(*MovementActionObject)->TryGetStringField(TEXT("primitive"), PrimitiveString))
		{
			OutErrorMessage = "primitive field not found in movement-action";
			return false;
		}
		OutModelResponse.MovementPrimitive = StringToMovementPrimitive(PrimitiveString);
		if (OutModelResponse.MovementPrimitive == EMovementPrimitive::Invalid)
		{
			OutErrorMessage = "Invalid primitive value: " + PrimitiveString;
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* ParametersArray;
		if ((*MovementActionObject)->TryGetArrayField(TEXT("parameters"), ParametersArray) && ParametersArray->Num() > 0)
		{
			const TSharedPtr<FJsonObject>* ParametersObject;
			if (!(*ParametersArray)[0]->TryGetObject(ParametersObject))
			{
				OutErrorMessage = "Invalid parameters array format";
				return false;
			}

			FString StayOrientationString;
			if ((*ParametersObject)->TryGetStringField(TEXT("stay-orientation"), StayOrientationString))
			{
				OutModelResponse.StayOrientation = StringToStayOrientation(StayOrientationString);
				if (OutModelResponse.StayOrientation == EStayOrientation::Invalid)
				{
					OutErrorMessage = "Invalid stay-orientation value: " + StayOrientationString;
					return false;
				}
			}
			else
			{
				FString TargetString;
				if ((*ParametersObject)->TryGetStringField(TEXT("target"), TargetString))
				{
					OutModelResponse.MovementTarget = StringToMovementTarget(TargetString);
					if (OutModelResponse.MovementTarget == EMovementTarget::Invalid)
					{
						OutErrorMessage = "Invalid target value: " + TargetString;
						return false;
					}
				}

				FString DistanceString;
				if ((*ParametersObject)->TryGetStringField(TEXT("desired-distance"), DistanceString))
				{
					OutModelResponse.MovementDistance = StringToMovementDistance(DistanceString);
					if (OutModelResponse.MovementDistance == EMovementDistance::Invalid)
					{
						OutErrorMessage = "Invalid desired-distance value: " + DistanceString;
						return false;
					}
				}
			}
		}
	}

	OutErrorMessage = "Success";
	return true;
}
