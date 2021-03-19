// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

/**
 * Interface for platform feature modules
 */

/** Defines the interface to platform's save game system (or a generic file based one) */
class ISaveGameSystem
{
public:

	// Possible result codes when using DoesSaveGameExist.
	// Not all codes are guaranteed to be returned on all platforms.
	enum class ESaveExistsResult
	{
		OK,						// Operation on the file completely successfully.
		DoesNotExist,			// Operation on the file failed, because the file was not found / does not exist.
		Corrupt,				// Operation on the file failed, because the file was corrupt.
		UnspecifiedError		// Operation on the file failed due to an unspecified error.
	};

	/** Returns true if the platform has a native UI (like many consoles) */
	virtual bool PlatformHasNativeUI() = 0;

	/** Return true if the named savegame exists (probably not useful with NativeUI */
	virtual bool DoesSaveGameExist(const TCHAR* Name, const int32 UserIndex) = 0;

	/** Similar to DoesSaveGameExist, except returns a result code with more information. */
	virtual ESaveExistsResult DoesSaveGameExistWithResult(const TCHAR* Name, const int32 UserIndex) = 0;

	/** Saves the game, blocking until complete. Platform may use FGameDelegates to get more information from the game */
	virtual bool SaveGame(bool bAttemptToUseUI, const TCHAR* Name, const int32 UserIndex, const TArray<uint8>& Data) = 0;

	/** Loads the game, blocking until complete */
	virtual bool LoadGame(bool bAttemptToUseUI, const TCHAR* Name, const int32 UserIndex, TArray<uint8>& Data) = 0;

	/** Delete an existing save game, blocking until complete */
	virtual bool DeleteGame(bool bAttemptToUseUI, const TCHAR* Name, const int32 UserIndex) = 0;


	// Engine Mod - Adam Rehberg Start
	/** Saves the game, blocking until complete. Platform may use FGameDelegates to get more information from the game. Mod uses custom path */
	bool SaveGameAtPath(bool bAttemptToUseUI, const TCHAR* Name, const TCHAR* Path, const int32 UserIndex, const TArray<uint8>& Data)
	{
		FString fullPath = FString::Printf(TEXT("%s%s.sav"), Path, Name);
		return FFileHelper::SaveArrayToFile(Data, *fullPath);
	};

	/** Return true if the named savegame exists (probably not useful with NativeUI */
	bool DoesSaveGameExistAtPath(const TCHAR* Name, const TCHAR* Path, const int32 UserIndex)
	{
		FString fullPath = FString::Printf(TEXT("%s%s.sav"), Path, Name);
		if (IFileManager::Get().FileSize(*fullPath) >= 0)
		{
			return true;
		}
		return false;
	};

	/** Loads the game, blocking until complete. Mod uses custom path.*/
	bool LoadGameFromPath(bool bAttemptToUseUI, const TCHAR* Name, const TCHAR* Path, const int32 UserIndex, TArray<uint8>& Data)
	{
		FString fullPath = FString::Printf(TEXT("%s%s.sav"), Path, Name);
		return FFileHelper::LoadFileToArray(Data, *fullPath);
	};

	/** Delete an existing save game, blocking until complete */
	bool DeleteGameAtPath(bool bAttemptToUseUI, const TCHAR* Name, const TCHAR* Path, const int32 UserIndex)
	{
		FString fullPath = FString::Printf(TEXT("%s%s.sav"), Path, Name);
		return IFileManager::Get().Delete(*fullPath, true, false, !bAttemptToUseUI);
	};

	// Engine Mod - Adam Rehberg End
};


/** A generic save game system that just uses IFileManager to save/load with normal files */
class FGenericSaveGameSystem : public ISaveGameSystem
{
public:
	virtual bool PlatformHasNativeUI() override
	{
		return false;
	}

	virtual ESaveExistsResult DoesSaveGameExistWithResult(const TCHAR* Name, const int32 UserIndex) override
	{
		if (IFileManager::Get().FileSize(*GetSaveGamePath(Name)) >= 0)
		{
			return ESaveExistsResult::OK;
		}
		return ESaveExistsResult::DoesNotExist;
	}

	virtual bool DoesSaveGameExist(const TCHAR* Name, const int32 UserIndex) override
	{
		return ESaveExistsResult::OK == DoesSaveGameExistWithResult(Name, UserIndex);
	}

	virtual bool SaveGame(bool bAttemptToUseUI, const TCHAR* Name, const int32 UserIndex, const TArray<uint8>& Data) override
	{
		return FFileHelper::SaveArrayToFile(Data, *GetSaveGamePath(Name));
	}

	virtual bool LoadGame(bool bAttemptToUseUI, const TCHAR* Name, const int32 UserIndex, TArray<uint8>& Data) override
	{
		return FFileHelper::LoadFileToArray(Data, *GetSaveGamePath(Name));
	}

	virtual bool DeleteGame(bool bAttemptToUseUI, const TCHAR* Name, const int32 UserIndex) override
	{
		return IFileManager::Get().Delete(*GetSaveGamePath(Name), true, false, !bAttemptToUseUI);
	}

protected:

	/** Get the path to save game file for the given name, a platform _may_ be able to simply override this and no other functions above */
	virtual FString GetSaveGamePath(const TCHAR* Name)
	{
		return FString::Printf(TEXT("%sSaveGames/%s.sav"), *FPaths::ProjectSavedDir(), Name);
	}
};
