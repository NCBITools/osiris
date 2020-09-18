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
*  FileName: DataSignalSM.cpp
*  Author:   Robert Goor
*
*/
//
//   Source file for class DataSignal and subclasses.  These classes represent the measurement data and transformed 
// measurement data.  Methods allow simple transforms and basic searches for signatures...SmartMessage functions only!
//

#include "DataSignal.h"
#include "rgfile.h"
#include "rgvstream.h"
#include "rgdefs.h"
#include "TracePrequalification.h"
#include "DataInterval.h"
#include "RGTextOutput.h"
#include "RGLogBook.h"
#include "rgstring.h"
#include "OsirisMsg.h"
#include "rgindexedlabel.h"
#include "OutputLevelManager.h"
#include "SpecialLinearRegression.h"
#include "coordtrans.h"
#include "Genetics.h"
#include "SmartMessage.h"
#include "STRSmartNotices.h"
#include "xmlwriter.h"

#include <math.h>


bool InterchannelLinkage :: RemoveDataSignalSM (DataSignal* oldSignal, SmartNotice& primaryTarget, SmartNotice& primaryReplace, SmartNotice& secondaryTarget, SmartNotice& secondaryReplace) {

	// Add notice objects for test and replacement on removal

	bool wasPrimary = false;

	if (mPrimarySignal == oldSignal) {

		mPrimarySignal->SetMessageValue (primaryTarget, false);
		mPrimarySignal->SetMessageValue (primaryReplace, true);
		mPrimarySignal = NULL;
		wasPrimary = true;
	}
	
	if (mSecondarySignals.RemoveReference (oldSignal) == NULL)
		return false;

	else if (!wasPrimary) {

		oldSignal->SetMessageValue (secondaryTarget, false);
		oldSignal->SetMessageValue (secondaryReplace, true);

		if (mPrimarySignal != NULL)
			mPrimarySignal->RemovePrimaryCrossChannelSignalLinkSM (oldSignal);
	}

	return true;
}



bool InterchannelLinkage :: RecalculatePrimarySignalSM (SmartNotice&  primaryTarget, SmartNotice&  primaryReplace) {

	// Add notice objects for test and replacement on removal

	if (mSecondarySignals.Entries () <= 1)
		return false;

	if (mPrimarySignal != NULL)
		return true;
	
	DataSignal* nextSignal;
	RGDListIterator it (mSecondarySignals);
	DataSignal* maxSignal = NULL;
	double maxPeak = 0.0;
	double thisPeak;

	while (nextSignal = (DataSignal*) it ()) {

		thisPeak = nextSignal->Peak ();

		if (thisPeak > maxPeak) {

			maxPeak = thisPeak;
			maxSignal = nextSignal;
		}
	}

	if (maxSignal == NULL)
		return false;

	mPrimarySignal = maxSignal;
	mPrimarySignal->SetMessageValue (primaryTarget, false);
	mPrimarySignal->SetMessageValue (primaryReplace, true);

	it.Reset ();
	mPrimarySignal->RemoveAllCrossChannelSignalLinks ();

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal != mPrimarySignal) {

			nextSignal->SetPrimaryCrossChannelSignalLink (mPrimarySignal);
			mPrimarySignal->AddCrossChannelSignalLink (nextSignal);
		}
	}

	return true;
}



bool InterchannelLinkage :: RemoveAllSM (SmartNotice&  primaryTarget, SmartNotice&  primaryReplace, SmartNotice&  secondaryTarget, SmartNotice&  secondaryReplace) {

	// Remove meessages

	DataSignal* nextSignal;

	if (mPrimarySignal != NULL)
		mPrimarySignal->RemoveCrossChannelSignalLinkSM ();

	else {

		while (nextSignal = (DataSignal*) mSecondarySignals.GetFirst ()) {

			nextSignal->RemoveSecondaryCrossChannelSignalLinkSM ();
			// These signals are all stored on other lists, so can't delete them here.
		}

		mPrimarySignal = NULL;
	}

	return true;
}


bool InterchannelLinkage :: RemoveDataSignalSM (DataSignal* oldSignal) {

	// Add notice objects for test and replacement on removal

	//
	//  This is sample stage 3
	//

	bool wasPrimary = false;
	smPrimaryInterchannelLink primaryLink;
	smNotInterchannelLink notInterchannelLink;

	if (mPrimarySignal == oldSignal) {

		mPrimarySignal->SetMessageValue (primaryLink, false);
		mPrimarySignal->SetMessageValue (notInterchannelLink, true);
		mPrimarySignal = NULL;
		wasPrimary = true;
	}
	
	if (mSecondarySignals.RemoveReference (oldSignal) == NULL)
		return false;

	else if (!wasPrimary) {

		oldSignal->SetMessageValue (notInterchannelLink, true);

		if (mPrimarySignal != NULL)
			mPrimarySignal->RemovePrimaryCrossChannelSignalLinkSM (oldSignal);	//?????????????????????????????????????????
	}

	return true;
}


bool InterchannelLinkage :: RemoveDataSignalFromSecondaryList (DataSignal* oldSignal) {

	if (mSecondarySignals.RemoveReference (oldSignal))
		return true;

	return false;
}


bool InterchannelLinkage :: RecalculatePrimarySignalSM () {

	// Add notice objects for test and replacement on removal

	//
	//  This is sample stage 1 and 3
	//

	smPrimaryInterchannelLink primaryLink;
	smPullUp pullup;

	if (mSecondarySignals.Entries () <= 1)
		return false;

	if (mPrimarySignal != NULL)
		return true;
	
	DataSignal* nextSignal;
	RGDListIterator it (mSecondarySignals);
	DataSignal* maxSignal = NULL;
	double maxPeak = 0.0;
	double thisPeak;

	while (nextSignal = (DataSignal*) it ()) {

		thisPeak = nextSignal->Peak ();

		if (thisPeak > maxPeak) {

			maxPeak = thisPeak;
			maxSignal = nextSignal;
		}
	}

	if (maxSignal == NULL)
		return false;

	mPrimarySignal = maxSignal;
	mPrimarySignal->SetMessageValue (primaryLink, true);
	mPrimarySignal->SetMessageValue (pullup, false);

	it.Reset ();
	mPrimarySignal->RemoveAllCrossChannelSignalLinksSM ();	//???????????????????????????????????????
	list<int> pChannels;
	double ratio;
	int nextChannel = mPrimarySignal->GetChannel ();

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal != mPrimarySignal) {

			nextSignal->SetPrimaryCrossChannelSignalLink (mPrimarySignal);
			mPrimarySignal->AddCrossChannelSignalLink (nextSignal);
			nextSignal->SetMessageValue (primaryLink, false);
			nextSignal->SetMessageValue (pullup, true);
			ratio = 0.01 * floor (10000.0 * (nextSignal->Peak () / mPrimarySignal->Peak ()) + 0.5);
			nextSignal->AppendDataForSmartMessage (pullup, nextChannel);
			nextSignal->AppendDataForSmartMessage (pullup, ratio);
			pChannels.push_back (nextSignal->GetChannel ());
			//mPrimarySignal->AppendDataForSmartMessage (primaryLink, nextSignal->GetChannel ());
		}
	}

	pChannels.sort ();

	while (!pChannels.empty ()) {

		nextChannel = pChannels.front ();
		mPrimarySignal->AppendDataForSmartMessage (primaryLink, nextChannel);
		pChannels.pop_front ();
	}

	return true;
}


bool InterchannelLinkage :: RemoveAllSM () {

	// Remove meessages

	//
	//  This is sample stage 3
	//

	smNotInterchannelLink notInterchannelLink;

	DataSignal* nextSignal;

	if (mPrimarySignal != NULL) {

		mPrimarySignal->RemoveCrossChannelSignalLinkSM ();
		mPrimarySignal->SetMessageValue (notInterchannelLink, true);
	}

	else {

		while (nextSignal = (DataSignal*) mSecondarySignals.GetFirst ()) {

			nextSignal->RemoveSecondaryCrossChannelSignalLinkSM ();	//?????????????????????????????????????????????????????
			nextSignal->SetMessageValue (notInterchannelLink, true);
			// These signals are all stored on other lists, so can't delete them here.
		}

		mPrimarySignal = NULL;
	}

	return true;
}


bool InterchannelLinkage :: RecalculatePrimarySignalBasedOnValiditySM () {

	// Add notice objects for test and replacement on removal

	//
	//  This is sample stage 1 and 3
	//

	smPrimaryInterchannelLink primaryLink;
	smNotInterchannelLink notInterchannelLink;

	if ((mPrimarySignal != NULL) && mPrimarySignal->cannotBePrimary ()) {

	//	mSecondarySignals.InsertWithNoReferenceDuplication (mPrimarySignal);
		mPrimarySignal->SetMessageValue (primaryLink, false);
		//mPrimarySignal->RemoveCrossChannelSignalLinkSM ();
		//mPrimarySignal->SetMessageValue (notInterchannelLink, true);
		mPrimarySignal = NULL;
	}

	if (mSecondarySignals.Entries () <= 1) {

		RemoveAllBasedOnValiditySM ();
		return true;
	}

	if (mPrimarySignal != NULL)
		return false;
	
	DataSignal* nextSignal;
	RGDListIterator it (mSecondarySignals);
	DataSignal* maxSignal = NULL;
	double maxPeak = 0.0;
	double thisPeak;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->cannotBePrimary ())
			continue;

		thisPeak = nextSignal->Peak ();

		if (thisPeak > maxPeak) {

			maxPeak = thisPeak;
			maxSignal = nextSignal;
		}
	}

	if (maxSignal == NULL) {

		RemoveAllBasedOnValiditySM ();
		return true;
	}

	mPrimarySignal = maxSignal;
	mPrimarySignal->SetMessageValue (primaryLink, true);

	it.Reset ();
	mPrimarySignal->RemoveAllCrossChannelSignalLinksSM ();	//???????????????????????????????????????

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal != mPrimarySignal) {

			nextSignal->SetPrimaryCrossChannelSignalLink (mPrimarySignal);
			nextSignal->AddCrossChannelSignalLink (nextSignal);
			nextSignal->SetMessageValue (primaryLink, false);
		}
	}

	return true;
}


