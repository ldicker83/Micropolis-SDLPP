// This file is part of Micropolis-SDLPP
// Micropolis-SDLPP is based on Micropolis
//
// Copyright © 2022 - 2026 Leeor Dicker
//
// Portions Copyright © 1989-2007 Electronic Arts Inc.
//
// Micropolis-SDLPP is free software; you can redistribute it and/or modify
// it under the terms of the GNU GPLv3, with additional terms. See the README
// file, included in this distribution, for details.
#pragma once

#include <SDL3/SDL.h>
//#include <SDL3/SDL_syswm.h>

#include <string>

class FileIoDialog
{
public:
	FileIoDialog() = delete;
	FileIoDialog(const FileIoDialog&) = delete;
	const FileIoDialog& operator=(const FileIoDialog&) = delete;

	FileIoDialog(SDL_Window& window);
	~FileIoDialog();

	const std::string& savePath() const { return mSavePath; }
	const std::string& fileName() const { return mFileName; }
	const std::string fullPath() const { return mSavePath + mSeparator + mFileName; }

    void clearSaveFilename();
    
	bool pickSaveFile();
	bool pickOpenFile();

	bool filePicked() const;

private:
	enum class FileOperation { Open, Save };

	bool showFileDialog(FileOperation);
	void extractFileName();

	std::string mSavePath;
	std::string mFileName;
    std::string mSeparator;
};
