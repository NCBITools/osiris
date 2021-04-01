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
*  FileName: CoreBioComponentSM.cpp
*  Author:   Robert Goor
*
*/
//
//     class CoreBioComponent and other abstract base classes that represent samples and control sets of various kinds:  SmartMessage
//     functions only
//

#include "CoreBioComponent.h"
#include "rgfile.h"
#include "rgvstream.h"
#include "DataSignal.h"
#include "SampleData.h"
#include "ChannelData.h"
#include "rgtokenizer.h"
#include "GenotypeSpecs.h"
#include "Notices.h"
#include "IndividualGenotype.h"
#include "ParameterServer.h"
#include "ListFunctions.h"
#include "xmlwriter.h"
#include "SmartMessage.h"
#include "SmartNotice.h"
#include "STRSmartNotices.h"
#include "TracePrequalification.h"
#include "DirectoryManager.h"
#include "LeastMedianOfSquares.h"
#include "STRLCAnalysis.h"
#include "ModPairs.h"


// Smart Message Functions**************************************************************************************************************
//**************************************************************************************************************************************


int CoreBioComponent :: OrganizeNoticeObjectsSM () {

	//
	//  This is ladder and sample stage 5
	//

	if (Progress < 4)
		return 0;

	for (int j=1; j<=mNumberOfChannels; j++)
		mDataChannels [j]->TestArtifactListForNoticesWithinLaneStandardSM (mLSData, mAssociatedGrid);

	return 0;
}


int CoreBioComponent :: TestSignalsForLaserOffScaleSM () {

	//
	//	Sample stage 1  (Ladder?)
	//

	int ans = 0;
	int temp;

	for (int j=1; j<=mNumberOfChannels; j++) {

		temp = mDataChannels [j]->TestSignalsForOffScaleSM ();

		if (temp < 0)
			ans = temp;
	}

	return ans;
}


int CoreBioComponent :: PreTestSignalsForLaserOffScaleSM () {

	//
	//	Sample stage 1  (Ladder?)
	//

	int ans = 0;
	int temp;

	for (int j=1; j<=mNumberOfChannels; j++) {

		temp = mDataChannels [j]->PreTestForSignalOffScaleSM ();

		if (temp < 0)
			ans = temp;
	}

	return ans;
}


int CoreBioComponent :: TestAllFractionalFiltersSMLF () {

	//
	// Ladder Free Stage 1

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i == mLaneStandardChannel)
			continue;

		mDataChannels [i]->ApplyFractionalFiltersSMLF ();
	}

	return 0;
}


bool CoreBioComponent :: EvaluateSmartMessagesForStage (int stage, bool allMessages, bool signalsOnly) {

	int i;
	bool status = true;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->EvaluateSmartMessagesForStage (stage, allMessages, signalsOnly);

	if (allMessages || (!signalsOnly))
		status = SmartMessage::EvaluateAllMessages (mMessageArray, mChannelList, stage, GetObjectScope ());

	return status;
}


bool CoreBioComponent :: EvaluateSmartMessagesForStage (SmartMessagingComm& comm, int numHigherObjects, int stage, bool allMessages, bool signalsOnly) {

	int i;
	bool status = true;
	comm.SMOStack [numHigherObjects] = (SmartMessagingObject*) this;
	int topNum = numHigherObjects + 1;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->EvaluateSmartMessagesForStage (comm, topNum, stage, allMessages, signalsOnly);

	if (allMessages || (!signalsOnly))
		status = SmartMessage::EvaluateAllMessages (comm, topNum, stage, GetObjectScope ());

	return status;
}


bool CoreBioComponent :: EvaluateSmartMessagesAndTriggersForStage (SmartMessagingComm& comm, int numHigherObjects, int stage, bool allMessages, bool signalsOnly) {

	int i;
	comm.SMOStack [numHigherObjects] = (SmartMessagingObject*) this;
	int topNum = numHigherObjects + 1;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->EvaluateSmartMessagesAndTriggersForStage (comm, topNum, stage, allMessages, signalsOnly);

	if (allMessages || (!signalsOnly)) {

		SmartMessage::EvaluateAllMessages (comm, topNum, stage, GetObjectScope ());
		SmartMessage::SetTriggersForAllMessages (comm, topNum, stage, GetObjectScope ());
	}

	return true;
}


bool CoreBioComponent :: SetTriggersForAllMessages (bool* const higherMsgMatrix, int stage, bool allMessages, bool signalsOnly) {

	int i;
	bool status = true;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->SetTriggersForAllMessages (mMessageArray, stage, allMessages, signalsOnly);

	if (allMessages || (!signalsOnly))
		status = SmartMessage::SetTriggersForAllMessages (mMessageArray, higherMsgMatrix, stage, GetObjectScope ());

	return status;
}


bool CoreBioComponent :: SetTriggersForAllMessages (SmartMessagingComm& comm, int numHigherObjects, int stage, bool allMessages, bool signalsOnly) {

	int i;
	bool status = true;
	comm.SMOStack [numHigherObjects] = (SmartMessagingObject*) this;
	int newNum = numHigherObjects + 1;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->SetTriggersForAllMessages (comm, newNum, stage, allMessages, signalsOnly);

	if (allMessages || (!signalsOnly))
		status = SmartMessage::SetTriggersForAllMessages (comm, newNum, stage, GetObjectScope ());

	return status;
}


bool CoreBioComponent :: EvaluateAllReports (bool* const reportMatrix) {

	return SmartMessage::EvaluateAllReports (mMessageArray, reportMatrix, GetObjectScope ());
}


bool CoreBioComponent :: TestAllMessagesForCall () {

	return SmartMessage::TestAllMessagesForCall (mMessageArray, GetObjectScope ());
}


bool CoreBioComponent :: EvaluateAllReportLevels (int* const reportLevelMatrix) {

	return SmartMessage::EvaluateAllReportLevels (mMessageArray, reportLevelMatrix, GetObjectScope ());
}


Boolean CoreBioComponent :: ReportSmartNoticeObjects (RGTextOutput& text, const RGString& indent, const RGString& delim, Boolean reportLink) {

	if (NumberOfSmartNoticeObjects () > 0) {

		int msgLevel = GetHighestMessageLevelWithRestrictionSM ();
		RGDListIterator it (*mSmartMessageReporters);
		SmartMessageReporter* nextNotice;
		text.SetOutputLevel (msgLevel);

		if (!text.TestCurrentLevel ()) {

			text.ResetOutputLevel ();
			return FALSE;
		}

		Endl endLine;
		text << endLine;

		if (reportLink)
			text << mTableLink;

		text << indent << GetSampleName () << " Notices:  " << endLine;

		while (nextNotice = (SmartMessageReporter*) it ()) {

			text << indent << nextNotice->GetMessage () << nextNotice->GetMessageData () << endLine;
		}

		text.ResetOutputLevel ();

		if (reportLink) {

			text.SetOutputLevel (msgLevel);
			text << mTableLink;
			text.ResetOutputLevel ();
		}

		text.Write (1, "\n");
	}

	else
		return FALSE;

	return TRUE;
}


Boolean CoreBioComponent :: ReportAllSmartNoticeObjects (RGTextOutput& text, const RGString& indent, const RGString& delim, Boolean reportLink) {

	int severity;	
	Boolean reportedNotices = ReportSmartNoticeObjects (text, indent, delim, reportLink);

	if (STRLCAnalysis::GetDirectoryCrashMode ())
		return TRUE;

	if (!reportedNotices) {

		severity = GetLocusAndChannelHighestMessageLevel ();

		if ((severity > 0) && (severity <= Notice::GetMessageTrigger())) {

			text.SetOutputLevel (severity);
			Endl endLine;
			text << indent << GetSampleName () << " Notices:  " << endLine;
			text.ResetOutputLevel ();
		}
	}

	RGString indent2 = indent + "\t";
	bool report = false;

	if (reportLink)
		report = true;

	mLSData->ReportSmartNoticeObjects (text, indent2, delim, report);

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels[i]->ReportAllSmartNoticeObjects (text, indent2, delim, reportLink);
	}

	return TRUE;
}


Boolean CoreBioComponent::ReportAllSmartNoticeObjectsCrashMode (RGTextOutput& text, const RGString& indent, const RGString& delim, Boolean reportLink) {

	Boolean reportedNotices = ReportSmartNoticeObjects (text, indent, delim, reportLink);

	return TRUE;
}


bool CoreBioComponent :: ReportXMLSmartNoticeObjects (RGTextOutput& text, RGTextOutput& tempText, const RGString& delim) {

	if (NumberOfSmartNoticeObjects () > 0) {

		int msgLevel = GetHighestMessageLevelWithRestrictionSM ();
		RGDListIterator it (*mSmartMessageReporters);
		SmartMessageReporter* nextNotice;
		text.SetOutputLevel (msgLevel);
		tempText.SetOutputLevel (1);
		int msgNum;
		int triggerLevel = Notice::GetMessageTrigger ();
		bool includesExportInfo = false;
		bool viable;

		while (nextNotice = (SmartMessageReporter*) it ()) {

			if (nextNotice->HasViableExportInfo ()) {

				includesExportInfo = true;
				break;
			}
		}

		if (!text.TestCurrentLevel () && !includesExportInfo) {

			text.ResetOutputLevel ();
			tempText.ResetOutputLevel ();
			return FALSE;
		}

		text.ResetOutputLevel ();
		text << CLevel (1) << "\t\t\t<SampleAlerts>\n" << PLevel ();
		it.Reset ();
		text.SetOutputLevel (1);

		while (nextNotice = (SmartMessageReporter*) it ()) {

			viable = nextNotice->HasViableExportInfo ();
			msgLevel = nextNotice->GetMessagePriority ();

			if (((msgLevel > 0) && (msgLevel <= triggerLevel)) || viable) {

				msgNum = Notice::GetNextMessageNumber ();
				nextNotice->SetMessageCount (msgNum);
				text << "\t\t\t\t<MessageNumber>" << msgNum << "</MessageNumber>\n";
				tempText << "\t\t<Message>\n";
				tempText << "\t\t\t<MessageNumber>" << msgNum << "</MessageNumber>\n";
				tempText << "\t\t\t<Text>" << nextNotice->GetMessage () << nextNotice->GetMessageData () << "</Text>\n";

				if (viable) {

					if (nextNotice->IsEnabled ())
						tempText << "\t\t\t<Hidden>false</Hidden>\n";

					else
						tempText << "\t\t\t<Hidden>true</Hidden>\n";

					if (!nextNotice->IsCritical ())
						tempText << "\t\t\t<Critical>false</Critical>\n";

					if (nextNotice->IsEnabled ())
						tempText << "\t\t\t<Enabled>true</Enabled>\n";

					else
						tempText << "\t\t\t<Enabled>false</Enabled>\n";

					if (!nextNotice->IsEditable ())
						tempText << "\t\t\t<Editable>false</Editable>\n";

					if (nextNotice->GetDisplayExportInfo ())
						tempText << "\t\t\t<DisplayExportInfo>true</DisplayExportInfo>\n";

					else
						tempText << "\t\t\t<DisplayExportInfo>false</DisplayExportInfo>\n";

					if (!nextNotice->GetDisplayOsirisInfo ())
						tempText << "\t\t\t<DisplayOsirisInfo>false</DisplayOsirisInfo>\n";

					tempText << "\t\t\t<MsgName>" << nextNotice->GetMessageName () << "</MsgName>\n";

					//tempText << "\t\t\t<ExportProtocolList>";
					//tempText << "\t\t\t" << nextNotice->GetExportProtocolInformation ();
					//tempText << "\t\t\t</ExportProtocolList>\n";
				}

				else
					tempText << "\t\t\t<MsgName>" << nextNotice->GetMessageName () << "</MsgName>\n";

				tempText << "\t\t</Message>\n";
			}
		}

		text.ResetOutputLevel ();
		text << CLevel (1) << "\t\t\t</SampleAlerts>\n" << PLevel ();
		tempText.ResetOutputLevel ();
	}

	else
		return false;

	return true;
}


int CoreBioComponent :: AddAllSmartMessageReporters () {

	int k = GetObjectScope ();
	int size = SmartMessage::GetSizeOfArrayForScope (k);
	int i;
	int nMsgs = 0;
	SmartMessageReporter* newMsg;
	SmartMessage* nextSmartMsg;
	SmartMessageData target;
	SmartMessageData* smd;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->AddAllSmartMessageReporters ();

	for (i=0; i<size; i++) {

		if (!mMessageArray [i])
			continue;

		nextSmartMsg = SmartMessage::GetSmartMessageForScopeAndElement (k, i);

		if (!nextSmartMsg->EvaluateReport (mMessageArray))
			continue;

		target.SetIndex (i);
		smd = (SmartMessageData*) mMessageDataTable->Find (&target);
		newMsg = new SmartMessageReporter;
		newMsg->SetSmartMessage (nextSmartMsg);
		
		if (smd != NULL)
			newMsg->SetData (smd);

		newMsg->SetPriorityLevel (nextSmartMsg->EvaluateReportLevel (mMessageArray));
		newMsg->SetRestrictionLevel (nextSmartMsg->EvaluateRestrictionLevel (mMessageArray));
		nMsgs = AddSmartMessageReporter (newMsg);
	}

	MergeAllSmartMessageReporters ();
	return nMsgs;
}


int CoreBioComponent :: AddAllSmartMessageReportersForSignals () {

	int i;
	int nMsgs = 0;

	for (i=1; i<=mNumberOfChannels; i++)
		nMsgs += mDataChannels [i]->AddAllSmartMessageReportersForSignals ();

	return nMsgs;
}


int CoreBioComponent :: AddAllSmartMessageReporters (SmartMessagingComm& comm, int numHigherObjects) {

	int k = GetObjectScope ();
	int size = SmartMessage::GetSizeOfArrayForScope (k);
	int i;
	int nMsgs = 0;
	SmartMessageReporter* newMsg;
	SmartMessage* nextSmartMsg;
	SmartMessageData target;
	SmartMessageData* smd;
	bool editable;
	bool enabled;
	bool hasExportProtocolInfo;
	bool report;
	bool mirror;
	bool displayExport;

	if (!STRLCAnalysis::GetDirectoryCrashMode ()) {

		for (i=1; i<=mNumberOfChannels; i++)
			mDataChannels [i]->AddAllSmartMessageReporters (comm, numHigherObjects);
	}

	for (i=0; i<size; i++) {

		nextSmartMsg = SmartMessage::GetSmartMessageForScopeAndElement (k, i);
		editable = nextSmartMsg->IsEditable ();
		hasExportProtocolInfo = nextSmartMsg->HasExportProtocolInfo ();

		if (!mMessageArray [i]) {

			enabled = false;

			if (!editable)
				continue;

			if (!hasExportProtocolInfo)
				continue;
		}

		else
			enabled = true;

		report = nextSmartMsg->EvaluateReportContingent (comm, numHigherObjects);
		mirror = nextSmartMsg->UseDefaultExportDisplayMode ();

		if (mirror)
			displayExport = report;

		else
			displayExport = nextSmartMsg->DisplayExportInfo ();

		if (!report && !displayExport)
			continue;

		target.SetIndex (i);
		smd = (SmartMessageData*) mMessageDataTable->Find (&target);
		newMsg = new SmartMessageReporter;
		newMsg->SetSmartMessage (nextSmartMsg);
		
		if (smd != NULL)
			newMsg->SetData (smd);

		newMsg->SetPriorityLevel (nextSmartMsg->EvaluateReportLevel (comm, numHigherObjects));
		newMsg->SetRestrictionLevel (nextSmartMsg->EvaluateRestrictionLevel (comm, numHigherObjects));
		newMsg->SetEditable (editable);
		newMsg->SetEnabled (enabled);
		newMsg->SetDisplayExportInfo (displayExport);
		newMsg->SetDisplayOsirisInfo (report);

		if (hasExportProtocolInfo) {
			
			newMsg->SetExportProtocolInformation (nextSmartMsg->GetExportProtocolList ());
			SmartMessagingObject::InsertExportSpecificationsIntoTable (nextSmartMsg);
		}

		newMsg->ComputeViabilityOfExportInfo ();
		nMsgs = AddSmartMessageReporter (newMsg);
	}

	MergeAllSmartMessageReporters ();
	return nMsgs;
}


int CoreBioComponent::AddAllSmartMessageReportersCrashMode (SmartMessagingComm& comm, int numHigherObjects) {

	int k = GetObjectScope ();
	int size = SmartMessage::GetSizeOfArrayForScope (k);
	int i;
	int nMsgs = 0;
	SmartMessageReporter* newMsg;
	SmartMessage* nextSmartMsg;
	SmartMessageData target;
	SmartMessageData* smd;
	bool editable;
	bool enabled;
	bool hasExportProtocolInfo;
	bool report;
	bool mirror;
	bool displayExport;

	for (i=0; i<size; i++) {

		nextSmartMsg = SmartMessage::GetSmartMessageForScopeAndElement (k, i);
		editable = nextSmartMsg->IsEditable ();
		hasExportProtocolInfo = nextSmartMsg->HasExportProtocolInfo ();

		if (!mMessageArray [i]) {

			enabled = false;

			if (!editable)
				continue;

			if (!hasExportProtocolInfo)
				continue;
		}

		else
			enabled = true;

		report = nextSmartMsg->EvaluateReportContingent (comm, numHigherObjects);
		mirror = nextSmartMsg->UseDefaultExportDisplayMode ();

		if (mirror)
			displayExport = report;

		else
			displayExport = nextSmartMsg->DisplayExportInfo ();

		if (!report && !displayExport)
			continue;

		target.SetIndex (i);
		smd = (SmartMessageData*)mMessageDataTable->Find (&target);
		newMsg = new SmartMessageReporter;
		newMsg->SetSmartMessage (nextSmartMsg);

		if (smd != NULL)
			newMsg->SetData (smd);

		newMsg->SetPriorityLevel (nextSmartMsg->EvaluateReportLevel (comm, numHigherObjects));
		newMsg->SetRestrictionLevel (nextSmartMsg->EvaluateRestrictionLevel (comm, numHigherObjects));
		newMsg->SetEditable (editable);
		newMsg->SetEnabled (enabled);
		newMsg->SetDisplayExportInfo (displayExport);
		newMsg->SetDisplayOsirisInfo (report);

		if (hasExportProtocolInfo) {

			newMsg->SetExportProtocolInformation (nextSmartMsg->GetExportProtocolList ());
			SmartMessagingObject::InsertExportSpecificationsIntoTable (nextSmartMsg);
		}

		newMsg->ComputeViabilityOfExportInfo ();
		nMsgs = AddSmartMessageReporter (newMsg);
	}

	MergeAllSmartMessageReporters ();
	return nMsgs;
}


int CoreBioComponent :: AddAllSmartMessageReportersForSignals (SmartMessagingComm& comm, int numHigherObjects) {

	int i;
	int nMsgs = 0;

	for (i=1; i<=mNumberOfChannels; i++)
		nMsgs += mDataChannels [i]->AddAllSmartMessageReportersForSignals (comm, numHigherObjects);

	return nMsgs;
}


void CoreBioComponent :: SetNegativeControlTrueSM () {

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		mDataChannels [i]->ChannelIsNegativeControlSM (true);

		//if (i != mLaneStandardChannel)
		//	mDataChannels [i]->ClearAllPeaksBelowAnalysisThreshold ();
	}

	mIsNegativeControl = true;
}


void CoreBioComponent :: SetNegativeControlFalseSM () {

	int i;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->ChannelIsNegativeControlSM (false);

	mIsNegativeControl = false;
}


void CoreBioComponent :: SetPositiveControlTrueSM () {

	int i;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->ChannelIsPositiveControlSM (true);

	mIsPositiveControl = true;
}


void CoreBioComponent :: SetPositiveControlFalseSM () {

	int i;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->ChannelIsPositiveControlSM (false);

	mIsPositiveControl = false;
}


int CoreBioComponent :: TestFractionalFiltersSM () {

	//
	//  This is sample stage 2
	//

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels [i]->TestFractionalFiltersSM ();   //  Let's try testing this here instead of further down...
	}

	return 0;
}


int CoreBioComponent :: MakePreliminaryCallsSM (GenotypesForAMarkerSet* pGenotypes) {

	//
	//  This is sample stage 3
	//

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels [i]->MakePreliminaryCallsSM (mIsNegativeControl, mIsPositiveControl, pGenotypes);
	}

	return 0;
}


