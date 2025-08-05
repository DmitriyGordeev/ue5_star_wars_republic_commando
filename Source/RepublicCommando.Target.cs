// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class RepublicCommandoTarget : TargetRules
{
	public RepublicCommandoTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		// DefaultBuildSettings = BuildSettingsVersion.V2;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("RepublicCommando");
	}
}
