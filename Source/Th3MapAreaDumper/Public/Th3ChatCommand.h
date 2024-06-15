/* SPDX-License-Identifier: MPL-2.0 */

#pragma once

#include "CoreMinimal.h"
#include "Command/ChatCommandInstance.h"
#include "Th3ChatCommand.generated.h"

UCLASS(Abstract)
class TH3MAPAREADUMPER_API ATh3ChatCommand : public AChatCommandInstance
{
	GENERATED_BODY()

public:
	EExecutionStatus ExecuteCommand_Implementation(UCommandSender* Sender, const TArray<FString>& Arguments, const FString& Label);
};