bool InterchannelLinkage :: RemoveAllBasedOnValiditySM () {

	// Remove meessages

	//
	//  This is sample stage 3
	//

	smNotInterchannelLink notInterchannelLink;
	smPullUp pullup;
	smPrimaryInterchannelLink primaryLink;

	DataSignal* nextSignal;

	if (mPrimarySignal != NULL) {

		mPrimarySignal->RemoveCrossChannelSignalLinkSM ();
		mPrimarySignal->SetMessageValue (notInterchannelLink, true);
		mPrimarySignal->SetMessageValue (pullup, false);
		mPrimarySignal->SetMessageValue (primaryLink, false);
		mPrimarySignal = NULL;
	}

	else {

		while (nextSignal = (DataSignal*) mSecondarySignals.GetFirst ()) {

			nextSignal->RemoveSecondaryCrossChannelSignalLinkSM ();	//?????????????????????????????????????????????????????
			nextSignal->SetMessageValue (notInterchannelLink, true);
			nextSignal->SetMessageValue (pullup, false);
			nextSignal->SetMessageValue (primaryLink, false);
			// These signals are all stored on other lists, so can't delete them here.
		}

		mPrimarySignal = NULL;
	}

	return true;
}


bool InterchannelLinkage :: PrimaryHasLaserOffScaleSM () const {

	smLaserOffScale primaryOffScale;

	if (mPrimarySignal == NULL)
		return false;

	return mPrimarySignal->GetMessageValue (primaryOffScale);
}


bool InterchannelLinkage :: SecondaryHasLaserOffScaleSM () {

	smLaserOffScale secondaryOffScale;
	RGDListIterator it (mSecondarySignals);
	DataSignal* nextSignal;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->GetMessageValue (secondaryOffScale))
			return true;
	}

	return false;
}


bool InterchannelLinkage :: AnySignalHasLaserOffScaleSM () {

	if (PrimaryHasLaserOffScaleSM ())
		return true;

	return SecondaryHasLaserOffScaleSM ();
}


bool InterchannelLinkage :: SecondaryIsSigmoidalSignalSM (int secondaryChannel) {

	smSigmoidalPullup sigmoidalPullup;
	RGDListIterator it (mSecondarySignals);
	DataSignal* nextSignal;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->GetChannel () == secondaryChannel)
			return nextSignal->GetMessageValue (sigmoidalPullup);
	}

	return false;
}


bool InterchannelLinkage :: PossibleSecondaryPullupWithNoOffScaleSM (int primaryChannel, int secondaryChannel, double& secondaryRatio, bool& isSigmoidal) {

	smSigmoidalPullup sigmoidalPullup;
	isSigmoidal = false;

	if (AnySignalHasLaserOffScaleSM ()) {

		secondaryRatio = 0.0;
		return false;
	}

	if ((mPrimarySignal == NULL) || (mPrimarySignal->GetChannel () != primaryChannel)) {

		secondaryRatio = 0.0;
		return false;
	}

	if (mPrimarySignal->Peak () < 0.1) {

		secondaryRatio = 0.0;
		return false;
	}

	RGDListIterator it (mSecondarySignals);
	DataSignal* nextSignal;
	bool foundSecondaryChannel = false;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->GetChannel () == secondaryChannel) {

			foundSecondaryChannel = true;
			secondaryRatio = nextSignal->Peak () / mPrimarySignal->Peak ();
			isSigmoidal = nextSignal->GetMessageValue (sigmoidalPullup);

			if (nextSignal->IsNegativePeak ())
				secondaryRatio = -secondaryRatio;

			break;
		}
	}

	return foundSecondaryChannel;
}


bool InterchannelLinkage :: PossibleSecondaryPullupSM (int primaryChannel, int secondaryChannel, double& secondaryRatio, bool& isSigmoidal, DataSignal*& secondarySignal) {

	smSigmoidalPullup sigmoidalPullup;
	isSigmoidal = false;

	if ((mPrimarySignal == NULL) || (mPrimarySignal->GetChannel () != primaryChannel)) {

		secondaryRatio = 0.0;
		return false;
	}

	if (mPrimarySignal->Peak () < 0.1) {

		secondaryRatio = 0.0;
		return false;
	}

	RGDListIterator it (mSecondarySignals);
	DataSignal* nextSignal;
	bool foundSecondaryChannel = false;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->GetChannel () == secondaryChannel) {

			foundSecondaryChannel = true;
			secondarySignal = nextSignal;
			secondaryRatio = nextSignal->Peak () / mPrimarySignal->Peak ();
			isSigmoidal = nextSignal->GetMessageValue (sigmoidalPullup);

			if (nextSignal->IsNegativePeak ())
				secondaryRatio = -secondaryRatio;

			break;
		}
	}

	return foundSecondaryChannel;
}


// Smart Message Functions***************************************************************************
//***************************************************************************************************


void DataSignal :: SetMessageValue (int scope, int location, bool value) {

	int myScope = GetObjectScope ();

	if (myScope == scope) {

		SmartMessage* msg = SmartMessage::GetSmartMessageForScopeAndElement (scope, location);
		msg->SetMessageValue (mMessageArray, mValueArray, location, value);

		if (value) {

			bool call = msg->EvaluateCall (mMessageArray);

			if (!call)
				mDoNotCall = true;
		}
	}
}


void DataSignal :: SetMessageValue (const SmartNotice& notice, bool value) {

	int scope = GetObjectScope ();

	if (notice.GetScope () == scope) {

		int index = notice.GetMessageIndex ();
		mMessageArray [index] = value;

		if (value) {

			SmartMessage* msg = SmartMessage::GetSmartMessageForScopeAndElement (scope, index);
			bool call = msg->EvaluateCall (mMessageArray);

			if (!call)
				mDoNotCall = true;
		}
	}
}


void DataSignal :: SetMessageValue (int scope, int location, bool value, bool useVirtualMethod) {

	if (useVirtualMethod)
		SetMessageValue (scope, location, value);

	else if (scope == GetObjectScope ()) {

		mMessageArray [location] = value;

		if (value) {

			SmartMessage* msg = SmartMessage::GetSmartMessageForScopeAndElement (scope, location);
			bool call = msg->EvaluateCall (mMessageArray);

			if (!call)
				mDoNotCall = true;
		}
	}
}


bool DataSignal :: EvaluateSmartMessagesForStage (int stage) {

	RGDList temp;

	if (stage <= mStageCompleted)
		return false;

	mStageCompleted = stage;
	return SmartMessage::EvaluateAllMessages (mMessageArray, temp, stage, GetObjectScope ());
}


bool DataSignal :: EvaluateSmartMessagesForStage (SmartMessagingComm& comm, int numHigherObjects, int stage) {

	if (stage <= mStageCompleted)
		return false;

	mStageCompleted = stage;
	comm.SMOStack [numHigherObjects] = (SmartMessagingObject*) this;
	return SmartMessage::EvaluateAllMessages (comm, numHigherObjects + 1, stage, GetObjectScope ());
}


bool DataSignal :: SetTriggersForAllMessages (bool* const higherMsgMatrix, RGHashTable* messageDataTable, int higherScope, int stage) {

	if (stage <= mTriggerStageCompleted)
		return false;

	mTriggerStageCompleted = stage;
	double bp = GetApproximateBioID ();
	int ibp = (int) floor (bp + 0.5);

	return SmartMessage::SetTriggersForAllMessages (mMessageArray, higherMsgMatrix, messageDataTable, stage, GetObjectScope (), higherScope, ibp, mAlleleName);
}


bool DataSignal :: SetTriggersForAllMessages (SmartMessagingComm& comm, int numHigherObjects, int stage) {

	if (stage <= mTriggerStageCompleted)
		return false;

	mTriggerStageCompleted = stage;
	double bp = GetApproximateBioID ();
	int ibp = (int) floor (bp + 0.5);
	comm.SMOStack [numHigherObjects] = (SmartMessagingObject*) this;

	return SmartMessage::SetTriggersForAllMessages (comm, numHigherObjects + 1, stage, GetObjectScope (), ibp, mAlleleName);
}


bool DataSignal :: EvaluateAllReports (bool* const reportMatrix) {

	return SmartMessage::EvaluateAllReports (mMessageArray, reportMatrix, GetObjectScope ());
}


bool DataSignal :: TestAllMessagesForCall () {

	return SmartMessage::TestAllMessagesForCall (mMessageArray, GetObjectScope ());
}


bool DataSignal :: EvaluateAllReportLevels (int* const reportLevelMatrix) {

	return SmartMessage::EvaluateAllReportLevels (mMessageArray, reportLevelMatrix, GetObjectScope ());
}