void CoreBioComponent :: ReportXMLSmartGridTableRowWithLinks (RGTextOutput& text, RGTextOutput& tempText) {

	text << CLevel (1) << "\t\t<Sample>\n";
	RGString SimpleFileName (mName);
	size_t startPos = 0;
	size_t endPos;
	size_t length = SimpleFileName.Length ();
	RGString pResult;
	
	if (SimpleFileName.FindLastSubstringCaseIndependent (DirectoryManager::GetDataFileType (), startPos, endPos)) {

		if (endPos == length - 1)
			SimpleFileName.ExtractAndRemoveLastCharacters (4);
	}

	SimpleFileName.FindAndReplaceAllSubstrings ("\\", "/");
	startPos = endPos = 0;

	if (SimpleFileName.FindLastSubstring ("/", startPos, endPos)) {

		SimpleFileName.ExtractAndRemoveSubstring (0, startPos);
	}

	text << "\t\t\t<Name>" << xmlwriter::EscAscii (SimpleFileName, &pResult) << "</Name>\n";
	text << "\t\t\t<SampleName>" << xmlwriter::EscAscii (mSampleName, &pResult) << "</SampleName>\n";
	text << "\t\t\t<Comment>" << xmlwriter::EscAscii (mComments, &pResult) << "</Comment>\n";
	text << "\t\t\t<RunStart>" << mRunStart.GetData () << "</RunStart>\n";
	text << "\t\t\t<Type>Ladder</Type>\n";

	ReportXMLSampleInfoBlock ("\t\t\t", text);
	//text << "\t\t\t<Info>\n";
	//text << "\t\t\t\t<MaxLinearPullup>0.0</MaxLinearPullup>\n";
	//text << "\t\t\t\t<MaxNonlinearPullup>0.0</MaxNonlinearPullup>\n";
	//int j;

	//for (j=1; j<=mNumberOfChannels; j++) {

	//	text << "\t\t\t\t<Channel>\n";
	//	text << "\t\t\t\t\t<Number>" << j << "</Number>\n";
	//	text << "\t\t\t\t\t<Noise>" << mDataChannels [j]->GetNoiseRange () << "</Noise>\n";
	//	text << "\t\t\t\t</Channel>\n";
	//}

	//text << "\t\t\t</Info>\n" << PLevel ();

	int trigger = Notice::GetMessageTrigger ();
//	int channelHighestLevel;
//	bool channelAlerts = false;
	int cbcHighestMsgLevel = GetHighestMessageLevelWithRestrictionSM ();
	bool includesExportInfo = false;

	RGDListIterator it (*mSmartMessageReporters);
	SmartMessageReporter* nextNotice;

	while (nextNotice = (SmartMessageReporter*) it ()) {

		if (nextNotice->HasViableExportInfo ()) {

			includesExportInfo = true;
			break;
		}
	}

	if (((cbcHighestMsgLevel > 0) && (cbcHighestMsgLevel <= trigger)) || includesExportInfo) {

//		text << CLevel (1) << "\t\t\t<SampleAlerts>\n" << PLevel ();

		// get message numbers and report
		ReportXMLSmartNoticeObjects (text, tempText, " ");

//		text << CLevel (1) << "\t\t\t</SampleAlerts>\n" << PLevel ();
	}

	mDataChannels [mLaneStandardChannel]->ReportXMLILSSmartNoticeObjects (text, tempText, " ");

	int i;

	//for (i=1; i<=mNumberOfChannels; i++) {

	//	if (i == mLaneStandardChannel)
	//		continue;

	//	channelHighestLevel = mDataChannels [i]->GetHighestMessageLevelWithRestrictionSM ();

	//	if ((channelHighestLevel > 0) && (channelHighestLevel <= trigger)) {

	//		channelAlerts = true;
	//		break;
	//	}
	//}

//	if (channelAlerts) {

		text << CLevel (1) << "\t\t\t<ChannelAlerts>\n" << PLevel ();

		for (i=1; i<=mNumberOfChannels; i++) {

			if (i == mLaneStandardChannel)
				continue;

			mDataChannels [i]->ReportXMLSmartNoticeObjects (text, tempText, " ");
		}

		text << CLevel (1) << "\t\t\t</ChannelAlerts>\n" << PLevel ();
//	}

	mMarkerSet->ResetLocusList ();
	Locus* nextLocus;
//	int locusHighestLevel;

	while (nextLocus = mMarkerSet->GetNextLocus ()) {

		//locusHighestLevel = nextLocus->GetHighestMessageLevelWithRestrictionSM ();

		//if ((locusHighestLevel > 0) && (locusHighestLevel <= trigger))

		nextLocus->ReportXMLSmartGridTableRowWithLinks (text, tempText, " ");
	}

	text << CLevel (1) << "\t\t</Sample>\n" << PLevel ();
}



void CoreBioComponent :: ReportXMLSmartSampleTableRowWithLinks (RGTextOutput& text, RGTextOutput& tempText) {

	RGString type;

	if (STRLCAnalysis::GetDirectoryCrashMode ())
		type = "Unknown";

	else if (mIsNegativeControl)
		type = "-Control";

	else if (mIsPositiveControl)
		type = "+Control";

	else
		type = "Sample";

	RGString pResult;

	RGString SimpleFileName (mName);
	size_t startPos = 0;
	size_t endPos;
	size_t length = SimpleFileName.Length ();
	
	if (SimpleFileName.FindLastSubstringCaseIndependent (DirectoryManager::GetDataFileType (), startPos, endPos)) {

		if (endPos == length - 1)
			SimpleFileName.ExtractAndRemoveLastCharacters (4);
	}

	SimpleFileName.FindAndReplaceAllSubstrings ("\\", "/");
	startPos = endPos = 0;

	if (SimpleFileName.FindLastSubstring ("/", startPos, endPos)) {

		SimpleFileName.ExtractAndRemoveSubstring (0, startPos);
	}
	
	text << CLevel (1) << "\t\t<Sample>\n";
	text << "\t\t\t<Name>" << xmlwriter::EscAscii (SimpleFileName, &pResult) << "</Name>\n";
	text << "\t\t\t<SampleName>" << xmlwriter::EscAscii (mSampleName, &pResult) << "</SampleName>\n";
	text << "\t\t\t<Comment>" << xmlwriter::EscAscii (mComments, &pResult) << "</Comment>\n";
	text << "\t\t\t<RunStart>" << mRunStart.GetData () << "</RunStart>\n";
	text << "\t\t\t<Type>" << type.GetData () << "</Type>\n";

	ReportXMLSampleInfoBlock ("\t\t\t", text);

	/*text << "\t\t\t<Info>\n";
	text << "\t\t\t\t<MaxLinearPullup>" << mQC.mMaxLinearPullupCoefficient << "</MaxLinearPullup>\n";
	text << "\t\t\t\t<MaxNonlinearPullup>" << mQC.mMaxNonlinearPullupCoefficient << "</MaxNonlinearPullup>\n";
	int j;

	for (j=1; j<=mNumberOfChannels; j++) {

		text << "\t\t\t\t<Channel>\n";
		text << "\t\t\t\t\t<Number>" << j << "</Number>\n";
		text << "\t\t\t\t\t<Noise>" << mDataChannels [j]->GetNoiseRange () << "</Noise>\n";
		text << "\t\t\t\t</Channel>\n";
	}

	text << "\t\t\t</Info>\n" << PLevel ();*/

	int trigger = Notice::GetMessageTrigger ();
//	int channelHighestLevel;
//	bool channelAlerts = false;
	int cbcHighestMsgLevel = GetHighestMessageLevelWithRestrictionSM ();
	RGDListIterator it (*mSmartMessageReporters);
	SmartMessageReporter* nextNotice;
	bool includesExportInfo = false;

	while (nextNotice = (SmartMessageReporter*) it ()) {

		if (nextNotice->HasViableExportInfo ()) {

			includesExportInfo = true;
			break;
		}
	}

	if (((cbcHighestMsgLevel > 0) && (cbcHighestMsgLevel <= trigger)) || includesExportInfo) {

//		text << CLevel (1) << "\t\t\t<SampleAlerts>\n" << PLevel ();

		// get message numbers and report
		ReportXMLSmartNoticeObjects (text, tempText, " ");

//		text << CLevel (1) << "\t\t\t</SampleAlerts>\n" << PLevel ();
	}

	if (STRLCAnalysis::GetDirectoryCrashMode ()) {

		text << CLevel (1) << "\t\t</Sample>\n" << PLevel ();
		return;
	}

	mDataChannels [mLaneStandardChannel]->ReportXMLILSSmartNoticeObjects (text, tempText, " ");
	int i;

	//for (i=1; i<=mNumberOfChannels; i++) {

	//	//if (i == mLaneStandardChannel)
	//	//	continue;

	//	channelHighestLevel = mDataChannels [i]->GetHighestMessageLevelWithRestrictionSM ();

	//	if ((channelHighestLevel > 0) && (channelHighestLevel <= trigger)) {

	//		channelAlerts = true;
	//		break;
	//	}
	//}

//	if (channelAlerts) {

		text << CLevel (1) << "\t\t\t<ChannelAlerts>\n" << PLevel ();

		for (i=1; i<=mNumberOfChannels; i++) {

			if (i == mLaneStandardChannel)
				continue;

			mDataChannels [i]->ReportXMLSmartNoticeObjects (text, tempText, " ");
		}

		text << CLevel (1) << "\t\t\t</ChannelAlerts>\n" << PLevel ();
//	}

	mMarkerSet->ResetLocusList ();
	Locus* nextLocus;

	while (nextLocus = mMarkerSet->GetNextLocus ()) {

		nextLocus->ReportXMLSmartSampleTableRowWithLinks (text, tempText, " ");
	}

	if (mIsPositiveControl)
		text << CLevel (1) << "\t\t\t<PositiveControl>" << mPositiveControlName << "</PositiveControl>\n";

	text << CLevel (1) << "\t\t</Sample>\n" << PLevel ();
}



void CoreBioComponent::ReportXMLSmartSampleTableRowWithLinksCrashMode (RGTextOutput& text, RGTextOutput& tempText) {

	RGString type;

	type = "Unknown";
	RGString pResult;

	RGString SimpleFileName (mName);
	size_t startPos = 0;
	size_t endPos;
	size_t length = SimpleFileName.Length ();

	if (SimpleFileName.FindLastSubstringCaseIndependent (DirectoryManager::GetDataFileType (), startPos, endPos)) {

		if (endPos == length - 1)
			SimpleFileName.ExtractAndRemoveLastCharacters (4);
	}

	SimpleFileName.FindAndReplaceAllSubstrings ("\\", "/");
	startPos = endPos = 0;

	if (SimpleFileName.FindLastSubstring ("/", startPos, endPos)) {

		SimpleFileName.ExtractAndRemoveSubstring (0, startPos);
	}

	text << CLevel (1) << "\t\t<Sample>\n";
	text << "\t\t\t<Name>" << xmlwriter::EscAscii (SimpleFileName, &pResult) << "</Name>\n";
	text << "\t\t\t<SampleName>" << xmlwriter::EscAscii (mSampleName, &pResult) << "</SampleName>\n";
	text << "\t\t\t<Comment>" << xmlwriter::EscAscii (mComments, &pResult) << "</Comment>\n";
	text << "\t\t\t<RunStart>" << mRunStart.GetData () << "</RunStart>\n";
	text << "\t\t\t<Type>" << type.GetData () << "</Type>\n";

	ReportXMLSampleInfoBlock ("\t\t\t", text);

	/*text << "\t\t\t<Info>\n";
	text << "\t\t\t\t<MaxLinearPullup>" << mQC.mMaxLinearPullupCoefficient << "</MaxLinearPullup>\n";
	text << "\t\t\t\t<MaxNonlinearPullup>" << mQC.mMaxNonlinearPullupCoefficient << "</MaxNonlinearPullup>\n";
	int j;

	for (j=1; j<=mNumberOfChannels; j++) {

		text << "\t\t\t\t<Channel>\n";
		text << "\t\t\t\t\t<Number>" << j << "</Number>\n";
		text << "\t\t\t\t\t<Noise>" << mDataChannels [j]->GetNoiseRange () << "</Noise>\n";
		text << "\t\t\t\t</Channel>\n";
	}

	text << "\t\t\t</Info>\n" << PLevel ();*/

	int trigger = Notice::GetMessageTrigger ();
	//	int channelHighestLevel;
	//	bool channelAlerts = false;
	int cbcHighestMsgLevel = GetHighestMessageLevelWithRestrictionSM ();
	RGDListIterator it (*mSmartMessageReporters);
	SmartMessageReporter* nextNotice;
	bool includesExportInfo = false;

	while (nextNotice = (SmartMessageReporter*)it ()) {

		if (nextNotice->HasViableExportInfo ()) {

			includesExportInfo = true;
			break;
		}
	}

	if (((cbcHighestMsgLevel > 0) && (cbcHighestMsgLevel <= trigger)) || includesExportInfo) {

		//		text << CLevel (1) << "\t\t\t<SampleAlerts>\n" << PLevel ();

				// get message numbers and report
		ReportXMLSmartNoticeObjects (text, tempText, " ");

		//		text << CLevel (1) << "\t\t\t</SampleAlerts>\n" << PLevel ();
	}

	text << CLevel (1) << "\t\t</Sample>\n" << PLevel ();
}



void CoreBioComponent :: ReportXMLSampleInfoBlock (const RGString& indent, RGTextOutput& text) {

	RGString indent1 = indent + "\t";
	RGString indent2 = indent + "\t\t";
	text << indent << "<Info>\n";
	text << indent1 << "<MaxLinearPullup>" << mQC.mMaxLinearPullupCoefficient << "</MaxLinearPullup>\n";
	text << indent1 << "<MaxNonlinearPullup>" << mQC.mMaxNonlinearPullupCoefficient << "</MaxNonlinearPullup>\n";
	text << indent1 << "<MaxBPErrorSampleToLadder>" << mQC.mMaxErrorInBP << "</MaxBPErrorSampleToLadder>\n";
	text << indent1 << "<WidthOfLastILSPeak>" << mQC.mWidthOfLastILSPeak << "</WidthOfLastILSPeak>\n";

	text << indent1 << "<SampleLocusTotalAreaRatioMaxToMin>" << mQC.mSampleLocusTotalAreaRatioMaxToMin << "</SampleLocusTotalAreaRatioMaxToMin>\n";
	text << indent1 << "<SampleYLinkedLocusTotalAreaRatioMaxToMin>" << mQC.mSampleYLinkedLocusTotalAreaRatioMaxToMin << "</SampleYLinkedLocusTotalAreaRatioMaxToMin>\n";
	text << indent1 << "<StartingTemperature>" << mQC.mStartingTemperature << "</StartingTemperature>\n";
	text << indent1 << "<MaxMinusMinTemperature>" << mQC.mMaxMinusMinTemperature << "</MaxMinusMinTemperature>\n";
	text << indent1 << "<StartingVoltage>" << mQC.mStartingVoltage << "</StartingVoltage>\n";
	text << indent1 << "<MaxMinusMinVoltage>" << mQC.mMaxMinusMinVoltage << "</MaxMinusMinVoltage>\n";
	text << indent1 << "<StartingCurrent>" << mQC.mStartingCurrent << "</StartingCurrent>\n";
	text << indent1 << "<MaxMinusMinCurrent>" << mQC.mMaxMinusMinCurrent << "</MaxMinusMinCurrent>\n";
	text << indent1 << "<StartingPower>" << mQC.mStartingPower << "</StartingPower>\n";
	text << indent1 << "<MaxMinusMinPower>" << mQC.mMaxMinusMinPower << "</MaxMinusMinPower>\n";

	text << indent1 << "<RunDate>" << mQC.mRunDate << "</RunDate>\n";
	text << indent1 << "<RunTime>" << mQC.mRunTime << "</RunTime>\n";
	text << indent1 << "<CapillaryNumber>" << mQC.mCapillaryNumber << "</CapillaryNumber>\n";
	text << indent1 << "<InjectionSeconds>" << mQC.mInjectionSeconds << "</InjectionSeconds>\n";
	text << indent1 << "<InjectionVoltage>" << mQC.mInjectionVoltage << "</InjectionVoltage>\n";

	int j;

	if (!CrashMode) {

		for (j=1; j<=mNumberOfChannels; j++) {

			text << indent1 << "<Channel>\n";
			text << indent2 << "<Number>" << j << "</Number>\n";
			text << indent2 << "<Noise>" << mDataChannels [j]->GetNoiseRange () << "</Noise>\n";
			text << indent2 << "<ChannelLocusTotalAreaRatioMaxToMin>" << mDataChannels [j]->GetMaxLocusAreaRatio () << "</ChannelLocusTotalAreaRatioMaxToMin>\n";  // Must generate and store and provide accessor for this number for each channel
			text << indent2 << "<ChannelYLinkedLocusTotalAreaMaxToMin>" << mDataChannels [j]->GetMaxYLinkedLocusAreaRatio () << "</ChannelYLinkedLocusTotalAreaMaxToMin>\n";
			text << indent1 << "</Channel>\n";
		}
	}

	text << indent << "</Info>\n" << PLevel ();
}


bool CoreBioComponent :: GetIgnoreNoiseAboveDetectionInSmoothingFlag () const {

	smIgnoreNoiseAnalysisAboveDetectionThresholdInSmoothing ignore;
	return GetMessageValue (ignore);
}


void CoreBioComponent :: WriteDataToHeightFileSM () {

	int i;
	double totals [12];
	Endl endLine;

	if (HeightFile == NULL)
		return;

	if (!HeightFile->FileIsValid ())
		return;

	for (i=0; i<12; i++)
		totals [i] = 0.0;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->AccumulatePeakHeightsForChannelAndAddToTotalsSM (totals, mNumberOfChannels);

	if (totals [1] != 0.0) {

		totals [2] = 100.0 * totals [0] / totals [1];
		totals [7] = 100.0 * totals [6] / totals [1];
		totals [9] = 100.0 * totals [8] / totals [1];
	}

	if (totals [4] != 0.0)
		totals [5] = 100.0 * totals [3] / totals [4];		

	if (totals [10] != 0.0)
		totals [11] = 100.0 * totals [8] / totals [10];

	for (i=0; i<12; i++)
		*HeightFile << totals [i] << "\t\t";

	*HeightFile << endLine;
}


void CoreBioComponent :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	RGString idData = "Sample:  " + mName;
	SmartMessage::OutputDebugString (idData);
}


int CoreBioComponent :: InitializeSM (SampleData& fileData, PopulationCollection* collection, const RGString& markerSetName, Boolean isGrid) {

	//
	//  This is ladder and sample stage 1
	//

	mTime = fileData.GetCollectionStartTime ();
	mDate = fileData.GetCollectionStartDate ();
	mName = fileData.GetName ();
	mRunStart = mDate.GetOARString () + mTime.GetOARString ();
	mMarkerSet = collection->GetNamedPopulationMarkerSet (markerSetName);
	Progress = 0;

	smMarkerSetNameUnknown noNamedMarkerSet;
	smNamedILSUnknown noNamedILS;
	smChannelIsILS channelIsILS;

	if (mMarkerSet == NULL) {

		ErrorString = "*******COULD NOT FIND MARKER SET NAMED ";
		ErrorString << markerSetName << " IN POPULATION COLLECTION********\n";
		SetMessageValue (noNamedMarkerSet, true);
		AppendDataForSmartMessage (noNamedMarkerSet, MainMessages::XML (markerSetName));
		return -1;
	}

	mLaneStandard = mMarkerSet->GetLaneStandard ();
	mNumberOfChannels = mMarkerSet->GetNumberOfChannels ();

	if ((mLaneStandard == NULL) || !mLaneStandard->IsValid ()) {

		ErrorString = "Could not find named internal lane standard associated with marker set named ";
		ErrorString << markerSetName << "\n";
		cout << "Could not find named internal lane standard associated with marker set named " << (char*)markerSetName.GetData () << endl;
		SetMessageValue (noNamedILS, true);
		AppendDataForSmartMessage (noNamedILS, MainMessages::XML (markerSetName));
		return -1;
	}

	mDataChannels = new ChannelData* [mNumberOfChannels + 1];
	int i;
	const int* fsaChannelMap = mMarkerSet->GetChannelMap ();

	for (i=0; i<=mNumberOfChannels; i++)
		mDataChannels [i] = NULL;

	int j;
	mPullupTestedMatrix = new bool* [mNumberOfChannels + 1];
	mLinearPullupMatrix = new double* [mNumberOfChannels + 1];
	mQuadraticPullupMatrix = new double* [mNumberOfChannels + 1];
	mLinearInScalePullupMatrix = new double* [mNumberOfChannels + 1];
	mQuadraticInScalePullupMatrix = new double* [mNumberOfChannels + 1];
	mMinimumInScalePrimaryPeak = new double* [mNumberOfChannels + 1];
	mPattern = new bool* [mNumberOfChannels + 1];
	mLeastMedianValue = new double* [mNumberOfChannels + 1];
	mOutlierThreshold = new double* [mNumberOfChannels + 1];

	for (i=1; i<=mNumberOfChannels; i++) {

		mPullupTestedMatrix [i] = new bool [mNumberOfChannels + 1];
		mLinearPullupMatrix [i] = new double [mNumberOfChannels + 1];
		mQuadraticPullupMatrix [i] = new double [mNumberOfChannels + 1];
		mLinearInScalePullupMatrix [i] = new double [mNumberOfChannels + 1];
		mQuadraticInScalePullupMatrix [i] = new double [mNumberOfChannels + 1];
		mMinimumInScalePrimaryPeak [i] = new double [mNumberOfChannels + 1];
		mPattern [i] = new bool [mNumberOfChannels + 1];
		mLeastMedianValue [i] = new double [mNumberOfChannels + 1];
		mOutlierThreshold [i] = new double [mNumberOfChannels + 1];

		for (j=1; j<=mNumberOfChannels; j++) {

			mPullupTestedMatrix [i][j] = false;
			mLinearPullupMatrix [i][j] = 0.0;
			mQuadraticPullupMatrix [i][j] = 0.0;
			mLinearInScalePullupMatrix [i][j] = 0.0;
			mQuadraticInScalePullupMatrix [i][j] = 0.0;
			mMinimumInScalePrimaryPeak [i] [j] = 0.0;
			mPattern [i] [j] = false;
			mLeastMedianValue [i][j] = 0.0;
			mOutlierThreshold [i][j] = 0.0;
		}
	}

	mLaneStandardChannel = mMarkerSet->GetLaneStandardChannel ();

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i == mLaneStandardChannel)
			mDataChannels [i] = GetNewLaneStandardChannel (i, mLaneStandard);

		else {

			if (isGrid)
				mDataChannels [i] = GetNewGridDataChannel (i, mLaneStandard);

			else
				mDataChannels [i] = GetNewDataChannel (i, mLaneStandard);
		}

		mDataChannels [i]->SetFsaChannel (fsaChannelMap [i]);
	}

//	InitializeAllSampleModifications ();
	mLSData = mDataChannels [mLaneStandardChannel];
	mLSData->SetMessageValue (channelIsILS, true);
	mMarkerSet->ResetLocusList ();
	Locus* nextLocus;

	while (nextLocus = mMarkerSet->GetNextLocus ()) {

		mDataChannels [nextLocus->GetLocusChannel ()]->AddLocus (nextLocus);
		nextLocus->InitializeMessageData ();
	}

	Progress = 1;
	return 0;
}


int CoreBioComponent :: SetAllDataSM (SampleData& fileData, TestCharacteristic* testControlPeak, TestCharacteristic* testSamplePeak) {

	//
	//  This is ladder and sample stage 1
	//

	int status = 0;
	ErrorString = "";

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (mDataChannels [i]->SetDataSM (fileData, testControlPeak, testSamplePeak) < 0) {

			ErrorString << mDataChannels [i]->GetError ();
			NoDataChannels.push_back (i);
			status = -1;
		}
	}

	if (status == 0)
		Progress = 2;

	return status;
}


