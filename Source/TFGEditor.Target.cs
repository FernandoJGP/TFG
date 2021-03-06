// Copyright (C) 2017 - Fernando Jos� Garc�a Padilla - Universidad de Sevilla

using UnrealBuildTool;
using System.Collections.Generic;

public class TFGEditorTarget : TargetRules
{
	public TFGEditorTarget(TargetInfo Target)
	{
		Type = TargetType.Editor;
	}

	//
	// TargetRules interface.
	//

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "TFG" } );
	}
}