void DataSignal :: CapturePullupDataFromSM (DataSignal* prevSignal, DataSignal* nextSignal) {

	smPullUp partialPullup;
	smCalculatedPurePullup purePullup;
	smPartialPullupBelowMinRFU partialPUBelowMinRFU;
	smPrimaryInterchannelLink primaryPullup;
	RGString prevData;
	RGString nextData;

	if (prevSignal->GetMessageValue (partialPullup) || nextSignal->GetMessageValue (partialPullup)) {

		prevData = prevSignal->GetDataForNoticeSM (partialPullup);
		nextData = nextSignal->GetDataForNoticeSM (partialPullup);
		SetMessageValue (partialPullup, true);

		if (nextData.Length () >= prevData.Length ()) {

			nextData.FindAndReplaceAllSubstrings (" from Primary Channel ", "");
			SetDataForSmartMessage (partialPullup, nextData);
			CapturePullupCorrections (nextSignal);
		}

		else {

			prevData.FindAndReplaceAllSubstrings (" from Primary Channel ", "");
			SetDataForSmartMessage (partialPullup, prevData);
			CapturePullupCorrections (prevSignal);
		}
	}

	else if (prevSignal->GetMessageValue (purePullup) || nextSignal->GetMessageValue (purePullup)) {

		prevData = prevSignal->GetDataForNoticeSM (purePullup);
		nextData = nextSignal->GetDataForNoticeSM (purePullup);
		SetMessageValue (purePullup, true);

		if (nextData.Length () >= prevData.Length ()) {

			nextData.FindAndReplaceAllSubstrings (" from Primary Channel ", "");
			SetDataForSmartMessage (purePullup, nextData);
			CapturePullupCorrections (nextSignal);
		}

		else {

			prevData.FindAndReplaceAllSubstrings (" from Primary Channel ", "");
			SetDataForSmartMessage (purePullup, prevData);
			CapturePullupCorrections (prevSignal);
		}
	}

	else if (prevSignal->GetMessageValue (partialPUBelowMinRFU) || nextSignal->GetMessageValue (partialPUBelowMinRFU)) {

		prevData = prevSignal->GetDataForNoticeSM (partialPUBelowMinRFU);
		nextData = nextSignal->GetDataForNoticeSM (partialPUBelowMinRFU);
		SetMessageValue (partialPUBelowMinRFU, true);

		if (nextData.Length () >= prevData.Length ()) {

			nextData.FindAndReplaceAllSubstrings (" from Primary Channel ", "");
			SetDataForSmartMessage (partialPUBelowMinRFU, nextData);
			CapturePullupCorrections (nextSignal);
		}

		else {

			prevData.FindAndReplaceAllSubstrings (" from Primary Channel ", "");
			SetDataForSmartMessage (partialPUBelowMinRFU, prevData);
			CapturePullupCorrections (prevSignal);
		}
	}

	 if (prevSignal->GetMessageValue (primaryPullup) || nextSignal->GetMessageValue (primaryPullup)) {

		prevData = prevSignal->GetDataForNoticeSM (primaryPullup);
		nextData = nextSignal->GetDataForNoticeSM (primaryPullup);
		SetMessageValue (primaryPullup, true);

		if (nextData.Length () >= prevData.Length ()) {

			nextData.FindAndReplaceAllSubstrings (" into channel(s): ", "");
			SetDataForSmartMessage (primaryPullup, nextData);
		}

		else {

			prevData.FindAndReplaceAllSubstrings (" into channel(s): ", "");
			SetDataForSmartMessage (primaryPullup, prevData);
		}
	}
}


double DataSignal::GetTotalPullupFromOtherChannelsSM (int numberOfChannels) const {

	smCalculatedPurePullup purePullup;
	smPullUp partialPullup;
	smPartialPullupBelowMinRFU partialPullupBelowMinRFU;

	if (mPullupCorrectionArray == NULL)
		return 0.0;

	if (GetMessageValue (purePullup))
		return Peak ();

	bool isPullup = GetMessageValue (partialPullup) || GetMessageValue (partialPullupBelowMinRFU);

	if (!isPullup)
		return 0.0;

	int i;
	double total = 0.0;

	for (i=1; i<=numberOfChannels; i++)
		total += GetPullupFromChannel (i);

	double p = Peak ();

	if (total >= p)
		return p;

	return total;
}


void DataSignal :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	int higherObjectIndex = numHigherObjects - 2;
	SmartMessagingObject* higherObj = comm.SMOStack [higherObjectIndex];
	higherObj->OutputDebugID (comm, higherObjectIndex + 1);
}


RGString DataSignal :: GetDebugIDIndent () const {

	return "\t\t\t";
}



void DataSignal :: AssociateDataWithPullMessageSM (int nChannels) {

	int i;
	RGString data;
	int n = 0;
	int k = 0;
	smPullUp pullup;
	smCalculatedPurePullup purePullup;
	smPartialPullupBelowMinRFU partialPullupBelowMin; 
	smUncertainPullUp uncertainPullup;
	DataSignal* primary;
	RGString uncertain;
	RGString narrow;
	RGString channelList;
	bool uncertainListNotEmpty = false;
	RGString data2;
	RGString temp;
	RGString pResult;
	RGString dataPure;
	int nPure = 0;

	if (!HasAnyPrimarySignals (nChannels))
		return;

	if (IsNegativePeak ())
		return;

	for (i=1; i<=nChannels; i++) {

		if (i == GetChannel ())
			continue;

		primary = HasPrimarySignalFromChannel (i);

		if (primary != NULL) {

			if (PullupChannelIsUncertain (i)) {

				k = 1;
				uncertainListNotEmpty = true;
				continue;
			}

			if (mPrimaryRatios == NULL)
				cout << "Primary signal is set but not ratio for signal on channel " << GetChannel () << " at time " << GetMean () << endl;

			temp.ConvertWithMin (GetPullupRatio (i), 0.01, 2);

			if (GetIsPurePullupFromChannel (i)) {

				if (nPure == 0)
					nPure = 1;

				else
					dataPure << "; ";

				dataPure << i << " with Ratio " << xmlwriter::EscAscii (temp, &pResult);
			}

			else {

				if (n == 0)
					n = 1;

				else
					data << "; ";

				data << i << " with Ratio " << xmlwriter::EscAscii (temp, &pResult);
			}
		}
	}

	if (n > 0)
		data << "%";

	if (nPure > 0)
		dataPure << "%";

	if (nPure + n + k > 0) {

		bool isPullup = GetMessageValue (pullup);
		bool isPurePullup = GetMessageValue (purePullup);
		bool isPartialPullupBelowMin = GetMessageValue (partialPullupBelowMin);
		bool isUncertainPullup = mIsPossiblePullup;
		data2 = data;

		if (isUncertainPullup) {

			channelList = CreateUncertainPullupString ();
			SetMessageValue (uncertainPullup, true);
			AppendDataForSmartMessage (uncertainPullup, channelList);
		}

		if (isPullup) {

			//if (mIsPossiblePullup) {

			//	uncertain << "(Uncertain Channel(s): ";
			//	channelList = CreateUncertainPullupString ();
			//	uncertain << channelList;
			//	uncertain << ") ";
			//	data = uncertain + data;
			//	//AppendDataForSmartMessage (pullup, uncertain);
			//}
			if (data.Length () > 0)
				AppendDataForSmartMessage (pullup, data);

			else
				SetMessageValue (pullup, false);
		}

		if (isPurePullup) {

			//if (mIsPossiblePullup) {

			//	narrow << "(Narrow Channels: ";
			//	channelList = CreateUncertainPullupString ();
			//	narrow << channelList;
			//	narrow << ") ";
			//	data2 = narrow + data2;
			//	//AppendDataForSmartMessage (pullup, uncertain);
			//}

			AppendDataForSmartMessage (purePullup, dataPure);
		}

		if (isPartialPullupBelowMin) {

			//if (mIsPossiblePullup) {

			//	uncertain << "(Uncertain Channel(s): ";
			//	channelList = CreateUncertainPullupString ();
			//	uncertain << channelList;
			//	uncertain << ") ";
			//	data = uncertain + data;
			//	//AppendDataForSmartMessage (pullup, uncertain);
			//}

			if (data.Length () > 0)
				AppendDataForSmartMessage (partialPullupBelowMin, data);

			else
				SetMessageValue (partialPullupBelowMin, false);
		}
	}
}


void DataSignal :: AddDataToStutterArtifactSM () {

	smStutter stutter;
	int disp;
	int nStutters = mStutterPrimaryList.Entries ();
	RGString data;
	int n = 0;
	DataSignal* nextSignal;
	//RGDListIterator it (mStutterPrimaryList);
	bool isStandardDisplacement;
	RGString ratioString;
	RGString pResult;
	mStutterDisplacements.sort ();
	double ratio;
	bool hasDuplicates = false;
	RGDList tempSignals;
	set<int> tempDisp;

	while (nextSignal = (DataSignal*) mStutterPrimaryList.GetLast ()) {

		disp = mStutterDisplacements.front ();
		mStutterDisplacements.pop_front ();
		tempSignals.Append (nextSignal);

		if (tempDisp.find (disp) != tempDisp.end ())
			hasDuplicates = true;

		tempDisp.insert (disp);

		if (nextSignal->SignalIsStandardStutter ((DataSignal*)this))
			isStandardDisplacement = true;

		else
			isStandardDisplacement = false;

		if (n > 0)
			data << ", ";

		else
			ratio = 100.0 * Peak () / nextSignal->Peak ();

		n++;

		if (disp > 0)
			data << "+";

		data << disp;

		if (isStandardDisplacement)
			data << " (std)";
	}

	data << " bps";

	if (nStutters == 1) {

		data << ":  ratio = ";
		nextSignal = (DataSignal*) mStutterPrimaryList.First ();
		ratioString.ConvertWithMin (ratio, 0.01, 2);
		data << xmlwriter::EscAscii (ratioString, &pResult) << "%";
	}

	if (hasDuplicates) {

		cout << "Duplicate displacements from channel " << GetChannel () << ":  ";
		n = 0;

		while (nextSignal = (DataSignal*) tempSignals.GetFirst ()) {

			if (n > 0)
				cout << "; ";

			n++;
			cout << "mean = " << nextSignal->GetMean () << " and name = " << nextSignal->GetAlleleName ();
		}

		cout << endl << endl;
	}

	tempDisp.clear ();
	tempSignals.Clear ();

	SetDataForSmartMessage (stutter, data);
}


void DataSignal :: CaptureSmartMessages (const DataSignal* signal) {

	int scope = GetObjectScope ();
	int size = SmartMessage::GetSizeOfArrayForScope (scope);
	int i;

	for (i=0; i<size; i++){

		if (signal->mMessageArray [i])
			SetMessageValue (scope, i, true);
	}

	smSpike spike;

	if (GetWidth () > 2.1)
		SetMessageValue (spike, false);

	//smPullUp pullup;

	//if (signal->GetMessageValue (pullup)) {

	//	int index = pullup.GetMessageIndex ();
	//	SmartMessageData target (index);
	//	SmartMessageData* smd = (SmartMessageData*)mMessageDataTable->Find (&target);

	//	if (smd == NULL)
	//		return;

	//	RGString text = smd->GetText ();
	//	AppendDataForSmartMessage (pullup, text);
	//}
}