int CoreBioComponent :: SetAllRawDataSM (SampleData& fileData, TestCharacteristic* testControlPeak, TestCharacteristic* testSamplePeak) {

	//
	//  This is ladder and sample stage 1
	//

	int status = 0;
	ErrorString = "";

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (mDataChannels [i]->SetRawDataSM (fileData, testControlPeak, testSamplePeak) < 0) {

			ErrorString << mDataChannels [i]->GetError ();
			NoDataChannels.push_back (i);
			status = -1;
		}
	}

	if (status == 0)
		Progress = 2;

	return status;
}


int CoreBioComponent :: SetAllRawDataWithMatrixSM (SampleData& fileData, TestCharacteristic* testControlPeak, TestCharacteristic* testSamplePeak) {

	//
	//  This is ladder and sample stage 1
	//

	int status = 0;
	ErrorString = "";
	int numElements;
	double* matrix = fileData.GetMatrix (numElements);
	int i;
	int j;
	int k;
	smColorCorrectionMatrixWrongSize matrixWrongSize;
	smColorCorrectionMatrixExpectedButNotFound matrixNotFound;

	if (matrix == NULL) {

		// add message here
		SetMessageValue (matrixNotFound, true);
		return SetAllRawDataSM (fileData, testControlPeak, testSamplePeak);
	}

	else if (numElements != mNumberOfChannels * mNumberOfChannels) {

		// add message here
		SetMessageValue (matrixWrongSize, true);
		return SetAllRawDataSM (fileData, testControlPeak, testSamplePeak);
	}

	double* matrixBase = matrix;
	double** rawChannelData = new double* [mNumberOfChannels + 1];
	int numDataPoints;
	double sum;

	for (i=1; i<=mNumberOfChannels; i++) {

		rawChannelData [i] = fileData.GetRawDataForDataChannel (i, numDataPoints);

		if (rawChannelData [i] == NULL) {

			mDataChannels [i]->SetRawDataFromColorCorrectedArraySM (NULL, numDataPoints, testControlPeak, testSamplePeak);
			ErrorString << mDataChannels [i]->GetError ();
			NoDataChannels.push_back (i);
			status = -1;
		}
	}

	if (status < 0) {

		for (i=1; i<=mNumberOfChannels; i++)
			delete[] rawChannelData [i];

		delete[] rawChannelData;
		delete[] matrix;
		return status;
	}
	
	double** correctedChannelData = new double* [mNumberOfChannels + 1];

	for (i=1; i<=mNumberOfChannels; i++)
		correctedChannelData [i] = new double [numDataPoints];

	for (i=1; i<=mNumberOfChannels; i++) {

		for (k=0; k<numDataPoints; k++){

			sum = 0.0;

			for (j=1; j<=mNumberOfChannels; j++)
				sum += (*(matrixBase + j - 1)) * rawChannelData [j][k];

			correctedChannelData [i][k] = sum;
		}

		matrixBase += mNumberOfChannels;
	}

	for (i=1; i<=mNumberOfChannels; i++)
		delete[] rawChannelData [i];

	delete[] rawChannelData;
	delete[] matrix;
	double* tempArray;

	for (i=1; i<=mNumberOfChannels; i++) {

		tempArray = new double [numDataPoints];

		for (j=0; j<numDataPoints; j++)
			tempArray [j] = correctedChannelData [i][j];

		if (mDataChannels [i]->SetRawDataFromColorCorrectedArraySM (tempArray, numDataPoints, testControlPeak, testSamplePeak) < 0) {

			ErrorString << mDataChannels [i]->GetError ();
			status = -1;
		}
	}

	for (i=1; i<=mNumberOfChannels; i++)
		delete[] correctedChannelData [i];

	delete[] correctedChannelData;

	if (status == 0)
		Progress = 2;

	return status;
}


int CoreBioComponent :: FitAllCharacteristicsSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	//
	//  This is sample stage 1
	//

	int status = 0;

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (mDataChannels [i]->FitAllCharacteristicsSM (text, ExcelText, msg, print) < 0) {

			ErrorString << mDataChannels [i]->GetError ();
			status = -i;
		}
	}

	return status;
}


int CoreBioComponent :: FitNonLaneStandardCharacteristicsSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	//
	//  This is ladder and sample stage 1
	//

	int status = 0;

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel) {

			if (mDataChannels [i]->FitAllCharacteristicsSM (text, ExcelText, msg, print) < 0) {

				ErrorString << mDataChannels [i]->GetError ();
				status = -i;
			}

	//		mDataChannels [i]->ClearAllPeaksBelowAnalysisThreshold ();
		}
	}

	return status;
}


int CoreBioComponent :: FitNonLaneStandardNegativeCharacteristicsSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	//
	//  This is sample stage 1
	//

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel) {

			mDataChannels [i]->FitAllNegativeCharacteristicsSM (text, ExcelText, msg, print);
		}
	}

	return 0;
}


int CoreBioComponent :: AssignSampleCharacteristicsToLociSM (CoreBioComponent* grid, CoordinateTransform* timeMap) {

	//
	//  This is sample stage 1
	//

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels [i]->AssignSampleCharacteristicsToLociSM (grid, timeMap);
	}

	return 0;
}


int CoreBioComponent :: AssignSampleCharacteristicsToLociSMLF () {

	//
	//  This is ladder free sample stage 1
	//

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels [i]->AssignSampleCharacteristicsToLociSMLF ();
	}

	return 0;
}


int CoreBioComponent :: TestForNearlyDuplicateAllelesSMLF () {

	//
	//  This is ladder free sample stage 3
	//

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels [i]->TestForNearlyDuplicateAllelesSMLF ();
	}

	return 0;
}


int CoreBioComponent :: AnalyzeLaneStandardChannelSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	//
	//  This is ladder and sample stage 1
	//

	int status;
	status =  mDataChannels [mLaneStandardChannel]->HierarchicalLaneStandardChannelAnalysisSM (text, ExcelText, msg, print);

	if (status < 0)
		ErrorString << "Could not analyze lane standard:  " << mDataChannels [mLaneStandardChannel]->GetError ();

	else {

		RGDList ilsList = mDataChannels [mLaneStandardChannel]->GetFinalCurveList ();
		DataSignal* lastILS = (DataSignal*)ilsList.Last ();
		mQC.mWidthOfLastILSPeak = lastILS->GetWidth ();
		mQC.mLastILSTime = lastILS->GetMean ();
		DataSignal* firstILS = (DataSignal*)ilsList.First ();
		mQC.mFirstILSTime = firstILS->GetMean ();
		mQC.mNumberOfSamples = mDataChannels [mLaneStandardChannel]->GetNumberOfSamples ();
	}

	return status;
}


int CoreBioComponent :: AssignCharacteristicsToLociSM () {

	//
	//  This is ladder stage 1
	//

	int status = 0;
	
	for (int i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel) {

			if (!mDataChannels [i]->AssignCharacteristicsToLociSM (mLSData)) {

				ErrorString << mDataChannels [i]->GetError ();
				status = -1;
			}
		}
	}

	return status;
}


int CoreBioComponent :: AnalyzeGridLociSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int CoreBioComponent :: AnalyzeSampleLociSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int CoreBioComponent :: AnalyzeCrossChannelSM () {

	return 0;
}


int CoreBioComponent :: AnalyzeCrossChannelWithNegativePeaksSM () {

	return 0;
}


int CoreBioComponent :: AnalyzeCrossChannelUsingPrimaryWidthAndNegativePeaksSM () {

	return 0;
}


int CoreBioComponent :: UseChannelPatternsToAssessCrossChannelWithNegativePeaksSM (RGDList*** notPrimaryLists) {

	return 0;
}


