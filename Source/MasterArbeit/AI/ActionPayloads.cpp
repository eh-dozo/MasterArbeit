#include "ActionPayloads.h"

UMovementActionWrapper* UMovementActionWrapper::CreateWrapper(UObject* Outer,
																		  const FMovementAction& Action)
{
	UMovementActionWrapper* Wrapper = NewObject<UMovementActionWrapper>(Outer);
	Wrapper->MovementAction = Action;
	return Wrapper;
}