void DataSignal :: CaptureSmartMessages () {

}


bool DataSignal :: TestForMultipleSignalsWithinLocus (DataSignal*& prev, DataSignal*& next, int location, bool isAmel, double adenylationLimit) {

	prev = next = NULL;
	return false;
}


bool DataSignal :: TestSignalGroupsWithinILS (double ilsLeft, double ilsRight, double minBioID) {

	//
	//  This is ladder and sample stage 1
	//

	return false;
}


void DataSignal :: WriteSmartPeakInfoToXML (RGTextOutput& text, const RGString& indent, const RGString& bracketTag, const RGString& locationTag) {

	int peak;
	Endl endLine;
	RGString suffix;
	double totalCorrection;
	
//	if (HasCrossChannelSignalLink ()) {

		if (HasAlleleName () && (!mDoNotCall)) {

			peak = (int) floor (Peak () + 0.5);

			if (mOffGrid)
				suffix = " OL";

			text << indent << "<" << bracketTag << ">" << endLine;
			text << indent << "\t<mean>" << GetMean () << "</mean>" << endLine;
			text << indent << "\t<height>" << peak << "</height>" << endLine;

			totalCorrection = GetTotalPullupFromOtherChannels (NumberOfChannels);

	//		if (totalCorrection != 0.0)
				text << indent << "\t<PullupHeightCorrection>" << totalCorrection << "</PullupHeightCorrection>" << endLine;
				text << indent << "\t<PullupCorrectedHeight>" << (int)floor (Peak () - totalCorrection + 0.5) << "</PullupCorrectedHeight>" << endLine;

			text << indent << "\t<BPS>" << GetBioID () << "</BPS>" << endLine;
//			text << indent << "\t<" << locationTag << ">" << (int) floor (GetApproximateBioID () + 0.5) << "</" << locationTag << ">" << endLine;
			text << indent << "\t<" << locationTag << ">" << GetApproximateBioID () << "</" << locationTag << ">" << endLine;
			text << indent << "\t<PeakArea>" << TheoreticalArea () << "</PeakArea>" << endLine;
			text << indent << "\t<Width>" << GetWidth () << "</Width>" << endLine;
			text << indent << "\t<allele>" << GetAlleleName () << suffix << "</allele>" << endLine;
			text << indent << "\t<fit>" << GetCurveFit () << "</fit>" << endLine;
			text << indent << "</" + bracketTag << ">" << endLine;
		}
//	}
}


void DataSignal :: WriteSmartArtifactInfoToXML (RGTextOutput& text, const RGString& indent, const RGString& bracketTag, const RGString& locationTag) {

	int peak;
	Endl endLine;
	RGString suffix;
	RGString label;
	SmartMessageReporter* notice;
	int i;
	RGString virtualAllele;
	int reportedMessageLevel;

	reportedMessageLevel = GetHighestMessageLevelWithRestrictionSM ();
	bool thisIsFirstNotice = true;
	double totalCorrection;

	if ((!DontLook ()) && (NumberOfSmartNoticeObjects () != 0)) {
	
		peak = (int) floor (Peak () + 0.5);

		if (mOffGrid)
			suffix = " OL";

		virtualAllele = GetVirtualAlleleName ();

		RGDListIterator it (*mSmartMessageReporters);
		i = 0;

		while (notice = (SmartMessageReporter*) it ()) {

			if (!notice->GetDisplayOsirisInfo ())
				continue;

			if (thisIsFirstNotice) {

				text << indent << "<" << bracketTag << ">" << endLine;
				text << indent << "\t<level>" << reportedMessageLevel << "</level>" << endLine;
				text << indent << "\t<mean>" << GetMean () << "</mean>" << endLine;
				text << indent << "\t<height>" << peak << "</height>" << endLine;

				totalCorrection = GetTotalPullupFromOtherChannels (NumberOfChannels);

	//			if (totalCorrection != 0.0)
					text << indent << "\t<PullupHeightCorrection>" << totalCorrection << "</PullupHeightCorrection>" << endLine;
					text << indent << "\t<PullupCorrectedHeight>" << (int)floor (Peak () - totalCorrection + 0.5) << "</PullupCorrectedHeight>" << endLine;

//				text << indent << "\t<" << locationTag << ">" << (int) floor (GetApproximateBioID () + 0.5) << "</" << locationTag << ">" << endLine;
				text << indent << "\t<" << locationTag << ">" << GetApproximateBioID () << "</" << locationTag << ">" << endLine;
				text << indent << "\t<PeakArea>" << TheoreticalArea () << "</PeakArea>" << endLine;
				text << indent << "\t<Width>" << GetWidth () << "</Width>" << endLine;

				if (virtualAllele.Length () > 0)
					text << indent << "\t<equivAllele>" << virtualAllele << suffix << "</equivAllele>" << endLine;

				text << indent << "\t<fit>" << GetCurveFit () << "</fit>" << endLine;
				label = indent + "\t<label>";
				thisIsFirstNotice = false;
			}

			if (i > 0)
				label << "&#10;";

			label += notice->GetMessage ();
			label += notice->GetMessageData ();
			i++;
		}

		RGString temp = GetVirtualAlleleName () + suffix;

		if (temp.Length () > 0) {

			if (i > 0)
				label << "&#10;";

			label += "Allele " + temp;
		}

		if (i > 0) {

			label << "</label>";
			text << label << endLine;
			text << indent << "</" + bracketTag << ">" << endLine;
		}

		/*if ((signalLink != NULL) && (!signalLink->xmlArtifactInfoWritten)) {

			signalLink->xmlArtifactInfoWritten = true;
			peak = signalLink->height;

			text << indent << "<" << bracketTag << ">" << endLine;
			text << indent << "\t<mean>" << signalLink->mean << "</mean>" << endLine;
			text << indent << "\t<height>" << peak << "</height>" << endLine;
			text << indent << "\t<" << locationTag << ">" << signalLink->bioID << "</" << locationTag << ">" << endLine;
			text << indent << "\t<fit>" << GetCurveFit () << "</fit>" << endLine;
			label = indent + "\t<label>";
			mNoticeObjectIterator.Reset ();
			i = 0;

			while (notice = (Notice*) mNoticeObjectIterator ()) {

				if (i > 0)
					label << "&#10;";

				label += notice->GetLabel ();
				i++;
			}

			if (temp.Length () > 0) {

				if (i > 0)
					label << "&#10;";

				label += "Allele " + temp;
			}

			label << "</label>";
			text << label << endLine;
			text << indent << "</" + bracketTag << ">" << endLine;
		}*/
	}
}