bool CoreBioComponent::CollectDataAndComputeCrossChannelEffectForChannelsSM (int primaryChannel, int pullupChannel, RGDList* primaryChannelPeaks, double& linearPart, double& quadraticPart, bool testLaserOffScale, bool testNegativePUOnly) {

	//
	// Sample Stage 1
	//

	list<InterchannelLinkage*>::const_iterator it;
	InterchannelLinkage* nextLink;
	InterchannelLinkage* iChannel;
	DataSignal* primarySignal;
	DataSignal* secondarySignal;
	DataSignal* nextSignal;
	list<PullupPair*> pairList;
	list<InterchannelLinkage*> tempLinkageList;
	PullupPair* nextPair;
	double minHeight = 0.0;
	double maxHeight = 0.0;
	bool hasNegativePullup = false;
	double currentPeak;
	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	smPartialPullupBelowMinRFU partialPullupBelowMin;
	smLaserOffScale laserOffScale;
	smCraterSidePeak sidePeak;
	smSigmoidalSidePeak sigmoidalSidePeak;
	smPrimaryInterchannelLink primaryPullup;
	smWeakPrimaryInterchannelLink weakPrimaryPullup;
	smZeroPullupPrimaryInterchannelLink zeroPullupPrimary;
	smSelectUserSpecifiedMinRFUForPrimaryPeakPreset selectUserSpecifiedMinPrimary;
	double currentRatio;
	double minRatio = 1.0;
	double maxRatio = 0.0;
	double numerator;
	//double minRFUForSecondaryChannel = 0.5 * (mDataChannels [pullupChannel]->GetDetectionThreshold () + mDataChannels [pullupChannel]->GetMinimumHeight ());  // Changed 11/25/2020
	double minRFUForSecondaryChannel = mDataChannels [pullupChannel]->GetDetectionThreshold ();
	double noiseLevelForSecondaryChannel = mDataChannels [pullupChannel]->GetNoiseRange ();
	double percentNoiseLevel = 100.0;
	RGDList ignore;
	double maxLaserInScalePeak = 0.0;
	bool testNonLaserOffScale = !testLaserOffScale;

	bool minRatioLessThan1 = false;
	bool maxRatioLargerThan0 = false;
	double primaryThreshold = CoreBioComponent::minPrimaryPullupThreshold;  // This is user-specified threshold if user says to use it.  Otherwise, this is 3.0
	double minPullupHeight = 0.0;
	double rawHeight;
	double detectionThreshold = mDataChannels [pullupChannel]->GetDetectionThreshold ();
	double primaryMinRFU = mDataChannels [primaryChannel]->GetMinimumHeight ();  // This is minRFU for primary channel
	bool atLeastOnePositivePullupAboveDetection = false;
	int minimumNumberOfSamples;

	RGDListIterator channelIterator (*primaryChannelPeaks);

	noiseLevelForSecondaryChannel = 0.01 * percentNoiseLevel * noiseLevelForSecondaryChannel;

	if (testLaserOffScale) {

		minimumNumberOfSamples = 3;
		LeastMedianOfSquares::SetMinimumNumberOfSamples (3);
	}

	else {

		minimumNumberOfSamples = 4;
		LeastMedianOfSquares::SetMinimumNumberOfSamples (4);
	}

	// Modify code below to account for pullup corrections to secondary peaks that are also primary, and maybe to primary peaks?

	//cout << "Chosen (time, primary, pullup) pairs:  ";
	int n = 0;
	int nNegatives = 0;
	int nSigmoids = 0;
	size_t pos = 0;
	int nPos = 0;
	bool mixedPositiveAndNegativePullup = false;
	bool atLeastOneNegativeAboveDetection = false;
	//double factor;
	linearPart = quadraticPart = 0.0;
	//	int nPossibleNegative;
	list<PullupPair*> negativePairs;

	for (it=mInterchannelLinkageList.begin(); it!=mInterchannelLinkageList.end(); it++) {

		nextLink = *it;
		primarySignal = nextLink->GetPrimarySignal ();

		if (primarySignal->GetChannel () != primaryChannel)
			continue;

		bool peakIsLaserOffScale = primarySignal->GetMessageValue (laserOffScale);

		if (testNonLaserOffScale && !peakIsLaserOffScale) {

			currentPeak = primarySignal->Peak ();

			if (currentPeak > maxLaserInScalePeak)
				maxLaserInScalePeak = currentPeak;
		}

		secondarySignal = nextLink->GetSecondarySignalOnChannel (pullupChannel);

		if (secondarySignal == NULL)
			continue;

		if (peakIsLaserOffScale != testLaserOffScale)
			continue;

		if (primarySignal->GetMessageValue (sidePeak)) {

			mPullupFromAnotherChannel.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		if (primarySignal->GetMessageValue (sigmoidalSidePeak)) {

			mPullupFromAnotherChannel.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		if (primarySignal->GetMessageValue (purePullup)) {

			mPullupFromAnotherChannel.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		if (primarySignal->IsNoisySidePeak ()) {

			ignore.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		if (primarySignal->IsDoNotCall ()) {

			ignore.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		if (primarySignal->DontLook ()) {

			ignore.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		if (primarySignal->Peak () <= secondarySignal->Peak ()) {

			mPullupFromAnotherChannel.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		if (primarySignal->GetMessageValue (pullup)) {

			//
			// create pair and add to local list to use later or clear
			//

			mPullupFromAnotherChannel.InsertWithNoReferenceDuplication (primarySignal);
			continue;
		}

		secondarySignal->ResetIgnoreWidthTest ();

		//if (secondarySignal->TestForIntersectionWithPrimary (primarySignal)) {

		//	secondarySignal->IgnoreWidthTest ();
		//	continue;
		//}

		//if (primarySignal->GetWidth () < secondarySignal->GetWidth ())
		//	continue;

		nextPair = new PullupPair (primarySignal, secondarySignal);

		if (secondarySignal->IsNegativePeak ()) {

			//if (secondarySignal->GetCurveFit () < 0.999) {

			//	delete nextPair;
			//	continue;
			//}

			hasNegativePullup = true;
			negativePairs.push_back (nextPair);
			nNegatives++;

			if (secondarySignal->Peak () > detectionThreshold)
				atLeastOneNegativeAboveDetection = true;
		}

		else {

			nPos++;

			if (secondarySignal->Peak () >= detectionThreshold)
				atLeastOnePositivePullupAboveDetection = true;

			if (secondarySignal->IsSigmoidalPeak ())
				nSigmoids++;
		}

		pairList.push_back (nextPair);
		currentPeak = primarySignal->Peak ();

		if (minHeight == 0.0)
			minHeight = currentPeak;

		else if (currentPeak < minHeight)
			minHeight = currentPeak;

		if (currentPeak > maxHeight)
			maxHeight = currentPeak;

		numerator = secondarySignal->Peak ();

		//if (n > 0)
		//	cout << ", ";

		//if (secondarySignal->IsNegativePeak ())
		//	factor = -1.0;

		//else
		//	factor = 1.0;

		//cout << "(" << primarySignal->GetMean () << ", " << currentPeak << ", " << factor * numerator << ")";
		n++;

		if (numerator > 3.0) {

			currentRatio = numerator / currentPeak;

			if (currentRatio < minRatio)
				minRatio = currentRatio;

			if (currentRatio > maxRatio)
				maxRatio = currentRatio;

			if (minPullupHeight == 0.0)
				minPullupHeight = numerator;

			else if (numerator < minPullupHeight)
				minPullupHeight = numerator;
		}
	}

	int additionalPairsRequired = 0;

	if (minRatio < 1.0) {

		//cout << "Minimum pullup ratio = " << minRatio << endl;
		minRatioLessThan1 = true;
	}

	if (maxRatio > 0.0) {

		maxRatioLargerThan0 = true;
	}

	if (testNonLaserOffScale) {

		while (nextSignal = (DataSignal*)channelIterator ()) {

			// Look for raw data primary pullups, classify if positive or negative, form new PullupPair and add to lists

			if (nextSignal->GetChannel () != primaryChannel)
				continue;

			if (nextSignal->GetMessageValue (laserOffScale) != false)
				continue;

			currentPeak = nextSignal->Peak ();

			if (currentPeak > maxLaserInScalePeak)
				maxLaserInScalePeak = currentPeak;
		}

		mDataChannels [primaryChannel]->SetMaxInScalePeak (maxLaserInScalePeak);
	}

	if (pairList.empty ()) {

		//cout << "There are no pullup peaks..." << endl;
		SetPullupTestedMatrix (primaryChannel, pullupChannel, true);
		SetLinearPullupMatrix (primaryChannel, pullupChannel, 0.0);
		SetQuadraticPullupMatrix (primaryChannel, pullupChannel, 0.0);
		mPattern [primaryChannel] [pullupChannel] = false;
		ignore.Clear ();
		return true;
	}

	smPrimaryInterchannelLink primaryLink;
	bool trulyMixedPositiveAndNegativePullup;

	if (atLeastOnePositivePullupAboveDetection && atLeastOneNegativeAboveDetection) {

		mixedPositiveAndNegativePullup = true;
		cout << "Primary channel " << primaryChannel << " into pullup chanel " << pullupChannel << " is mixed pullup:  with " << nNegatives << " negatives and " << pairList.size () - nNegatives << " positives\n";
	}

	else
		cout << "Primary channel " << primaryChannel << " into pullup chanel " << pullupChannel << " is non-mixed pullup" << " with " << pairList.size () << " total pairs\n";


	trulyMixedPositiveAndNegativePullup = mixedPositiveAndNegativePullup;

//	if (!atLeastOnePositivePullupAboveDetection)
//		LeastMedianOfSquares::SetMinimumNumberOfSamples (3);  //***** Why do this when we won't report in this case?  If the sigmoid(s) are high enough, we won't come here and, if not, we don't need to analyze.   Fixed:  11/25/2020

	if (!atLeastOnePositivePullupAboveDetection) {

		mPattern [primaryChannel] [pullupChannel] = false;
		SetPullupTestedMatrix(primaryChannel, pullupChannel, true);
		SetLinearPullupMatrix(primaryChannel, pullupChannel, 0.0);
		SetQuadraticPullupMatrix(primaryChannel, pullupChannel, 0.0);
		ignore.Clear ();

		while (!pairList.empty()) {

			nextPair = pairList.front();

			primarySignal = nextPair->mPrimary;
			secondarySignal = nextPair->mPullup;

			if (secondarySignal != NULL) {

				primarySignal->RemoveProbablePullup (secondarySignal);
				secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

				if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

					secondarySignal->SetMessageValue (pullup, false);
				}
			}

			InterchannelLinkage* iChannelPrimary = primarySignal->GetInterchannelLink ();

			if (iChannelPrimary == NULL) {

				primarySignal->SetMessageValue (primaryLink, false);
				continue; // error?
			}

			primarySignal->RemoveProbablePullup (secondarySignal);
			iChannelPrimary->RemoveDataSignalFromSecondaryList (secondarySignal);
			secondarySignal->SetPrimarySignalFromChannel (pullupChannel, NULL, mNumberOfChannels);

			if (iChannelPrimary->IsEmpty ()) {

				primarySignal->SetInterchannelLink (NULL);
				primarySignal->SetMessageValue (primaryLink, false);
				//delete iChannelPrimary;
				tempLinkageList.push_back (iChannelPrimary);
			}

			delete nextPair;
			pairList.pop_front();
		}

		while (!tempLinkageList.empty ()) {

			nextLink = tempLinkageList.front ();
			tempLinkageList.pop_front ();
			mInterchannelLinkageList.remove (nextLink);
			delete nextLink;
		}

		for (it=mInterchannelLinkageList.begin(); it!=mInterchannelLinkageList.end(); it++) {

			nextLink = *it;
			primarySignal = nextLink->GetPrimarySignal ();

			if (primarySignal->GetChannel () != primaryChannel)
				continue;

			if (primarySignal->GetMessageValue (laserOffScale) != testLaserOffScale)
				continue;

			InterchannelLinkage* iChannelPrimary = primarySignal->GetInterchannelLink ();
			secondarySignal = nextLink->GetSecondarySignalOnChannel (pullupChannel);

			if (secondarySignal != NULL) {

				primarySignal->RemoveProbablePullup (secondarySignal);
				secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

				if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

					secondarySignal->SetMessageValue (pullup, false);
				}

				if (iChannelPrimary != NULL) {

					iChannelPrimary->RemoveDataSignalFromSecondaryList (secondarySignal);

					if (iChannelPrimary->IsEmpty ()) {

						primarySignal->SetMessageValue (primaryLink, false);
						tempLinkageList.push_back (nextLink);
						//delete iChannelPrimary;
						//iChannelPrimary = NULL;
						primarySignal->SetInterchannelLink (NULL);
					}
				}

				else
					primarySignal->SetMessageValue (primaryLink, false);
			}
		}

		while (!tempLinkageList.empty ()) {

			nextLink = tempLinkageList.front ();
			tempLinkageList.pop_front ();
			mInterchannelLinkageList.remove (nextLink);
			delete nextLink;
		}

		return true;
	}

	// Add in raw data only pullup here

	RGDList rawDataPullupPrimaries;  // add raw data primary pullup peaks here so can test later to see if a peak falls in this category
	RGDList occludedDataPrimaries;  // add primary peaks whose potential pullups are occluded by a nearby peak, but not near enough to cause actual pullup
	RGDList noPullupPrimaries;  // add non-primary peaks which are not occluded, have no paired pullup and have no raw data pullup

	// Perform additional tests to see if hasNegativePullup accurately reflects reality... (10/16/2016)
	
	channelIterator.Reset ();
	smRawDataPrimaryInterchannelLink rawDataPrimary;

	while (nextSignal = (DataSignal*) channelIterator ()) {

		// Look for raw data primary pullups, classify if positive or negative, form new PullupPair and add to lists

		if (nextSignal->GetMessageValue (laserOffScale) != testLaserOffScale)
				continue;

		if (ignore.ContainsReference (nextSignal))
			continue;

		if (mPullupFromAnotherChannel.ContainsReference (nextSignal))
			continue;

		if (nextSignal->GetMessageValue (pullup))   // we leave this in because if the "primary" is also a pullup, we can't know if a cross channel effect is due to this peak
			continue;

		if (nextSignal->GetMessageValue (sidePeak))
			continue;

		if (nextSignal->IsNegativePeak ()) {

			ignore.InsertWithNoReferenceDuplication (nextSignal);
			continue;
		}

		if (nextSignal->Peak () < primaryMinRFU)
			continue;

		currentPeak = nextSignal->Peak ();

		if (nextSignal->HasWeakPullupInChannel (pullupChannel)) {

			//weakPullupPeaks.push_back (nextSignal);
			//nextSignal->SetMessageValue (weakPrimaryPullup, true);  // call message here and add data even if no pullup found to expose the pattern, whatever it is
			//RGString data;
			//data << pullupChannel;
			//nextSignal->AppendDataForSmartMessage (weakPrimaryPullup, data);
			occludedDataPrimaries.Append (nextSignal);
		}

		else if (TestMaxAbsoluteRawDataInInterval (pullupChannel, nextSignal->GetMean (), 0.7 * nextSignal->GetWidth (), 0.75, rawHeight)) {  // Modify min primary and min ratio based on these...

			if ((rawHeight < 0.0) && mixedPositiveAndNegativePullup) {   //  This is modified from positive to negative on 02/11/2121.  Including positive raw data pull-up essentially invalidates the whole curve-fitting and noise rejection
				                     //  character of the Osiris data analysis.  this is the difference between this version and a previous 2.15 beta.  I'm sure it was regarded as a bug, but, it was not a bug.
				                     //  It was intended.  If a "hign noise" region was not fit as a peak, it's because it isn't one and the pull-up algorithm should not be applied to it.  (If it were truly pull-up,
				                     //  it would almost certainly be better formed.  So, then why allow raw data negative peaks?  Negative peaks are subject to stricter fit conditions than positive ones, so a raw-data
				                     //  negative peak may actually represent a poor fit negative peak that was rejected.  The reason for this is that noise and positive peaks can interfere more with negative peaks than is 
				                     //  likely for positive ones.

				//  The other problem with including positive raw data "peaks" is that they tend to bias the pull-up ratio higher, because purported raw data pull-up peaks tend to arise in situations in which the primary peak
				//  is shorter compared to the noise peak.  This makes spurious pure pull-up calls more likely.  It was just such a case that gave rise to the investigation that uncovered this discrepancy between the previous 
				//  Version-2.15 and this one.

				if (currentPeak <= abs (rawHeight)) {

						mPullupFromAnotherChannel.InsertWithNoReferenceDuplication (nextSignal);
						continue;
				}

				nextPair = new PullupPair (nextSignal, rawHeight);
				rawDataPullupPrimaries.Append (nextSignal);
				pairList.push_back (nextPair);

				if (currentPeak < minHeight)
					minHeight = currentPeak;

				if (currentPeak > maxHeight)
					maxHeight = currentPeak;

				nNegatives++;
				negativePairs.push_back (nextPair);
				mixedPositiveAndNegativePullup = true;
			}

			//else if ((rawHeight < 0.0) && (currentPeak <= abs (rawHeight)))   //  Removed 02/11/2121:  see above
			//	ignore.InsertWithNoReferenceDuplication (nextSignal);

			// If no negative pullup pairs and rawHeight < 0, should we add it to the list?  12/2/2020...Now it's 12/9/2020 and I don't think so.

			//else if ((rawHeight < 0.0) && mixedPositiveAndNegativePullup) {

			//	nextPair = new PullupPair (nextSignal, rawHeight);
			//	rawDataPullupPrimaries.Append (nextSignal);
			//	pairList.push_back (nextPair);

			//	if (currentPeak < minHeight)
			//		minHeight = currentPeak;

			//	if (currentPeak > maxHeight)
			//		maxHeight = currentPeak;

			//	nNegatives++;
			//	negativePairs.push_back (nextPair);
			//}
		}
	}

	smUseNonlinearLMSAlgorithmForAllChannels useNonLinearLMSMessage;
	bool useNonlinearLMS = GetMessageValue (useNonLinearLMSMessage);

	if (useNonlinearLMS && (pairList.size () >= 5))
		mixedPositiveAndNegativePullup = true;

	RGDListIterator itRaw (rawDataPullupPrimaries);

	if (!mixedPositiveAndNegativePullup) {

		while (nextSignal = (DataSignal*)itRaw ()) {

			TestMaxAbsoluteRawDataInInterval (pullupChannel, nextSignal->GetMean (), 0.7 * nextSignal->GetWidth (), 0.75, rawHeight);  // Find a way to avoid calling this twice.

			currentPeak = nextSignal->Peak ();

			if (currentPeak >= 0.9 * maxHeight) {

				additionalPairsRequired += 2;
				nextPair = new PullupPair (nextSignal, rawHeight);
				pairList.push_back (nextPair);
				nextPair = new PullupPair (nextSignal, rawHeight);
				pairList.push_back (nextPair);
				RGString data;
				data << pullupChannel << "(3)";
				//nextSignal->AppendDataForSmartMessage (rawDataPrimary, data);
				nextSignal->SetTempDataForPrimaryRawDataPullup (data);
			}

			else if (currentPeak >= minHeight) {

				additionalPairsRequired += 1;
				nextPair = new PullupPair (nextSignal, rawHeight);
				pairList.push_back (nextPair);
				//				cout << "Sample File " << (char*)mFileName.GetData () << " has 1 extra primary from channel " << primaryChannel << " into " << pullupChannel << " at time = " << nextSignal->GetMean () << "\n";
				RGString data;
				data << pullupChannel << "(2)";
				//nextSignal->AppendDataForSmartMessage (rawDataPrimary, data);
				nextSignal->SetTempDataForPrimaryRawDataPullup (data);
			}

			else {

				RGString data;
				data << pullupChannel << "(1)";
				//nextSignal->AppendDataForSmartMessage (rawDataPrimary, data);
				nextSignal->SetTempDataForPrimaryRawDataPullup (data);
			}
		}
	}

	negativePairs.clear ();  // we're not using this anyway...

	//minRatioLessThan1 = false;   // Consider removing this and the next line...
	//minRatio = 1.1;

	int numberOfOriginalPairedPeaks = pairList.size ();

	//list<DataSignal*> weakPullupPeaks;
	//if (!hasNegativePullup && !testLaserOffScale) {  // Do we really want to skip this step if there is negative pullup???  I think not

	if (!testLaserOffScale) {

		// recruit additional peaks from channel list that may be tall enough to be primary pullup but are not included
		// because there was no cross channel effect

		double threshold = 0.75 * minHeight;
		//cout << "Adding unpaired signals (time, primary):  ";

		double minPullupThreshold = 0.75 * minRFUForSecondaryChannel;

		//if (minPullupThreshold < minRFUForSecondaryChannel)
		//	minRFUForSecondaryChannel = minPullupThreshold;

		channelIterator.Reset ();

		while (nextSignal = (DataSignal*) channelIterator ()) {

			if (nextSignal->GetMessageValue (laserOffScale) != testLaserOffScale)
				continue;

			if (nextSignal->GetMessageValue (pullup))
				continue;

			if (nextSignal->GetMessageValue (sidePeak))
				continue;

			if (rawDataPullupPrimaries.ContainsReference (nextSignal))
				continue;

			if (occludedDataPrimaries.ContainsReference (nextSignal))
				continue;

			if (mPullupFromAnotherChannel.ContainsReference (nextSignal))
				continue;

			if (ignore.ContainsReference (nextSignal))
				continue;

			//if (nextSignal->Peak () < primaryThreshold)
			//	continue;

			currentPeak = nextSignal->Peak ();

			bool aboveMinHeight = (currentPeak >= minHeight);
			bool aboveThreshold = (currentPeak > threshold);
			bool aboveMinPullup = (minRatio * currentPeak >= minPullupThreshold);
			bool aboveMinRFU = (currentPeak > primaryMinRFU);

			// the minimum ratio is more likely to be representative of the true pullup (if any) and the noise level on the pullup channel is a better reflection of the likelihood of being able
			// to have found a peak in the pullup position.

			if ((aboveMinHeight || (aboveThreshold && aboveMinPullup && minRatioLessThan1)) && aboveMinRFU) {

				nextPair = new PullupPair (nextSignal);
				pairList.push_back (nextPair);
				nextSignal->SetMessageValue (zeroPullupPrimary, true);
				noPullupPrimaries.Append (nextSignal);

				if ((currentPeak >= 0.9 * maxLaserInScalePeak) && !mixedPositiveAndNegativePullup) {

					additionalPairsRequired += 2;
					nextPair = new PullupPair (nextSignal, true);
					pairList.push_back (nextPair);
					nextPair = new PullupPair (nextSignal, true);
					pairList.push_back (nextPair);
					RGString data;
					data << pullupChannel << "(3)";
					//nextSignal->AppendDataForSmartMessage (zeroPullupPrimary, data);
					nextSignal->SetTempDataForPrimaryNoPullup (data);
				}

				else if ((currentPeak >= minHeight) && !mixedPositiveAndNegativePullup) {

					additionalPairsRequired += 1;
					nextPair = new PullupPair (nextSignal, true);
					pairList.push_back (nextPair);
	//				cout << "Sample File " << (char*)mFileName.GetData () << " has 1 extra primary from channel " << primaryChannel << " into " << pullupChannel << " at time = " << nextSignal->GetMean () << "\n";
					RGString data;
					data << pullupChannel << "(2)";
					//nextSignal->AppendDataForSmartMessage (zeroPullupPrimary, data);
					nextSignal->SetTempDataForPrimaryNoPullup (data);
				}

				else {

					RGString data;
					data << pullupChannel << "(1)";
					//nextSignal->AppendDataForSmartMessage (zeroPullupPrimary, data);
					nextSignal->SetTempDataForPrimaryNoPullup (data);
				}
			}
		}
	}

	double outlierThreshold;
	double leastMedianValue;
	double estimatedMinHeight = minHeight;
	double pullupChannelNoise = 0.5 * mDataChannels [pullupChannel]->GetNoiseRange ();
	int estimatedMinPrimary;
	mMinimumInScalePrimaryPeak [primaryChannel] [pullupChannel] = estimatedMinHeight;
	InterchannelLinkage* iChannelPrimary;
	ignore.Clear ();   //  Does this belong here???????

	//TEST!!!
	//trulyMixedPositiveAndNegativePullup = false;

	if (GetMessageValue (selectUserSpecifiedMinPrimary)) {

		estimatedMinHeight = CoreBioComponent::minPrimaryPullupThreshold;
		FinalizeArtifactCallsGivenCalculatedPrimaryThresholdSM (primaryChannel, pullupChannel, estimatedMinHeight, pairList, noPullupPrimaries, rawDataPullupPrimaries, occludedDataPrimaries);
		mMinimumInScalePrimaryPeak [primaryChannel] [pullupChannel] = estimatedMinHeight;
	}

	else if (!testLaserOffScale && !trulyMixedPositiveAndNegativePullup) {

		size_t position = 0;

		if (!mixedPositiveAndNegativePullup)	
			estimatedMinPrimary = EstimateMinimumPrimaryPullupHeightSM (primaryChannel, pullupChannel, estimatedMinHeight, pairList, pullupChannelNoise);

		else
			estimatedMinPrimary = EstimateMinimumPrimaryPullupHeightQuadraticFitSM (primaryChannel, pullupChannel, estimatedMinHeight, pairList, pullupChannelNoise);

			// Do we need to do anything with the result:  estimatedMinPrimary?  Yes. I think so...

			if (estimatedMinPrimary == 5) {

				// There is no pullup, so cleanup and return

				SetPullupTestedMatrix (primaryChannel, pullupChannel, true);
				SetLinearPullupMatrix (primaryChannel, pullupChannel, 0.0);
				SetQuadraticPullupMatrix (primaryChannel, pullupChannel, 0.0);
				mPattern [primaryChannel] [pullupChannel] = true;
				RGString data;
				//occludedDataPrimaries.Clear ();  // Maybe don't do this...

				while (primarySignal = (DataSignal*)noPullupPrimaries.GetFirst ()) {

					if (primarySignal->Peak () >= 0.50 * maxLaserInScalePeak) {

						data = primarySignal->GetTempDataForPrimaryNoPullup ();
						primarySignal->SetMessageValue (zeroPullupPrimary, true);
						primarySignal->AppendDataForSmartMessage (zeroPullupPrimary, data);
						primarySignal->SetTempDataForPrimaryNoPullup ("");
					}

					else {

						primarySignal->SetMessageValue (zeroPullupPrimary, false);
						primarySignal->SetTempDataForPrimaryNoPullup ("");
					}
				}

				while (primarySignal = (DataSignal*)rawDataPullupPrimaries.GetFirst ()) {

					data = primarySignal->GetTempDataForPrimaryRawDataPullup ();

					if (data.Length () == 0) {

						primarySignal->SetMessageValue (rawDataPrimary, false);
					}

					else {

						primarySignal->SetMessageValue (rawDataPrimary, true);
						primarySignal->AppendDataForSmartMessage (rawDataPrimary, data);
					}

					primarySignal->SetTempDataForPrimaryRawDataPullup ("");
				}

				data = "";
				data << pullupChannel;

				while (primarySignal = (DataSignal*)occludedDataPrimaries.GetFirst ()) {

					if (primarySignal->Peak () >= 0.50 * maxLaserInScalePeak) {

						primarySignal->SetMessageValue (weakPrimaryPullup, true);  // call message here and add data even if no pullup found to expose the pattern, whatever it is
						primarySignal->AppendDataForSmartMessage (weakPrimaryPullup, data);
					}
				}

				while (!pairList.empty()) {

					nextPair = pairList.front();

					primarySignal = nextPair->mPrimary;
					secondarySignal = nextPair->mPullup;

					iChannelPrimary = primarySignal->GetInterchannelLink ();

					delete nextPair;
					pairList.pop_front();

					if (secondarySignal == NULL)
						continue;

					primarySignal->RemoveProbablePullup (secondarySignal);

					secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

					if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

						secondarySignal->SetMessageValue (pullup, false);
						secondarySignal->SetPullupFromChannel (primaryChannel, 0.0, mNumberOfChannels);
					}

					if (iChannelPrimary == NULL) {

						primarySignal->SetMessageValue (primaryLink, false);
						continue; // error??
					}

					iChannelPrimary->RemoveDataSignalFromSecondaryList (secondarySignal);

					if (iChannelPrimary->IsEmpty ()) {

						primarySignal->SetInterchannelLink (NULL);
						primarySignal->SetMessageValue (primaryLink, false);
						//delete iChannelPrimary;
						tempLinkageList.push_back (iChannelPrimary);
					}
				}

				while (!tempLinkageList.empty ()) {

					nextLink = tempLinkageList.front ();
					tempLinkageList.pop_front ();
					mInterchannelLinkageList.remove (nextLink);
					delete nextLink;
				}

				for (it=mInterchannelLinkageList.begin(); it!=mInterchannelLinkageList.end(); it++) {

					nextLink = *it;
					primarySignal = nextLink->GetPrimarySignal ();

					if (primarySignal->GetChannel () != primaryChannel)
						continue;

					if (primarySignal->GetMessageValue (laserOffScale) != testLaserOffScale)
						continue;

					InterchannelLinkage* iChannelPrimary = primarySignal->GetInterchannelLink ();
					secondarySignal = nextLink->GetSecondarySignalOnChannel (pullupChannel);

					if (secondarySignal != NULL) {

						primarySignal->RemoveProbablePullup (secondarySignal);
						secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

						if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

							secondarySignal->SetMessageValue (pullup, false);
						}

						if (iChannelPrimary != NULL) {

							iChannelPrimary->RemoveDataSignalFromSecondaryList (secondarySignal);

							if (iChannelPrimary->IsEmpty ()) {

								primarySignal->SetMessageValue (primaryLink, false);
								tempLinkageList.push_back (nextLink);
								//delete iChannelPrimary;
								//iChannelPrimary = NULL;
								primarySignal->SetInterchannelLink (NULL);
							}
						}

						else
							primarySignal->SetMessageValue (primaryLink, false);
					}
				}

				while (!tempLinkageList.empty ()) {

					nextLink = tempLinkageList.front ();
					tempLinkageList.pop_front ();
					mInterchannelLinkageList.remove (nextLink);
					delete nextLink;
				}

				return true;

			}

			else if (estimatedMinPrimary == -1) {

				// Insufficient data for pattern
				// Remove peaks from primary list that are below user specified height and proceed to LMS
				// Make a function for this based on passing pairList and the three peak lists, plus height to test for, plus primary channel and pullup channel.
			
				mPattern [primaryChannel] [pullupChannel] = false;

				FinalizeArtifactCallsGivenCalculatedPrimaryThresholdSM (primaryChannel, pullupChannel, primaryThreshold, pairList, noPullupPrimaries, rawDataPullupPrimaries, occludedDataPrimaries);
				mPattern [primaryChannel] [pullupChannel] = false;
			}

			else if (estimatedMinPrimary == 0) {

				// We have data for minimum height
				// Remove primaries that are less than calculated value along with artifact calls
				// Continue to LMS

				FinalizeArtifactCallsGivenCalculatedPrimaryThresholdSM (primaryChannel, pullupChannel, estimatedMinHeight, pairList, noPullupPrimaries, rawDataPullupPrimaries, occludedDataPrimaries);
				mMinimumInScalePrimaryPeak [primaryChannel] [pullupChannel] = estimatedMinHeight;
				cout << "Minimum primary height for channel " << primaryChannel << " pulling up into pullup Channel " << pullupChannel << " = " << estimatedMinHeight << "\n";
			}
	}

	else if (testLaserOffScale){ // this is laser off scale test; get laser in scale max peak height

		maxLaserInScalePeak = mDataChannels [primaryChannel]->GetMaxInScalePeak ();
		list<PullupPair*> tempPairList;

		while (!pairList.empty()) {

			nextPair = pairList.front();

			primarySignal = nextPair->mPrimary;
			secondarySignal = nextPair->mPullup;

			iChannelPrimary = primarySignal->GetInterchannelLink ();
			pairList.pop_front();

			if (primarySignal->Peak () > maxLaserInScalePeak) {

				tempPairList.push_back (nextPair);
				continue;
			}

			if (!primarySignal->GetCouldBePullup ()) {

				tempPairList.push_back (nextPair);
				continue;
			}

			if (mDataChannels [primaryChannel]->GetMaxAbsoluteRawDataInInterval (primarySignal->GetMean (), 3.0) > maxLaserInScalePeak) {

				tempPairList.push_back (nextPair);
				continue;
			}

			delete nextPair;

			if (secondarySignal == NULL)
				continue;

			primarySignal->RemoveProbablePullup (secondarySignal);

			secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

			if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

				secondarySignal->SetMessageValue (pullup, false);
				secondarySignal->SetPullupFromChannel (primaryChannel, 0.0, mNumberOfChannels);
			}

			if (iChannelPrimary == NULL) {

				primarySignal->SetMessageValue (primaryLink, false);
				continue; // error??
			}

			iChannelPrimary->RemoveDataSignalFromSecondaryList (secondarySignal);

			if (iChannelPrimary->IsEmpty ()) {

				primarySignal->SetInterchannelLink (NULL);
				primarySignal->SetMessageValue (primaryLink, false);
				//delete iChannelPrimary;
				tempLinkageList.push_back (iChannelPrimary);
			}
		}

		while (!tempLinkageList.empty ()) {

			nextLink = tempLinkageList.front ();
			tempLinkageList.pop_front ();
			mInterchannelLinkageList.remove (nextLink);
			delete nextLink;
		}

		while (!tempPairList.empty ()) {

			nextPair = tempPairList.front ();
			tempPairList.pop_front ();
			pairList.push_back (nextPair);
		}

		for (it=mInterchannelLinkageList.begin(); it!=mInterchannelLinkageList.end(); it++) {

			nextLink = *it;
			primarySignal = nextLink->GetPrimarySignal ();

			if (primarySignal->GetChannel () != primaryChannel)
				continue;

			if (primarySignal->GetMessageValue (laserOffScale) != true)
				continue;

			if (primarySignal->Peak () > maxLaserInScalePeak) {

				continue;
			}

			if (!primarySignal->GetCouldBePullup ()) {

				continue;
			}

			if (mDataChannels [primaryChannel]->GetMaxAbsoluteRawDataInInterval (primarySignal->GetMean (), 3.0) > maxLaserInScalePeak) {

				continue;
			}

			InterchannelLinkage* iChannelPrimary = primarySignal->GetInterchannelLink ();
			secondarySignal = nextLink->GetSecondarySignalOnChannel (pullupChannel);

			if (secondarySignal != NULL) {

				primarySignal->RemoveProbablePullup (secondarySignal);
				secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

				if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

					secondarySignal->SetMessageValue (pullup, false);
				}

				if (iChannelPrimary != NULL) {

					iChannelPrimary->RemoveDataSignalFromSecondaryList (secondarySignal);

					if (iChannelPrimary->IsEmpty ()) {

						primarySignal->SetMessageValue (primaryLink, false);
						tempLinkageList.push_back (nextLink);
						//delete iChannelPrimary;
						//iChannelPrimary = NULL;
						primarySignal->SetInterchannelLink (NULL);
					}
				}

				else
					primarySignal->SetMessageValue (primaryLink, false);
			}
		}

		while (!tempLinkageList.empty ()) {

			nextLink = tempLinkageList.front ();
			tempLinkageList.pop_front ();
			mInterchannelLinkageList.remove (nextLink);
			delete nextLink;
		}
	}

	ignore.Clear ();

	// Add test for required number of peaks here.  If too few, create uncertain pull-ups, except if half width criterion applies.  If enough call ComputePullupParameers below.
	//  Don't forget to set mPattern appropriately.

	if ((int)pairList.size () < minimumNumberOfSamples) {

		// Set uncertain pullups, clean up and get out
		mPattern [primaryChannel] [pullupChannel] = false;
		SetPullupTestedMatrix(primaryChannel, pullupChannel, true);
		SetLinearPullupMatrix(primaryChannel, pullupChannel, 0.0);
		SetQuadraticPullupMatrix(primaryChannel, pullupChannel, 0.0);
		ignore.Clear ();

		while (!pairList.empty()) {

			nextPair = pairList.front();

			primarySignal = nextPair->mPrimary;
			secondarySignal = nextPair->mPullup;

			if (secondarySignal != NULL) {

				bool secondaryNarrow2 = (secondarySignal->GetWidth () < 0.5 * primarySignal->GetWidth ()) && !secondarySignal->IgnoreWidthTest ();

				//correctedHeight = nextPair->mPullupHeight - pullupPeak->GetTotalPullupFromOtherChannels (mNumberOfChannels);
				//belowMinRFU = (correctedHeight < analysisThreshold);

				if (secondaryNarrow2) {  // Removed secondaryNarrow 12/2/2020

					// This is a pure pullup.  Assign appropriate message and if also a primary, change that status

					secondarySignal->SetMessageValue (purePullup, true);
					secondarySignal->SetMessageValue (pullup, false);
					secondarySignal->SetIsPurePullupFromChannel (primaryChannel, true, mNumberOfChannels);
					double primaryHeight = primarySignal->Peak ();
					double pullupHeight = secondarySignal->Peak ();
					double ratio = pullupHeight / primaryHeight;  //(linearPart + quadraticPart * primaryHeight);
					double pullupLevel = pullupHeight;  //ratio * primaryHeight;

					//ratio = 100.0 * (pullupPeak->Peak () / primarySignal->Peak ());
					secondarySignal->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
					secondarySignal->SetPullupFromChannel (primaryChannel, pullupLevel, mNumberOfChannels);
					secondarySignal->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);

					if (secondarySignal->HasCrossChannelSignalLink ()) {

						// remove link; this is not a primary peak
						RemovePrimaryLinksAndSecondaryLinksFrom (secondarySignal);
					}
				}

				else {

					secondarySignal->SetIsPossiblePullup (true);
					secondarySignal->AddUncertainPullupChannel (primaryChannel);
					secondarySignal->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);
				}
			}

			delete nextPair;
			pairList.pop_front();
		}

		return true;
	}

