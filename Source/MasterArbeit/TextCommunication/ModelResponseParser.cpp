#include "ModelResponseParser.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "JsonObjectWrapper.h"
#include "Serialization/JsonSerializer.h"

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

bool UModelResponseParser::ParseVerbalInteractionFromJson(const FJsonObjectWrapper& JsonObjectWrapper,
                                                          FVerbalInteraction& OutVerbalInteraction,
                                                          FString& OutErrorMessage)
{
	const TSharedPtr<FJsonObject>* VerbalInteractionObject;
	if (!JsonObjectWrapper.JsonObject->TryGetObjectField(FString("verbal-interaction"), VerbalInteractionObject))
	{
		OutErrorMessage = "verbal-interaction field not found in JSON";
		return false;
	}

	FString ResponseTypeString;
	if ((*VerbalInteractionObject)->TryGetStringField(FString("response-type"), ResponseTypeString))
	{
		OutVerbalInteraction.ResponseType = StringToResponseType(ResponseTypeString);
	}
	else
	{
		OutErrorMessage = "response-type field not found or invalid";
		return false;
	}

	if (!(*VerbalInteractionObject)->TryGetStringField(FString("content"), OutVerbalInteraction.Content))
	{
		OutErrorMessage = "content field not found or invalid";
		return false;
	}

	if (OutVerbalInteraction.Content.Len() < 1000)
	{
		OutErrorMessage = FString::Printf(TEXT("content length is %d, but minimum required is 1000"),
		                                  OutVerbalInteraction.Content.Len());
	}

	FString EmotionalToneString;
	if ((*VerbalInteractionObject)->TryGetStringField(FString("emotional_tone"), EmotionalToneString))
	{
		OutVerbalInteraction.EmotionalTone = StringToEmotionalTone(EmotionalToneString);
	}
	else
	{
		OutErrorMessage = "emotional_tone field not found or invalid";
		return false;
	}

	if (OutVerbalInteraction.ResponseType == EVerbalResponseType::Invalid)
	{
		OutErrorMessage = "Invalid response-type value: " + ResponseTypeString;
		return false;
	}

	if (OutVerbalInteraction.EmotionalTone == EEmotionalTone::Invalid)
	{
		OutErrorMessage = "Invalid emotional_tone value: " + EmotionalToneString;
		return false;
	}

	OutErrorMessage = "Success";
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