void DataSignal :: WriteSmartTableArtifactInfoToXML (RGTextOutput& text, RGTextOutput& tempText, const RGString& indent, const RGString& bracketTag, const RGString& locationTag) {

	int peak;
	Endl endLine;
	RGString suffix;
	RGString label;
	SmartMessageReporter* notice;
	int i;
	RGString virtualAllele;
	SmartMessageReporter* nextNotice;
	text.SetOutputLevel (1);
	int msgNum;
	double totalCorrection;

	smAcceptedOLLeft acceptedOLLeft;
	smAcceptedOLRight acceptedOLRight;

	if (mHasReportedArtifacts)
		return;

	mHasReportedArtifacts = true;

	int reportedMessageLevel = GetHighestMessageLevelWithRestrictionSM ();
	//bool hasThreeLoci;
	//bool needLocus0;

	if ((!DontLook ()) && (NumberOfSmartNoticeObjects () != 0)) {
	
		bool firstNotice = true;
		peak = (int) floor (Peak () + 0.5);
		virtualAllele = GetVirtualAlleleName ();

		text << indent << "<" << bracketTag << ">" << endLine;  // Should be <Artifact>
		text << indent << "\t<Id>" << GetSignalID () << "</Id>" << endLine;
		text << indent << "\t<Level>" << reportedMessageLevel << "</Level>" << endLine;
		text << indent << "\t<RFU>" << peak << "</RFU>" << endLine;

		totalCorrection = GetTotalPullupFromOtherChannels (NumberOfChannels);

	//	if (totalCorrection != 0.0)
			text << indent << "\t<PullupHeightCorrection>" << totalCorrection << "</PullupHeightCorrection>" << endLine;
			text << indent << "\t<PullupCorrectedHeight>" << (int)floor (Peak () - totalCorrection + 0.5) << "</PullupCorrectedHeight>" << endLine;

		text << indent << "\t<" << locationTag << ">" << GetApproximateBioID () << "</" << locationTag << ">" << endLine;
		text << indent << "\t<PeakArea>" << TheoreticalArea () << "</PeakArea>" << endLine;
		text << indent << "\t<Width>" << GetWidth () << "</Width>" << endLine;
		text << indent << "\t<Time>" << GetMean () << "</Time>" << endLine;
		text << indent << "\t<Fit>" << GetCurveFit () << "</Fit>" << endLine;

		if (!mAllowPeakEdit)
			text << indent << "\t<AllowPeakEdit>false</AllowPeakEdit>" << endLine;
		
		RGDListIterator it (*mSmartMessageReporters);
		i = 0;

		while (notice = (SmartMessageReporter*) it ()) {

			if (!notice->GetDisplayOsirisInfo ())
				continue;

			if (firstNotice) {

				label = indent + "\t<Label>";
				firstNotice = false;
			}

			if (i > 0)
				label << "&#10;";

			label += notice->GetMessage ();
			label += notice->GetMessageData ();
			i++;
		}

		if (i > 0) {

			label << "</Label>";
			text << label << endLine;
		}

		// Now add list of notices...

		it.Reset ();

		while (nextNotice = (SmartMessageReporter*) it ()) {

			msgNum = Notice::GetNextMessageNumber ();
			nextNotice->SetMessageCount (msgNum);
			text << indent << "\t<MessageNumber>" << msgNum << "</MessageNumber>" << endLine;
			tempText << "\t\t<Message>\n";
			tempText << "\t\t\t<MessageNumber>" << msgNum << "</MessageNumber>\n";
			tempText << "\t\t\t<Text>" << nextNotice->GetMessage () << nextNotice->GetMessageData () << "</Text>\n";

			if (nextNotice->HasViableExportInfo ()) {

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

		// Now add list of alleles

		//hasThreeLoci = (mLocus != NULL) && (mLeftLocus != NULL) && (mRightLocus != NULL);

		//if (mLocus != NULL) {

		//	if ((mLeftLocus == NULL) && (mRightLocus == NULL))
		//		needLocus0 = true;

		//	else
		//		needLocus0 = false;
		//}

		//else
		//	needLocus0 = false;

		//needLocus0 = (!hasThreeLoci) && ((mLocus != mLeftLocus) || (mLocus != mRightLocus));

		if (mLocus != NULL) {

			//testing
			RGString locusName = mLocus->GetLocusName ();
			suffix = GetAlleleName (0);

			if ((suffix.Length () > 0) || (virtualAllele.Length () > 0)) {

				text << indent << "\t<Allele>" << endLine;

				if (suffix.Length () > 0)
					text << indent << "\t\t<Name>" << suffix << "</Name>" << endLine;

				else
					text << indent << "\t\t<Name>" << virtualAllele << "</Name>" << endLine;

				if (mOffGrid)
					suffix = "true";

				else if (mAcceptedOffGrid)
					suffix = "accepted";

				else
					suffix = "false";

				text << indent << "\t\t<OffLadder>" << suffix << "</OffLadder>" << endLine;
				text << indent << "\t\t<BPS>" << GetBioID (0) << "</BPS>" << endLine;
				text << indent << "\t\t<Locus>" << mLocus->GetLocusName () << "</Locus>" << endLine;
				text << indent << "\t\t<Location>0</Location>" << endLine;
				text << indent << "\t</Allele>" << endLine;
			}
		}

		if ((mLeftLocus != NULL) && (mLeftLocus != mLocus)) {

			if (mAlleleNameLeft.Length () > 0) {

				text << indent << "\t<Allele>" << endLine;
				text << indent << "\t\t<Name>" << mAlleleNameLeft << "</Name>" << endLine;

				if (mIsOffGridLeft)
					suffix = "true";

				else if (GetMessageValue (acceptedOLLeft))
					suffix = "accepted";

				else
					suffix = "false";

				text << indent << "\t\t<OffLadder>" << suffix << "</OffLadder>" << endLine;
				text << indent << "\t\t<BPS>" << GetBioID (-1) << "</BPS>" << endLine;
				text << indent << "\t\t<Locus>" << mLeftLocus->GetLocusName () << "</Locus>" << endLine;
				text << indent << "\t\t<Location>-1</Location>" << endLine;
				text << indent << "\t</Allele>" << endLine;
			}
		}

		if ((mRightLocus != NULL) && (mRightLocus != mLocus)) {

			if (mAlleleNameRight.Length () > 0) {

				text << indent << "\t<Allele>" << endLine;
				text << indent << "\t\t<Name>" << mAlleleNameRight << "</Name>" << endLine;

				if (mIsOffGridRight)
					suffix = "true";

				else if (GetMessageValue (acceptedOLRight))
					suffix = "accepted";

				else
					suffix = "false";

				text << indent << "\t\t<OffLadder>" << suffix << "</OffLadder>" << endLine;
				text << indent << "\t\t<BPS>" << GetBioID (1) << "</BPS>" << endLine;
				text << indent << "\t\t<Locus>" << mRightLocus->GetLocusName () << "</Locus>" << endLine;
				text << indent << "\t\t<Location>1</Location>" << endLine;
				text << indent << "\t</Allele>" << endLine;
			}
		}

		text << indent << "</" + bracketTag << ">" << endLine;
	}

	text.ResetOutputLevel ();
}


bool DataSignal :: ReportSmartNoticeObjects (RGTextOutput& text, const RGString& indent, const RGString& delim) {

	if (NumberOfSmartNoticeObjects () > 0) {

		int msgLevel = GetHighestMessageLevelWithRestrictionSM ();
		RGDListIterator it (*mSmartMessageReporters);
		SmartMessageReporter* nextNotice;
		text.SetOutputLevel (msgLevel);

		if (!text.TestCurrentLevel ()) {

			text.ResetOutputLevel ();
			return false;
		}

		Endl endLine;
		text << endLine;
		text << indent << "Notices for curve with (Mean, Sigma, Peak, 2Content, Fit) = " << delim << delim << delim << delim << delim << delim;
		text << GetMean () << delim << GetStandardDeviation () << delim << Peak () << delim << GetScale (2) << delim << Fit << endLine;

		while (nextNotice = (SmartMessageReporter*) it ())
			text << indent << nextNotice->GetMessage () << nextNotice->GetMessageData () << endLine;

		text.ResetOutputLevel ();
		text.Write (1, "\n");
	}

	else
		return false;

	return true;
}


int DataSignal :: AddAllSmartMessageReporters () {

	int k = GetObjectScope ();
	int size = SmartMessage::GetSizeOfArrayForScope (k);
	int i;
	int nMsgs = 0;
	SmartMessageReporter* newMsg;
	SmartMessage* nextSmartMsg;
	SmartMessageData target;
	SmartMessageData* smd;
	bool call;

	for (i=0; i<size; i++) {

		if (!mMessageArray [i])
			continue;

		nextSmartMsg = SmartMessage::GetSmartMessageForScopeAndElement (k, i);
		call = nextSmartMsg->EvaluateCall (mMessageArray);

		if (!call)
			mDoNotCall = true;

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
		newMsg->SetDoNotCall (!call);
		nMsgs = AddSmartMessageReporter (newMsg);
	}

	MergeAllSmartMessageReporters ();
	return nMsgs;
}


int DataSignal :: AddAllSmartMessageReporters (SmartMessagingComm& comm, int numHigherObjects) {

	if (mReportersAdded)
		return 0;

	mReportersAdded = true;

	int k = GetObjectScope ();
	int size = SmartMessage::GetSizeOfArrayForScope (k);
	int i;
	int nMsgs = 0;
	SmartMessageReporter* newMsg;
	SmartMessage* nextSmartMsg;
	SmartMessageData target;
	SmartMessageData* smd;
	bool call;
	RGString exportProtocol;
	bool editable;
	bool enabled;
	bool hasExportProtocolInfo;
	bool report;
	bool mirror;
	bool displayExport;

	comm.SMOStack [numHigherObjects] = (SmartMessagingObject*) this;
	int topNum = numHigherObjects + 1;

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

		call = nextSmartMsg->EvaluateCall (comm, topNum);

		if (!call)
			mDoNotCall = true;

		if (!nextSmartMsg->GetAllowPeakEdit ())
			mAllowPeakEdit = false;

		report = nextSmartMsg->EvaluateReportContingent (comm, topNum);
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

		newMsg->SetPriorityLevel (nextSmartMsg->EvaluateReportLevel (comm, topNum));
		newMsg->SetRestrictionLevel (nextSmartMsg->EvaluateRestrictionLevel (comm, topNum));
		newMsg->SetDoNotCall (!call);
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


void DataSignal :: InitializeMessageData () {

	int size = SmartMessage::GetSizeOfArrayForScope (GetObjectScope ());
	DataSignal::InitializeMessageMatrix (mMessageArray, size);
}


void DataSignal :: RemoveCrossChannelSignalLinkSM () {

	//
	//  This is sample stage 3
	//

	smNotInterchannelLink notALink;
	smPrimaryInterchannelLink primaryLink;

	if (mPrimaryCrossChannelLink != NULL) {

		// This is a secondary link

		mPrimaryCrossChannelLink->RemovePrimaryCrossChannelSignalLinkSM (this);
		mPrimaryCrossChannelLink = NULL;
		SetMessageValue (notALink, true);
	}

	else if (mCrossChannelSignalLinks.Entries () > 0) {

		// This is a primary link

		DataSignal* nextSignal;

		while (nextSignal = (DataSignal*) mCrossChannelSignalLinks.GetFirst ())
			nextSignal->RemoveSecondaryCrossChannelSignalLinkSM ();

		SetMessageValue (notALink, true);
		SetMessageValue (primaryLink, false);
	}
}



void DataSignal :: RemovePrimaryCrossChannelSignalLinkSM (DataSignal* remove) {
	
	// for primary, from secondary
	// now not a link

	//
	//  This is sample stage 3
	//

	smNotInterchannelLink notALink;
	smPrimaryInterchannelLink primaryLink;


	if (mCrossChannelSignalLinks.Entries () > 0) {

		mCrossChannelSignalLinks.RemoveReference (remove);
		remove->SetMessageValue (notALink, true);
		
		if (mCrossChannelSignalLinks.Entries () == 0) {

			SetMessageValue (notALink, true);
			SetMessageValue (primaryLink, false);
		}
	}
}


void DataSignal :: RemoveSecondaryCrossChannelSignalLinkSM () {

	// for secondary, from primary

	//
	//  This is sample stage 3
	//

	smNotInterchannelLink notALink;

	if (mPrimaryCrossChannelLink != NULL) {

		mPrimaryCrossChannelLink = NULL;
		SetMessageValue (notALink, true);
	}
}


void DataSignal :: RemoveAllCrossChannelSignalLinksSM () {

	//
	//  This is sample stage 3
	//

	DataSignal* nextSignal;
	smNotInterchannelLink notALink;
	smPrimaryInterchannelLink primaryLink;

	while (nextSignal = (DataSignal*)mCrossChannelSignalLinks.GetFirst ())
		nextSignal->SetMessageValue (notALink, true);

	if (mPrimaryCrossChannelLink != NULL) {

		mPrimaryCrossChannelLink->SetMessageValue (primaryLink, false);
		mPrimaryCrossChannelLink->SetMessageValue (notALink, true);
	}

	mPrimaryCrossChannelLink = NULL;
}


bool DataSignal :: IsPullupFromChannelsOtherThan (int primaryChannel, int numberOfChannels) const {

	if (mPrimaryPullupInChannel == NULL)
		return false;

	int i;

	for (i=1; i<=numberOfChannels; i++) {

		if (i == primaryChannel)
			continue;

		if (HasPrimarySignalFromChannel (i) != NULL)
			return true;
	}

	return false;
}


bool DataSignal :: SetPullupMessageDataSM (int numberOfChannels) {

	//if (!HasAnyPrimarySignals (numberOfChannels))
	//	return false;

	//smPullUp pullup;
	//smCalculatedPurePullup purePullup;
	//smPartialPullupBelowMinRFU partialPullupBelowMin;
	//smSigmoidalPullup sigmoid;

	//bool isPullup = GetMessageValue (pullup);
	//bool isPurePullup = GetMessageValue (purePullup);
	//bool isSigmoidal = GetMessageValue (sigmoid);
	//bool isPartialBelowMin = GetMessageValue (partialPullupBelowMin);
	//bool noPositiveCorrection = true;
	//bool noPositiveAndSigmoid;
	//int i;
	//DataSignal* primarySignal;
	//double primaryHeight;
	//double ratio;
	//int myChannel = GetChannel ();
	//bool addedRatio = false;
	//double totalNegative = 0.0;
	//double totalPositive = 0.0;
	//double totalPure = 0.0;
	//double totalPartial = 0.0;
	//double totalPurePrimaryHeights = 0.0;
	//double correctedHeight;
	//double peakCorrectedForNegative;
	//double pullupHeight;
	//bool atLeastOnePureIsNegative = false;

	//if (isPurePullup) {

	//	double secondaryHeight = Peak ();
	//	double halfSecondaryHeight = 0.5 * secondaryHeight;

	//	for (i=1; i<=numberOfChannels; i++) {

	//		primarySignal = HasPrimarySignalFromChannel (i);

	//		if (primarySignal != NULL) {

	//			correctedHeight = GetPullupFromChannel (i);

	//			if (GetIsPurePullupFromChannel (i)) {

	//				if (correctedHeight <= 0.0)
	//					atLeastOnePureIsNegative = true;

	//				totalPure += correctedHeight;
	//				totalPurePrimaryHeights += primarySignal->Peak ();
	//			}

	//			else
	//				totalPartial += correctedHeight;
	//		}
	//	}

	//	if (totalPartial > halfSecondaryHeight)
	//		totalPartial = halfSecondaryHeight;

	//	correctedHeight = secondaryHeight - totalPartial;

	//	if (atLeastOnePureIsNegative) {

	//		for (i=1; i<=numberOfChannels; i++) {

	//			if (GetIsPurePullupFromChannel (i)) {

	//				primarySignal = HasPrimarySignalFromChannel (i);
	//				pullupHeight = correctedHeight * primarySignal->Peak () / totalPurePrimaryHeights;
	//				SetPullupFromChannel (i, pullupHeight, numberOfChannels);

	//				if (primarySignal != NULL) {

	//					SetPullupRatio (i, 100.0 * pullupHeight / primarySignal->Peak (), numberOfChannels);
	//					addedRatio = true;
	//				}

	//				else {
	//					// this can't happen!?
	//					addedRatio = false;
	//				}
	//			}
	//		}
	//	}

	//	else if (totalPure > 0.0) {

	//		double alpha = correctedHeight / totalPure;

	//		for (i=1; i<=numberOfChannels; i++) {

	//			if (GetIsPurePullupFromChannel (i)) {

	//				primarySignal = HasPrimarySignalFromChannel (i);
	//				pullupHeight = alpha * GetPullupFromChannel (i);
	//				SetPullupFromChannel (i, pullupHeight, numberOfChannels);

	//				if (primarySignal != NULL) {

	//					SetPullupRatio (i, 100.0 * pullupHeight / primarySignal->Peak (), numberOfChannels);
	//					addedRatio = true;
	//				}

	//				else {
	//					// this can't happen!?
	//					addedRatio = false;
	//				}
	//			}
	//		}
	//	}

	//	//else {  // totalPure is <= 0.0 so, now what?  Revert to previous algorithm

	//	//}
	//}

	//else if (isSigmoidal) {

	//	for (i=1; i<=numberOfChannels; i++) {

	//		primarySignal = HasPrimarySignalFromChannel (i);

	//		if (primarySignal != NULL) {

	//			correctedHeight = GetPullupFromChannel (i);

	//			if (correctedHeight > 0.0) {

	//				noPositiveCorrection = false;
	//				break;
	//			}
	//		}
	//	}

	//	noPositiveAndSigmoid = noPositiveCorrection && isSigmoidal;

	//	for (i=1; i<=numberOfChannels; i++) {

	//		primarySignal = HasPrimarySignalFromChannel (i);

	//		if (primarySignal != NULL) {

	//			correctedHeight = GetPullupFromChannel (i);

	//			if (correctedHeight > 0.0) {

	//				totalPositive += correctedHeight;
	//				continue;
	//			}

	//			else if (correctedHeight < 0.0) {

	//				totalNegative += correctedHeight;
	//			}

	//			//else
	//			//	cout << "Corrected height = 0 for pure pullup peak on channel " << GetChannel () << " at time " << GetMean () << endl;
	//		}
	//	}

	//	if (noPositiveAndSigmoid && (totalNegative != 0.0)) {

	//		for (i=1; i<=numberOfChannels; i++) {

	//			primarySignal = HasPrimarySignalFromChannel (i);

	//			if (primarySignal != NULL) {

	//				correctedHeight = GetPullupFromChannel (i);

	//				if (correctedHeight < 0.0) {

	//					primaryHeight = primarySignal->Peak ();
	//					ratio = -100.0 * (correctedHeight / totalNegative) * (1.0 / primaryHeight);
	//					SetPullupRatio (i, ratio, numberOfChannels);
	//					SetPullupFromChannel (i, -(correctedHeight / totalNegative), DataSignal::NumberOfChannels);
	//					addedRatio = true;
	//				}
	//			}
	//		}

	//		return addedRatio;
	//	}

	//	else {

	//		for (i=1; i<=numberOfChannels; i++) {

	//			primarySignal = HasPrimarySignalFromChannel (i);

	//			if (primarySignal != NULL) {

	//				correctedHeight = GetPullupFromChannel (i);

	//				if (correctedHeight < 0.0) {

	//					primaryHeight = primarySignal->Peak ();
	//					ratio = 100.0 * (correctedHeight / primaryHeight);
	//					SetPullupRatio (i, ratio, numberOfChannels);
	//					addedRatio = true;
	//				}
	//			}
	//		}
	//	}

	//	peakCorrectedForNegative = Peak () - totalNegative;
	//	
	//	// Now do positive

	//	if (totalPositive > 0.0) {

	//		for (i=1; i<=numberOfChannels; i++) {

	//			primarySignal = HasPrimarySignalFromChannel (i);

	//			if (primarySignal != NULL) {

	//				correctedHeight = GetPullupFromChannel (i);

	//				if (correctedHeight > 0.0) {

	//					primaryHeight = primarySignal->Peak ();
	//					ratio = 100.0 * (correctedHeight / totalPositive) * (peakCorrectedForNegative / primaryHeight);
	//					SetPullupRatio (i, ratio, numberOfChannels);
	//					//mPullupCorrectionArray [i] = (correctedHeight / totalPositive) * peakCorrectedForNegative;
	//					SetPullupFromChannel (i, (correctedHeight / totalPositive) * peakCorrectedForNegative, DataSignal::NumberOfChannels);
	//					addedRatio = true;
	//				}
	//			}
	//		}
	//	}
	//}

	//else if (isPullup || isPartialBelowMin) {

	//	for (i=1; i<=numberOfChannels; i++) {

	//		if (i == myChannel)
	//			continue;

	//		primarySignal = HasPrimarySignalFromChannel (i);

	//		if (primarySignal == NULL)
	//			continue;

	//		if (PullupChannelIsUncertain (i))
	//			continue;

	//		if (GetPullupFromChannel (i) != 0.0) {

	//			primaryHeight = primarySignal->Peak ();  // Use corrected value?
	////			ratio = 0.01 * floor (10000.0 * (GetPullupFromChannel (i) / primaryHeight) + 0.5);  // this is a percent
	//			ratio = 100.0 * (GetPullupFromChannel (i) / primaryHeight);
	//			SetPullupRatio (i, ratio, numberOfChannels);
	//			addedRatio = true;
	//		}

	//		//else
	//		//	cout << "Corrected height = 0 for partial pullup peak on channel " << GetChannel () << " at time " << GetMean () << endl;
	//	}
	//}

	//return addedRatio;

	return true;
}


bool DataSignal :: SetPrimaryPullupMessageDataSM (int numberOfChannels) {

	smPrimaryInterchannelLink primary;
	bool isPrimary = GetMessageValue (primary);

	if (!isPrimary)
		return false;

	InterchannelLinkage* nextLink = GetInterchannelLink ();

	if (nextLink == NULL)
		return false;

	list<int> pChannels;
	DataSignal* nextSignal;
	int pullupChannel;

	nextLink->ResetSecondaryIterator ();

	while (nextSignal = nextLink->GetNextSecondarySignal ())
		pChannels.push_back (nextSignal->GetChannel ());

	pChannels.sort ();
//	pChannels.unique ();

	while (!pChannels.empty ()) {

		pullupChannel = pChannels.front ();
		AppendDataForSmartMessage (primary, pullupChannel);
		pChannels.pop_front ();
	}

	return true;
}


bool DataSignal :: HasPullupFromSameChannelAsSM (DataSignal* ds, int numberOfChannels) {

	smPullUp pullup;
	
	if (!GetMessageValue (pullup))
		return false;

	if (!ds->GetMessageValue (pullup))
		return false;

	int i;
	DataSignal* myPrimary;
	DataSignal* dsPrimary;

	for (i=1; i<=numberOfChannels; i++) {

		myPrimary = HasPrimarySignalFromChannel (i);
		dsPrimary = ds->HasPrimarySignalFromChannel (i);

		if (myPrimary != dsPrimary)
			return false;
	}

	return true;
}


double DataSignal::GetPullupCorrectionFromPrimariesWithNoPullupSM (int numberOfChannels) {

	DataSignal* nextSignal;
	int i;
	double totalCorrection = 0.0;
	smPullUp pullup;
	smCalculatedPurePullup purePullup;

	for (i=1; i<=numberOfChannels; i++) {

		nextSignal = HasPrimarySignalFromChannel (i);

		if (nextSignal == NULL)
			continue;

		if (nextSignal->GetMessageValue (pullup))
			continue;

		if (nextSignal->GetMessageValue (purePullup))
			continue;

		totalCorrection += GetPullupFromChannel (i);
	}

	return totalCorrection;
}


RGString DataSignal :: GetDataForNoticeSM (SmartNotice& sn) {

	RGString str;
	SmartMessageData target;
	SmartMessageData* smd;

	int k = sn.GetScope ();

	if (k != 1)
		return str;

	int i = sn.GetMessageIndex ();

	target.SetIndex (i);
	smd = (SmartMessageData*) mMessageDataTable->Find (&target);

	if (smd != NULL) {

		str = smd->GetText ();
	}

	return str;
}


void DataSignal :: CapturePullupCorrections (DataSignal* ds) {

	int i;

	for (i=1; i<=NumberOfChannels; i++) {

		SetPullupRatio (i, ds->GetPullupRatio (i), NumberOfChannels);
		SetPullupFromChannel (i, ds->GetPullupFromChannel (i), NumberOfChannels);
		SetPrimarySignalFromChannel (i, ds->HasPrimarySignalFromChannel (i), NumberOfChannels);
	}
}


void DataSignal :: CreateInitializationData (int scope) {

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


void DataSignal :: InitializeMessageMatrix (bool* matrix, int size) {

	int i;

	for (i=0; i<size; i++)
		matrix [i] = InitialMatrix [i];
}


bool DataSignal :: IsNegativeOrSigmoid (DataSignal* ds) {

	smSigmoidalPullup sigmoid;

	if (ds == NULL)
		return false;

	if (ds->IsNegativePeak ())
		return true;

	if (ds->GetMessageValue (sigmoid))
		return true;

	return false;
}


bool DataSignal :: PeakCannotBePurePullup (DataSignal* pullup, DataSignal* primary) {

	smSignalIsCtrlPeak controlPeak;

	if (pullup == NULL)
		return false;

	if (pullup->GetMessageValue (controlPeak))
		return true;

	if (pullup->GetWidth () >= primary->GetWidth ())
		return true;

	if (DataSignal::IsNegativeOrSigmoid (pullup))
		return false;

	return false;
}


void Gaussian :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	DataSignal::OutputDebugID (comm, numHigherObjects);
	RGString idData;
	idData << "\t\t\t\tSignal with Mean:  " << Mean;
	SmartMessage::OutputDebugString (idData);
}


double Gaussian :: GetPullupToleranceInBP (double noise) const {

	//pullUpToleranceFactor
	double P = Peak ();

	if (P <= 0.0)
		return (mPullupTolerance + (2.0 * sin (0.5 * acos (Fit)) / 4.47));

	double localFit = Fit;

	if (localFit < 0.1)
		localFit = 0.1;

	double localNoise = noise;
	
	double LN = 0.95 * P;

	if (localNoise > LN)
		localNoise = LN;

	double temp1 = 1.0 / localFit;
	double temp = 2.0 * (temp1 * temp1 - 1.0) * StandardDeviation * log (P / (P - localNoise));
	return (mPullupTolerance + pullUpToleranceFactor * mApproxBioIDPrime * sqrt (temp));
}


void DoubleGaussian :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	DataSignal::OutputDebugID (comm, numHigherObjects);
	RGString idData;
	idData << "\t\t\t\tSignal with Mean:  " << PrimaryCurve->GetMean ();
	SmartMessage::OutputDebugString (idData);
}


void SuperGaussian :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	DataSignal::OutputDebugID (comm, numHigherObjects);
	RGString idData;
	idData << "\t\t\t\tSignal with Mean:  " << Mean;
	SmartMessage::OutputDebugString (idData);
}


double SuperGaussian :: GetPullupToleranceInBP (double noise) const {

	//pullUpToleranceFactor
	double P = Peak ();

	if (P <= 0.0)
		return (mPullupTolerance + (2.0 * sin (0.5 * acos (Fit)) / 4.47));

	double localFit = Fit;

	if (localFit < 0.1)
		localFit = 0.1;

	double localNoise = noise;
	
	double LN = 0.95 * P;

	if (localNoise > LN)
		localNoise = LN;

	double temp1 = 1.0 / localFit;
	double temp = 2.0 * (temp1 * temp1 - 1.0) * StandardDeviation * log (P / (P - localNoise));
	return (mPullupTolerance + pullUpToleranceFactor * mApproxBioIDPrime * sqrt (temp));
}


double CraterSignal :: GetPullupToleranceInBP (double noise) const {

	if ((mPrevious == NULL) || (mNext == NULL))
		return GetPullupToleranceInBP ();

	//double bpLeft = mPrevious->GetApproximateBioID () - mPrevious->GetPullupToleranceInBP (noise);  // altered 09/15/2015
	//double bpRight = mNext->GetApproximateBioID () + mNext->GetPullupToleranceInBP (noise);
	double bpLeft = mPrevious->GetApproximateBioID ();
	double bpRight = mNext->GetApproximateBioID ();
	double bp = GetApproximateBioID ();
	bpLeft = bp - bpLeft;
	bpRight = bpRight - bp;
	double rtnValue;

	if (bpLeft > bpRight)
		rtnValue = bpLeft;

	else
		rtnValue = bpRight;

	return rtnValue;
}


double CraterSignal :: GetPrimaryPullupDisplacementThreshold () {

	if ((mNext == NULL) || (mPrevious == NULL))
		return 2.0;

	return 0.5 * GetWidth ();   //0.5 * fabs (mNext->GetMean () - mPrevious->GetMean ());
}


double CraterSignal :: GetPrimaryPullupDisplacementThreshold (double nSigmas) {

	if ((mNext == NULL) || (mPrevious == NULL))
		return 2.0;

	return 0.5 * nSigmas * GetWidth ();
}


bool CraterSignal :: LiesBelowHeightAt (double x, double height) {

	if ((mNext == NULL) || (mPrevious == NULL))
		return false;

	double mu1 = mPrevious->GetMean ();
	double mu2 = mNext->GetMean ();

	if ((mu1 < x) && (x < mu2))
		return false;

	if (x <= mu1)
		return mPrevious->LiesBelowHeightAt (x, height);

	return mNext->LiesBelowHeightAt (x, height);

}


bool CraterSignal :: TestForIntersectionWithPrimary (DataSignal* primary) {

	int i = 0;

	if ((mPrevious == NULL) || (mNext == NULL))
		return false;

	double sigma1 = 0.5 * mPrevious->GetWidth ();
	double sigma2 = 0.5 * mNext->GetWidth ();
	double mu1 = mPrevious->GetMean ();
	double mu2 = mNext->GetMean ();
	double peakTest1 = 0.25 * mPrevious->Peak ();
	double peakTest2 = 0.25 * mNext->Peak ();
	double testPlus;
	double testMinus;
	double heightPlus;
	double heightMinus;
	double delta1;
	double delta2;

	if (primary->LiesBelowHeightAt (mu1, mPrevious->Value (mu1)))
		return true;

	if (primary->LiesBelowHeightAt (mu2, mNext->Value (mu2)))
		return true;

	while (true) {

		i++;
		delta1 = (double)i * sigma1;
		delta2 = (double)i * sigma2;
		testPlus = mu2 + delta2;
		testMinus = mu1 - delta1;
		heightPlus = mNext->Value (testPlus);
		heightMinus = mPrevious->Value (testMinus);

		if ((heightPlus < peakTest2) || (heightMinus < peakTest1))
			break;

		if (primary->LiesBelowHeightAt (testPlus, heightPlus))
			return true;

		if (primary->LiesBelowHeightAt (testMinus, heightMinus))
			return true;

		if (i > 5)
			break;
	}

	return false;
}


void CraterSignal :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	DataSignal::OutputDebugID (comm, numHigherObjects);
	RGString idData;
	idData << "\t\t\t\tSignal with Mean:  " << mMean;
	SmartMessage::OutputDebugString (idData);
}