//	RemovePrimaryLinksForChannelsSM (primaryChannel, pullupChannel, testLaserOffScale, ignore);
//	pullupFromAnotherChannel.Clear ();

	list<PullupPair*>::iterator pairIt;

	for (pairIt=pairList.begin(); pairIt!=pairList.end(); pairIt++) {

		nextPair = *pairIt;
		primarySignal = nextPair->mPrimary;

		if (primarySignal != NULL) {

			double time = primarySignal->GetMean ();
		//	cout << "Primary Time = " << time << "\n";
		}

		else
			cout << "Null primary\n";
	}

	bool answer = ComputePullupParameters (pairList, linearPart, quadraticPart, leastMedianValue, outlierThreshold, mixedPositiveAndNegativePullup);
	
	list<InterchannelLinkage*>::iterator linkIt;
	size_t position = 0;

	// check for all pullup peaks being outliers, vs all non-outliers from 0-peaks
	// what to do if answer = false?
	bool secondaryNarrow2;
	bool halfWidth;
	DataSignal* pullupPeak;
	double ratio;
	list<InterchannelLinkage*> emptyLinks;
	RGDList testedPullups;
	RGString data;

	if (answer) {

		// insert further processing based on answer and clear pairList;

		mPattern [primaryChannel] [pullupChannel] = true;
		SetPullupTestedMatrix (primaryChannel, pullupChannel, true);
		SetLinearPullupMatrix (primaryChannel, pullupChannel, linearPart);
		SetQuadraticPullupMatrix (primaryChannel, pullupChannel, quadraticPart);
		double analysisThreshold = mDataChannels [pullupChannel]->GetMinimumHeight ();
		double pullupThreshold = CoreBioComponent::minPrimaryPullupThreshold;
//		CalculatePullupCorrection (primaryChannel, pullupChannel, pairList, testLaserOffScale);
//		bool belowMinRFU;
		mLeastMedianValue [primaryChannel][pullupChannel] = leastMedianValue;
		mOutlierThreshold [primaryChannel][pullupChannel] = outlierThreshold;
		
		data = "";
		data << pullupChannel;

		while (nextSignal = (DataSignal*) occludedDataPrimaries.GetFirst ()) {

			if (nextSignal->Peak () >= estimatedMinHeight) {

				nextSignal->SetMessageValue (weakPrimaryPullup, true);  // call message here and add data even if no pullup found to expose the pattern, whatever it is
				nextSignal->AppendDataForSmartMessage (weakPrimaryPullup, data);
			}
		}

		while (nextSignal = (DataSignal*)rawDataPullupPrimaries.GetFirst ()) {

			if (nextSignal->Peak () >= estimatedMinHeight) {

				data = nextSignal->GetTempDataForPrimaryRawDataPullup ();
				nextSignal->SetMessageValue (rawDataPrimary, true);	// call message here and add data even if no pullup found to expose the pattern, whatever it is
				nextSignal->AppendDataForSmartMessage (rawDataPrimary, data);  // already done?  No. Don't think so...
				nextSignal->SetTempDataForPrimaryRawDataPullup ("");
			}

			else {

				nextSignal->SetTempDataForPrimaryRawDataPullup ("");

				if (!nextSignal->SmartMessageHasData (rawDataPrimary))
					nextSignal->SetMessageValue (rawDataPrimary, false);
			}
		}

		while (nextSignal = (DataSignal*)noPullupPrimaries.GetFirst ()) {

			if (nextSignal->Peak () >= estimatedMinHeight) {

				data = nextSignal->GetTempDataForPrimaryNoPullup ();
				nextSignal->SetMessageValue (zeroPullupPrimary, true);	// call message here and add data even if no pullup found to expose the pattern, whatever it is
				nextSignal->AppendDataForSmartMessage (zeroPullupPrimary, data);  // already done?  No. Don't think so...
				nextSignal->SetTempDataForPrimaryNoPullup ("");
			}

			else {

				nextSignal->SetTempDataForPrimaryNoPullup ("");
				nextSignal->SetMessageValue (zeroPullupPrimary, false);					
			}
		}
		
		list<DataSignal*> removePrimaryList;
		
		if ((linearPart == 0.0) && (quadraticPart == 0.0)) {  // There is a pattern and it is that there is no pullup

			// remove all cross channel links except those for which pullup is sufficiently narrow

			//weakPullupPeaks.clear ();

			for (it=mInterchannelLinkageList.begin (); it!=mInterchannelLinkageList.end (); it++) {

				iChannel = *it;
				primarySignal = iChannel->GetPrimarySignal ();

				if (primarySignal->GetChannel () != primaryChannel)
					continue;

				if (primarySignal->GetMessageValue (laserOffScale) != testLaserOffScale)
					continue;

				secondarySignal = iChannel->GetSecondarySignalOnChannel (pullupChannel);

				if (secondarySignal == NULL)
					continue;

				secondarySignal->SetPullupFromChannel (primaryChannel, 0.0, mNumberOfChannels);
				secondarySignal->SetPullupRatio (primaryChannel, 0.0, mNumberOfChannels);
				secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

				iChannel->RemoveDataSignalFromSecondaryList (secondarySignal);

				if (iChannel->IsEmpty ()) {

					primarySignal->SetMessageValue (primaryPullup, false);
					primarySignal->SetInterchannelLink (NULL);
					emptyLinks.push_back (iChannel);
				}

				if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

					secondarySignal->SetMessageValue (pullup, false);
					secondarySignal->SetMessageValue (purePullup, false);
					secondarySignal->SetMessageValue (partialPullupBelowMin, false);
				}
			}

			while (!emptyLinks.empty ()) {

				iChannel = emptyLinks.front ();
				emptyLinks.pop_front ();
				mInterchannelLinkageList.remove (iChannel);
				delete iChannel;
			}
		}

		else {  // There is a pattern and there is actual pullup

			for (pairIt=pairList.begin(); pairIt!=pairList.end(); pairIt++) {

				nextPair = *pairIt;
				pullupPeak = nextPair->mPullup;

				if (pullupPeak == NULL)
					continue;

				primarySignal = nextPair->mPrimary;
				//sigmaPrimary = primarySignal->GetStandardDeviation ();  // standard deviations are not reliable measures of width.
				//sigmaSecondary = pullupPeak->GetStandardDeviation ();

				//secondaryNarrow = (sigmaSecondary < 0.5* sigmaPrimary)  && !pullupPeak->IgnoreWidthTest ();
				secondaryNarrow2 = (pullupPeak->GetWidth () < 0.5 * primarySignal->GetWidth ()) && !pullupPeak->IgnoreWidthTest ();
				halfWidth = (secondaryNarrow2) && (testLaserOffScale || primarySignal->IsCraterPeak ());  // removed secondaryNarrow 12/2/2020

				//correctedHeight = nextPair->mPullupHeight - pullupPeak->GetTotalPullupFromOtherChannels (mNumberOfChannels);
				//belowMinRFU = (correctedHeight < analysisThreshold);

				if ((!nextPair->mIsOutlier) || halfWidth) {

					// This is a pure pullup.  Assign appropriate message and if also a primary, change that status

					pullupPeak->SetMessageValue (purePullup, true);
					pullupPeak->SetIsPurePullupFromChannel (primaryChannel, true, mNumberOfChannels);
					pullupPeak->SetMessageValue (pullup, false);
					nextPair->mIsOutlier = false;
					testedPullups.Append (pullupPeak);
					//ratio = 100.0 * (pullupPeak->Peak () / primarySignal->Peak ());
					double primaryHeight = primarySignal->Peak ();
					double pullupHeight = pullupPeak->Peak ();
					ratio = pullupHeight / primaryHeight;  //(linearPart + quadraticPart * primaryHeight);
					pullupPeak->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
					double pullupLevel = pullupHeight;  //ratio * primaryHeight;
					pullupPeak->SetPullupFromChannel (primaryChannel, pullupLevel, mNumberOfChannels);
					pullupPeak->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);

					if (halfWidth && nextPair->mIsOutlier) {

						cout << "Half width criterion (original peaks) triggered for outlier in sample name " << (char*)mName.GetData () << " for channel " << pullupChannel << " at time " << pullupPeak->GetMean () << " for primary ";

						if (testLaserOffScale)
							cout << "laser off-scale ";

						else
							cout << "laser not off-scale ";

						if (primarySignal->IsCraterPeak ())
							cout << "and crater\n";

						else
							cout << "and not crater\n";
					}

					if (pullupPeak->HasCrossChannelSignalLink ()) {

						// remove link; this is not a primary peak
						RemovePrimaryLinksAndSecondaryLinksFrom (pullupPeak);
					}
				}

				else {  // peak is outlier and half width criterion does not apply

					pullupPeak->SetMessageValue (pullup, true);
					nextPair->mIsOutlier = true;
					testedPullups.Append (pullupPeak);
					//ratio = 100.0 * (pullupPeak->Peak () / primarySignal->Peak ());
					double primaryHeight = primarySignal->Peak ();
					ratio = (linearPart + quadraticPart * primaryHeight);
					pullupPeak->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
					double pullupLevel = ratio * primaryHeight;
					pullupPeak->SetPullupFromChannel (primaryChannel, pullupLevel, mNumberOfChannels);
					pullupPeak->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);
				}
			}

			// for other outliers, the jury is still out until all channel contributions are tallied
			removePrimaryList.unique ();
			testedPullups.Clear ();
			//weakPullupPeaks.clear ();

			while (!removePrimaryList.empty ()) {

				pullupPeak = removePrimaryList.front ();
				removePrimaryList.pop_front ();
				RemovePrimaryLinksAndSecondaryLinksFrom (pullupPeak);
			}
		}
	}

	else {

		// Test widths of primaries and secondaries...there is insufficient data to detect a pattern

		data = "";
		data << pullupChannel;
		mPattern [primaryChannel] [pullupChannel] = false;

		while (nextSignal = (DataSignal*)occludedDataPrimaries.GetFirst ()) {

			if (nextSignal->Peak () >= estimatedMinHeight) {

				nextSignal->SetMessageValue (weakPrimaryPullup, true);  // call message here and add data even if no pullup found to expose the pattern, whatever it is
				nextSignal->AppendDataForSmartMessage (weakPrimaryPullup, data);
			}
		}

		while (nextSignal = (DataSignal*)rawDataPullupPrimaries.GetFirst ()) {

			if (nextSignal->Peak () >= estimatedMinHeight) {

				data = nextSignal->GetTempDataForPrimaryRawDataPullup ();
				nextSignal->SetMessageValue (rawDataPrimary, true);	// call message here and add data even if no pullup found to expose the pattern, whatever it is
				nextSignal->AppendDataForSmartMessage (rawDataPrimary, data);  // already done?  No. Don't think so...
				nextSignal->SetTempDataForPrimaryRawDataPullup ("");
			}

			else {

				nextSignal->SetTempDataForPrimaryRawDataPullup ("");

				if (!nextSignal->SmartMessageHasData (rawDataPrimary))
					nextSignal->SetMessageValue (rawDataPrimary, false);
			}
		}

		while (nextSignal = (DataSignal*)noPullupPrimaries.GetFirst ()) {

			if (nextSignal->Peak () >= estimatedMinHeight) {

				data = nextSignal->GetTempDataForPrimaryNoPullup ();
				nextSignal->SetMessageValue (zeroPullupPrimary, true);	// call message here and add data even if no pullup found to expose the pattern, whatever it is
				nextSignal->AppendDataForSmartMessage (zeroPullupPrimary, data);  // already done?  No. Don't think so...
				nextSignal->SetTempDataForPrimaryNoPullup ("");
			}

			else {

				nextSignal->SetTempDataForPrimaryNoPullup ("");
				nextSignal->SetMessageValue (zeroPullupPrimary, false);
			}
		}

		for (pairIt=pairList.begin(); pairIt!=pairList.end(); pairIt++) {

			nextPair = *pairIt;
			pullupPeak = nextPair->mPullup;

			if (pullupPeak == NULL)
				continue;

			primarySignal = nextPair->mPrimary;
			//sigmaPrimary = primarySignal->GetStandardDeviation ();
			//sigmaSecondary = pullupPeak->GetStandardDeviation ();

			//secondaryNarrow = (sigmaSecondary < 0.5* sigmaPrimary) && !pullupPeak->IgnoreWidthTest ();
			secondaryNarrow2 = (pullupPeak->GetWidth () < 0.5 * primarySignal->GetWidth ()) && !pullupPeak->IgnoreWidthTest ();

			//correctedHeight = nextPair->mPullupHeight - pullupPeak->GetTotalPullupFromOtherChannels (mNumberOfChannels);
			//belowMinRFU = (correctedHeight < analysisThreshold);

			if (secondaryNarrow2) {  // Removed secondaryNarrow 12/2/2020

				// This is a pure pullup.  Assign appropriate message and if also a primary, change that status

				pullupPeak->SetMessageValue (purePullup, true);
				pullupPeak->SetMessageValue (pullup, false);
				pullupPeak->SetIsPurePullupFromChannel (primaryChannel, true, mNumberOfChannels);
				double primaryHeight = primarySignal->Peak ();
				double pullupHeight = pullupPeak->Peak ();
				ratio = pullupHeight / primaryHeight;  //(linearPart + quadraticPart * primaryHeight);
				double pullupLevel = pullupHeight;  //ratio * primaryHeight;

				//ratio = 100.0 * (pullupPeak->Peak () / primarySignal->Peak ());
				pullupPeak->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
				pullupPeak->SetPullupFromChannel (primaryChannel, pullupLevel, mNumberOfChannels);
				pullupPeak->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);

				if (pullupPeak->HasCrossChannelSignalLink ()) {

					// remove link; this is not a primary peak
					RemovePrimaryLinksAndSecondaryLinksFrom (pullupPeak);
				}
			}

			else {

				pullupPeak->SetIsPossiblePullup (true);
				pullupPeak->AddUncertainPullupChannel (primaryChannel);
				pullupPeak->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);
			}
		}
	}

	while (!pairList.empty ()) {

		nextPair = pairList.front ();
		delete nextPair;
		pairList.pop_front ();
	}

	return answer;
}


int CoreBioComponent :: EstimateMinimumPrimaryPullupHeightSM (int primaryChannel, int pullupChannel, double& estimatedMinHeight, list<PullupPair*>& pairList, double pullupChannelNoise) {

	//
	//  Phase 1 for samples only.  Part of pull-up algorithm.  Returns -1 if insufficient information; returns 0 if the pattern is 0 pull-up and returns 1 if sufficient info to restrict primary pull-up height.
	//

	PullupPair* nextPair;
//	DataSignal* pullupPeak;
	DataSignal* primaryPeak;
	double primaryHeight;
	double topFiveHeights [5];
	double topFiveRatios [5];
	int i;

	if (pairList.size() < 4) {

		estimatedMinHeight = 0.0;
		return -1;
	}

	PullupPair** topFivePairs = new PullupPair* [5];
	std::list<PullupPair*>::iterator pairIt;

	for (i = 0; i < 5; i++) {

		topFivePairs [i] = NULL;
		topFiveHeights [i] = 0.0;
		topFiveRatios [i] = 0.0;
	}

	for (pairIt = pairList.begin(); pairIt != pairList.end(); pairIt++) {

		nextPair = *pairIt;
		primaryPeak = nextPair->mPrimary;

		if (primaryPeak == NULL)
			continue;

		primaryHeight = primaryPeak->Peak ();

		if (primaryHeight >= topFiveHeights [0]) {

			for (i=4; i>=1; i--) {

				topFiveHeights [i] = topFiveHeights [i - 1];
				topFivePairs [i] = topFivePairs [i - 1];
			}

			topFiveHeights [0] = primaryHeight;
			topFivePairs [0] = nextPair;
		}

		else if (primaryHeight >= topFiveHeights [1]) {

			for (i=4; i>=2; i--) {

				topFiveHeights [i] = topFiveHeights [i - 1];
				topFivePairs [i] = topFivePairs [i - 1];
			}

			topFiveHeights [1] = primaryHeight;
			topFivePairs [1] = nextPair;
		}

		else if (primaryHeight >= topFiveHeights [2]) {

			for (i=4; i>=3; i--) {

				topFiveHeights [i] = topFiveHeights [i - 1];
				topFivePairs [i] = topFivePairs [i - 1];
			}

			topFiveHeights [2] = primaryHeight;
			topFivePairs [2] = nextPair;
		}

		else if (primaryHeight >= topFiveHeights [3]) {

			topFiveHeights [4] = topFiveHeights [3];
			topFivePairs [4] = topFivePairs [3];

			topFiveHeights [3] = primaryHeight;
			topFivePairs [3] = nextPair;
		}

		else if (primaryHeight >= topFiveHeights [4]) {

			topFiveHeights [4] = primaryHeight;
			topFivePairs [4] = nextPair;
		}
	}

	int nPeaks = 0;

	for (i=0; i<5; i++) {

		if (topFivePairs [i] != NULL)
			nPeaks++;

		else
			break;
	}

	if (nPeaks < 4) {

		estimatedMinHeight = 0.0;
		delete[] topFivePairs;
		return -1;
	}

	// Now calculate median ratio for these 4 or 5 peaks

	list<double> ratioList;
	double temp;
	double ratios [5];

	for (i=0; i<nPeaks; i++) {

		temp = topFivePairs [i]->mPullupHeight / topFiveHeights [i];
		ratioList.push_back (temp);
	}

	ratioList.sort ();
	int nRanges = nPeaks - 2;

	for (i=0; i<nPeaks; i++) {

		ratios [i] = ratioList.front ();
		ratioList.pop_front ();
	}

	double minRange = 1.1;
	double nextRange;
	int bestRangeIndex = 0;;

	for (i=0; i<nRanges; i++) {

		nextRange = ratios [i+2] - ratios [i];

		if (nextRange < minRange) {

			minRange = nextRange;
			bestRangeIndex = i;
		}
	}

	double medianRatio = 0.5 * (ratios [bestRangeIndex] + ratios [bestRangeIndex + 2]);
	double absMedianRatio = abs (medianRatio);
	list<PullupPair*> tempPairs;

	if (absMedianRatio > 0.0) {

		// Calculate estimated minimum height and remove all pairs and solos from pairList with primary lower than that height
		// Then delete topFivePairs and return 1

		estimatedMinHeight = pullupChannelNoise / absMedianRatio;

		//while (!pairList.empty ()) {

		//	nextPair = pairList.front ();
		//	pairList.pop_front ();
		//	primaryHeight = nextPair->mPrimaryHeight;

		//	if (primaryHeight < estimatedMinHeight) {

		//		delete nextPair;
		//		continue;
		//	}

		//	tempPairs.push_back (nextPair);
		//}

		//while (!tempPairs.empty ()) {

		//	nextPair = tempPairs.front ();
		//	tempPairs.pop_front ();
		//	pairList.push_back (nextPair);
		//}

		delete[] topFivePairs;
		return 0;
	}

	else {

		// There is no pullup.  This happens when 3+ of the top 5 peaks have no pullup, so set estimated minimum height to 0.0, delete topFivePairs and return 0
		estimatedMinHeight = 0.0;
		delete[] topFivePairs;
		return 5;
	}

	return 0;
}


