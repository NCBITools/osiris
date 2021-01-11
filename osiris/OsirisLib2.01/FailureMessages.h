#pragma once
/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*  FileName: FailureMessages.h
*  Author:   Robert Goor
*
*/
//
//     class CoreBioComponent and other abstract base classes that represent samples and control sets of various kinds
//

#ifndef _FAILUREMESSAGES_H_
#define _FAILUREMESSAGES_H_

#include "rgstring.h"
#include "rgdlist.h"



class MainMessages {

public: 
	MainMessages () {}
	~MainMessages () {
		mMessages.ClearAndDelete ();
	}

	static void ResetPingValue (int p) { PingValue = p; }

	void SetPingValue (int p) { MainMessages::ResetPingValue (p); }

	void WriteAndResetCurrentPingValue () {
		cout << "<Ping>" << MainMessages::PingValue << "</Ping>\n";}

// Parameter input file/standard in failure
	void InputFileUnreadable (const RGString& fileName);
	void InputStringFormatError ();
	void PrematureFileEnd ();
	void IncompleteSetOfInputs ();

// Input file failure (lab settings standard settings, etc)
	void CouldNotOpenFile (const RGString& fileType);
	void FileInvalid (const RGString& fileType);
	void OutputFileCouldNotBeCreated (const RGString& fullPathName);
	void FsaHidFileInvalid (const RGString& fileName);

// Analysis failure
	void CouldNotInitialize ();
	void LadderBasedAnalysisFailed ();
	void FragmentAnalysisFailed ();

// MessageBook failures
	void MessageBookIsEmpty ();
	void CouldNotIdentifyMessageType ();
	void CouldNotFind (const RGString& msg);
	void GenericMessageBook (const RGString& msg);
	void CouldNotCompile ();
	void CouldNotInitializeMessageBook ();
	void CouldNotLoadMessageBook ();

// Ladder issues
	void LadderInfoInvalid ();
	void LocusSpecificThresholdsInvalid ();
	void CouldNotFindNamedMarkerSet (const RGString& name);
	void CouldNotFindLocusToSetThreshold ();

// Initialization issues
	void CouldNotRetrieveGenotypeCollection ();
	void CouldNotRetrieveGenotype (const RGString& name);
	void CouldNotFindSampleDirectory (const RGString& path);

// Ladder Analysis Issues
	void NoLadderDataFound ();
	void NoLaddersAnalyzedSuccessfully ();
	void LadderSpecificationMismatch (const RGString& msg);

// Message functions

	void AddMessage (const RGString& message);
	void DeleteAllMessages ();

	static RGString XML (const RGString& input);

protected:
	RGDList mMessages;

	void Header ();
	void Tail ();
	void ContinueTail ();
	void OutputAllMessages ();

	static int PingValue;
};



#endif  /*  _FAILUREMESSAGES_H_  */