bool CraterSignal :: TestForMultipleSignalsWithinLocus (DataSignal*& prev, DataSignal*& next, int location, bool isAmel, double adenylationLimit) {

	//
	//  This is sample stage 3:  comment added 03/26/2012
	//

	// Assumes that both mPrevious and mNext have been "called"
	
	prev = mPrevious;
	next = mNext;
	
	if ((mNext == NULL) || (mPrevious == NULL))
		return false;

	double appBpPrev = mPrevious->GetApproximateBioID ();
	double appBpNext = mNext->GetApproximateBioID ();
//	int IntAppBpPrev = (int) floor (appBpPrev + 0.5);
//	int IntAppBpNext = (int) floor (appBpNext + 0.5);

	double bpPrev = mPrevious->GetBioID (-location);	// location is relative to locus; must reverse to make relative to mPrevious (03/26/2012)
	double bpNext = mNext->GetBioID (-location);	// location is relative to locus; must reverse to make relative to mNext (03/26/2012)
	double absDiff = fabs (bpNext - bpPrev);
	//int IntBPPrev = (int) floor (bpPrev + 0.5);
	//int IntBPNext = (int) floor (bpNext + 0.5);

	RGString prevAlleleName = mPrevious->GetAlleleName ();
	RGString nextAlleleName = mNext->GetAlleleName ();

	if (prevAlleleName == nextAlleleName)
		return false;
	
	double maxResidual = Locus::GetMaxResidualForAlleleCalls ();
	int order;
//	double adenylationLimit = Locus::GetSampleAdenylationThreshold ();
	smAdenylation adenylationFound;
	double prevPeak;
	double nextPeak;

	double prevResidual;
	double nextResidual;
	double thisResidual;

	if (isAmel) {

		prevResidual = fabs (mPrevious->GetBioIDResidual (-location));	// location is relative to locus; must reverse to make relative to mPrevious (03/26/2012)
		nextResidual = fabs (mNext->GetBioIDResidual (-location));	// location is relative to locus; must reverse to make relative to mNext (03/26/2012)
		thisResidual = fabs (GetBioIDResidual (-location));	// location is relative to locus; must reverse to make relative to this signal (03/26/2012)

		order = LessOrder (prevResidual, nextResidual, thisResidual);

		if (order == 4)
			return false;

		return true;	
	}

	if (absDiff > 0.7) {

		// Might be separate signals, but, first, check residuals.  If side signals have residuals that are too high and
		// "center" signal does not, it's probably a crater after all.

		if ((adenylationLimit > 0.0) && !Locus::GetDisableAdenylationFilter ()) {

			prevPeak = mPrevious->Peak ();
			nextPeak = mNext->Peak ();

			if (prevPeak <= adenylationLimit * nextPeak) {

				mPrevious->SetMessageValue (adenylationFound, true);
				return true;
			}

			//  Commenting out next lines because there is no +1 adenylation...12/15/2013
			//if (nextPeak <= adenylationLimit * prevPeak) {

			//	mNext->SetMessageValue (adenylationFound, true);
			//	return true;
			//}
		}

		prevResidual = fabs (mPrevious->GetBioIDResidual (location));
		nextResidual = fabs (mNext->GetBioIDResidual (location));
		thisResidual = fabs (GetBioIDResidual (location));

		if ((prevResidual >= maxResidual) || (nextResidual >= maxResidual)) {

			if (thisResidual < maxResidual)
				return false;
		}

		order = LessOrder (prevResidual, nextResidual, thisResidual);

		if ((order >= 4) && (order < 7))
			return false;

		return true;		
	}

	return false;
}