int CoreBioComponent::EstimateMinimumPrimaryPullupHeightQuadraticFitSM (int primaryChannel, int pullupChannel, double& estimatedMinHeight, list<PullupPair*>& pairList, double pullupChannelNoise) {

	//
	//  Phase 1 for samples only.  Part of pull-up algorithm.  Returns -1 if insufficient information; returns 5 if the pattern is 0 pull-up and returns 0 if sufficient info to restrict primary pull-up height.
	//

	PullupPair* nextPair;
	DataSignal* pullupPeak;
	DataSignal* primaryPeak;
	double primaryHeight;
	double topFiveHeights [5];
	double topFiveRatios [5];
	int i;
	double linearCoefficient = 0.0;
	double quadraticCoefficient = 0.0;
	double maxPrimary = 0.0;

	if (pairList.size() <= 4) {

		return EstimateMinimumPrimaryPullupHeightSM (primaryChannel, pullupChannel, estimatedMinHeight, pairList, pullupChannelNoise);
	}

	PullupPair** topFivePairs = new PullupPair* [5];
	std::list<PullupPair*>::iterator pairIt;

	double minPullupHeight = 0.0;
	double pullupHeight;
	list<double> primaries;
	list<double> pullups;
	double minPrimaryHeightWithNegativePU = 0.0;

	for (i = 0; i < 5; i++) {

		topFivePairs [i] = NULL;
		topFiveHeights [i] = 0.0;
		topFiveRatios [i] = 0.0;
	}

	for (pairIt = pairList.begin(); pairIt != pairList.end(); pairIt++) {

		nextPair = *pairIt;
		primaryPeak = nextPair->mPrimary;

		if (primaryPeak == NULL)
			continue;

		if ((nextPair->mPullup == NULL) || (nextPair->mPullupHeight == 0.0))  // test
			continue;

		primaryHeight = primaryPeak->Peak ();
		pullupHeight = nextPair->mPullupHeight;

		if (pullupHeight < 0.0) {

			if (minPrimaryHeightWithNegativePU == 0.0)
				minPrimaryHeightWithNegativePU = primaryHeight;

			else if (primaryHeight < minPrimaryHeightWithNegativePU)
				minPrimaryHeightWithNegativePU = primaryHeight;
		}

		if (primaryHeight > maxPrimary)
			maxPrimary = primaryHeight;

		if (minPullupHeight == 0.0)
			minPullupHeight = pullupHeight;

		else if (pullupHeight < minPullupHeight)
			minPullupHeight = pullupHeight;

		primaries.push_back (primaryHeight);
		pullups.push_back (pullupHeight);

		//if (primaryHeight >= topFiveHeights [0]) {

		//	for (i=4; i>=1; i--) {

		//		topFiveHeights [i] = topFiveHeights [i - 1];
		//		topFivePairs [i] = topFivePairs [i - 1];
		//	}

		//	topFiveHeights [0] = primaryHeight;
		//	topFivePairs [0] = nextPair;
		//}

		//else if (primaryHeight >= topFiveHeights [1]) {

		//	for (i=4; i>=2; i--) {

		//		topFiveHeights [i] = topFiveHeights [i - 1];
		//		topFivePairs [i] = topFivePairs [i - 1];
		//	}

		//	topFiveHeights [1] = primaryHeight;
		//	topFivePairs [1] = nextPair;
		//}

		//else if (primaryHeight >= topFiveHeights [2]) {

		//	for (i=4; i>=3; i--) {

		//		topFiveHeights [i] = topFiveHeights [i - 1];
		//		topFivePairs [i] = topFivePairs [i - 1];
		//	}

		//	topFiveHeights [2] = primaryHeight;
		//	topFivePairs [2] = nextPair;
		//}

		//else if (primaryHeight >= topFiveHeights [3]) {

		//	topFiveHeights [4] = topFiveHeights [3];
		//	topFivePairs [4] = topFivePairs [3];

		//	topFiveHeights [3] = primaryHeight;
		//	topFivePairs [3] = nextPair;
		//}

		//else if (primaryHeight >= topFiveHeights [4]) {

		//	topFiveHeights [4] = primaryHeight;
		//	topFivePairs [4] = nextPair;
		//}
	}

	int nPeaks = primaries.size ();
//	double topFivePullups [5];

	//for (i = 0; i < 5; i++) {

	//	if (topFivePairs [i] != NULL) {

	//		nPeaks++;
	//		topFivePullups [i] = topFivePairs [i]->mPullupHeight;
	//	}

	//	else
	//		break;
	//}

	if (nPeaks <= 4) {

		delete[] topFivePairs;
		primaries.clear ();
		pullups.clear ();
		return EstimateMinimumPrimaryPullupHeightSM (primaryChannel, pullupChannel, estimatedMinHeight, pairList, pullupChannelNoise);
	}

	// Now calculate median quadratic coefficients for these 5 peaks

	//QuadraticLMSExact* qLMS = new QuadraticLMSExact (5, topFiveHeights, topFivePullups);
	QuadraticLMSExact* qLMS = new QuadraticLMSExact (primaries, pullups);
	qLMS->CalculateLMS ();
	linearCoefficient = qLMS->GetLinearTerm ();
	quadraticCoefficient = qLMS->GetQuadraticTerm ();
	delete qLMS;
	delete[] topFivePairs;
	primaries.clear ();
	pullups.clear ();

	if (FindMinPositiveHeightHeight (linearCoefficient, quadraticCoefficient, pullupChannelNoise, estimatedMinHeight) < 0) {

		// There is no pullup.  This happens when 3+ of the top 5 peaks have no pullup, so set estimated minimum height to 0.0, delete topFivePairs and return 0
		//estimatedMinHeight = 0.0;
		//return 5;
		// Try the linear estimate
		return EstimateMinimumPrimaryPullupHeightSM (primaryChannel, pullupChannel, estimatedMinHeight, pairList, pullupChannelNoise);
	}

	// There shouldn't be any negative pull-up, but, just in case....

	if ((minPrimaryHeightWithNegativePU > 0.0) && (minPrimaryHeightWithNegativePU < estimatedMinHeight))
		estimatedMinHeight = minPrimaryHeightWithNegativePU;

	//if ((primaryChannel == 2) && (pullupChannel == 4)) {

	//	cout << "Minimum primary calculation:  linear = " << linearCoefficient << ", quadratic = " << quadraticCoefficient << " and estimated Min Height = " << estimatedMinHeight << "\n";
	//}

	//else {

	//	return 0;
	//}
	//  FindMinPositiveHeight should return estimatedMinHeight which gets passed on...

	return 0;
}


int CoreBioComponent::FindMinPositiveHeightHeight (double alpha, double beta, double noise, double& minPositiveHeight) {

	// returns -1 if no primary height can cause measurable positive pull-up
	// returns 0 if there is a minimum positive height that causes measureable pull-up, although positive pull-up may arise from larger primaries
	minPositiveHeight = 0.0;

	if ((alpha <= 0.0) && (beta <= 0))
		return -1;

	if (beta == 0.0) {

		// alpha > 0
		minPositiveHeight = noise / alpha;
		return 0;
	}

	else if (alpha == 0) {

		// beta > 0
		minPositiveHeight = sqrt (noise / beta);
		return 0;
	}

	else if (alpha < 0) {

		// beta > 0
		// test negative noise for intersection.  If none, return positive noise intersection

		double dNegNoise = alpha * alpha - 4.0 * beta * noise;

		if (dNegNoise <= 0.0) {

			// No pull-down reaches -noise level.  Look for lowest height to hit + noise level.  That value is the larger quadratic solution

			double dPosNoise = alpha * alpha + 4.0 * beta * noise;
			minPositiveHeight = 0.5 * (-alpha + sqrt (dPosNoise)) / beta;
			return 0;
		}

		else {

			// Pull-down reaches -noise level.  Look for lowest height to hit -noise level

			double q = - 0.5 * (alpha - sqrt (dNegNoise));
			double r1 = q / beta;
			double r2 = noise / q;

			if (r1 < r2)
				minPositiveHeight = r1;

			else
				minPositiveHeight = r2;

			return 0;
		}
	}

	else {

		// alpha > 0
		// test positive noise for intersection

		double dPosNoise = alpha * alpha + 4.0 * beta * noise;

		if (dPosNoise <= 0.0) {

			// We never hit positive noise.  There is 0 pull-up
			return -1;
		}

		double q = - 0.5 * (alpha - sqrt (dPosNoise));
		double r1 = q / beta;
		double r2 = noise / q;

		if (r1 <= 0)
			minPositiveHeight = r2;

		else if (r2 <= 0)
			minPositiveHeight = r1;

		else if (r1 < r2)
			minPositiveHeight = r1;

		else
			minPositiveHeight = r2;

		return 0;
	}

	return -1;  // It should never get to here
}


int CoreBioComponent::FinalizeArtifactCallsGivenCalculatedPrimaryThresholdSM (int primaryChannel, int pullupChannel, double primaryThreshold, list<PullupPair*>& pairList, RGDList& noPullupPrimaries, RGDList& rawDataPullupPrimaries, RGDList& occludedPrimaries) {

	list<PullupPair*> tempPairs;
	list<InterchannelLinkage*>::iterator it;
	list<InterchannelLinkage*> tempLinkageList;
	double primaryHeight;
	PullupPair* nextPair;
	InterchannelLinkage* nextLink;
	InterchannelLinkage* iChannelPrimary;
	DataSignal* primarySignal;
	DataSignal* secondarySignal;
	RGString data;

	smPullUp pullup;
	smPrimaryInterchannelLink primaryLink;
	smWeakPrimaryInterchannelLink weakPrimaryPullup;
	smZeroPullupPrimaryInterchannelLink zeroPullupPrimary;
	smRawDataPrimaryInterchannelLink rawDataPrimary;

	while (!pairList.empty ()) {

		nextPair = pairList.front ();
		pairList.pop_front ();
		primaryHeight = nextPair->mPrimaryHeight;

		if (primaryHeight < primaryThreshold) {

			secondarySignal = nextPair->mPullup;
			primarySignal = nextPair->mPrimary;

			if (secondarySignal != NULL) {

				// test for removing all pullup info

				iChannelPrimary = primarySignal->GetInterchannelLink ();
				primarySignal->RemoveProbablePullup (secondarySignal);

				secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

				if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

					secondarySignal->SetMessageValue (pullup, false);
					secondarySignal->SetPullupFromChannel (primaryChannel, 0.0, mNumberOfChannels);
				}

				if (iChannelPrimary != NULL) {

					iChannelPrimary->RemoveDataSignalFromSecondaryList (secondarySignal);

					if (iChannelPrimary->IsEmpty ()) {

						primarySignal->SetInterchannelLink (NULL);
						primarySignal->SetMessageValue (primaryLink, false);
						mInterchannelLinkageList.remove (iChannelPrimary);
						delete iChannelPrimary;
					}
				}
			}

			else {

				iChannelPrimary = primarySignal->GetInterchannelLink ();

				if (iChannelPrimary == NULL) {

					primarySignal->SetMessageValue (primaryLink, false);
				}

				else if (iChannelPrimary->IsEmpty ()) {

					primarySignal->SetInterchannelLink (NULL);
					primarySignal->SetMessageValue (primaryLink, false);
					mInterchannelLinkageList.remove (iChannelPrimary);
					delete iChannelPrimary;
				}
			}

			delete nextPair;
			continue;
		}

		tempPairs.push_back (nextPair);
	}

	while (!tempPairs.empty ()) {

		nextPair = tempPairs.front ();
		tempPairs.pop_front ();
		pairList.push_back (nextPair);
	}

	// Now remove artifacts from primaries with no Osiris pull-up

	while (primarySignal = (DataSignal*)noPullupPrimaries.GetFirst ()) {
		
		if (primarySignal->Peak () >= primaryThreshold) {

			data = primarySignal->GetTempDataForPrimaryNoPullup ();
			primarySignal->SetMessageValue (zeroPullupPrimary, true);
			primarySignal->AppendDataForSmartMessage (zeroPullupPrimary, data);
			primarySignal->SetTempDataForPrimaryNoPullup ("");
		}

		else {

			primarySignal->SetMessageValue (zeroPullupPrimary, false);
			primarySignal->SetTempDataForPrimaryNoPullup ("");
		}
	}

	while (primarySignal = (DataSignal*)rawDataPullupPrimaries.GetFirst ()) {

		if (primarySignal->Peak () >= primaryThreshold) {

			data = primarySignal->GetTempDataForPrimaryRawDataPullup ();
			primarySignal->SetMessageValue (rawDataPrimary, true);
			primarySignal->AppendDataForSmartMessage (rawDataPrimary, data);
			primarySignal->SetTempDataForPrimaryRawDataPullup ("");
		}

		else {

			primarySignal->SetTempDataForPrimaryRawDataPullup ("");
			primarySignal->SetMessageValue (rawDataPrimary, false);
		}
	}

	data = "";
	data << pullupChannel;

	while (primarySignal = (DataSignal*)occludedPrimaries.GetFirst ()) {

		if (primarySignal->Peak () >= primaryThreshold) {

			primarySignal->SetMessageValue (weakPrimaryPullup, true);
			primarySignal->AppendDataForSmartMessage (weakPrimaryPullup, data);
			primarySignal->SetTempDataForOccludedPrimary ("");
		}

		else {

			primarySignal->SetTempDataForOccludedPrimary ("");
			primarySignal->SetMessageValue (weakPrimaryPullup, false);
		}
	}

	for (it=mInterchannelLinkageList.begin(); it!=mInterchannelLinkageList.end(); it++) {

		nextLink = *it;
		primarySignal = nextLink->GetPrimarySignal ();

		if (primarySignal->GetChannel () != primaryChannel)
			continue;

		InterchannelLinkage* iChannelPrimary = primarySignal->GetInterchannelLink ();

		if ((iChannelPrimary == NULL) || (!primarySignal->GetMessageValue (primaryLink)) || (iChannelPrimary->IsEmpty ())) {

			tempLinkageList.push_back (nextLink);
			//mInterchannelLinkageList.remove (nextLink);
			//delete nextLink;
		}
	}

	while (!tempLinkageList.empty ()) {

		nextLink = tempLinkageList.front ();
		tempLinkageList.pop_front ();
		mInterchannelLinkageList.remove (nextLink);
		delete nextLink;
	}

	return 0;
}


bool CoreBioComponent :: NegatePullupForChannelsSM (int primaryChannel, int pullupChannel, list<PullupPair*>& pairList, bool testLaserOffScale) {

	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	smPrimaryInterchannelLink primaryPullup;
	smLaserOffScale laserOffScale;
	list<InterchannelLinkage*>::iterator it;
	InterchannelLinkage* iChannel;
	DataSignal* primary;
	DataSignal* secondary;
	DataSignal* firstPrimary;
	list<InterchannelLinkage*> emptyLinks;
	PullupPair* firstPair = pairList.front ();

	if (firstPair == NULL)
		return false;

	firstPrimary = firstPair->mPrimary;

	for (it=mInterchannelLinkageList.begin (); it!=mInterchannelLinkageList.end (); it++) {

		iChannel = *it;
		primary = iChannel->GetPrimarySignal ();

		if (primary->GetChannel () != primaryChannel)
			continue;

		if (primary->GetMessageValue (laserOffScale) != testLaserOffScale)
			continue;

		secondary = iChannel->GetSecondarySignalOnChannel (pullupChannel);

		if (secondary == NULL)
			continue;

		secondary->SetPullupFromChannel (primaryChannel, 0.0, mNumberOfChannels);
		secondary->SetPullupRatio (primaryChannel, 0.0, mNumberOfChannels);
		secondary->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

		iChannel->RemoveDataSignalFromSecondaryList (secondary);

		if (iChannel->IsEmpty ()) {

			primary->SetMessageValue (primaryPullup, false);
			primary->SetInterchannelLink (NULL);
			emptyLinks.push_back (iChannel);
		}

		if (!secondary->HasAnyPrimarySignals (mNumberOfChannels)) {

			secondary->SetMessageValue (pullup, false);
			secondary->SetMessageValue (purePullup, false);
		}
	}

	while (!emptyLinks.empty ()) {

		iChannel = emptyLinks.front ();
		emptyLinks.pop_front ();
		mInterchannelLinkageList.remove (iChannel);
		delete iChannel;
	}

	return true;
}



DataSignal** CoreBioComponent :: CollectAndSortPullupPeaksSM (DataSignal* primarySignal, RGDList& pullupSignals) {

	//
	//   This is sample stage 1.  
	//
	//   It inserts pullup peaks into primary signal's pullup array.  We should arrange that if primaryChannel == currentChannel, we continue.
	//   Consider commenting out preferential treatment for possible sigmoidal peaks.  We used to need this to prevent negative peaks from messing up the determination of
	//   the pullup pattern, but we no longer need to do that.  A single negative peak no longer determines that the pattern has to be negative (04/01/2017...no fooling!)
	//

	DataSignal** pullupArray = new DataSignal* [mNumberOfChannels + 1];
	int i;

	for (i=1; i<=mNumberOfChannels; i++)
		pullupArray [i] = NULL;

	DataSignal* nextSignal;
	DataSignal* currentSignal;
	int currentChannel;
	int primaryChannel = primarySignal->GetChannel ();
	double primaryMean = primarySignal->GetMean ();
	double currentMean;
	double nextMean;
	RGDListIterator it (pullupSignals);

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal == primarySignal)
			continue;

		currentChannel = nextSignal->GetChannel ();
		currentSignal = pullupArray [currentChannel];

		if (currentSignal == NULL)
			pullupArray [currentChannel] = nextSignal;

		else {

			currentMean = currentSignal->GetMean ();
			nextMean = nextSignal->GetMean ();
			double deltaCurrent = fabs (primaryMean - currentMean);
			double deltaNext = fabs (primaryMean - nextMean);

			//  Experiment with commenting out the next few lines...sigmoids and craters are not special...04/01/2017...no fooling!

			if (nextSignal->IsSigmoidalPeak ()) {
				
				pullupArray [currentChannel] = nextSignal;
				continue;
			}

			else if (currentSignal->IsSigmoidalPeak ())
				continue;

			if (nextMean == primaryMean) {

				pullupArray [currentChannel] = nextSignal;
				continue;
			}

			if (deltaCurrent < deltaNext) {
				
				continue;
			}

			else if (deltaCurrent > deltaNext) {

				pullupArray [currentChannel] = nextSignal;
			}

			else {

				if (nextSignal->IsNegativePeak () && currentSignal->IsNegativePeak ())
					pullupArray [currentChannel] = nextSignal;

				else if (nextSignal->IsNegativePeak () && !currentSignal->IsNegativePeak ())
					pullupArray [currentChannel] = nextSignal;

				else if (!nextSignal->IsNegativePeak () && currentSignal->IsNegativePeak ())
					continue;

				else if (nextSignal->IsCraterPeak ())
					pullupArray [currentChannel] = nextSignal;

				else if (currentSignal->IsCraterPeak ())
					continue;

				else if (nextSignal->Peak () < currentSignal->Peak ())
					pullupArray [currentChannel] = nextSignal;
			}
		}
	}

	//it.Reset ();

	//while (nextSignal = (DataSignal*)it ()) {   //  **** Added this loop 11/24/2020...and removed it 12/3/2020

	//	currentChannel = nextSignal->GetChannel ();
	//	currentSignal = pullupArray [currentChannel];

	//	if (currentSignal != nextSignal)
	//		it.RemoveCurrentItem ();
	//}

	return pullupArray;
}