bool CraterSignal :: TestSignalGroupsWithinILS (double ilsLeft, double ilsRight, double minBioID) {

	//
	//  This is ladder and sample stage 1
	//

	smSignalNotACrater notACrater;
	smSignalNotACraterSidePeak notASidePeak;
	double mean;

	if ((mMean < ilsLeft) || (mMean > ilsRight) || (GetApproximateBioID () < minBioID)) {

		SetMessageValue (notACrater, true);

		if (mPrevious != NULL)
			mPrevious->SetMessageValue (notASidePeak, true);

		if (mNext != NULL)
			mNext->SetMessageValue (notASidePeak, true);
	}

	else if (mPrevious != NULL) {

		mean = mPrevious->GetMean ();

		if ((mean < ilsLeft) || (mean > ilsRight) || (mPrevious->GetApproximateBioID () < minBioID)) {

			SetMessageValue (notACrater, true);
			mPrevious->SetMessageValue (notASidePeak, true);

			if (mNext != NULL)
				mNext->SetMessageValue (notASidePeak, true);
		}
	}

	else if (mNext != NULL) {

		mean = mNext->GetMean ();

		if ((mean < ilsLeft) || (mean > ilsRight) || (mNext->GetApproximateBioID () < minBioID)) {

			SetMessageValue (notACrater, true);
			mNext->SetMessageValue (notASidePeak, true);

			if (mPrevious != NULL)
				mPrevious->SetMessageValue (notASidePeak, true);
		}
	}

	return true;
}