bool CoreBioComponent :: AcknowledgePullupPeaksWhenThereIsNoPatternSM (int primaryChannel, int secondaryChannel, bool testLaserOffScale) {

	DataSignal* primarySignal;
	DataSignal* secondarySignal;
	InterchannelLinkage* nextLink;
	list<InterchannelLinkage*>::iterator it;
	smLaserOffScale laserOffScale;
	smCalculatedPurePullup purePullup;
	smPartialPullupBelowMinRFU partialPullupBelowMinRFU;
	smPrimaryInterchannelLink primaryLink;
	smPullUp pullup;
	list<InterchannelLinkage*> removeList;
	double linearPart;
	double quadraticPart;
	double p;
	double ratio;
	bool isOutlier;
	bool belowMinRFU;
	bool isNarrow2;
	bool halfWidth;
	double minRFUForSecondaryChannel = mDataChannels [secondaryChannel]->GetMinimumHeight ();
	double primaryThreshold = CoreBioComponent::minPrimaryPullupThreshold;

	linearPart = mLinearPullupMatrix [primaryChannel][secondaryChannel];
	quadraticPart = mQuadraticPullupMatrix [primaryChannel][secondaryChannel];
	double LMV = mLeastMedianValue [primaryChannel][secondaryChannel];
	double outlierThreshold = mOutlierThreshold [primaryChannel][secondaryChannel];
	bool pattern = mPattern [primaryChannel][secondaryChannel];
	bool patternButNoPullup = pattern && (linearPart == 0.0) && (quadraticPart == 0.0);
	smPrimaryInterchannelLink primaryPullup;
	InterchannelLinkage* secondaryLink;

	for (it=mInterchannelLinkageList.begin (); it!=mInterchannelLinkageList.end (); it++) {

		nextLink = *it;
		primarySignal = nextLink->GetPrimarySignal ();

		if (primarySignal->GetChannel () != primaryChannel)
			continue;

		if (primarySignal->GetMessageValue (laserOffScale) != testLaserOffScale)
			continue;

		secondarySignal = nextLink->GetSecondarySignalOnChannel (secondaryChannel);

		if (secondarySignal == NULL)
			continue;

		//if ((secondarySignal->GetPullupFromChannel (primaryChannel) != 0.0) || (secondarySignal->HasPrimarySignalFromChannel (primaryChannel) != NULL))  // skip this test???
		//	continue;

		if (!pattern) {

			secondarySignal->SetIsPossiblePullup (true);
			secondarySignal->AddUncertainPullupChannel (primaryChannel);
			secondarySignal->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);
			primarySignal->SetMessageValue (primaryPullup, true);
			//secondarySignal->SetPullupRatio (primaryChannel, 100.0 * secondarySignal->Peak () / primarySignal->Peak (), mNumberOfChannels);
			//secondarySignal->SetPullupFromChannel (primaryChannel, secondarySignal->Peak (), mNumberOfChannels);
			continue;
		}

		else if (patternButNoPullup) {

			// Add code to remove pullup artifacts if there is no other pullup from other channels

			secondarySignal->SetPullupFromChannel (primaryChannel, 0.0, mNumberOfChannels);
			secondarySignal->SetPullupRatio (primaryChannel, 0.0, mNumberOfChannels);
			secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

			nextLink->RemoveDataSignalFromSecondaryList (secondarySignal);

			if (nextLink->IsEmpty ()) {

				primarySignal->SetMessageValue (primaryPullup, false);
				primarySignal->SetInterchannelLink (NULL);
				removeList.push_back (nextLink);
			}

			if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

				secondarySignal->SetMessageValue (pullup, false);
				secondarySignal->SetMessageValue (purePullup, false);
				secondarySignal->SetMessageValue (partialPullupBelowMinRFU, false);
			}

			continue;
		}

		p = primarySignal->Peak ();
		double directRatio = secondarySignal->Peak () / p;
		double calculatedRatio = (linearPart + p * quadraticPart);
		isOutlier = pattern && (fabs (calculatedRatio - directRatio) > outlierThreshold);
		ratio = (linearPart + p * quadraticPart);
		double h = secondarySignal->Peak () - secondarySignal->GetTotalPullupFromOtherChannels (mNumberOfChannels);

		isNarrow2 = (secondarySignal->GetWidth () < 0.5 * primarySignal->GetWidth ()) && !secondarySignal->IgnoreWidthTest ();
		halfWidth = pattern && (isNarrow2) && (testLaserOffScale || primarySignal->IsCraterPeak ());  // Removed isNarrow 12/2/2020

		if (secondarySignal->GetMessageValue (purePullup) || (pattern && !isOutlier) || halfWidth)   // include pattern?!
			ratio = directRatio;

		else
			ratio = calculatedRatio;

		bool originalBelowMinRFU = (h < minRFUForSecondaryChannel);

		if (secondarySignal->GetPullupFromChannel (primaryChannel) == 0.0)  //????
			h = h - p * ratio;   // ????

		belowMinRFU = (secondarySignal->Peak () < minRFUForSecondaryChannel);

		if (originalBelowMinRFU && (ratio < 0.0) && !belowMinRFU) {

			ratio = 0.005;
			belowMinRFU = true;
		}

		//sigmaPrimary = primarySignal->GetStandardDeviation ();  // standard deviation is not a reliable measure of width
		//sigmaSecondary = secondarySignal->GetStandardDeviation ();

		//isNarrow = (sigmaSecondary < 0.5* sigmaPrimary)  && !secondarySignal->IgnoreWidthTest ();

		if (secondarySignal->GetMessageValue (purePullup) || (pattern && !isOutlier) || halfWidth) {   // include pattern?!

			// secondary is pure pullup.  Do not insert possible pullup designation and remove from link, with appropriate consequences...

			if (!secondarySignal->GetMessageValue (purePullup)) {

				if (halfWidth && isOutlier) {

					cout << "Half width criterion triggered for outlier in sample name " << (char*)mName.GetData () << " for channel " << secondaryChannel << " at time " << secondarySignal->GetMean () << " for primary ";

					if (testLaserOffScale)
						cout << "laser off-scale ";

					else
						cout << "laser not off-scale ";

					if (primarySignal->IsCraterPeak ())
						cout << "and crater\n";

					else
						cout << "and not crater\n";
				}

			}

			//secondarySignal->SetIsPurePullupFromChannel (primaryChannel, true, mNumberOfChannels);
			//double primaryHeight = primarySignal->Peak ();
			//ratio = 100.0 * (linearPart + quadraticPart * primaryHeight);
			double pullupLevel = ratio * p;
			secondarySignal->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);
			secondarySignal->SetPullupFromChannel (primaryChannel, pullupLevel, mNumberOfChannels);  // Fix this and next line to reflect contributions from primary peaks on other channels
			secondarySignal->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
			secondarySignal->SetMessageValue (pullup, false);   // need this?  if purePullup = true, than ordinary pullup should already be false
			secondarySignal->SetMessageValue (purePullup, true);
			secondarySignal->SetMessageValue (partialPullupBelowMinRFU, false);

			if ((pattern && !isOutlier) || halfWidth)
				secondarySignal->SetIsPurePullupFromChannel (primaryChannel, true, mNumberOfChannels);
		}

		else if (secondarySignal->GetMessageValue (partialPullupBelowMinRFU) || belowMinRFU) {

			secondarySignal->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);
			secondarySignal->SetPullupFromChannel (primaryChannel, h, mNumberOfChannels);  // Fix this and next line to reflect contributions from primary peaks on other channels
			secondarySignal->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
			secondarySignal->SetMessageValue (pullup, false);   // need this?  if purePullup = true, than ordinary pullup should already be false
			secondarySignal->SetMessageValue (purePullup, false);
			secondarySignal->SetMessageValue (partialPullupBelowMinRFU, true);
		}

		else {

			//p = primarySignal->Peak ();
			//ratio = (linearPart + p * quadraticPart);
			secondarySignal->SetPrimarySignalFromChannel (primaryChannel, primarySignal, mNumberOfChannels);
			secondarySignal->SetPullupFromChannel (primaryChannel, ratio * p, mNumberOfChannels);  // Fix this and next line to reflect contributions from primary peaks on other channels
			secondarySignal->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
			secondarySignal->SetMessageValue (pullup, true);   // need this?  if purePullup = true, than ordinary pullup should already be false
			secondarySignal->SetMessageValue (purePullup, false);
			secondarySignal->SetMessageValue (partialPullupBelowMinRFU, false);
		}

		if (secondarySignal->GetMessageValue (purePullup) || secondarySignal->GetMessageValue (partialPullupBelowMinRFU)) {

			if (secondarySignal->HasCrossChannelSignalLink ()) {

				// remove link; this is not a primary peak
				
				secondarySignal->SetMessageValue (primaryLink, false);
				secondaryLink = secondarySignal->GetInterchannelLink ();

				if (secondaryLink != NULL) {

					removeList.push_back (secondaryLink);
				}
			}
		}
	}

	removeList.unique ();

	while (!removeList.empty ()) {

		nextLink = removeList.front ();
		removeList.pop_front ();

		if (nextLink != NULL) {

			primarySignal = nextLink->GetPrimarySignal ();
			RemovePrimaryLinksAndSecondaryLinksFrom (primarySignal);
		}
	}

	return true;
}


bool CoreBioComponent::RemovePrimaryLinksForChannelsSM (int primaryChannel, int pullupChannel, bool testLaserOffScale, RGDList& peakList) {

	smPrimaryInterchannelLink primaryPullup;
	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	smLaserOffScale laserOffScale;
	RGDListIterator it (peakList);
	DataSignal* nextSignal;
	DataSignal* pullupSignal;
	InterchannelLinkage* iLink;
	RGDList tempList;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->GetChannel () != primaryChannel)
			continue;

		if (nextSignal->GetMessageValue (laserOffScale) != testLaserOffScale)
			continue;

		iLink = nextSignal->GetInterchannelLink ();

		if (iLink != NULL) {

			pullupSignal = iLink->GetSecondarySignalOnChannel (pullupChannel);

			if (pullupSignal != NULL) {

				pullupSignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

				if (!pullupSignal->HasAnyPrimarySignals (mNumberOfChannels)) {

					pullupSignal->SetMessageValue (pullup, false);
					pullupSignal->SetMessageValue (purePullup, false);
				}

				iLink->RemoveDataSignalFromSecondaryList (pullupSignal);
			}

			if (iLink->IsEmpty ()) {

				nextSignal->SetMessageValue (primaryPullup, false);
				nextSignal->SetInterchannelLink (NULL);
				mInterchannelLinkageList.remove (iLink);
				delete iLink;
				it.RemoveCurrentItem ();
			}
		}

		else {

			nextSignal->SetMessageValue (primaryPullup, false);
			it.RemoveCurrentItem ();
		}
	}

	return true;
}


bool CoreBioComponent::ScavengePullupFromOtherChannelListLaserInScale () {

	// sample phase 1...still in progress.
	
	DataSignal* primarySignal;
	DataSignal* secondarySignal;
	int primaryChannel;
	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	smPrimaryInterchannelLink primaryPullup;
	smLaserOffScale laserOffScale;
	double linearPullupCoefficient;
	double quadraticPullupCoefficient;
	double minPrimaryHeight;
	InterchannelLinkage* iLink;
	int i;
	bool coefficientsZero;
	bool belowMinPrimaryHeight;
	double primaryHeight;

	while (primarySignal = (DataSignal*) mPullupFromAnotherChannel.GetFirst ()) {

		if (primarySignal->GetMessageValue (laserOffScale))  // This function is for laser in-scale only
			continue;
		
		iLink = primarySignal->GetInterchannelLink ();
		primaryChannel = primarySignal->GetChannel ();

		// First test for pure pullup below.  If so, remove all primary links

		if (primarySignal->GetMessageValue (purePullup)) {

			primarySignal->SetMessageValue (primaryPullup, false);

			if (iLink != NULL) {

				for (i=1; i<=mNumberOfChannels; i++) {

					if (i == primaryChannel)
						continue;

					secondarySignal = iLink->GetSecondarySignalOnChannel (i);

					if (secondarySignal == NULL)
						continue;

					secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);
					iLink->RemoveDataSignalFromSecondaryList (secondarySignal);

					if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

						// set all pull-up artifacts to false...
						secondarySignal->SetMessageValue (purePullup, false);
						secondarySignal->SetMessageValue (pullup, false);
					}
				}

				mInterchannelLinkageList.remove (iLink);
				delete iLink;
				primarySignal->SetInterchannelLink (NULL);
			}

			continue;
		}

		primaryHeight = primarySignal->Peak () - primarySignal->GetPullupCorrectionFromPrimariesWithNoPullupSM (mNumberOfChannels);

		if (iLink != NULL) {

			for (i=1; i<=mNumberOfChannels; i++) {

				if (i == primaryChannel)
					continue;

				secondarySignal = iLink->GetSecondarySignalOnChannel (i);

				if (secondarySignal == NULL)
					continue;

				linearPullupCoefficient = mLinearInScalePullupMatrix [primaryChannel] [i];
				quadraticPullupCoefficient = mQuadraticInScalePullupMatrix [primaryChannel] [i];
				minPrimaryHeight = mMinimumInScalePrimaryPeak [primaryChannel] [i];
				coefficientsZero = (linearPullupCoefficient == 0.0) && (quadraticPullupCoefficient == 0.0);
				belowMinPrimaryHeight = (primaryHeight < minPrimaryHeight);

				if (coefficientsZero || belowMinPrimaryHeight) {

					secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

					if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

						// set all pull-up artifacts to false...
						secondarySignal->SetMessageValue (purePullup, false);
						secondarySignal->SetMessageValue (pullup, false);
					}

					iLink->RemoveDataSignalFromSecondaryList (secondarySignal);
					
					if (iLink->IsEmpty ()) {

						primarySignal->SetInterchannelLink (NULL);
						primarySignal->SetMessageValue (primaryPullup, false);
						mInterchannelLinkageList.remove (iLink);
						delete iLink;
						break;
					}
				}

				else {  // this pullup is also a primary:  calculate corrected heights

					double ratio = linearPullupCoefficient + primaryHeight * quadraticPullupCoefficient;
					double correction = primaryHeight * ratio;
					secondarySignal->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
					secondarySignal->SetPullupFromChannel (primaryChannel, correction, mNumberOfChannels);
				}
			}
		}

		else {  // iLink = NULL so set artifacts accordingly

			primarySignal->SetInterchannelLink (NULL);
			primarySignal->SetMessageValue (primaryPullup, false);
		}
	}

	return true;
}


bool CoreBioComponent::ScavengePullupFromOtherChannelListLaserOffScale () {

	// sample phase 1...still in progress.

	DataSignal* primarySignal;
	DataSignal* secondarySignal;
	int primaryChannel;
	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	smPrimaryInterchannelLink primaryPullup;
	smLaserOffScale laserOffScale;
	double linearPullupCoefficient;
	double quadraticPullupCoefficient;
	double linearInScalePullupCoefficient;
	double quadraticInScalePullupCoefficient;
	double minInScalePrimaryHeight;
	double minOffScalePrimaryHeight;
	InterchannelLinkage* iLink;
	int i;
	bool coefficientsZero;
	bool belowMinPrimaryHeight;
	double primaryHeight;
	double ratio;
	double correction;

	while (primarySignal = (DataSignal*)mPullupFromAnotherChannel.GetFirst ()) {

		if (!primarySignal->GetMessageValue (laserOffScale))  // This function is for laser off-scale only
			continue;

		iLink = primarySignal->GetInterchannelLink ();
		primaryChannel = primarySignal->GetChannel ();
		minOffScalePrimaryHeight = mDataChannels [primaryChannel]->GetMaxInScalePeak ();

		// First test for pure pullup below.  If so, remove all primary links

		if (primarySignal->GetMessageValue (purePullup)) {

			primarySignal->SetMessageValue (primaryPullup, false);

			if (iLink != NULL) {

				for (i=1; i<=mNumberOfChannels; i++) {

					if (i == primaryChannel)
						continue;

					secondarySignal = iLink->GetSecondarySignalOnChannel (i);

					if (secondarySignal == NULL)
						continue;

					secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);
					iLink->RemoveDataSignalFromSecondaryList (secondarySignal);

					if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

						// set all pull-up artifacts to false...
						secondarySignal->SetMessageValue (purePullup, false);
						secondarySignal->SetMessageValue (pullup, false);
					}
				}

				mInterchannelLinkageList.remove (iLink);
				delete iLink;
				primarySignal->SetInterchannelLink (NULL);
			}

			continue;
		}

		primaryHeight = primarySignal->Peak () - primarySignal->GetPullupCorrectionFromPrimariesWithNoPullupSM (mNumberOfChannels);

		if (iLink != NULL) {

			for (i=1; i<=mNumberOfChannels; i++) {

				if (i == primaryChannel)
					continue;

				secondarySignal = iLink->GetSecondarySignalOnChannel (i);

				if (secondarySignal == NULL)
					continue;

				linearPullupCoefficient = mLinearPullupMatrix [primaryChannel] [i];
				quadraticPullupCoefficient = mQuadraticPullupMatrix [primaryChannel] [i];
				linearInScalePullupCoefficient = mLinearInScalePullupMatrix [primaryChannel] [i];
				quadraticInScalePullupCoefficient = mQuadraticInScalePullupMatrix [primaryChannel] [i];
				minInScalePrimaryHeight = mMinimumInScalePrimaryPeak [primaryChannel] [i];
				coefficientsZero = (linearPullupCoefficient == 0.0) && (quadraticPullupCoefficient == 0.0);
				belowMinPrimaryHeight = (primaryHeight < minInScalePrimaryHeight);

				if (coefficientsZero || belowMinPrimaryHeight) {

					secondarySignal->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

					if (!secondarySignal->HasAnyPrimarySignals (mNumberOfChannels)) {

						// set all pull-up artifacts to false...
						secondarySignal->SetMessageValue (purePullup, false);
						secondarySignal->SetMessageValue (pullup, false);
					}

					iLink->RemoveDataSignalFromSecondaryList (secondarySignal);

					if (iLink->IsEmpty ()) {

						primarySignal->SetInterchannelLink (NULL);
						primarySignal->SetMessageValue (primaryPullup, false);
						mInterchannelLinkageList.remove (iLink);
						delete iLink;
						break;
					}
				}

				else {  // this pullup is also a primary:  calculate corrected heights

					if (primaryHeight >= minOffScalePrimaryHeight)
						ratio = linearPullupCoefficient + primaryHeight * quadraticPullupCoefficient;

					else
						ratio = linearInScalePullupCoefficient + primaryHeight * quadraticInScalePullupCoefficient;

					correction = primaryHeight * ratio;
					secondarySignal->SetPullupRatio (primaryChannel, 100.0 * ratio, mNumberOfChannels);
					secondarySignal->SetPullupFromChannel (primaryChannel, correction, mNumberOfChannels);
				}
			}
		}

		else {  // iLink = NULL so set artifacts accordingly

			primarySignal->SetInterchannelLink (NULL);
			primarySignal->SetMessageValue (primaryPullup, false);
		}
	}

	return true;
}


void CoreBioComponent :: RemovePrimaryLinksAndSecondaryLinksFrom (DataSignal* ds) {

	InterchannelLinkage* nextLink = ds->GetInterchannelLink ();
	DataSignal* nextSignal;
	int pullupChannel = ds->GetChannel ();
	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	smPrimaryInterchannelLink primaryPullup;

	ds->SetMessageValue (primaryPullup, false);

	if (nextLink != NULL) {

		nextLink->ResetSecondaryIterator ();

		while (nextSignal = nextLink->GetNextSecondarySignal ()) {

			if (nextSignal == ds)
				continue;

			nextSignal->SetPullupFromChannel (pullupChannel, 0.0, mNumberOfChannels);
			nextSignal->SetPrimarySignalFromChannel (pullupChannel, NULL, mNumberOfChannels);

			if (!nextSignal->IsPullupFromChannelsOtherThan (pullupChannel, mNumberOfChannels)) {

				nextSignal->SetMessageValue (pullup, false);
				nextSignal->SetMessageValue (purePullup, false);
			}
		}

		ds->RemoveAllCrossChannelSignalLinks ();  // Check that this is all we have to do.
		mInterchannelLinkageList.remove (nextLink);
		delete nextLink;
	}
}


bool CoreBioComponent :: ValidateAndCorrectCrossChannelAnalysesSM () {

	int i;
	
	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->FindLimitsOnPrimaryPullupPeaks ();

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->ValidateAndCorrectCrossChannelAnalysisSM ();

	return true;
}


void CoreBioComponent :: RemovePeakAsPrimarySM (DataSignal* ds) {

	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	InterchannelLinkage* link = ds->GetInterchannelLink ();

	if (link == NULL)
		return;

	link->ResetSecondaryIterator ();
	DataSignal* nextPullup;
	int primaryChannel = ds->GetChannel ();

	while (nextPullup = link->GetNextSecondarySignal ()) {


		if (nextPullup == ds)
			continue;

		nextPullup->SetPullupFromChannel (primaryChannel, 0.0, mNumberOfChannels);
		nextPullup->SetPrimarySignalFromChannel (primaryChannel, NULL, mNumberOfChannels);

		if (nextPullup->HasAnyPrimarySignals (mNumberOfChannels))
			continue;

		// reset nextPullup messages so it is no longer a pull up
	}
}


int CoreBioComponent :: SetLaneStandardDataSM (SampleData& fileData, TestCharacteristic* testControlPeak, TestCharacteristic* testSamplePeak) {

	int status = mDataChannels [mLaneStandardChannel]->SetRawDataSM (fileData, testControlPeak, testSamplePeak);

	if (status < 0)
		ErrorString << mDataChannels [mLaneStandardChannel]->GetError ();

	return status;
}


int CoreBioComponent :: FitLaneStandardCharacteristicsSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	//
	//  This is ladder and sample stage 1
	//

	int status = mDataChannels [mLaneStandardChannel]->FitAllCharacteristicsSM (text, ExcelText, msg, print);

	if (status < 0)
		ErrorString << mDataChannels [mLaneStandardChannel]->GetError ();

	return status;
}