void SimpleSigmoidSignal :: RecalculatePullupTolerance () {

	if ((mPrevious == NULL) || (mNext == NULL))
		return;

	double bpLeft = mPrevious->GetApproximateBioID () - mPrevious->GetPullupToleranceInBP ();
	double bpRight = mNext->GetApproximateBioID () + mNext->GetPullupToleranceInBP ();
	double bp = GetApproximateBioID ();
	bpLeft = bp - bpLeft;
	bpRight = bpRight - bp;

	if (bpLeft > bpRight)
		mPullupTolerance = bpLeft;

	else
		mPullupTolerance = bpRight;
}


double SimpleSigmoidSignal :: GetPullupToleranceInBP (double noise) const {

	if ((mPrevious == NULL) || (mNext == NULL))
		return CraterSignal::GetPullupToleranceInBP ();

	//double bpLeft = mPrevious->GetApproximateBioID () - mPrevious->GetPullupToleranceInBP (noise);  // altered 09/15/2015
	//double bpRight = mNext->GetApproximateBioID () + mNext->GetPullupToleranceInBP (noise);
	double bpLeft = mPrevious->GetApproximateBioID ();
	double bpRight = mNext->GetApproximateBioID ();
	double bp = GetApproximateBioID ();
	bpLeft = bp - bpLeft;
	bpRight = bpRight - bp;
	double rtnValue;

//	cout << "Sigmoid at " << bp << " bps with left tolerance = " << bpLeft << " and right tolerance = " << bpRight << "\n";

	if (bpLeft > bpRight)
		rtnValue = bpLeft;

	else
		rtnValue = bpRight;

	return rtnValue;
}


void SimpleSigmoidSignal :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	DataSignal::OutputDebugID (comm, numHigherObjects);
	RGString idData;
	idData << "\t\t\t\tSignal with Mean:  " << mMean;
	SmartMessage::OutputDebugString (idData);
}



void SimpleSigmoidSignal :: CaptureSmartMessages () {

	if ((mPrevious != NULL) && !mPrevious->IsNegativePeak ())
		DataSignal::CaptureSmartMessages (mPrevious);

	if ((mNext != NULL) && !mNext->IsNegativePeak ())
		DataSignal::CaptureSmartMessages (mNext);
}


bool SimpleSigmoidSignal :: TestSignalGroupsWithinILS (double ilsLeft, double ilsRight, double minBioID) {

	//
	//  This is ladder and sample stage 1
	//

	smSignalNotACrater notACrater;
	smSignalNotACraterSidePeak notASidePeak;
	double mean;

	if ((mMean < ilsLeft) || (mMean > ilsRight) || (GetApproximateBioID () < minBioID)) {

		SetMessageValue (notACrater, true);

		if (mPrevious != NULL)
			mPrevious->SetMessageValue (notASidePeak, true);

		if (mNext != NULL)
			mNext->SetMessageValue (notASidePeak, true);
	}

	else if (mPrevious != NULL) {

		mean = mPrevious->GetMean ();

		if ((mean < ilsLeft) || (mean > ilsRight) || (mPrevious->GetApproximateBioID () < minBioID)) {

			SetMessageValue (notACrater, true);
			mPrevious->SetMessageValue (notASidePeak, true);

			if (mNext != NULL)
				mNext->SetMessageValue (notASidePeak, true);
		}
	}

	else if (mNext != NULL) {

		mean = mNext->GetMean ();

		if ((mean < ilsLeft) || (mean > ilsRight) || (mNext->GetApproximateBioID () < minBioID)) {

			SetMessageValue (notACrater, true);
			mNext->SetMessageValue (notASidePeak, true);

			if (mPrevious != NULL)
				mPrevious->SetMessageValue (notASidePeak, true);
		}
	}

	return true;
}


NegativeSignal :: NegativeSignal () : ParametricCurve () {

	smNegativePeak negPeak;
	SetMessageValue (negPeak, true);
	mOriginal = new Gaussian ();
}



NegativeSignal :: NegativeSignal (const NegativeSignal& neg) : ParametricCurve (neg) {

	smNegativePeak negPeak;
	SetMessageValue (negPeak, true);
	mOriginal = neg.mOriginal->MakeCopy (neg.GetMean ());
}



NegativeSignal :: NegativeSignal (double mean, double peak, const ParametricCurve& sig) : ParametricCurve (sig) {

	smNegativePeak negPeak;
	SetMessageValue (negPeak, true);
	mOriginal = sig.MakeCopy (mean);
	mOriginal->SetPeak (-sig.Peak ());
}



NegativeSignal :: ~NegativeSignal () {

	delete mOriginal;
}


double NoisyPeak :: GetPullupToleranceInBP (double noise) const {

	if ((mPrevious == NULL) || (mNext == NULL))
		return GetPullupToleranceInBP ();

	double bpLeft = mPrevious->GetApproximateBioID ();
	double bpRight = mNext->GetApproximateBioID ();
	double bp = GetApproximateBioID ();
	bpLeft = bp - bpLeft;
	bpRight = bpRight - bp;
	double rtnValue;

	if (bpLeft > bpRight)
		rtnValue = bpLeft;

	else
		rtnValue = bpRight;

	return rtnValue;
}


void NoisyPeak :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	DataSignal::OutputDebugID (comm, numHigherObjects);
	RGString idData;
	idData << "\t\t\t\tSignal with Mean:  " << mMean;
	SmartMessage::OutputDebugString (idData);
}



void NoisyPeak :: CaptureSmartMessages () {

	if (mPrevious != NULL)
		DataSignal::CaptureSmartMessages (mPrevious);

	if (mNext != NULL)
		DataSignal::CaptureSmartMessages (mNext);
}



void SpikeSignal :: OutputDebugID (SmartMessagingComm& comm, int numHigherObjects) {

	DataSignal::OutputDebugID (comm, numHigherObjects);
	RGString idData;
	idData << "\t\t\t\tSignal with Mean:  " << mMean;
	SmartMessage::OutputDebugString (idData);
}