int CoreBioComponent :: FitAllSampleCharacteristicsSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int CoreBioComponent :: AnalyzeGridSM (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int CoreBioComponent :: AnalyzeGridSM (SampleData& fileData, GridDataStruct* gridData) {

	//
	//  This is ladder stage 1
	//

	Endl endLine;
	RGString Notice;
	smTestForColorCorrectionMatrixPreset testForColorCorrectionMatrixPreset;
	int status = InitializeSM (fileData, gridData->mCollection, gridData->mMarkerSetName, TRUE);

	if (status < 0) {

		Notice << "BioComponent could not initialize:";
		cout << Notice << endl;
		gridData->mExcelText << CLevel (1) << Notice << "\n" << ErrorString << "Skipping...\n" << PLevel ();
		gridData->mText << Notice << "\n" << ErrorString << "Skipping\n";
		return -1;
	}

	if (CoreBioComponent::UseRawData) {

		if (GetMessageValue (testForColorCorrectionMatrixPreset))
			status = SetAllRawDataWithMatrixSM (fileData, gridData->mTestControlPeak, gridData->mTestControlPeak);

		else
			status = SetAllRawDataSM (fileData, gridData->mTestControlPeak, gridData->mTestControlPeak);
	}
	
	else		
		status = SetAllDataSM (fileData, gridData->mTestControlPeak, gridData->mTestControlPeak);

	if (status < 0) {

		Notice << "BioComponent could not set data:";
		cout << Notice << endl;
		gridData->mExcelText << CLevel (1) << Notice << "\n" << ErrorString << "Skipping...\n" << PLevel ();
		gridData->mText << Notice << "\n" << ErrorString << "Skipping...\n";
		return -2;
	}

	InitializeAllSampleModifications ();

	if (CoreBioComponent::UseRawData)
		FindAndRemoveFixedOffsets ();

	status = AnalyzeGridSM (gridData->mText, gridData->mExcelText, gridData->mMsg);

	if (status < 0) {

		Notice << "BioComponent could not analyze grid.  Skipping...";
		cout << Notice << endl;
		Notice << "\n";
		gridData->mExcelText.Write (1, Notice);
		gridData->mText << Notice;
		return -3;
	}

	return 0;
}


int CoreBioComponent :: PrepareSampleForAnalysisSM (SampleData& fileData, SampleDataStruct* sampleData) {

	//
	//  This is sample stage 1
	//

	Endl endLine;

	smTestForColorCorrectionMatrixPreset testForColorCorrectionMatrixPreset;
	RGString notice;
	notice << "Analyzing sample named " << GetSampleName () << "\n";
	sampleData->mExcelText.Write (1, notice);
	sampleData->mText << notice;
	notice = "";
//	Notice* newNotice;
//	BlobFound newNotice;
	smILSFailed ilsRequiresReview;
	smNormalizeRawDataRelativeToBaselinePreset normalizeRawData;
	smEnableRawDataFilterForNormalizationPreset enableFilteringForNormalization;

	smIgnoreNoiseAnalysisAboveDetectionThresholdInSmoothing useDetectionLevelInSmoothing;
	smOverrideNoiseLevelPercentsInSmoothing overrideNoiseLevelPercentsInSmoothing;
	smPercentOverrideOfNoiseThresholdForNormalizationPhase percentOverrideNormalizationPhase;
	smPercentOverrideOfNoiseThresholdForFinalPhase percentOverrideFinalPhase;

	Progress = 0;
	int j;

	int status = InitializeSM (fileData, sampleData->mCollection, sampleData->mMarkerSetName, FALSE);

	if (status < 0) {

		notice << "BIOCOMPONENT COULD NOT INITIALIZE:";
		cout << notice << endl;
		sampleData->mExcelText.Write (1, notice);
		sampleData->mExcelText << CLevel (1) << notice << "\n" << ErrorString << "Skipping...\n" << PLevel ();
		sampleData->mText << notice << "\n" << ErrorString << "Skipping...\n";
		return -1;
	}

	Progress = 1;

	if (CoreBioComponent::UseRawData) {

		if (GetMessageValue (testForColorCorrectionMatrixPreset))
			status = SetAllRawDataWithMatrixSM (fileData, sampleData->mTestControlPeak, sampleData->mTestSamplePeak);

		else
			status = SetAllRawDataSM (fileData, sampleData->mTestControlPeak, sampleData->mTestSamplePeak);
	}
	
	else		
		status = SetAllDataSM (fileData, sampleData->mTestControlPeak, sampleData->mTestSamplePeak);

	if (status < 0) {

		notice << "BIOCOMPONENT COULD NOT SET DATA:";
		cout << notice << endl;
		sampleData->mExcelText << CLevel (1) << notice << "\n" << ErrorString << "Skipping...\n" << PLevel ();
		sampleData->mText << notice << "\n" << ErrorString << "Skipping...\n";
		SetCrashCode (42);
		throw 42;
		return -2;
	}

	CoreBioComponent::InitializeOffScaleData (fileData);
	InitializeAllSampleModifications ();
	Progress = 2;

	if (CoreBioComponent::UseRawData) {

		status = FindAndRemoveFixedOffsets ();

		if (status < 0) {

			notice << "BIOCOMPONENT COULD NOT COMPUTE OFFSETS ACCURATELY.  Skipping...";
			cout << notice << endl;
			sampleData->mExcelText.Write (1, notice);
			sampleData->mText << notice << "\n" << ErrorString << " Skipping...\n";
			return -5;
		}
	}

	if (GetMessageValue (enableFilteringForNormalization))
		ChannelData::SetUseNormalizationFilter (true);

	else
		ChannelData::SetUseNormalizationFilter (false);

	//if (GetMessageValue (normalizeRawData) && GetMessageValue (enableFilteringForNormalization))
	//	CreateAndSubstituteFilteredDataSignalForRawDataNonILS ();

	if (GetMessageValue (normalizeRawData))
		ChannelData::SetUseEnhancedShoulderAlgorithm (false);

	else
		ChannelData::SetUseEnhancedShoulderAlgorithm (true);  // It will still only use the algorithm if directed to by the preset.

	ChannelData::SetUseNoiseLevelDefaultForFit (true);
	bool useDetectionLevel = GetMessageValue (useDetectionLevelInSmoothing);
	bool usePercentNoiseLevel = GetMessageValue (overrideNoiseLevelPercentsInSmoothing);

	if (useDetectionLevel) {

		ChannelData::SetUseDetectionLevelForFit (true);
		ChannelData::SetUseNoiseLevelDefaultForFit (false);
		SampledData::SetIgnoreNoiseAnalysisAboveDetectionInSmoothing (true);
	}

	else
		ChannelData::SetUseDetectionLevelForFit (false);

	if (usePercentNoiseLevel) {

		ChannelData::SetUseNoiseLevelPercentForFit (true);
		ChannelData::SetUseDetectionLevelForFit (false);
		ChannelData::SetUseNoiseLevelDefaultForFit (false);
		SampledData::SetIgnoreNoiseAnalysisAboveDetectionInSmoothing (true);
		ChannelData::SetNoisePercentForFinalPass ((double)GetThreshold (percentOverrideFinalPhase));
		ChannelData::SetNoisePercentForNormalizationPass ((double)GetThreshold (percentOverrideNormalizationPhase));
	}

	else
		ChannelData::SetUseNoiseLevelPercentForFit (false);

	if (!useDetectionLevel && !usePercentNoiseLevel)
		SampledData::SetIgnoreNoiseAnalysisAboveDetectionInSmoothing (false);

	if (GetMessageValue (normalizeRawData))
		ChannelData::SetIsNormalizationPass (true);

	else
		ChannelData::SetIsNormalizationPass (false);

	status = FitAllSampleCharacteristicsSM (sampleData->mText, sampleData->mExcelText, sampleData->mMsg, FALSE);	// ->FALSE

	if (status < 0) {

		notice << "BIOCOMPONENT COULD NOT FIT ALL CHARACTERISTICS.  Skipping...Return status = " << status;
		cout << notice << endl;
		sampleData->mExcelText.Write (1, notice);
		sampleData->mText << notice << "\n" << ErrorString << "Skipping...\n";
		return -3;
	}

	Progress = 3;
//	mLSData->ClearAllPeaksBelowAnalysisThreshold ();

	if (!GetMessageValue (normalizeRawData)) {

		//AnalyzeCrossChannelSM ();	// Moved below - 07/31/2013
		/*status = AnalyzeLaneStandardChannelSM (sampleData->mText, sampleData->mExcelText, sampleData->mMsg, sampleData->mPrint);

		if (status < 0) {

			notice << "BIOCOMPONENT COULD NOT ANALYZE INTERNAL LANE STANDARD.  Skipping...";
			cout << notice << endl;
			sampleData->mExcelText << CLevel (1) << notice << "\n" << ErrorString << "Skipping...\n" << PLevel ();
			sampleData->mText << notice << "\n" << ErrorString << "Skipping...\n";
			SetMessageValue (ilsRequiresReview, true);
			return -4;
		}*/

		status = 0;

		for (j=1; j<=mNumberOfChannels; j++) {

			if (mDataChannels [j]->SetAllApproximateIDs (mLSData) < 0)
				status = -1;
		}

		if (status < 0) {

			notice << "BIOCOMPONENT COULD NOT UTILIZE INTERNAL LANE STANDARD.  Skipping...";
			cout << notice << endl;
			SetMessageValue (ilsRequiresReview, true);
			notice = "Could not create ILS time to base pairs transform";
			SetDataForSmartMessage (ilsRequiresReview, notice);
			return -5;
		}

		//AnalyzeCrossChannelSM ();	//	Moved here 07/31/2013...happy birthday, Mom.  You'd be 99 today.
		//AnalyzeCrossChannelWithNegativePeaksSM ();  // Commented out 01/30/2016


		// This is new algorithm that is primary peak centric...01/30/2016
		AnalyzeCrossChannelUsingPrimaryWidthAndNegativePeaksSM ();


	//	TestSignalsForLaserOffScaleSM ();	//  Moved inside AnalyzeCrossChannelWithNegativePeaksSM 09/09/2014

		cout << "Analyzed cross channel links with negative peaks" << endl;
		Progress = 4;
		return 0;
	}

	//
	//  In this branch, we normalize the raw data with respect to the dynamic baseline.  First, analyze the lane standard, then find
	//  the baseline and subtract it from the raw data.  Finally, refit all non-lane standard channels and continue as before.
	//

	/*status = AnalyzeLaneStandardChannelSM (sampleData->mText, sampleData->mExcelText, sampleData->mMsg, sampleData->mPrint);

	if (status < 0) {

		notice << "BIOCOMPONENT COULD NOT ANALYZE INTERNAL LANE STANDARD.  Skipping...";
		cout << notice << endl;
		sampleData->mExcelText << CLevel (1) << notice << "\n" << ErrorString << "Skipping...\n" << PLevel ();
		sampleData->mText << notice << "\n" << ErrorString << "Skipping...\n";
		SetMessageValue (ilsRequiresReview, true);
		return -4;
	}*/

	status = 0;

	for (j=1; j<=mNumberOfChannels; j++) {

		if (mDataChannels [j]->SetAllApproximateIDs (mLSData) < 0)
			status = -1;
	}

	if (NormalizeBaselineForNonILSChannelsSM () < 0) {  //  Here is where we have to modify the normalization algorithm for derivative filters

		notice << "BIOCOMPONENT COULD NOT ANALYZE BASELINE.  OMITTING BASELINE ANALYSIS...";
		cout << notice << endl;
		status = -1;
	}

	//cout << "Normalized all baselines" << endl;

	ChannelData::SetIsNormalizationPass (false);

	//RestoreRawDataAndDeleteFilteredSignalNonILS ();	// 02/02/2014:  This is now done at the channel level within normalization function.
	ChannelData::SetUseEnhancedShoulderAlgorithm (true);  // It will still only use the algorithm if directed to by the preset.
	status = FitNonLaneStandardCharacteristicsSM (sampleData->mText, sampleData->mExcelText, sampleData->mMsg, FALSE);	// ->FALSE
	FitNonLaneStandardNegativeCharacteristicsSM (sampleData->mText, sampleData->mExcelText, sampleData->mMsg, FALSE);

	if (status < 0) {

		notice << "BIOCOMPONENT COULD NOT FIT ALL NON ILS CHARACTERISTICS.  Skipping...";
		cout << notice << endl;
		sampleData->mExcelText.Write (1, notice);
		sampleData->mText << notice << "\n" << ErrorString << "Skipping...\n";
		return -3;
	}

	for (j=1; j<=mNumberOfChannels; j++) {

		if (mDataChannels [j]->SetAllApproximateIDs (mLSData) < 0)
			status = -1;
	}

	if (status < 0) {

		notice << "BIOCOMPONENT COULD NOT UTILIZE INTERNAL LANE STANDARD.  Skipping...";
		cout << notice << endl;
		SetMessageValue (ilsRequiresReview, true);
		notice = "Could not create ILS time to base pairs transform";
		SetDataForSmartMessage (ilsRequiresReview, notice);
		return -5;
	}

	//AnalyzeCrossChannelSM ();
	//AnalyzeCrossChannelWithNegativePeaksSM ();  // Commented out 01/31/2016

	//cout << "Done fitting normalized positive and negative peaks,  Preparing to perform cross channel analysis." << endl;

	AnalyzeCrossChannelUsingPrimaryWidthAndNegativePeaksSM ();
//	TestSignalsForLaserOffScaleSM ();	//  Moved inside AnalyzeCrossChannelWithNegativePeaksSM 09/09/2014

	//cout << "Analyze cross channel successful" << endl;

	Progress = 4;
	return 0;
}


int CoreBioComponent :: NormalizeBaselineForNonILSChannelsSM () {

	int i;
	int left = (int) floor (mDataChannels [mLaneStandardChannel]->GetFirstAnalyzedMean ());
	int status = 0;
	double dLeft = mDataChannels [mLaneStandardChannel]->GetFirstAnalyzedMean ();
	double dRight = mDataChannels [mLaneStandardChannel]->GetLastAnalyzedMean ();
	double dFirstChar = mLaneStandard->GetMinimumCharacteristic ();
	double dLastChar = mLaneStandard->GetMaximumCharacteristic ();
	ChannelData::SetAveSecondsPerBP ((dRight - dLeft)/(dLastChar - dFirstChar));

	double reportMin = (double) CoreBioComponent::GetMinBioIDForArtifacts ();
	double reportMinTime;

	// below if...else clause added 03/13/2015

	if (reportMin > 0.0)
		reportMinTime = mDataChannels [mLaneStandardChannel]->GetTimeForSpecifiedID (reportMin);

	else
		reportMinTime = -1.0;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel) {

			if (mDataChannels [i]->AnalyzeDynamicBaselineAndNormalizeRawDataSM (left, reportMinTime) <= 0) {

				status = -i;
			}
		}
	}

	return status;
}


int CoreBioComponent :: ResolveAmbiguousInterlocusSignalsSM () {

	return -1;
}


int CoreBioComponent :: SampleQualityTestSM (GenotypesForAMarkerSet* genotypes) {

	return -1;
}


int CoreBioComponent :: SampleQualityTestSMLF () {

	return -1;
}


int CoreBioComponent :: SignalQualityTestSM () {

	return -1;
}


bool CoreBioComponent :: IsLabPositiveControl (const RGString& name, GenotypesForAMarkerSet* genotypes) {

	IndividualGenotype* genotype;

	genotype = genotypes->FindGenotypeForFileName (name);

	if (genotype == NULL)
		return false;

	return true;
}


int CoreBioComponent :: TestPositiveControlSM (GenotypesForAMarkerSet* genotypes) {

	//
	//  This is sample stage 5
	//

	IndividualGenotype* genotype;
	smNamedPosCtrlNotFound posCtrlNotFound;
	int returnValue = 0;

	if (!mIsPositiveControl)
		return 0;

	genotype = genotypes->FindGenotypeForFileName (mControlIdName);

	if (genotype == NULL) {

		ParameterServer* pServer = new ParameterServer;
		mPositiveControlName = pServer->GetStandardPositiveControlName ();
		genotype = genotypes->FindGenotypeForFileName (mPositiveControlName);
		delete pServer;

		if (genotype == NULL) {
		
			SetMessageValue (posCtrlNotFound, true);

			if (mPositiveControlName.Length () > 0)
				AppendDataForSmartMessage (posCtrlNotFound, MainMessages::XML (mPositiveControlName));

			return -1;
		}
	}

	else
		mPositiveControlName = genotype->GetName ();

	for (int i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel) {

			if (mDataChannels [i]->TestPositiveControlSM (genotype) < 0)
				returnValue = -1;
		}			
	}

	return returnValue;
}


int CoreBioComponent :: GridQualityTestSM () {

	return -1;
}


int CoreBioComponent :: GridQualityTestSMPart2 (SmartMessagingComm& comm, int numHigherObjects) {

	return -1;
}


int CoreBioComponent :: FilterSmartNoticesBelowMinBioID () {

	if (mLSData == NULL)
		return 0;

	if (Progress < 4)
		return 0;

	int minBioID = CoreBioComponent::GetMinBioIDForArtifacts ();

	if (minBioID <= 0)
		return 0;

	int i;

	for (i=1; i<=mNumberOfChannels; i++)
		mDataChannels [i]->FilterSmartNoticesBelowMinimumBioID (mLSData, minBioID);

	return 0;
}


int CoreBioComponent :: RemoveAllSignalsOutsideLaneStandardSM () {

	//
	//  This is ladder and sample stage 1
	//

	int i;

	double dLeft = mDataChannels [mLaneStandardChannel]->GetFirstAnalyzedMean ();
	double dRight = mDataChannels [mLaneStandardChannel]->GetLastAnalyzedMean ();
	double dFirstChar = mLaneStandard->GetMinimumCharacteristic ();
	double dLastChar = mLaneStandard->GetMaximumCharacteristic ();
	ChannelData::SetAveSecondsPerBP ((dRight - dLeft)/(dLastChar - dFirstChar));

	for (i=1; i<=mNumberOfChannels; i++) {

//		if (i != mLaneStandardChannel)
			mDataChannels [i]->RemoveSignalsOutsideLaneStandardSM (mLSData);
	}

	return 0;
}


int CoreBioComponent::RemoveAllSignalsOutsideLaneStandardSMLF() {

	//
	//  This is ladder and sample stage 1
	//

	int i;

	double dLeft = mDataChannels[mLaneStandardChannel]->GetFirstAnalyzedMean();
	double dRight = mDataChannels[mLaneStandardChannel]->GetLastAnalyzedMean();
	double dFirstChar = mLaneStandard->GetMinimumCharacteristic();
	double dLastChar = mLaneStandard->GetMaximumCharacteristic();
	ChannelData::SetAveSecondsPerBP((dRight - dLeft) / (dLastChar - dFirstChar));

	for (i = 1; i <= mNumberOfChannels; i++) {

		//		if (i != mLaneStandardChannel)
		mDataChannels[i]->RemoveSignalsOutsideLaneStandardSM(mLSData);
	}

	return 0;
}


int CoreBioComponent :: PreliminarySampleAnalysisSM (RGDList& gridList, SampleDataStruct* sampleData) {

	//
	//  This is sample stage 1
	//

	smAssociatedLadderIsCritical associatedLadderIsCritical;

//	CoreBioComponent* grid = GetBestGridBasedOnTimeForAnalysis (gridList);

	const double* characteristicArray;
	mLSData->GetCharacteristicArray (characteristicArray);
	smUseMaxSecondDerivativesForSampleToLadderFit use2ndDeriv;
	bool useSecondDerivative = GetMessageValue (use2ndDeriv);

	//CSplineTransform* timeMap;
	CoreBioComponent* grid;
//	CoreBioComponent* grid = GetBestGridBasedOnMaxDelta3DerivForAnalysis (gridList, timeMap);

	if (useSecondDerivative) {

		cout << "Using 2nd derivative criterion for ladder fit..." << endl;
		grid = GetBestGridBasedOnMax2DerivForAnalysis (gridList, mTimeMap);
	}

	else {

		cout << "Using minimum error criterion for ladder fit..." << endl;
		grid = GetBestGridBasedOnLeastTransformError (gridList, mTimeMap, characteristicArray);
	}

	if (grid == NULL)
		return -1;

	//
	//	Get other fit data from timeMap
	//

	mQC.mMaxErrorInBP = mTimeMap->OutputHighDerivativesAndErrors (characteristicArray);

	//smTempUseNaturalCubicSplineForTimeTransform useNaturalCubicSpline;
	//smTempUseChordalDerivApproxHermiteSplinesForTimeTransform useChordalDerivsForHermiteSpline;
	//bool useHermite = !GetMessageValue (useNaturalCubicSpline);
	//bool useChords = GetMessageValue (useChordalDerivsForHermiteSpline);

	bool useHermite = !UseNaturalCubicSplineTimeTransform;
	bool useChords = false;

	int gridArtifactLevel = grid->GetHighestMessageLevelWithRestrictionSM ();

	if ((gridArtifactLevel > 0) && (gridArtifactLevel <= Notice::GetSeverityTrigger ()))
		SetMessageValue (associatedLadderIsCritical, true);

//	CSplineTransform* timeMap = TimeTransform (*this, *grid);
	CSplineTransform* InverseTimeMap = TimeTransform (*grid, *this, useHermite, useChords);	// Could augment calling sequence to use Hermite Cubic Spline transform 04/10/2014

	if (InverseTimeMap != NULL) {

		mAssociatedGrid = grid->CreateNewTransformedBioComponent (*grid, InverseTimeMap);

//		if (!ComputeExtendedLocusTimes (grid, InverseTimeMap))
//			cout << "Could not compute extended locus times..." << endl;

		delete InverseTimeMap;
	}

	Endl endLine;
	RGString Notice;
	Notice << "ANALYSIS WILL USE GRID NAMED " << grid->GetSampleName () << "\n";
	sampleData->mExcelText.Write (1, Notice);

	RemoveAllSignalsOutsideLaneStandardSM ();
//	ValidateAndCorrectCrossChannelAnalysesSM ();
	int status = AssignSampleCharacteristicsToLociSM (grid, mTimeMap);
	//delete timeMap;	// Added 09/26/2014 to prevent memory leak

	//int j;
	//double currentLast;
	//double largestTime = 0.0;

	//for (j=1; j<=mNumberOfChannels; j++) {

	//	if (j != mLaneStandardChannel) {

	//		currentLast = mDataChannels [j]->GetLastTime ();

	//		if (currentLast > largestTime)
	//			largestTime = currentLast;
	//	}
	//}

	//if (largestTime > mQC.mLastILSTime)
	//	mQC.mLastILSTime = largestTime;

	return status;
}


int CoreBioComponent :: PreliminarySampleAnalysisSMLF () {

	//
	//  This is sample stage 1
	//

	RemoveAllSignalsOutsideLaneStandardSM ();
//	ValidateAndCorrectCrossChannelAnalysesSM ();
	int status = AssignSampleCharacteristicsToLociSMLF ();
	//delete timeMap;	// Added 09/26/2014 to prevent memory leak

	//int j;
	//double currentLast;
	//double largestTime = 0.0;

	//for (j=1; j<=mNumberOfChannels; j++) {

	//	if (j != mLaneStandardChannel) {

	//		currentLast = mDataChannels [j]->GetLastTime ();

	//		if (currentLast > largestTime)
	//			largestTime = currentLast;
	//	}
	//}

	//if (largestTime > mQC.mLastILSTime)
	//	mQC.mLastILSTime = largestTime;

	return status;
}


int CoreBioComponent :: MeasureAllInterlocusSignalAttributesSM () {

	//
	//  This is sample stage 3
	//

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels [i]->MeasureInterlocusSignalAttributesSM ();
	}

	return 0;
}


int CoreBioComponent :: ResolveAmbiguousInterlocusSignalsUsingSmartMessageDataSM () {

	//
	//  This is sample stage 4
	//

	int i;

	for (i=1; i<=mNumberOfChannels; i++) {

		if (i != mLaneStandardChannel)
			mDataChannels [i]->ResolveAmbiguousInterlocusSignalsUsingSmartMessageDataSM ();
	}

	return 0;
}


int CoreBioComponent :: RemoveInterlocusSignalsSM () {

	return -1;
}


int CoreBioComponent :: WriteXMLGraphicDataSM (const RGString& graphicDirectory, const RGString& localFileName, SampleData* data, int analysisStage, const RGString& intro) {

	return -1;
}


int CoreBioComponent :: WriteSmartPeakInfoToXMLForChannel (int channel, RGTextOutput& text, const RGString& indent, const RGString& tagName) {

	mDataChannels [channel]->WriteSmartPeakInfoToXML (text, indent, tagName);
	return 0;
}


int CoreBioComponent :: WriteSmartArtifactInfoToXMLForChannel (int channel, RGTextOutput& text, const RGString& indent) {

	mDataChannels [channel]->WriteSmartArtifactInfoToXML (text, indent, mLSData);
	return 0;
}


void CoreBioComponent :: InitializeMessageData () {

	int size = SmartMessage::GetSizeOfArrayForScope (GetObjectScope ());
	CoreBioComponent::InitializeMessageMatrix (mMessageArray, size);
}


bool CoreBioComponent :: SignalIsWithinAnalysisRegion (DataSignal* testSignal, double firstILSTime) {

	double approxBP = testSignal->GetApproximateBioID ();

	if (approxBP >= CoreBioComponent::GetMinBioIDForLadderLoci ())
		return true;

	if (minBioIDForArtifacts > 0) {

		if (approxBP >= (double) minBioIDForArtifacts)
			return true;

		return false;
	}

	if (testSignal->GetMean () >= firstILSTime)
		return true;

	return false;
}


void CoreBioComponent :: CreateInitializationData (int scope) {

	int size = SmartMessage::GetSizeOfArrayForScope (scope);
	int i;
	SmartMessage* msg;
	delete[] InitialMatrix;
	InitialMatrix = new bool [size];

	for (i=0; i<size; i++) {

		msg = SmartMessage::GetSmartMessageForScopeAndElement (scope, i);

		if (msg != NULL)
			InitialMatrix [i] = msg->GetInitialValue ();
	}
}


void CoreBioComponent :: InitializeMessageMatrix (bool* matrix, int size) {

	int i;

	for (i=0; i<size; i++)
		matrix [i] = InitialMatrix [i];
}


//**************************************************************************************************************************************
//**************************************************************************************************************************************


