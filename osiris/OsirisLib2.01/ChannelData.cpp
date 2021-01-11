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
*  FileName: ChannelData.cpp
*  Author:   Robert Goor
*
*/
//
//     Abstract base class for sample data based on file input
//

#include "RGTextOutput.h"
#include "OsirisMsg.h"
#include "ChannelData.h"
#include "TestCharacteristic.h"
#include "TracePrequalification.h"
#include "SampleData.h"
#include "CoreBioComponent.h"
#include "rgstring.h"
#include "OutputLevelManager.h"
#include "Notices.h"
#include "ListFunctions.h"
#include "IndividualGenotype.h"
#include "SmartMessage.h"
#include "SmartNotice.h"
#include "STRSmartNotices.h"
#include "ModPairs.h"


ABSTRACT_DEFINITION (ChannelData)

double ChannelData::MinDistanceBetweenPeaks = 1.5;
bool* ChannelData::InitialMatrix = NULL;
double ChannelData::AveSecondsPerBP = 6.0;
bool ChannelData::UseFilterForNormalization = false;
bool ChannelData::DisableStutterFilter = false;
bool ChannelData::DisableAdenylationFilter = false;
bool ChannelData::TestForDualSignal = true;
bool ChannelData::UseILSHistory = false;
bool ChannelData::UseLadderILSEndPointAlgorithm = false;
double ChannelData::LatitudeFactorForILSHistory = 0.0;
double ChannelData::LatitudeFactorForLadderILS = 0.01;
double ChannelData::BeginAnalysis = -1.0;
bool ChannelData::UseEnhancedShoulderAlgorithm = false;

bool ChannelData::UseNoiseLevelDefaultForFit = true;
bool ChannelData::UseDetectionLevelForFit = false;
bool ChannelData::UseNoiseLevelPercentForFit = false;

double ChannelData::NoisePercentNormalizationPass = 50.0;
double ChannelData::NoisePercentFinalPass = 75.0;

bool ChannelData::IsNormalizationPass = true;
double ChannelData::NormalizationSplitTime = -1.0;


bool operator== (const RaisedBaseLineData& first, const RaisedBaseLineData& second) {

	if ((first.mHeight == second.mHeight) && (first.mLeft == second.mLeft) && (first.mRight == second.mRight))
		return true;

	return false;
}


ProspectiveIntervalForNormalization :: ProspectiveIntervalForNormalization (int start, DataSignal* sampledData, double* derivFilter, double filterHeight, int averagingWidth, double noiseThreshold) : mData (sampledData) {

	int numberOfSamples = mData->GetNumberOfSamples ();
	int i;
	//int j;
	bool foundFirstPoint = false;
	bool foundLastPoint = false;
	bool derivOK;
	//bool displacementOK;
	//double maxDisplacement;
	//int upperLimit;
	//int lowerLimit;
	//double currentValue;
	//int left = (int)mData->LeftEndPoint ();

	for (i=start; i<numberOfSamples; i++) {

		derivOK = (derivFilter [i] <= filterHeight);
		//maxDisplacement = 0.0;
		//upperLimit = i + averagingWidth;
		//lowerLimit = i - averagingWidth;
		//displacementOK = true;
		//currentValue = mData->Value (i);

		//for (j=lowerLimit; j<=upperLimit; j++) {

		//	if (j < left)
		//		continue;

		//	if (j >= numberOfSamples)
		//		break;

		//	if (fabs (currentValue - mData->Value (j)) > noiseThreshold) {

		//		displacementOK = false;
		//		break;
		//	}
		//}

		if (derivOK) {

			if (!foundFirstPoint) {

				foundFirstPoint = true;
				mStart = i;
			}
		}

		else if (foundFirstPoint) {

			mEnd = i - 1;
			foundLastPoint = true;
			break;
		}
	}

	if (!foundFirstPoint)
		mStart = mEnd = numberOfSamples - 1;

	else if (!foundLastPoint)
		mEnd = numberOfSamples - 1;

	mStrictStart = mStart;
	mStrictEnd = mEnd;
}



ProspectiveIntervalForNormalization :: ~ProspectiveIntervalForNormalization () {

	mNormalizationIntervals.clear ();  // we assume that this list has already been emptied and deletion of pointers is an outside responsibility
}



void ProspectiveIntervalForNormalization :: DivideIntoNormalizationIntervals (int minLength, double noiseRange, int subLength, int neighborTestLimit) {

	if (!IsLongEnough (minLength))
		return;

	NormalizationInterval* nextInterval;
	int currentStart = mStart;
	int currentEnd;

	while (true) {

		nextInterval = new NormalizationInterval (currentStart, mEnd, mData, minLength, subLength, noiseRange, neighborTestLimit);
		currentEnd = nextInterval->GetEnd ();

		if (!nextInterval->IsLongEnough (minLength))
			delete nextInterval;

		else
			mNormalizationIntervals.push_back (nextInterval);

		currentStart = currentEnd + 1;

		if (mEnd - currentStart + 1 < minLength)
			break;
	}
}


void ProspectiveIntervalForNormalization :: SmoothInteriorPoints (int nSmoothPoints, int nSlackPoints, double* smoothedData, double* tempData) {

	int disp = nSmoothPoints + nSlackPoints;
	int startPt = mStart + disp;
	int endPt = mEnd - disp;
	int i;
	int j;
	double divisor = (double) (2 * nSmoothPoints + 1);
	bool FirstTime = true;
	double previous = 0.0;
	int k;

	for (k=0; k<2; k++) {

		FirstTime = true;

		for (i=startPt; i<=endPt; i++) {

			double sum = 0.0;
			int upperLimit = i + nSmoothPoints;
			int lowerLimit = i - nSmoothPoints;

			if (FirstTime) {

				FirstTime = false;

				for (j=i-nSmoothPoints; j<=upperLimit; j++)
					sum += smoothedData [j]; //mData->Value (j);

				previous = sum;
				tempData [i] = sum / divisor;
			}

			else {

				previous = previous - smoothedData [lowerLimit - 1] + smoothedData [upperLimit]; //previous - mData->Value (lowerLimit - 1) + mData->Value (upperLimit);
				tempData [i] = previous / divisor;
			}
		}

		for (i=startPt; i<=endPt; i++) {

			smoothedData [i] = tempData [i];
		}
	}

	mStrictStart = startPt;
	mStrictEnd = endPt;
}



NormalizationInterval* ProspectiveIntervalForNormalization :: GetNextNormalizationInterval (int minLength) {

	NormalizationInterval* nextInterval;

	if (mNormalizationIntervals.empty ())
		return NULL;

	nextInterval = mNormalizationIntervals.front ();
	mNormalizationIntervals.pop_front ();
	return nextInterval;
}


NormalizationInterval :: NormalizationInterval (int start, int endPt, DataSignal* sampledData, int minLength, int subLength, double noiseRange, int neighborTestLimit) : mData (sampledData), mSubLength (subLength) {

	int numberOfSamples = mData->GetNumberOfSamples ();
	int i;
	bool foundFirstPoint = false;
	bool foundLastPoint = false;

	for (i=start; i<=endPt; i++) {

		if (mData->TestIfNeighboringDataWithinRange (i, neighborTestLimit, noiseRange)) {

			if (!foundFirstPoint) {

				foundFirstPoint = true;
				mStart = i;
			}
		}

		else if (foundFirstPoint) {

			mEnd = i - 1;
			foundLastPoint = true;
			break;
		}
	}

	if (!foundFirstPoint)
		mStart = mEnd = endPt;

	else if (!foundLastPoint)
		mEnd = endPt;
}



NormalizationInterval :: ~NormalizationInterval () {

	mKnotTimes.clear ();
	mKnotValues.clear ();
}


bool NormalizationInterval :: DivideIntoSubIntervalsAndCalculateKnots () {

	int length = mEnd - mStart + 1;
	int nSubIntervals = length / mSubLength;

	if (nSubIntervals == 0)
		return false;

	int remainder = nSubIntervals * mSubLength;
	int remainderRight = remainder / 2;
	int remainderLeft = remainder - remainderRight;
	int subIntervalsStart = mStart + remainderLeft;
	int subIntervalsEnd = mEnd - remainderRight;
	int i;

	int localStart = subIntervalsStart;
	int localEnd = localStart + mSubLength - 1;
	double averageHeight;
	double averageTime;

	for (i=1; i<=nSubIntervals; i++) {

		averageHeight = ComputeSubIntervalAverage (localStart, localEnd);
		mKnotValues.push_back (averageHeight);
		averageTime = 0.5 * (double)(localStart + localEnd);
		mKnotTimes.push_back (averageTime);
		localStart += mSubLength;
		localEnd += mSubLength;
	}

	return true;
}



bool NormalizationInterval :: AddKnotsToLists (list<double>& knotTimes, list<double>& knotValues) {

	double nextTime;
	double nextValue;

	if (mKnotTimes.size () != mKnotValues.size ())
		return false;

	while (!mKnotTimes.empty ()) {

		nextTime = mKnotTimes.front ();
		mKnotTimes.pop_front ();
		knotTimes.push_back (nextTime);
	}

	while (!mKnotValues.empty ()) {

		nextValue = mKnotValues.front ();
		mKnotValues.pop_front ();
		knotValues.push_back (nextValue);
	}
	
	return true;
}


double NormalizationInterval :: ComputeSubIntervalAverage (int startPt, int endPt) {

	double average = 0.0;
	int i;

	for (i=startPt; i<=endPt; i++)
		average += mData->Value (i);

	average = average /(double) (endPt - startPt + 1);
	return average;

	//list<double> samplePts;
	//double* sampleArray = new double [mSubLength];
	//int i;
	//int keepPts = mSubLength / 2;

	//if (mSubLength%2 != 0)
	//	keepPts++;

	//for (i=startPt; i<=endPt; i++)
	//	samplePts.push_back (mData->Value (i));

	//samplePts.sort ();

	//for (i=0; i<mSubLength; i++) {

	//	sampleArray [i] = samplePts.front ();
	//	samplePts.pop_front ();
	//}

	//double minDiff = sampleArray [mSubLength - 1] - sampleArray [0];
	//double average;
	//int minStartIndex = 0;

	//if (minDiff == 0.0) {

	//	average = sampleArray [0];
	//	samplePts.clear ();
	//	delete[] sampleArray;
	//	return average;
	//}

	//int lastEnd = mSubLength - 1;
	//int subArrayIncrement = keepPts - 1;
	//double delta;
	//int endIndex;

	//for (i=0; i<lastEnd; i++) {

	//	endIndex = i + subArrayIncrement;

	//	if (endIndex > lastEnd)
	//		break;

	//	delta = sampleArray [endIndex] - sampleArray [i];

	//	if (delta <= minDiff) {

	//		minDiff = delta;
	//		minStartIndex = i;
	//	}
	//}

	//average = 0.0;
	//endIndex = minStartIndex + subArrayIncrement;

	//for (i=minStartIndex; i<=endIndex; i++)
	//	average += sampleArray [i];

	//average = average /(double)keepPts;
	//average = 0.5 * (sampleArray [minStartIndex] + sampleArray [endIndex]);
	//samplePts.clear ();
	//delete[] sampleArray;
	//return average;
}


ChannelData :: ChannelData () : SmartMessagingObject (), mChannel (-1), mData (NULL), mBackupData (NULL),
mTestPeak (NULL), Valid (FALSE), PreliminaryIterator (PreliminaryCurveList), CompleteIterator (CompleteCurveList), NegativeCurveIterator (mNegativeCurveList), NumberOfAcceptedCurves (0), SetSize (0), MaxCorrelationIndex (0), 
Means (NULL), Sigmas (NULL), Fits (NULL), Peaks (NULL), SecondaryContent (NULL), mLaneStandard (NULL), mDeleteLoci (false), mFsaChannel (-1), mBaseLine (NULL), mBaselineStart (-1), mTimeMap (NULL), mFilterChangeArray (NULL), mFractionOfChangedFilterPoints (1.0),
mMaxLocusArea (0.0), mMaxYLinkedLocusArea (0.0), mMinLocusArea (0.0), mMinYLinkedLocusArea (0.0), mMaxLocusAreaRatio (0.0), mMaxYLinkedLocusRatio (0.0), mMaxLaserInScalePeak (0.0), mModsData (NULL) {

	InitializeSmartMessages ();
	//mNegativeCurveList.ClearAndDelete ();
}


ChannelData :: ChannelData (int channel) : SmartMessagingObject (), mChannel (channel), mData (NULL), mBackupData (NULL), 
mTestPeak (NULL), Valid (FALSE), PreliminaryIterator (PreliminaryCurveList), CompleteIterator (CompleteCurveList), NegativeCurveIterator (mNegativeCurveList), NumberOfAcceptedCurves (0), SetSize (0), MaxCorrelationIndex (0), 
Means (NULL), Sigmas (NULL), Fits (NULL), Peaks (NULL), SecondaryContent (NULL), mLaneStandard (NULL), mDeleteLoci (false), mFsaChannel (channel), mBaseLine (NULL), mBaselineStart (-1), mTimeMap (NULL), mFilterChangeArray (NULL), mFractionOfChangedFilterPoints (1.0),
mMaxLocusArea (0.0), mMaxYLinkedLocusArea (0.0), mMinLocusArea (0.0), mMinYLinkedLocusArea (0.0), mMaxLocusAreaRatio (0.0), mMaxYLinkedLocusRatio (0.0), mMaxLaserInScalePeak (0.0), mModsData (NULL) {

	InitializeSmartMessages ();
	//mNegativeCurveList.ClearAndDelete ();
}


ChannelData :: ChannelData (int channel, LaneStandard* inputLS) : SmartMessagingObject (), mChannel (channel), mData (NULL), mBackupData (NULL), 
mTestPeak (NULL), Valid (FALSE), PreliminaryIterator (PreliminaryCurveList), CompleteIterator (CompleteCurveList), NegativeCurveIterator (mNegativeCurveList), NumberOfAcceptedCurves (0), SetSize (0), MaxCorrelationIndex (0), 
Means (NULL), Sigmas (NULL), Fits (NULL), Peaks (NULL), SecondaryContent (NULL), mLaneStandard (inputLS), mDeleteLoci (false), mFsaChannel (channel), mBaseLine (NULL), mBaselineStart (-1), mTimeMap (NULL), mFilterChangeArray (NULL), mFractionOfChangedFilterPoints (1.0),
mMaxLocusArea (0.0), mMaxYLinkedLocusArea (0.0), mMinLocusArea (0.0), mMinYLinkedLocusArea (0.0), mMaxLocusAreaRatio (0.0), mMaxYLinkedLocusRatio (0.0), mMaxLaserInScalePeak (0.0), mModsData (NULL) {

	InitializeSmartMessages ();
	//mNegativeCurveList.ClearAndDelete ();
}


ChannelData :: ChannelData (const ChannelData& cd) : SmartMessagingObject ((SmartMessagingObject&)cd), mChannel (cd.mChannel), mBackupData (NULL),
Valid (cd.Valid), mTestPeak (cd.mTestPeak), PreliminaryIterator (PreliminaryCurveList), CompleteIterator (CompleteCurveList), NegativeCurveIterator (mNegativeCurveList), NumberOfAcceptedCurves (cd.NumberOfAcceptedCurves),
SetSize (cd.SetSize), MaxCorrelationIndex (cd.MaxCorrelationIndex), Means (NULL), Sigmas (NULL), Fits (NULL), Peaks (NULL), SecondaryContent (NULL), 
mLaneStandard (NULL), mDeleteLoci (true), mFsaChannel (cd.mFsaChannel), mBaseLine (NULL), mBaselineStart (-1), mTimeMap (NULL), mFilterChangeArray (NULL), mFractionOfChangedFilterPoints (1.0),
mMaxLocusArea (cd.mMaxLocusArea), mMaxYLinkedLocusArea (cd.mMaxYLinkedLocusArea), mMinLocusArea (cd.mMinLocusArea), mMinYLinkedLocusArea (cd.mMinYLinkedLocusArea), mMaxLocusAreaRatio (cd.mMaxLocusAreaRatio), mMaxYLinkedLocusRatio (cd.mMaxYLinkedLocusRatio),
mMaxLaserInScalePeak (cd.mMaxLaserInScalePeak), mModsData (NULL) {

	mData = (DataSignal*)cd.mData->Copy ();
	mLocusList = cd.mLocusList;
	NewNoticeList = cd.NewNoticeList;
	InitializeSmartMessages (cd);
	//mNegativeCurveList.ClearAndDelete ();
}


ChannelData :: ChannelData (const ChannelData& cd, CoordinateTransform* trans) : SmartMessagingObject ((SmartMessagingObject&)cd), mChannel (cd.mChannel), mBackupData (NULL),
Valid (cd.Valid), mTestPeak (cd.mTestPeak), PreliminaryIterator (PreliminaryCurveList), CompleteIterator (CompleteCurveList), NegativeCurveIterator (mNegativeCurveList), NumberOfAcceptedCurves (cd.NumberOfAcceptedCurves),
SetSize (cd.SetSize), MaxCorrelationIndex (cd.MaxCorrelationIndex), 
Means (NULL), Sigmas (NULL), Fits (NULL), Peaks (NULL), SecondaryContent (NULL), mLaneStandard (NULL), mDeleteLoci (true), mFsaChannel (cd.mFsaChannel), mBaseLine (NULL), mBaselineStart (-1), mTimeMap (NULL), mFilterChangeArray (NULL), mFractionOfChangedFilterPoints (1.0),
mMaxLocusArea (cd.mMaxLocusArea), mMaxYLinkedLocusArea (cd.mMaxYLinkedLocusArea), mMinLocusArea (cd.mMinLocusArea), mMinYLinkedLocusArea (cd.mMinYLinkedLocusArea), mMaxLocusAreaRatio (cd.mMaxLocusAreaRatio), mMaxYLinkedLocusRatio (cd.mMaxYLinkedLocusRatio),
mMaxLaserInScalePeak (cd.mMaxLaserInScalePeak), mModsData (NULL) {

	mData = NULL;
	RGDList tempLocusList = cd.mLocusList;
	Locus* nextLocus;
	Locus* newLocus;

	while (nextLocus = (Locus*) tempLocusList.GetFirst ()) {

		newLocus = new Locus (*nextLocus, trans);
		mLocusList.Append (newLocus);
		delete nextLocus;
		MergeAndSaveListAWithListB (newLocus->GetFinalSignalList (), CompleteCurveList);
	}

	NewNoticeList = cd.NewNoticeList;
	InitializeSmartMessages (cd);
	//mNegativeCurveList.ClearAndDelete ();
}


ChannelData :: ~ChannelData () {

	delete mData;
	delete mBackupData;
	delete mBaseLine;
	delete[] mFilterChangeArray;
	delete[] mModsData;

	if (mDeleteLoci)
		mLocusList.ClearAndDelete ();

	else
		mLocusList.Clear ();

	CompleteCurveList.ClearAndDelete ();
	FinalCurveList.Clear ();
	MarginalCurveList.Clear ();
	ArtifactList.Clear ();
	SmartPeaks.Clear ();
	BleedThroughCandidateList.Clear ();
	mIgnorePeaks.Clear ();
	PreliminaryCurveList.Clear ();
	SupplementalArtifacts.Clear ();
	NewNoticeList.ClearAndDelete ();
	mRaisedBaseLines.clearAndDestroy ();
	mPoorFits.clearAndDestroy ();
	//cout << "Deleted all positive curves, channel " << mChannel << endl;
	mNegativeCurveList.ClearAndDelete ();
	//cout << "Deleted all negative curves, channel " << mChannel << endl;
	// do not delete mTimeMap...it is deleted in CoreBioComponent

	delete[] Means;
	delete[] Sigmas;
	delete[] Fits;
	delete[] Peaks;
	delete[] SecondaryContent;
}


void ChannelData :: SetTableLink (int linkNumber) {

	RGString temp;
	temp.Convert (linkNumber, 10);
	mTableLink = "&" + temp + "&";
}


Locus* ChannelData :: FindLocus (const RGString& locusName) {

	mTargetLocus.SetLocusName (locusName);
	return (Locus*) mLocusList.Find (&mTargetLocus);
}


double ChannelData :: TotalPeakHeight (int& numberOfPeaks) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	double totalHeight = 0.0;
	int N = 0;
	int numPeaks;
	double temp;

	while (nextLocus = (Locus*) it ()) {

		temp = nextLocus->TotalPeakHeight (numPeaks);

		if (numPeaks >= 2) {
		
			N += numPeaks;
			totalHeight += temp;
		}
	}

	numberOfPeaks = N;
	return totalHeight;
}


void ChannelData :: ReportSampleData (RGTextOutput& ExcelText, const RGString& indent) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	RGString indent2 = indent + "     ";
	Endl endLine;
	ExcelText << CLevel (1) << endLine << indent << "Sample data for Channel Number " << mChannel << endLine << PLevel ();

	while (nextLocus = (Locus*) it ())
		nextLocus->ReportSampleData (ExcelText, indent2);
}


void ChannelData :: ReportArtifacts (RGTextOutput& ExcelText, const RGString& indent) {

	RGDListIterator it (ArtifactList);
	DataSignal* nextSignal;

	while (nextSignal = (DataSignal*) it ()) {

//		nextSignal->ReportAbbreviated (ExcelText, indent);
		nextSignal->ReportNotices (ExcelText, indent, "\t");
	}
}


void ChannelData :: MakePreliminaryCalls (bool isNegCntl, bool isPosCntl, GenotypesForAMarkerSet* pGenotypes) {

	Locus* nextLocus;
	RGDListIterator it (mLocusList);
	RGDList discardList;

	TestFractionalFilters ();   //  Let's try testing this here instead of further down...

	while (nextLocus = (Locus*) it ())
		nextLocus->CallAlleles (isNegCntl, pGenotypes, ArtifactList, PreliminaryCurveList, CompleteCurveList);
	// This is what it used to be:  nextLocus->CallAlleles (CompleteCurveList, isNegCntl, pGenotypes, ArtifactList, PreliminaryCurveList);

	// The code below commented out on 01/29/2016 because, with new pull-up algorithms, we should not be reviewing prior decisions about craters/sigmoidal pull-ups
	//TestForMultiSignals ();
	//it.Reset ();

	//while (nextLocus = (Locus*) it ())
	//	nextLocus->TestForMultiSignals (ArtifactList, PreliminaryCurveList, CompleteCurveList);

	TestProximityArtifacts ();
	TestForInterlocusProximityArtifacts ();
//	TestFractionalFilters ();   //  Why wouldn't we test this earlier???
	it.Reset ();

	while (nextLocus = (Locus*) it ())
		nextLocus->TestForAcceptedTriAllele (isNegCntl, isPosCntl, pGenotypes);
}


int ChannelData :: GetNumberOfSamples () const {

	if (mData == NULL)
		return 0;

	return mData->GetNumberOfSamples ();
}


double ChannelData :: GetLeftEndpoint () const {

	if (mData == NULL)
		return 0.0;

	return mData->LeftEndPoint ();
}


double ChannelData :: GetRightEndpoint () const {

	if (mData == NULL)
		return 0.0;

	return mData->RightEndPoint ();
}


int ChannelData :: AddNoticeToList (Notice* newNotice) {

	if (NewNoticeList.Entries () == 0) {

		mHighestSeverityLevel = newNotice->GetSeverityLevel ();
		mHighestMessageLevel = newNotice->GetMessageLevel ();
		NewNoticeList.Append (newNotice);
	}

	else {

		int temp = newNotice->GetSeverityLevel ();

		if (mHighestSeverityLevel >= temp) {

			mHighestSeverityLevel = temp;
			NewNoticeList.Prepend (newNotice);
		}

		else {

			RGDListIterator it (NewNoticeList);
			Notice* nextNotice;
			bool noticeInserted = false;

			while (nextNotice = (Notice*) it ()) {

				if (temp < nextNotice->GetSeverityLevel ()) {

					--it;
					it.InsertAfterCurrentItem (newNotice);
					noticeInserted = true;
					break;
				}
			}

			if (!noticeInserted)
				NewNoticeList.Append (newNotice);
		}

		temp = newNotice->GetMessageLevel ();

		if (mHighestMessageLevel > temp)
			mHighestMessageLevel = temp;
	}

	return NewNoticeList.Entries ();
}



Boolean ChannelData :: IsNoticeInList (const Notice* target) {

	if (NewNoticeList.Find (target))
		return TRUE;

	return FALSE;
}


Notice* ChannelData :: GetNotice (const Notice* target) {

	return (Notice*) NewNoticeList.Find (target);
}



Boolean ChannelData :: ReportNoticeObjects (RGTextOutput& text, const RGString& indent, const RGString& delim, Boolean reportLink) {

	if (NewNoticeList.Entries () > 0) {

		RGDListIterator it (NewNoticeList);
		Notice* nextNotice;
		text.SetOutputLevel (mHighestMessageLevel);
		RGString indent2 = indent + "\t";

		if (!text.TestCurrentLevel ()) {

			text.ResetOutputLevel ();
			return FALSE;
		}

		Endl endLine;
		text << endLine;

		if (reportLink)
			text << mTableLink;

		text << indent;

		if (mLocusList.Entries () > 0)
			text << "Channel " << mChannel << " Notices:  " << endLine;

		else
			text << "ILS Channel Notices:  " << endLine;

		text.ResetOutputLevel ();

		while (nextNotice = (Notice*) it ())
			nextNotice->Report (text, indent2, delim);

		if (reportLink) {

			text.SetOutputLevel (mHighestMessageLevel);
			text << mTableLink;
			text.ResetOutputLevel ();
		}

		text.Write (1, "\n");
	}

	else
		return FALSE;

	return TRUE;
}


void ChannelData :: ClearNoticeObjects () {

	NewNoticeList.ClearAndDelete ();
}



int ChannelData :: NumberOfNoticeObjects () {

	return NewNoticeList.Entries ();
}


Boolean ChannelData :: ReportAllNoticeObjects (RGTextOutput& text, const RGString& indent, const RGString& delim, Boolean reportLink) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	RGString indent2 = indent + "\t";

	ReportNoticeObjects (text, indent, delim, reportLink);

	while (nextLocus = (Locus*) it())
		nextLocus->ReportNoticeObjects (text, indent2, delim, reportLink);

	return TRUE;
}


Boolean ChannelData :: ReportXMLNoticeObjects (RGTextOutput& text, RGTextOutput& tempText, const RGString& delim) {

	RGDListIterator it (NewNoticeList);
	Notice* nextNotice;
//		text.SetOutputLevel (mHighestMessageLevel);
//		tempText.SetOutputLevel (1);
	int msgNum;
	int triggerLevel = Notice::GetMessageTrigger ();

	//if (!text.TestCurrentLevel () && ArtifactList.Entries () == 0) {

	//	text.ResetOutputLevel ();
	//	tempText.ResetOutputLevel ();
	//	return FALSE;
	//}

//		text.ResetOutputLevel ();
	text.SetOutputLevel (1);
	text << "\t\t\t\t<Channel>\n";
	text << "\t\t\t\t\t<ChannelNr>" << mChannel << "</ChannelNr>\n";
	text.ResetOutputLevel ();

	while (nextNotice = (Notice*) it ()) {

		if (nextNotice->GetMessageLevel () <= triggerLevel) {

			text.SetOutputLevel (1);
			tempText.SetOutputLevel (1);
			msgNum = Notice::GetNextMessageNumber ();
			nextNotice->SetMessageNumber (msgNum);
			text << "\t\t\t\t\t<MessageNumber>" << msgNum << "</MessageNumber>\n";
			tempText << "\t\t<Message>\n";
			tempText << "\t\t\t<MessageNumber>" << msgNum << "</MessageNumber>\n";
			tempText << "\t\t\t<Index>" << nextNotice->GetID () << "</Index>\n";
			tempText << "\t\t\t<Text>" << (nextNotice->AssembleString (delim)).GetData () << "</Text>\n";
			tempText << "\t\t</Message>\n";
			text.ResetOutputLevel ();
			tempText.ResetOutputLevel ();
		}
	}

	RGDListIterator art (ArtifactList);
	DataSignal* nextSignal;
	text.SetOutputLevel (1);
	tempText.SetOutputLevel (1);

	while (nextSignal = (DataSignal*) art ())
		nextSignal->WriteTableArtifactInfoToXML (text, tempText, "\t\t\t\t\t", "Artifact", "meanbps");

	text << "\t\t\t\t</Channel>\n";

	text.ResetOutputLevel ();
	tempText.ResetOutputLevel ();

	return TRUE;
}


Boolean ChannelData :: ReportXMLILSNoticeObjects (RGTextOutput& text, RGTextOutput& tempText, const RGString& delim) {

	if (NewNoticeList.Entries () > 0) {

		RGDListIterator it (NewNoticeList);
		Notice* nextNotice;
		text.SetOutputLevel (mHighestMessageLevel);
		tempText.SetOutputLevel (1);
		int msgNum;
		int triggerLevel = Notice::GetMessageTrigger ();

		if (!text.TestCurrentLevel ()) {

			text.ResetOutputLevel ();
			tempText.ResetOutputLevel ();
			return FALSE;
		}

		text.SetOutputLevel (1);
		text << "\t\t\t<ILSAlerts>\n";
		text.ResetOutputLevel ();

		while (nextNotice = (Notice*) it ()) {

			if (nextNotice->GetMessageLevel () <= triggerLevel) {

				msgNum = Notice::GetNextMessageNumber ();
				nextNotice->SetMessageNumber (msgNum);
				text << "\t\t\t\t<MessageNumber>" << msgNum << "</MessageNumber>\n";
				tempText << "\t\t<Message>\n";
				tempText << "\t\t\t<MessageNumber>" << msgNum << "</MessageNumber>\n";
				tempText << "\t\t\t<Index>" << nextNotice->GetID () << "</Index>\n";
				tempText << "\t\t\t<Text>" << (nextNotice->AssembleString (delim)).GetData () << "</Text>\n";
				tempText << "\t\t</Message>\n";
			}
		}

		text << CLevel (1) << "\t\t\t</ILSAlerts>\n";

		text.ResetOutputLevel ();
		tempText.ResetOutputLevel ();
	}

	else
		return FALSE;

	return TRUE;
}



int ChannelData :: CountSignalsWithNotice (const Notice* target, ChannelData* laneStd) {

	int count = 0;
	RGDListIterator it (ArtifactList);
	DataSignal* nextSignal;
	double minMean = laneStd->GetFirstAnalyzedMean ();
	double maxMean = laneStd->GetLastAnalyzedMean ();
	double mean;

	while (nextSignal = (DataSignal*) it()) {

		mean = nextSignal->GetMean ();

		if (!CoreBioComponent::SignalIsWithinAnalysisRegion (nextSignal, minMean))	// Modified 03/13/2015
			continue;

		if (mean > maxMean)
			break;

		if (nextSignal->IsNoticeInList (target))
			count++;
	}

	return count;
}


RGString ChannelData :: IsNoticeInAnyLocusList (const Notice* target) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	RGString info;
	int i = 0;

	while (nextLocus = (Locus*) it()) {

		if (nextLocus->IsNoticeInList (target)) {

			if (i == 0)
				info << nextLocus->GetLocusName ();

			else
				info << ", " << nextLocus->GetLocusName ();

			i++;
		}
	}

	return info;
}


bool ChannelData :: HasPrimerPeaks (ChannelData* laneStd) {

	smMinimumHeightForPrimerPeaks minPrimerHeight;
	smMinimumNumberOfPrimerPeaksForNegativeControlChannel minNumberOfPrimerPeaks;
	
	double minHeightToBePrimer = (double)GetThreshold (minPrimerHeight);
	int numberOfPrimerPeaksRequired = GetThreshold (minNumberOfPrimerPeaks);

	RGDListIterator it (CompleteCurveList);
	DataSignal* nextSignal;
	double minimumTimeForILS;
	int nPeaks = 0;

	if (laneStd != NULL)
		minimumTimeForILS = laneStd->GetFirstAnalyzedMean ();

	else
		minimumTimeForILS = 0.0;

	while (nextSignal = (DataSignal*) it ()) {

		if (CoreBioComponent::SignalIsWithinAnalysisRegion (nextSignal, minimumTimeForILS))	// Modified 03/13/2015
			break;

		if (nextSignal->Peak () >= minHeightToBePrimer) {

			nPeaks++;

			if (nPeaks >= numberOfPrimerPeaksRequired)
				return true;
		}
	}

	return false;
}



double ChannelData :: GetLastTime () const {

	DataSignal* last = (DataSignal*) CompleteCurveList.Last ();

	if (last == NULL)
		return (double) (mData->GetNumberOfSamples () - 10);

	return last->GetMean ();
}



void ChannelData :: SetCompleteSignalListSequence () {

	//RGDListIterator it (CompleteCurveList);
	RGDListIterator it (SmartPeaks);
	DataSignal* nextSignal;
	DataSignal* prevSignal = NULL;
	smSigmoidalSidePeak sigmoidalSidePeak;
	smCraterSidePeak craterSidePeak;

	while (nextSignal = (DataSignal*) it ()) {

		//if (nextSignal->IsDoNotCall ())
		//	continue;

		//if (nextSignal->DontLook ())
		//	continue;

		//if (nextSignal->GetMessageValue (sigmoidalSidePeak))
		//	continue;

		//if (nextSignal->GetMessageValue (craterSidePeak))
		//	continue;

		if (prevSignal != NULL)
			prevSignal->SetNextSignal (nextSignal);

		nextSignal->SetPreviousSignal (prevSignal);
		prevSignal = nextSignal;
	}
}



double ChannelData :: EvaluateBaselineAtTime (double time) {

	if (mBaseLine == NULL)
		return 0.0;

	return mBaseLine->EvaluateWithExtrapolation (time);
}


double ChannelData :: GetMaxAbsoluteRawDataInInterval (double center, double halfWidth) const {

	if (mData == NULL)
		return 0.0;

	int startPt;
	int endPt;

	startPt = (int) floor (center - halfWidth);
	endPt = (int) ceil (center + halfWidth);

	if (startPt < 0)
		startPt = 0;

	if (endPt >= mData->GetNumberOfSamples ())
		endPt = mData->GetNumberOfSamples () - 1;

	int i;
	double max = 0.0;
	double temp;
	double aTemp;
	double answer = 0.0;
	int displacement = endPt - startPt;

	if (displacement < 2)
		return answer;

	double nPoints = (double) (displacement + 1);
	double average = 0.0;

	for (i=startPt; i<=endPt; i++)
		average += mData->Value (i);

	average = average / nPoints;

	for (i=startPt; i<=endPt; i++) {

		temp = mData->Value (i) - average;
		aTemp = fabs (temp);

		if (aTemp > max) {

			max = aTemp;
			answer = temp;
		}
	}

	return answer;
}


bool ChannelData :: TestMaxAbsoluteRawDataInInterval (double center, double halfWidth, double fractionNoiseRange, double& value) const {

	if (mData == NULL) {

		value = 0.0;
		return false;
	}

	int startPt;
	int endPt;

	startPt = (int) floor (center - halfWidth);
	endPt = (int) ceil (center + halfWidth);

	if (startPt < 0)
		startPt = 0;

	if (endPt >= mData->GetNumberOfSamples ())
		endPt = mData->GetNumberOfSamples () - 1;

	int i;
	
	int minTime = startPt;
	int maxTime = startPt;
	double minDistanceFromCenter = center - minTime;
	double maxDistanceFromCenter = center - maxTime;
	double temp;
	value = 0.0;
	int displacement = endPt - startPt;

	if (displacement < 2)
		return false;

	double nPoints = (double) (displacement + 1);
	double max = mData->Value (startPt);
	double min = mData->Value (startPt);
	//double average = 0.0;

	//for (i=startPt; i<=endPt; i++)
	//	average += mData->Value (i);

	//average = average / nPoints;

	for (i=startPt+1; i<=endPt; i++) {

		temp = mData->Value (i);
		//aTemp = fabs (temp);

		if (temp == 0.0)
			continue;

		if (temp > max) {

			maxTime = i;
			max = temp;
			maxDistanceFromCenter = fabs (center - maxTime);
		}

		else if (temp == max) {

			if (fabs (i - center) < maxDistanceFromCenter) {

				maxTime = i;
				max = temp;
				maxDistanceFromCenter = fabs (center - maxTime);
			}
		}

		if (temp < min) {

			minTime = i;
			min = temp;
			minDistanceFromCenter = fabs (center - minTime);
		}

		else if (temp == min) {

			if (fabs (i - center) < minDistanceFromCenter) {

				minTime = i;
				min = temp;
				minDistanceFromCenter = fabs (center - minTime);
			}
		}
	}

	double noiseRange = fractionNoiseRange * mData->GetNoiseRange ();
	bool maxMinusMinBelowNoise = (fabs (max- min) <= noiseRange);

	if (maxMinusMinBelowNoise)
		return false;

	bool maxAboveThreshold = (max >= noiseRange);
	bool minAboveThreshold = (-min >= noiseRange);
	bool maxAboveZero = (max > 0.0);
	bool minBelowZero = (min < 0.0);
	double sigmoidPosition;
	double sigmoidDistanceFromCenter;

	if (!maxAboveZero && !minBelowZero)
		return false;

	if (maxAboveZero && !minBelowZero) {

		// This is a positive max

		value = max;
		return true;
	}
	
	if (!maxAboveZero && minBelowZero) {

		// This is a negative min, but (12/2/2020), do we need to save this? 

		value = min;
		return true;
	}

	// The remaining case is that max > 0 and min < 0

	if (maxAboveThreshold && minAboveThreshold) {

		// Then both are non-zero as well

		sigmoidPosition = (minTime * max - maxTime * min) / (max - min);
		sigmoidDistanceFromCenter = fabs (center - sigmoidPosition);
		value = max;  // sigmoids take on the height of the positive peak (12/2/2020)

		//if (maxDistanceFromCenter < minDistanceFromCenter) {

		//	if (maxDistanceFromCenter <= sigmoidDistanceFromCenter) {

		//		value = max;
		//		return true;
		//	}

		//	else {

		//		value = 1.0;  // The raw data implies a sigmoidal peak
		//		return true;
		//	}
		//}

		//else if (minDistanceFromCenter == maxDistanceFromCenter){

		//	// This is sigmoidal raw data because the min and max must straddle the center, which implies the sigmoid is closest

		//	value = 1.0;
		//	return true;
		//}

		//else { //  minDistanceFromCenter < maxDiistanceFromCenter

		//	if (minDistanceFromCenter <= sigmoidDistanceFromCenter) {

		//		value = min;
		//		return true;
		//	}

		//	else {

		//		value = 1.0;  // The raw data implies a sigmoidal peak
		//		return true;
		//	}
		//}
	}

	else if (maxAboveThreshold) {

		value = max;
		return true;
	}

	else if (minAboveThreshold) {

		value = min;
		return true;
	}

	value = 0.0;
	return false;
}



int ChannelData :: CreateAndSubstituteSinglePassFilteredSignalForRawData (int window) {

	if (mBackupData != NULL)
		delete mBackupData;

	mBackupData = mData;
	mData = mBackupData->CreateMovingAverageFilteredSignal (window);
	return 0;
}



int ChannelData :: CreateAndSubstituteTriplePassFilteredSignalForRawData (int window) {

	if (mBackupData != NULL)
		delete mBackupData;

	mBackupData = mData;
	mData = mBackupData->CreateThreeMovingAverageFilteredSignal (window);
	return 0;
}


int ChannelData :: CreateAndSubstituteAveragingFilteredSignalForRawData (int nPasses, int halfWidth, double fractionNoiseLevelForLevelChange, double splitTime) {

	if (mBackupData != NULL)
		delete mBackupData;

	smPrePrimerPercentOfNoiseRangeForLevelChange prePrimerPercentMsg;
	double startFraction = 0.01 * (double) GetThreshold (prePrimerPercentMsg);

	double postPrimerNoiseLevel = fractionNoiseLevelForLevelChange * mData->GetNoiseRange ();
	double prePrimerNoiseLevel = startFraction * mData->GetNoiseRange ();
	mFilterChangeArray = new bool [GetNumberOfSamples ()];

	mBackupData = mData;
	mData = mBackupData->CreateAveragingFilteredSignal (nPasses, halfWidth, postPrimerNoiseLevel, prePrimerNoiseLevel, mFilterChangeArray, mFractionOfChangedFilterPoints, splitTime);
	return 0;
}



int ChannelData :: RestoreRawDataAndDeleteFilteredSignal () {

	if (mBackupData == NULL)
		return -1;

	delete mData;
	mData = mBackupData;
	mBackupData = NULL;
	return 0;
}


bool ChannelData :: HasFilteredData () const {

	if (mBackupData == NULL)
		return false;

	return true;
}



int ChannelData :: GetHighestMessageLevelFromLoci () {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	int highest = mHighestMessageLevel;
	int locusHighest;

	while (nextLocus = (Locus*) it()) {

		locusHighest = nextLocus->GetHighestMessageLevel ();

		if (locusHighest < 0)
			continue;

		if (highest < 0)
			highest = locusHighest;

		else if (locusHighest < highest)
			highest = locusHighest;
			
	}

	return highest;
}


int ChannelData :: HierarchicalLaneStandardChannelAnalysis (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int ChannelData :: SetData (SampleData& fileData, TestCharacteristic* testConrolPeak, TestCharacteristic* testSamplePeak) {

	mData = fileData.GetDataSignalForDataChannel (mFsaChannel);

	Notice* newNotice;
	RGString noticeInfo;

	if (mData == NULL) {

		ErrorString = "Could not get sample data signal from file for channel ";
		ErrorString << mChannel << "\n";
		newNotice = new CouldNotReadFSA;
		noticeInfo << " for channel " << mChannel;
		newNotice->AddDataItem (noticeInfo);
		AddNoticeToList (newNotice);
		return -1;
	}

	FinalCurveList.Clear ();
	MarginalCurveList.Clear ();
	ArtifactList.Clear ();
	BleedThroughCandidateList.Clear ();
	PreliminaryCurveList.Clear ();
	CompleteCurveList.ClearAndDelete ();

	return 0;
}


int ChannelData :: SetRawData (SampleData& fileData, TestCharacteristic* testConrolPeak, TestCharacteristic* testSamplePeak) {

	mData = fileData.GetRawDataSignalForDataChannel (mFsaChannel);
	Notice* newNotice;
	RGString noticeInfo;

	if (mData == NULL) {

		ErrorString = "Could not get sample data signal from file for channel ";
		ErrorString << mChannel << "\n";
		newNotice = new CouldNotReadFSA;
		noticeInfo << " for channel " << mChannel;
		newNotice->AddDataItem (noticeInfo);
		AddNoticeToList (newNotice);
		return -1;
	}

	FinalCurveList.Clear ();
	MarginalCurveList.Clear ();
	ArtifactList.Clear ();
	BleedThroughCandidateList.Clear ();
	PreliminaryCurveList.Clear ();
	CompleteCurveList.ClearAndDelete ();

	return 0;
}


int ChannelData :: AddLocus (Locus* locus) {

	mLocusList.Append (locus);
	return 0;
}


int ChannelData :: FitAllCharacteristics (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	// This code should be moved to STRChannelData...it's not generic enough for ChannelData
	
	STRTracePrequalification trace;
	DataSignal* nextSignal;
	double fit;
	int TestResult;
	DataSignal* signature;
	double secondaryContent;
	double minRFU = GetMinimumHeight ();
	double maxRFU = GetMaximumHeight ();
	double minAcceptableFit = ParametricCurve::GetMinimumFitThreshold ();
	double minFitForArtifactTest = ParametricCurve::GetTriggerForArtifactTest ();
	double minFit = minFitForArtifactTest;
	double absoluteMinFit = ParametricCurve::GetAbsoluteMinimumFit ();

	if (minAcceptableFit > minFit)
		minFit = minAcceptableFit;

	if (CoreBioComponent::GetGaussianSignature ())
		signature = new NormalizedGaussian (0.0, ParametricCurve::GetSigmaForSignature ());

	else
		signature = new DoubleGaussian (0.0, ParametricCurve::GetSigmaForSignature ());

	ArtifactList.Clear ();
	MarginalCurveList.Clear ();
	FinalCurveList.Clear ();
	msg.ResetMessage ();
	PreliminaryCurveList.Clear ();
	CompleteCurveList.ClearAndDelete ();
	double lineFit;

	mData->ResetCharacteristicsFromRight (trace, text, minRFU, print);

	Endl endLine;
	ExcelText.SetOutputLevel (1);
	ExcelText << "Using minimum RFU = " << minRFU << endLine;
	ExcelText.ResetOutputLevel ();
	int dualReturn;
	double absoluteMinFitLessEpsilon = absoluteMinFit - 0.01;

	while (nextSignal = mData->FindNextCharacteristicFromRight (*signature, fit, CompleteCurveList)) {

		secondaryContent = fabs(nextSignal->GetScale (2));
		double mean = nextSignal->GetMean ();
		lineFit = mData->TestConstantCharacteristicRetry ();

		if (lineFit > minFitForArtifactTest) {

			delete nextSignal;
			continue;
		}

		if ((!nextSignal->IsUnimodal ()) || (fit < absoluteMinFit)) {

			dualReturn = TestForDualPeak (minRFU, maxRFU, nextSignal, fit, CompleteCurveList, true);

			if (dualReturn <= 0)
				TestForArtifacts (nextSignal, absoluteMinFitLessEpsilon);

			continue;
		}

		if (secondaryContent > 0.9 * nextSignal->Peak ()) {

			TestForArtifacts (nextSignal, fit);
			continue;
		}

		if (fit < minFit) {

			dualReturn = TestForDualPeak (minRFU, maxRFU, nextSignal, fit, CompleteCurveList);

			if (dualReturn <= 0)
				TestForArtifacts (nextSignal, fit);

			continue;
		}

		else {  // nextSignal is acceptable for now, so add it to the CurveList

			PreliminaryCurveList.Prepend (nextSignal);
			CompleteCurveList.Prepend (nextSignal);
//			i++;
		}		
	}   //  We are done finding characteristics

	RGDListIterator it (PreliminaryCurveList);

	while (nextSignal = (DataSignal*) it ()) {

		TestResult = mTestPeak->Test (nextSignal, minRFU, maxRFU);

		if (TestResult < 0) {

			it.RemoveCurrentItem ();

			if (TestResult != -20) {

				ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
				nextSignal->ClearNoticeObjects ();
			}
		}
	}

	DataSignal* prevSignal = NULL;
	RGDList tempList;
	prevSignal = (DataSignal*)PreliminaryCurveList.GetFirst ();
	double minDistance = ChannelData::GetMinimumDistanceBetweenPeaks ();

	while (nextSignal = (DataSignal*) PreliminaryCurveList.GetFirst ()) {

		if (prevSignal != NULL) {

			if (fabs(prevSignal->GetMean () - nextSignal->GetMean ()) < minDistance) {

				// "get rid" of the one that fits least well and use the other for the next test.
				// later, if we want, we can add redundant signal to artifact list with a notice...

				if (prevSignal->GetCurveFit () > nextSignal->GetCurveFit ()) {

					// keep prevSignal and "lose" nextSignal
					CompleteCurveList.RemoveReference (nextSignal);
					ArtifactList.RemoveReference (nextSignal);
					delete nextSignal;
					continue;
				}

				else {

					CompleteCurveList.RemoveReference (prevSignal);
					ArtifactList.RemoveReference (prevSignal);
					delete prevSignal;
					prevSignal = nextSignal;
					continue;
				}
			}

			else {

				tempList.Append (prevSignal);
				prevSignal = nextSignal;
			}
		}
	}

	if (prevSignal != NULL)
		tempList.Append (prevSignal);

	while (nextSignal = (DataSignal*) tempList.GetFirst ())
		PreliminaryCurveList.Append (nextSignal);

	delete signature;
//	ProjectNeighboringSignalsAndTest (1.0, 1.0);
	return 0;
}


int ChannelData :: AnalyzeLaneStandardChannel (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int ChannelData :: AnalyzeLaneStandardChannelRecursively (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


Boolean ChannelData :: AssignCharacteristicsToLoci (ChannelData* lsData) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	Boolean returnValue = TRUE;

	//  All the characteristics we are looking for are in the PreliminaryCurveList

	while (nextLocus = (Locus*) it()) {

		if (!nextLocus->ExtractGridSignals (PreliminaryCurveList, mLaneStandard, lsData)) {

			ErrorString << nextLocus->GetErrorString ();
			returnValue = FALSE;
		}
	}

	if (!returnValue)
		ErrorString << "Channel number " << mChannel << " could not assign characteristics to loci.  Skipping...\n";

	return returnValue;
}


int ChannelData :: AnalyzeGridLoci (RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int ChannelData :: AnalyzeSampleLoci (ChannelData* lsData, RGTextOutput& text, RGTextOutput& ExcelText, OsirisMsg& msg, Boolean print) {

	return -1;
}


int ChannelData :: TestFractionalFilters () {

	return -1;
}


int ChannelData :: FinalTestForPeakSizeAndNumber (double averageHeight, Boolean isNegCntl, Boolean isPosCntl, GenotypesForAMarkerSet* pGenotypes) {

	return -1;
}


int ChannelData :: FinalTestForCriticalLaneStandardNotices () {

	return -1;
}


int ChannelData :: SetAllApproximateIDs (ChannelData* laneStd) {

	CoordinateTransform* globalSouthern = laneStd->GetIDMap ();

	if (globalSouthern == NULL)
		return -1;

	RGDListIterator it (CompleteCurveList);
	DataSignal* nextSignal;
	double bp;
//	int ibp;
	double first = laneStd->GetFirstAnalyzedMean ();
	double last = laneStd->GetLastAnalyzedMean ();
	double mean;
	double yPrime;

	while (nextSignal = (DataSignal*) it ()) {

		mean = nextSignal->GetMean ();

		//if (!CoreBioComponent::SignalIsWithinAnalysisRegion (nextSignal, first))	// Modified 03/13/2015
		//	continue;

		//if (mean > last)	// no longer "eliminate" peaks to right of ILS
		//	break;

		bp = globalSouthern->EvaluateWithExtrapolation (mean, yPrime);
//		ibp = (int) floor (bp + 0.5);
//		nextSignal->SetApproximateBioID ((double) ibp);
		nextSignal->SetApproximateBioID (bp);
		nextSignal->SetApproxBioIDPrime (yPrime);
		nextSignal->CalculateTheoreticalArea ();
	}

	while (nextSignal = (DataSignal*)mIgnorePeaks.GetFirst ()) {

		SmartPeaks.InsertWithNoReferenceDuplication (nextSignal);
	}

	//if (mNegativeCurveList.IsEmpty ())
	//	return 0;

	//int n = mNegativeCurveList.Entries ();

	RGDListIterator itneg (mNegativeCurveList);
	//cout << "Setting neg ILS-bps" << endl;

	while (nextSignal = (DataSignal*) itneg ()) {

		mean = nextSignal->GetMean ();

		if (mean < first)
			continue;

		//if (mean > last)	// no longer "eliminate" peaks to right of ILS
		//	break;

		bp = globalSouthern->EvaluateWithExtrapolation (mean, yPrime);
		//ibp = (int) floor (bp + 0.5);
		//nextSignal->SetApproximateBioID ((double) ibp);
		nextSignal->SetApproximateBioID (bp);
		nextSignal->SetApproxBioIDPrime (yPrime);
	}

	//cout << "Done setting neg ILS-bps" << endl;

	return 0;
}


int ChannelData :: RemoveAllShouldersTooCloseToPrimary () {

	RGDListIterator it (PreliminaryCurveList);
	DataSignal* nextSignal;
	DataSignal* prevSignal;
	double nextBP;
	double prevBP;
	double nextWidth;
	double prevWidth;
	bool tooNarrow;
	//bool tooCloseByWidths;

	prevSignal = NULL;
	RGDList tempSignals;

	while (nextSignal = (DataSignal*) it ()) {

		nextBP = nextSignal->GetApproximateBioID ();
		nextWidth = nextSignal->GetWidth ();

		if (prevSignal == NULL) {

			prevSignal = nextSignal;
			prevBP = nextBP;
			prevWidth = nextWidth;
			continue;
		}

		if (prevSignal->IsShoulderSignal ()) {

			if (nextBP - prevBP < 0.5)
				tempSignals.InsertWithNoReferenceDuplication (prevSignal);

			else if (nextBP - prevBP < 0.8) {

				tooNarrow = (prevWidth < 0.5 * nextWidth);
		//		tooCloseByWidths = (nextSignal->GetMean () - prevSignal->GetMean () < 0.35 * (prevWidth + nextWidth));

				if (tooNarrow)  // || tooCloseByWidths)
					tempSignals.InsertWithNoReferenceDuplication (prevSignal);
			}
		}

		else if (nextSignal->IsShoulderSignal ()) {

			if (nextBP - prevBP < 0.5)
				tempSignals.InsertWithNoReferenceDuplication (nextSignal);

			else if (nextBP - prevBP < 0.8) {

				tooNarrow = (nextWidth < 0.5 * prevWidth);
			//	tooCloseByWidths = (nextSignal->GetMean () - prevSignal->GetMean () < 0.35 * (prevWidth + nextWidth));

				if (tooNarrow)  // || tooCloseByWidths)
					tempSignals.InsertWithNoReferenceDuplication (nextSignal);
			}
		}

		prevBP = nextBP;
		prevWidth = nextWidth;
		prevSignal = nextSignal;
	}

	RGDListIterator tempIt (tempSignals);

	while (nextSignal = (DataSignal*) tempIt ()) {

		PreliminaryCurveList.RemoveReference (nextSignal);
		CompleteCurveList.RemoveReference (nextSignal);
		FinalCurveList.RemoveReference (nextSignal);
		ArtifactList.RemoveReference (nextSignal);
		SmartPeaks.RemoveReference (nextSignal);
		nextSignal->RemoveCrossChannelSignalLinkSM ();
	}

	tempSignals.ClearAndDelete ();
	return 0;
}


bool ChannelData :: TestForArtifacts (DataSignal* currentSignal, double fit) {

	NormalizedSuperGaussian BlobSignature3 (0.0, ParametricCurve::GetSigmaForSignature (), 6);
	NormalizedGaussian GaussianSignature (0.0, ParametricCurve::GetSigmaForSignature ());
	NormalizedSuperGaussian JustSuperGaussianSignature (0.0, ParametricCurve::GetSigmaForSignature (), 3);
	Notice* newNotice;
	DataSignal* TestSignal3;
	double fit3;
	bool rtn;
	double absoluteMinFit = ParametricCurve::GetAbsoluteMinimumFit ();
	double minFitForArtifactTest = ParametricCurve::GetTriggerForArtifactTest ();

//	fit3 = NormalizedGaussian...oops, mData has to do this linear test

	// Later...add test for spikes

	TestSignal3 = mData->FindNextCharacteristicRetry (BlobSignature3, fit3, CompleteCurveList);

	if ((TestSignal3 != NULL) && (fit3 > fit) && (TestSignal3->GetStandardDeviation () >= 6.0) && (fit3 >= absoluteMinFit)) {

		// it's a dye blob!  NOTE!!!  WE MAY HAVE TO CHANGE THIS WHEN WE USE RAW DATA!!!

		ArtifactList.Prepend (TestSignal3);
		PreliminaryCurveList.Prepend (TestSignal3);
		newNotice = new BlobFound;
		TestSignal3->AddNoticeToList (newNotice);
		TestSignal3->AddNoticeToList (1, "", "Suspected dye blob");
		CompleteCurveList.Prepend (TestSignal3);
		delete currentSignal;
		rtn = true;
	}

	else {

		delete TestSignal3;
		TestSignal3 = mData->FindNextCharacteristicRetry (GaussianSignature, fit3, CompleteCurveList);

		if ((TestSignal3 != NULL) && (fit3 > fit) && (fit3 >= absoluteMinFit)) {

			// It's Gaussian and not an artifact
			CompleteCurveList.InsertWithNoReferenceDuplication (TestSignal3);
			PreliminaryCurveList.InsertWithNoReferenceDuplication (TestSignal3);
			delete currentSignal;
			rtn = true;
		}

		else {

			delete TestSignal3;
			TestSignal3 = mData->FindNextCharacteristicRetry (JustSuperGaussianSignature, fit3, CompleteCurveList);

			if ((TestSignal3 != NULL) && (fit3 > fit) && (fit3 >= absoluteMinFit)) {

				// It's barely super Gaussian and not an artifact
				CompleteCurveList.InsertWithNoReferenceDuplication (TestSignal3);
				PreliminaryCurveList.InsertWithNoReferenceDuplication (TestSignal3);
				delete currentSignal;
				rtn = true;
			}

			else {

				delete TestSignal3;

				if (fit >= absoluteMinFit) {

					CompleteCurveList.InsertWithNoReferenceDuplication (currentSignal);
					PreliminaryCurveList.InsertWithNoReferenceDuplication (currentSignal);  // This is a test!  We keep poor fits in case they are needed later, at which point we point them out.
					ArtifactList.InsertWithNoReferenceDuplication (currentSignal);
					rtn = false;
				}

				else {

					delete currentSignal;
					rtn = true;
				}				
			}
		}
	}

	return rtn;
}


bool ChannelData :: TestForArtifacts (DataSignal* currentSignal, double fit, int dualSignal) {

	if (dualSignal == 0)
		return TestForArtifacts (currentSignal, fit);

	NormalizedSuperGaussian BlobSignature3 (0.0, ParametricCurve::GetSigmaForSignature (), 6);
	Notice* newNotice;
	DataSignal* TestSignal3;
	double fit3;
	bool rtn;
	double absoluteMinFit = ParametricCurve::GetAbsoluteMinimumFit ();

	// Later...add test for spikes

	TestSignal3 = mData->FindNextCharacteristicRetry (BlobSignature3, fit3, CompleteCurveList, dualSignal);

	if ((TestSignal3 != NULL) && (fit3 > fit) && (TestSignal3->GetStandardDeviation () >= 6.0) && (fit3 >= absoluteMinFit)) {

		// it's a dye blob!  NOTE!!!  WE MAY HAVE TO CHANGE THIS WHEN WE USE RAW DATA!!!

		ArtifactList.Prepend (TestSignal3);
		PreliminaryCurveList.Prepend (TestSignal3);
		newNotice = new BlobFound;
		TestSignal3->AddNoticeToList (newNotice);
		TestSignal3->AddNoticeToList (1, "", "Suspected dye blob");
		CompleteCurveList.Prepend (TestSignal3);
		delete currentSignal;
		rtn = true;
	}

	else {

		delete TestSignal3;

		if (fit >= absoluteMinFit) {

			CompleteCurveList.Prepend (currentSignal);
			PreliminaryCurveList.Prepend (currentSignal);  // This is a test!  We keep poor fits in case they are needed later, at which point we point them out.
			ArtifactList.Prepend (currentSignal);
			rtn = false;
		}

		else {

			delete currentSignal;
			rtn = true;
		}
	}

	return rtn;

}


int ChannelData :: TestForDualPeak (double minRFU, double maxRFU, DataSignal* currentSignal, double currentFit, RGDList& previous, bool originalUnacceptable) {

	DualDoubleGaussian signature;
	double fit;
	DualDoubleGaussian* TestDual = (DualDoubleGaussian*) mData->FindNextCharacteristicRetry (signature, fit, previous);
	DataSignal* leftSignal;
	DataSignal* rightSignal;
	double secondaryContent;
	int rtnValue = 2;
	double minAcceptableFit = ParametricCurve::GetMinimumFitThreshold ();
	double minFitForArtifactTest = ParametricCurve::GetTriggerForArtifactTest ();
	double minFit = minFitForArtifactTest;
	double absoluteMinFit = ParametricCurve::GetAbsoluteMinimumFit ();

	if (minAcceptableFit > minFit)
		minFit = minAcceptableFit;

	if (TestDual != NULL) {

		leftSignal = TestDual->GetFirstCurve ();
		rightSignal = TestDual->GetSecondCurve ();

		if ((leftSignal == NULL) || (rightSignal == NULL)) {

			delete leftSignal;
			delete rightSignal;
			delete TestDual;
			return -1;
		}

		if ((!originalUnacceptable) && (rightSignal->GetCurveFit () < currentFit) && (leftSignal->GetCurveFit () < currentFit)) {

			delete leftSignal;
			delete rightSignal;
			delete TestDual;
			return -1;
		}

		secondaryContent = fabs(rightSignal->GetScale (2));

		if ((!rightSignal->IsUnimodal ()) || (secondaryContent > 0.9 * rightSignal->Peak ()) || (rightSignal->GetCurveFit () < absoluteMinFit)) {

			delete rightSignal;
			rtnValue--;
		}

		else {

			fit = rightSignal->GetCurveFit ();

			if (fit < minFit) {

				// In this case, the fit is totally unacceptable or marginal, so try for alternate signatures
				if (TestForArtifacts (rightSignal, fit, 2))
					rtnValue--;
			}

			else {  // rightSignal is acceptable for now, so add it to the CurveList

				PreliminaryCurveList.Prepend (rightSignal);
				CompleteCurveList.Prepend (rightSignal);
			}		
		}

		secondaryContent = fabs(leftSignal->GetScale (2));

		if ((!leftSignal->IsUnimodal ()) || (secondaryContent > 0.9 * leftSignal->Peak ()) || (leftSignal->GetCurveFit () < absoluteMinFit)) {

			delete leftSignal;
			rtnValue--;
		}

		else {

			fit = leftSignal->GetCurveFit ();

			if (fit < minFit) {

				// In this case, the fit is totally unacceptable or marginal, so try for alternate signatures
				if (TestForArtifacts (leftSignal, fit, 1))
					rtnValue--;
			}

			else {  // leftSignal is acceptable for now, so add it to the CurveList

				PreliminaryCurveList.Prepend (leftSignal);
				CompleteCurveList.Prepend (leftSignal);
			}		
		}
	}

	else
		rtnValue = -1;

	if (rtnValue > 0)
		delete currentSignal;

	delete TestDual;
	return rtnValue;
}


int ChannelData :: RemoveSignalsOutsideLaneStandard (ChannelData* laneStandard) {

	RGDListIterator it (PreliminaryCurveList);
	DataSignal* nextSignal;
	double left = laneStandard->GetFirstAnalyzedMean ();
	double right = laneStandard->GetLastAnalyzedMean ();
	double mean;

	//double minBioID = (double) CoreBioComponent::GetMinBioIDForArtifacts ();
	//double approxBioID;

	while (nextSignal = (DataSignal*) it()) {

		mean = nextSignal->GetMean ();
		//approxBioID = nextSignal->GetApproximateBioID ();

		if ((mean > right) || (!CoreBioComponent::SignalIsWithinAnalysisRegion (nextSignal, left))) {	// modified 03/13/2015

			it.RemoveCurrentItem ();
			ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
			nextSignal->AddNoticeToList (OutputLevelManager::PeakOutsideLaneStandard, "", "Signal lies outside internal lane standard interval");
		}
	}

	return 0;
}


int ChannelData :: AssignSampleCharacteristicsToLoci (CoreBioComponent* grid, CoordinateTransform* timeMap) {

	return -1;
}


int ChannelData :: TestArtifactListForNotice (const Notice* notice) {

	Notice* newNotice;
	RGDListIterator it (ArtifactList);
	DataSignal* nextSignal;
	RGString info;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->IsNoticeInList (notice)) {

			newNotice = (Notice*)notice->Copy ();
			info = "at sample location ";
			info << nextSignal->GetMean ();
			newNotice->AddDataItem (info);
			AddNoticeToList (newNotice);
		}
	}

	return 0;
}


int ChannelData :: TestArtifactListForNotices (RGDList& listOfTargetNotices) {

	Notice* newNotice;
	RGDListIterator it (ArtifactList);
	DataSignal* nextSignal;
	RGString info;
	RGDListIterator it2 (listOfTargetNotices);
	Notice* nextNotice;

	while (nextSignal = (DataSignal*) it ()) {

		it2.Reset ();

		while (nextNotice = (Notice*) it2 ()) {

			if (nextSignal->IsNoticeInList (nextNotice)) {

				newNotice = (Notice*)nextNotice->Copy ();
				info = "at sample location ";
				info << nextSignal->GetMean ();
				newNotice->AddDataItem (info);
				AddNoticeToList (newNotice);
			}
		}
	}

	return 0;
}


int ChannelData :: TestArtifactListForNoticeWithinLaneStandard (const Notice* notice, ChannelData* laneStandard) {

	Notice* newNotice;
	RGDListIterator it (ArtifactList);
	DataSignal* nextSignal;
	RGString info;
	double left = laneStandard->GetFirstAnalyzedMean ();
	double right = laneStandard->GetLastAnalyzedMean ();
	double bp;
	int ibp;
	CoordinateTransform* transform = laneStandard->GetIDMap ();
	double mean;

	while (nextSignal = (DataSignal*) it ()) {

		mean = nextSignal->GetMean ();

		if ((mean < left) || (mean > right))
			continue;

		if (nextSignal->IsNoticeInList (notice)) {

			newNotice = (Notice*)notice->Copy ();
			info = "at location ~";
			bp = nextSignal->GetApproximateBioID ();
			ibp = (int) floor (bp + 0.5);
			info << ibp << " bps";
			newNotice->AddDataItem (info);
			AddNoticeToList (newNotice);
		}
	}

	return 0;
}


int ChannelData :: TestArtifactListForNoticesWithinLaneStandard (RGDList& listOfTargetNotices, ChannelData* laneStandard) {

	Notice* newNotice;
	RGDListIterator it (ArtifactList);
	DataSignal* nextSignal;
	RGString info;
	double left = laneStandard->GetFirstAnalyzedMean ();
	double right = laneStandard->GetLastAnalyzedMean ();
	double bp;
	int ibp;
	CoordinateTransform* transform = laneStandard->GetIDMap ();
	double mean;
	RGDListIterator it2 (listOfTargetNotices);
	Notice* nextNotice;

	while (nextSignal = (DataSignal*) it ()) {

		mean = nextSignal->GetMean ();

		if ((mean < left) || (mean > right))
			continue;

		it2.Reset ();

		while (nextNotice = (Notice*) it2()) {
		
			if (nextSignal->IsNoticeInList (nextNotice)) {

				newNotice = (Notice*)nextNotice->Copy ();
				info = "at location ~";
				bp = nextSignal->GetApproximateBioID ();
				ibp = (int) floor (bp + 0.5);
				info << ibp << " bps";
				newNotice->AddDataItem (info);
				AddNoticeToList (newNotice);
			}
		}
	}

	return 0;
}


int ChannelData :: TestArtifactListForNoticesWithinLaneStandard (ChannelData* laneStandard, CoreBioComponent* associatedGrid) {

	Notice* newNotice;
	RGDListIterator it (ArtifactList);
	DataSignal* nextSignal;
	RGString info;
	double left = laneStandard->GetFirstAnalyzedMean ();
	double right = laneStandard->GetLastAnalyzedMean ();
	double bp;
	int ibp;
	CoordinateTransform* transform = laneStandard->GetIDMap ();
	double minID = (double)CoreBioComponent::GetMinBioIDForArtifacts ();
	double minReportTime; 
	
	if (minID > 0.0)
		minReportTime = laneStandard->GetTimeForSpecifiedID (minID);

	else
		minReportTime = 0.0;

	double trueLeft = left;

	if (minReportTime > left)
		trueLeft = minReportTime;

	double mean;
	Notice* nextNotice;
	SignalPeakAboveMaxRFU target;
	bool foundMaxExceeded = false;

	while (nextSignal = (DataSignal*) it ()) {

		mean = nextSignal->GetMean ();

		if (mean < trueLeft)
			continue;

		if (mean > right)
			break;

//		if (nextSignal->HasCrossChannelSignalLink ())
//			continue;

		nextSignal->ResetNotices ();
		info = "at location ~";
		bp = nextSignal->GetApproximateBioID ();
		ibp = (int) floor (bp + 0.5);
		info << ibp << " bps";

		while (nextNotice = nextSignal->GetNextNoticeObject ()) {

			if ((!foundMaxExceeded) && (target.IsEqualTo (nextNotice)))
				foundMaxExceeded = true;
			
			newNotice = (Notice*)nextNotice->Copy ();
			newNotice->AddDataItem (info);
			AddNoticeToList (newNotice);
			
		}
	}

//	RGDListIterator it2 (SupplementalArtifacts);
//
//	while (nextSignal = (DataSignal*) it2 ()) {
//
//		mean = nextSignal->GetMean ();
//
//		if (mean < trueLeft)
//			continue;
//
//		if (mean > right)
//			break;
//
//		nextSignal->ResetNotices ();
//
//		while (nextNotice = nextSignal->GetNextNoticeObject ()) {
//
//			if ((!foundMaxExceeded) && (target.IsEqualTo (nextNotice)))
//				foundMaxExceeded = true;
//			
//			newNotice = (Notice*)nextNotice->Copy ();
//			info = "at location ~";
//			bp = nextSignal->GetApproximateID ();
//			ibp = (int) floor (bp + 0.5);
//			info << ibp << " bps";
//			newNotice->AddDataItem (info);
//			AddNoticeToList (newNotice);
//		}
//
////		TestLociForInterLocusPeakWithinLaneStandard (nextSignal, trueLeft, right, associatedGrid, transform);
//	}

	if (foundMaxExceeded) {

		nextNotice = new OneOrMorePeaksAboveMaxRFU;
		AddNoticeToList (nextNotice);
	}

	return 0;
}


int ChannelData :: TestLociForInterLocusPeakWithinLaneStandard (DataSignal* signal, double left, double right, CoreBioComponent* assocGrid, CoordinateTransform* transform) {

	InterLocusPeak target;
	InterlocusPeakToLeftOfLocus leftTarget;
	InterlocusPeakToRightOfLocus rightTarget;
	Notice* newNotice;
	RGString info;
	double bp;
	int ibp;

	if (assocGrid == NULL)
		return -1;

	if (!signal->GetNotice (&target))
		return -1;

	double mean = signal->GetMean ();

	if ((mean < left) || (mean > right))
		return -1;

	if (TestInterlocusSignalHeightBelowThreshold (signal))
		return 0;

	Locus* nextLocus;
	Locus* previousLocus = NULL;
	Locus* gridLocus;
	RGDListIterator it (mLocusList);
	bp = signal->GetApproximateBioID ();
	ibp = (int) floor (bp + 0.5);
	info << "~" << ibp << " bps";

	while (nextLocus = (Locus*) it ()) {

		gridLocus = assocGrid->FindLocus (mChannel, nextLocus->GetLocusName ());

		if (gridLocus == NULL)
			return -1;
		
		if (nextLocus->TestSignalPositionRelativeToLocus (signal, gridLocus) < 0) {

			// this peak is to the left of this locus

			if (!nextLocus->GetNotice (&leftTarget)) {

				newNotice = new InterlocusPeakToLeftOfLocus;
				nextLocus->AddNoticeToList (newNotice);
			}

			newNotice = nextLocus->GetNotice (&leftTarget);

			if (newNotice == NULL)
				return -1;

			newNotice->AddDataItem (info);

			if (previousLocus != NULL) {

				if (!previousLocus->GetNotice (&rightTarget)) {

					newNotice = new InterlocusPeakToRightOfLocus;
					previousLocus->AddNoticeToList (newNotice);
				}

				newNotice = previousLocus->GetNotice (&rightTarget);

				if (newNotice == NULL)
					return -1;

				newNotice->AddDataItem (info);
			}

			return 0;
		}

		previousLocus = nextLocus;
	}

	nextLocus = (Locus*) mLocusList.Last ();

	if (nextLocus == NULL)
		return -1;

	if (!nextLocus->GetNotice (&rightTarget)) {

		newNotice = new InterlocusPeakToRightOfLocus;
		nextLocus->AddNoticeToList (newNotice);
	}

	newNotice = nextLocus->GetNotice (&rightTarget);

	if (newNotice == NULL)
		return -1;

	newNotice->AddDataItem (info);
	return 0;
}


int ChannelData :: AnalyzeCrossChannel (ChannelData** channels, int nChannels) {

	return -1;
}


int ChannelData :: TestForMultiSignals () {

	return -1;
}



int ChannelData :: TestProximityArtifacts () {

	return -1;
}


int ChannelData :: AssignSignalToFirstLocusAndDeleteFromSecond (DataSignal* target, Locus* locus1, Locus* locus2) {

	return -1;
}


int ChannelData :: RemoveSignalFromBothLoci (DataSignal* target, Locus* locus1, Locus* locus2) {

	return -1;
}


bool ChannelData :: TestIfSignalBelongsToFirstLocus (DataSignal* target, Locus* locus1, Locus* locus2) {

	return false;
}


int ChannelData :: ResolveAmbiguousInterlocusSignals () {

	return -1;
}


bool ChannelData :: TestInterlocusSignalHeightBelowThreshold (DataSignal* signal) {

	return false;
}


int ChannelData :: CorrectCrossChannelAnalyses (bool isNegCntl) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	StutterFound stutter;
	PullUpFound pullup;
	AdenylationFound adenylation;
	PullUpPrimaryChannel primary;
	RemovablePrimaryPullUp removable;
	SignalPeakBelowFractionalFilterHeight fractFilter;
	PeakBelowFractionalFilterLeft fractFilterLeft;
	PeakBelowFractionalFilterRight fractFilterRight;
	PeakBelowPullupFractionalFilterLeft pullupFFLeft;
	PeakBelowPullupFractionalFilterRight pullupFFRight;
	PeakBelowPullupFractionalFilter pullupFractionalFilter;
	Notice* newNotice;
	UnexpectedNumberOfPeaks target;
//	InterchannelLinkage* iChannel;
	bool pullupFF;
	bool FF;

	while (nextLocus = (Locus*) it ())
		nextLocus->CorrectCrossChannelAnalyses (ArtifactList, isNegCntl);

	RGDListIterator art (ArtifactList);
	DataSignal* nextSignal;

	while (nextSignal = (DataSignal*) art ()) {

		if (nextSignal->IsNoticeInList (&pullup)) {

			if (nextSignal->IsPossibleInterlocusAllele (-1)) {

				if (nextSignal->IsPossibleInterlocusAllele (1))
					pullupFF = ((nextSignal->IsNoticeInList (&pullupFFLeft)) && (nextSignal->IsNoticeInList (&pullupFFRight)));

				else if (nextSignal->IsNoticeInList (&pullupFFLeft))
					pullupFF = true;

				else
					pullupFF = false;
			}

			else {

				if (nextSignal->IsPossibleInterlocusAllele (1)) {
					
					if (nextSignal->IsNoticeInList (&pullupFFRight))
						pullupFF = true;

					else
						pullupFF = false;
				}

				else
					pullupFF = false;
			}

			if (pullupFF) {

				newNotice = new PeakBelowPullupFractionalFilter;
				nextSignal->AddNoticeToList (newNotice);
				nextSignal->SetAlleleName ("");
				nextSignal->SetAlleleName ("", -1);
				nextSignal->SetAlleleName ("", 1);
			}
			
			//if ((nextSignal->IsNoticeInList (&stutter)) || (nextSignal->IsNoticeInList (&adenylation)) || (nextSignal->IsNoticeInList (&fractFilter)) || pullupFF || (nextSignal->IsNoticeInList (&pullupFractionalFilter))) {

			//	iChannel = nextSignal->GetInterchannelLink ();

			//	if (iChannel != NULL) {

			//		iChannel->RemoveDataSignal (nextSignal, &pullup, NULL, &primary, &removable);

			//		if (iChannel->IsEmpty ())
			//			iChannel->RemoveAll (&primary, NULL, &pullup, NULL);

			//		else
			//			iChannel->RecalculatePrimarySignal (&pullup, &primary);
			//	}

			//	else
			//		nextSignal->RemoveCrossChannelSignalLink (&primary, &pullup, &removable, NULL);
			//}
		}

		if (!nextSignal->IsNoticeInList (&fractFilter)) {

			if (nextSignal->IsPossibleInterlocusAllele (-1)) {

				if (nextSignal->IsPossibleInterlocusAllele (1))
					FF = ((nextSignal->IsNoticeInList (&fractFilterLeft)) && (nextSignal->IsNoticeInList (&fractFilterRight)));

				else if (nextSignal->IsNoticeInList (&fractFilterLeft))
					FF = true;

				else
					FF = false;
			}

			else {

				if (nextSignal->IsPossibleInterlocusAllele (1)) {
					
					if (nextSignal->IsNoticeInList (&fractFilterRight))
						FF = true;

					else
						FF = false;
				}

				else
					FF = false;
			}

			if (FF) {

				newNotice = new SignalPeakBelowFractionalFilterHeight;
				nextSignal->AddNoticeToList (newNotice);
				nextSignal->SetAlleleName ("");
			}
		}
	}

	if (isNegCntl) {

		it.Reset ();

		while (nextLocus = (Locus*) it ()) {

			if (nextLocus->IsNoticeInList (&target)) {

				if (!IsNoticeInList (&target)) {

					newNotice = new UnexpectedNumberOfPeaks;
					AddNoticeToList (newNotice);
				}
			}
		}
	}

	return 0;
}


int ChannelData :: CorrectLaneStandardCrossChannelAnalysis () {

	return 0;
}


int ChannelData :: CleanUpLocusSignalLists () {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;

	while (nextLocus = (Locus*) it ())
		nextLocus->CleanUpSignalList (ArtifactList);

	return 0;
}


int ChannelData :: CorrectGridLocusSignals () {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	StutterFound stutter;
	PullUpFound pullup;
	AdenylationFound adenylation;
	PullUpPrimaryChannel primary;


	while (nextLocus = (Locus*) it ())
		nextLocus->CorrectGridSignalList (ArtifactList);

	RGDListIterator art (ArtifactList);
	DataSignal* nextSignal;

	while (nextSignal = (DataSignal*) art ()) {

		if (nextSignal->IsNoticeInList (&pullup)) {

			if ((nextSignal->IsNoticeInList (&stutter)) || (nextSignal->IsNoticeInList (&adenylation)))
				nextSignal->RemoveCrossChannelSignalLink (&primary, &pullup, NULL, NULL);
		}
	}

	return 0;
}


int ChannelData :: CleanUpGridLocusSignals () {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	int retValue = 0;

	while (nextLocus = (Locus*) it ()) {

		if (nextLocus->CleanUpGridSignalList (ArtifactList) < 0)
			retValue = -1;
	}

	return retValue;
}


double ChannelData :: GetWidthAtTime (double t) const {

	return -1.0;
}


double ChannelData :: GetSecondaryContentAtTime (double t) const {

	return -1.0;
}


double ChannelData :: GetTimeForSpecifiedID (double id) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	Locus* nextLocus2;
	RGDList ExtendedList;

	while (nextLocus = (Locus*)it ()) {

		if (nextLocus->GridLocusContainsID (id))
			return nextLocus->GetTimeFromBP (id);
	}

	it.Reset ();

	while (nextLocus = (Locus*)it ()) {

		if (nextLocus->ExtendedLocusContainsID (id))
			ExtendedList.Append (nextLocus);
	}

	int size = ExtendedList.Entries ();
	
	if (size > 0) {

		if (size > 1) {

			nextLocus = (Locus*)ExtendedList.GetFirst ();
			nextLocus2 = (Locus*)ExtendedList.GetFirst ();

			if (nextLocus->GridDistance (id) <= nextLocus2->GridDistance (id))
				return nextLocus->GetTimeFromBP (id);
			
			return nextLocus2->GetTimeFromBP (id);
		}

		nextLocus = (Locus*)ExtendedList.GetFirst ();
		return nextLocus->GetTimeFromBP (id);
	}

	it.Reset ();

	nextLocus = (Locus*) it ();

	if (nextLocus == NULL)
		return -1.0;

	nextLocus2 = nextLocus;
	double minDist = nextLocus->GridDistance (id);
	double currentDist;

	while (nextLocus = (Locus*)it ()) {

		currentDist = nextLocus->GridDistance (id);

		if (currentDist <= minDist) {

			minDist = currentDist;
			nextLocus2 = nextLocus;
		}
	}

	return nextLocus2->GetTimeFromBP (id);
}


CoordinateTransform* ChannelData :: GetIDMap () {

	return NULL;
}


int ChannelData :: FindAndRemoveFixedOffset () {

	int status = mData->FindAndRemoveFixedOffset ();
	cout << "      Channel " << mChannel << " noise = " << mData->GetNoiseRange () << endl;

	return status;
}


int ChannelData :: ProjectNeighboringSignalsAndTest (double horizontalResolution, double verticalTolerance) {

	DataSignal* nextSignal;
	DataSignal* previousSignal = NULL;
	RGDListIterator CurveIterator (PreliminaryCurveList);
	double mean;
	double nextMean;
	double peak;
	double nextPeak;
	double tail;
	double nextTail;
	double spacing = DataSignal::GetSampleSpacing ();
	double minRFU = GetMinimumHeight ();

	double f0, f1, x0, x1, xm, fm;

	while (nextSignal = (DataSignal*) CurveIterator()) {

		if (previousSignal != NULL) {

			mean = previousSignal->GetMean ();
			nextMean = nextSignal->GetMean ();
			peak = previousSignal->Peak ();
			nextPeak = nextSignal->Peak ();
			tail = previousSignal->Value (nextMean);
			nextTail = nextSignal->Value (mean);

			if (nextPeak <= tail)
				nextSignal->MarkForDeletion (true);

			if (peak <= nextTail)
				previousSignal->MarkForDeletion (true);
		}
	}

	CurveIterator.Reset ();

	while (nextSignal = (DataSignal*) CurveIterator()) {

		if (nextSignal->GetMarkForDeletion ()) {

			CurveIterator.RemoveCurrentItem ();
			delete nextSignal;
		}
	}

	CurveIterator.Reset ();

	while (nextSignal = (DataSignal*) CurveIterator()) {

		if (previousSignal != NULL) {

			mean = previousSignal->GetMean ();
			nextMean = nextSignal->GetMean ();
			peak = previousSignal->Peak ();
			nextPeak = nextSignal->Peak ();
			tail = previousSignal->Value (nextMean);
			nextTail = nextSignal->Value (mean);

			if (nextPeak <= tail)
				nextSignal->MarkForDeletion (true);

			if (peak <= nextTail)
				previousSignal->MarkForDeletion (true);
		}
	}

	CurveIterator.Reset ();

	while (nextSignal = (DataSignal*) CurveIterator()) {

		if (nextSignal->GetMarkForDeletion ()) {

			CurveIterator.RemoveCurrentItem ();
			delete nextSignal;
		}
	}

	CurveIterator.Reset ();

	while (nextSignal = (DataSignal*) CurveIterator()) {

		if (previousSignal != NULL) {

			mean = previousSignal->GetMean ();
			nextMean = nextSignal->GetMean ();
			peak = previousSignal->Peak ();
			nextPeak = nextSignal->Peak ();
			tail = previousSignal->Value (nextMean);
			nextTail = nextSignal->Value (mean);

			if (nextPeak <= tail)
				nextSignal->MarkForDeletion (true);

			if (peak <= nextTail)
				previousSignal->MarkForDeletion (true);
		}
	}

	CurveIterator.Reset ();

	while (nextSignal = (DataSignal*) CurveIterator()) {

		if (nextSignal->GetMarkForDeletion ()) {

			CurveIterator.RemoveCurrentItem ();
			delete nextSignal;
		}
	}

	CurveIterator.Reset ();

	while (nextSignal = (DataSignal*) CurveIterator()) {

		if (previousSignal != NULL) {

			mean = previousSignal->GetMean ();
			nextMean = nextSignal->GetMean ();
			peak = previousSignal->Peak ();
			nextPeak = nextSignal->Peak ();
			tail = previousSignal->Value (nextMean);
			nextTail = nextSignal->Value (mean);

			if (((tail > verticalTolerance) || (nextTail > verticalTolerance)) && (nextPeak >= tail) && (peak >= nextTail)) {

				// Search for the intersection point of the two curves and make that point
				// the dividing line between the two

				x0 = mean;
				x1 = nextMean;
				f0 = peak - nextTail;
				f1 = tail - nextPeak;
				xm = 0.5 * (x0 + x1);

				while (x1 - x0 > horizontalResolution) {

					xm = 0.5 * (x0 + x1);
					fm = previousSignal->Value (xm) - nextSignal->Value (xm);

					if (fabs(fm) < verticalTolerance)
						break;

					if (fm > 0.0) {

						x0 = xm;
						f0 = fm;
					}

					else {

						x1 = xm;
						f1 = fm;
					}
				}

//				xm = spacing * (floor ((xm - Left) / spacing) + 0.5) + Left;
				previousSignal->SetTestRightEndPoint (xm);
				nextSignal->SetTestLeftEndPoint (xm);
			}
		}

		previousSignal = nextSignal;
	}

	/*CurveIterator.Reset ();
	double testLevel;

	while (nextSignal = (DataSignal*) CurveIterator()) {

		testLevel = nextSignal->Peak () - minRFU;

		if ((nextSignal->LeftTestValue () >= testLevel) && (nextSignal->RightTestValue () >= testLevel) && (!nextSignal->isMarginallyAboveMinimum ())) {

			nextSignal->AddNoticeToList (OutputLevelManager::PeakHeightBelowMinimumStandard, "", "Peak below minimum threshold");
			nextSignal->SetDontLook (true);
			CurveIterator.RemoveCurrentItem ();
			ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
		}
	}*/

	return 0;
}


int ChannelData :: RemoveInterlocusSignals (bool isNegCntl) {

	DataSignal* nextSignal;
	Notice* newNotice;
//	RGString info;

	while (nextSignal = (DataSignal*) PreliminaryCurveList.GetFirst ()) {

		ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
		nextSignal->AddNoticeToList (OutputLevelManager::OffLadderSignalBetweenLoci, "", "Off ladder signal between loci");
		newNotice = new InterLocusPeak;
//		info = " for channel ";
//		info << mChannel;
//		newNotice->AddDataItem (info);
		nextSignal->AddNoticeToList (newNotice);

		if (isNegCntl) {

			newNotice = new UnexpectedNumberOfPeaks;

			if (IsNoticeInList (newNotice))
				delete newNotice;

			else
				AddNoticeToList (newNotice);
		}
	}

	return 0;
}


int ChannelData :: RemoveInterlocusSignals (bool isNegCntl, double left, double ilsLeft, double right, CoreBioComponent* assocGrid, CoordinateTransform* transform) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	Locus* previousLocus = NULL;
	Notice* newNotice;
	RGString infoLeft;
	RGString infoRight;
	bool foundInterlocusLeft = false;
	bool foundInterlocusRight = false;
	bool finishedLastSignal = false;
	DataSignal* nextSignal;
	double mean;
	Locus* gridLocus;
	double bp;
	int ibp;
	Locus* previousGridLocus;
//	double fractionalFilter = GetFractionalFilter ();
	bool thisPeakRight;
	bool thisPeakLeft;
	PullUpFound pullup;
	PullUpPrimaryChannel primaryPullup;
//	double peak;

	if (isNegCntl) {

		nextSignal = (DataSignal*) PreliminaryCurveList.Last ();

		if (nextSignal == NULL)
			return 0;

		if (nextSignal->GetMean () >= ilsLeft) {

			newNotice = new UnexpectedNumberOfPeaks;

			if (IsNoticeInList (newNotice))
				delete newNotice;

			else
				AddNoticeToList (newNotice);
		}

		MergeListAIntoListBByReference (PreliminaryCurveList, ArtifactList);
		return 0;
	}

	RGDListIterator pIt (PreliminaryCurveList);

	while (nextSignal = (DataSignal*) pIt ()) {

		if (TestInterlocusSignalHeightBelowThreshold (nextSignal)) {

			pIt.RemoveCurrentItem ();
			newNotice = new InterlocusPeakBelowInterlocusMinRFU;
			nextSignal->AddNoticeToList (newNotice);
			ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
		}
	}

//	double maxLeft;
//	double maxRight;
//	double filterThresholdLeft;
//	double filterThresholdRight;

	while (nextSignal = (DataSignal*) PreliminaryCurveList.GetFirst ()) {

		mean = nextSignal->GetMean ();

		if (mean < left) {

			ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
			continue;
		}

		else {

			PreliminaryCurveList.Prepend (nextSignal);
			break;
		}
	}

	if (PreliminaryCurveList.Entries () == 0)
		return 0;

	while (nextLocus = (Locus*) it ()) {

		foundInterlocusLeft = false;
		foundInterlocusRight = false;
		infoLeft = "";
		infoRight = "";

		if (assocGrid == NULL)
			gridLocus = NULL;

		else {

			gridLocus = assocGrid->FindLocus (mChannel, nextLocus->GetLocusName ());

			if (gridLocus == NULL)
				return -1;
		}

//		maxRight = nextLocus->GetMaximumHeight ();
//		filterThresholdRight = 0.0;
//		filterThresholdLeft = 0.0;

//		if (fractionalFilter > 0.0)
//			filterThresholdRight = fractionalFilter * maxRight;

		//if (previousLocus != NULL) {

		//	maxLeft = previousLocus->GetMaximumHeight ();

		//	if (fractionalFilter > 0.0)
		//		filterThresholdLeft = fractionalFilter * maxLeft;
		//}

		while (nextSignal = (DataSignal*) PreliminaryCurveList.GetFirst ()) {

			mean = nextSignal->GetMean ();
			thisPeakLeft = thisPeakRight = false;

			if (mean > right) {

				ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
				finishedLastSignal = true;
				MergeListAIntoListBByReference (PreliminaryCurveList, ArtifactList);
				break;
			}

			else if (TestIfTimeIsLeftOfLocus (mean, nextLocus, gridLocus)) {

				if (!TestInterlocusSignalHeightBelowThreshold (nextSignal)) {

	//				peak = nextSignal->Peak ();
					bp = nextSignal->GetApproximateBioID ();
					ibp = (int) floor (bp + 0.5);

					if (foundInterlocusRight)
						infoRight << ", ";
					
					else
						foundInterlocusRight = true;

					infoRight << "~" << ibp << " bps";
					thisPeakRight = true;

					if (previousLocus != NULL) {

						if (foundInterlocusLeft)
							infoLeft << ", ";
						
						else
							foundInterlocusLeft = true;

						infoLeft << "~" << ibp << " bps";
						thisPeakLeft = true;
					}

					ArtifactList.InsertWithNoReferenceDuplication (nextSignal);

					if (thisPeakRight || thisPeakLeft) {

						newNotice = new InterLocusPeak;
						nextSignal->AddNoticeToList (newNotice);
					}

					//else {

					//	nextSignal->RemoveCrossChannelSignalLink (&primaryPullup, &pullup, NULL, NULL);
					//	nextSignal->ClearNoticeObjects ();
					//	newNotice = new SignalPeakBelowFractionalFilterHeightPlus;
					//	nextSignal->AddNoticeToList (newNotice);
					//}
				}
			}

			else {

				PreliminaryCurveList.Prepend (nextSignal);
				break;
			}
		}

		if (foundInterlocusRight) {

			newNotice = new InterlocusPeakToLeftOfLocus;
			newNotice->AddDataItem (infoRight);
			nextLocus->AddNoticeToList (newNotice);
		}

		if ((previousLocus != NULL) && (foundInterlocusLeft)) {

			newNotice = new InterlocusPeakToRightOfLocus;
			newNotice->AddDataItem (infoLeft);
			previousLocus->AddNoticeToList (newNotice);
		}

		if (finishedLastSignal)
			break;

		previousLocus = nextLocus;
		previousGridLocus = gridLocus;
	}

	if (finishedLastSignal)
		return 0;

	if (previousLocus == NULL)
		return -1;

	foundInterlocusLeft = false;
	infoLeft = "";

	while (nextSignal = (DataSignal*) PreliminaryCurveList.GetFirst ()) {

		mean = nextSignal->GetMean ();
		thisPeakLeft = false;

		if (mean > right) {

			ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
			finishedLastSignal = true;
			MergeListAIntoListBByReference (PreliminaryCurveList, ArtifactList);
			break;
		}

		else if (!TestIfTimeIsLeftOfLocus (mean, previousLocus, previousGridLocus)) {

			bp = nextSignal->GetApproximateBioID ();
			ibp = (int) floor (bp + 0.5);
//			peak = nextSignal->Peak ();

			if (foundInterlocusLeft)
				infoLeft << ", ";
			
			else
				foundInterlocusLeft = true;

			infoLeft << "~" << ibp << " bps";
			thisPeakLeft = true;
			newNotice = new InterLocusPeak;
			nextSignal->AddNoticeToList (newNotice);

			//else {

			//	nextSignal->RemoveCrossChannelSignalLink (&primaryPullup, &pullup, NULL, NULL);
			//	nextSignal->ClearNoticeObjects ();
			//	newNotice = new SignalPeakBelowFractionalFilterHeightPlus;
			//	nextSignal->AddNoticeToList (newNotice);
			//}

			ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
		}

		else {

			ArtifactList.InsertWithNoReferenceDuplication (nextSignal);
			continue;
		}
	}

	if (foundInterlocusLeft) {

		newNotice = new InterlocusPeakToRightOfLocus;
		newNotice->AddDataItem (infoLeft);
		previousLocus->AddNoticeToList (newNotice);
	}

	return 0;
}



int ChannelData :: TestInterlocusCharacteristics (double left, double ilsLeft, double right, CoreBioComponent* assocGrid, ChannelData* laneStd, bool isNegCntl) {

//	Locus* nextLocus;
	CoordinateTransform* globalSouthern = laneStd->GetIDMap ();

	if (globalSouthern == NULL)
		return -1;

	// Code below is no longer needed:  Locus::TestInterlocusSignals essentially looks for stutter and adenylation, which is now done earlier.  Must modify RemoveInterlocusSignals
	// to remove only those PreliminaryCurveList peaks that are not ambiguous...they are truly interlocus.  Also, it no longer needs to test for fractional filters and stutter and adenylation.
	// These are already done.

//	RGDListIterator it (mLocusList);

	//while (nextLocus = (Locus*) it ()) {

	//	nextLocus->TestInterlocusSignals (PreliminaryCurveList, ArtifactList, laneStd);
	//}

	RemoveInterlocusSignals (isNegCntl, left, ilsLeft, right, assocGrid, globalSouthern);
	return 0;
}


int ChannelData :: TestForInterlocusProximityArtifacts () {

	return -1;
}


int ChannelData :: TestPositiveControl (IndividualGenotype* genotype) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	RGString locusName;
	IndividualLocus* locus;
	Notice* newNotice;
	Notice* oldNotice;
	int returnValue = 0;

	while (nextLocus = (Locus*) it ()) {

		locusName = nextLocus->GetLocusName ();
		locus = genotype->FindLocus (locusName);

		if (locus == NULL) {

			newNotice = new PositiveCOntrolLocusNotFound;
			oldNotice = GetNotice (newNotice);

			if (oldNotice != NULL) {

				delete newNotice;
				oldNotice->AddDataItem (locusName);
			}

			else {

				newNotice->AddDataItem (locusName);
				AddNoticeToList (newNotice);
			}

			returnValue = -1;
		}

		else {

			if (nextLocus->TestPositiveControl (locus, ArtifactList) < 0)
				returnValue = -1;
		}
	}

	return returnValue;
}


int ChannelData :: FilterNoticesBelowMinimumBioID (ChannelData* laneStd, int minBioID) {

	if (minBioID <= 0)
		return 0;

	double cutOffTime = laneStd->GetTimeForSpecifiedID ((double) minBioID);
	RGDListIterator it (CompleteCurveList);
	DataSignal* nextSignal;

	while (nextSignal = (DataSignal*) it ()) {

		if (nextSignal->GetMean () < cutOffTime)
			nextSignal->ClearNoticeObjects ();

		else
			return 0;
	}

	return 0;
}


bool ChannelData :: TestIfTimeIsLeftOfLocus (double time, Locus* locus, Locus* assocGridLocus) {

	return false;
}


double ChannelData :: GetFractionalFilter () const {

	return 0.0;
}


double ChannelData :: GetMeasurementRatio () const {

	return 0.0;
}


void ChannelData :: AppendAllBaseLoci (RGDList& locusList) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;

	while (nextLocus = (Locus*) it ())
		nextLocus->AppendBaseLocusToList (locusList);
}


DataSignal* ChannelData :: FindPreliminarySignalWithinToleranceOf (DataSignal* target, double tolerance) {

	DataSignal* currentSignal = (DataSignal*)PreliminaryIterator.CurrentItem ();

	if (currentSignal == NULL)
		return NULL;

	DataSignal* returnValue;
	double mean = target->GetMean ();

	if (currentSignal->GetMean () > mean + tolerance)
		return NULL;

	if (fabs(currentSignal->GetMean () - mean) <= tolerance) {

		returnValue = currentSignal;
		PreliminaryIterator ();
		return returnValue;
	}

	while (currentSignal = (DataSignal*) PreliminaryIterator ()) {

		if (currentSignal->GetMean () > mean + tolerance)
			return NULL;

		if (fabs(currentSignal->GetMean () - mean) <= tolerance) {

			returnValue = currentSignal;
			PreliminaryIterator ();
			return returnValue;
		}
	}

	return NULL;
}


bool ChannelData :: ComputeExtendedLocusTimes (CoreBioComponent* grid, CoordinateTransform* inverseTransform, CoreBioComponent* associatedGrid) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	bool status = true;

	while (nextLocus = (Locus*) it ()) {

		if (!nextLocus->ComputeExtendedLocusTimes (grid, inverseTransform, mChannel, associatedGrid))
			status = false;
	}

	return status;
}


bool ChannelData :: AddILSToHistoryList () {

	return false;
}


int ChannelData :: CompareTo (const RGPersistent* p) const {

	ChannelData* q = (ChannelData*) p;

	if (mChannel < q->mChannel)
		return -1;

	if (mChannel == q->mChannel)
		return 0;

	return 1;
}


unsigned ChannelData :: HashNumber (unsigned long Base) const {

	return mChannel%Base;
}


Boolean ChannelData :: IsEqualTo (const RGPersistent* p) const {

	ChannelData* q = (ChannelData*) p;
	return mChannel == q->mChannel;
}


int ChannelData :: WriteRawDataAndFitCurveToOutputRows (RGTextOutput& text, const RGString& delim, const RGString& dyeName, const RGString& sampleName) {

	int NSamples = mData->GetNumberOfSamples ();

	DataSignal* FitCurve = new CompositeCurve (mData->LeftEndPoint (), mData->RightEndPoint (), CompleteCurveList);
	FitCurve->ProjectNeighboringSignals (1.0, 1.0);
	DataSignal* FitData = FitCurve->Digitize (NSamples, mData->LeftEndPoint (), 1.0);

	//cout << "Prepared curve sets for output..." << endl;
	text << sampleName << "Ch" << mChannel << "\n";
	text << dyeName << delim;
	WriteLine (text, delim, FitData);
	text << "BLK" << delim;
	WriteLine (text, delim, mData);

	delete FitCurve;
	delete FitData;
	return 0;
}



int ChannelData :: WriteLine (RGTextOutput& text, const RGString& delim, const DataSignal* ds) {

	Endl endLine;
	int saveSamples = mData->GetNumberOfSamples ();
	
	for (int j=0; j<saveSamples; j++) {

		text << (int)floor (ds->Value (j)) << delim;
	}

	text << endLine;
	return 0;
}


int ChannelData :: WriteRawData (RGTextOutput& text, const RGString& delim) {

	if (mData == NULL)
		return 0;

	int saveSamples = mData->GetNumberOfSamples () - 1;
	
	for (int j=0; j<saveSamples; j++) {

		text << (int)floor (mData->Value (j)) << delim;
	}

	text << (int)floor (mData->Value (saveSamples));
	return 0;
}



int ChannelData :: WriteFitData (RGTextOutput& text, const RGString& delim, bool useMaxValueMethod) {

	if (mData == NULL)
		return 0;

	int NSamples = mData->GetNumberOfSamples ();

	DataSignal* FitCurve = new CompositeCurve (mData->LeftEndPoint (), mData->RightEndPoint (), CompleteCurveList);
	DataSignal* FitData;

	if (useMaxValueMethod) {

		FitData = FitCurve->BuildSample (NSamples, mData->LeftEndPoint (), 1.0);
	}

	else {

		FitCurve->ProjectNeighboringSignals (1.0, 1.0);
		FitData = FitCurve->Digitize (NSamples, mData->LeftEndPoint (), 1.0);
	}

	//cout << "Prepared curve sets for output..." << endl;
	int saveSamples = mData->GetNumberOfSamples () - 1;
	int dataValue;
	
	for (int j=0; j<saveSamples; j++) {

		dataValue = (int)floor (FitData->Value (j));

		if (dataValue < 0)
			dataValue = 0;

		text << dataValue << delim;
	}

	dataValue = (int)floor (FitData->Value (saveSamples));

	if (dataValue < 0)
		dataValue = 0;

	text << dataValue;
	delete FitCurve;
	delete FitData;
	return 0;
}


int ChannelData :: WriteFitData (RGTextOutput& text, const RGString& delim, int numSamples, double left, double right, bool useMaxValueMethod) {

	DataSignal* FitCurve = new CompositeCurve (left, right, CompleteCurveList);
	DataSignal* FitData;

	if (useMaxValueMethod) {

		FitData = FitCurve->BuildSample (numSamples, left, 1.0);
	}

	else {

		FitCurve->ProjectNeighboringSignals (1.0, 1.0);
		FitData = FitCurve->Digitize (numSamples, left, 1.0);
	}

	//cout << "Prepared curve sets for output..." << endl;
	int saveSamples = numSamples - 1;
	int dataValue;
	
	for (int j=0; j<saveSamples; j++) {

		dataValue = (int)floor (FitData->Value (j));

		if (dataValue < 0)
			dataValue = 0;

		text << dataValue << delim;
	}

	dataValue = (int)floor (FitData->Value (saveSamples));

	if (dataValue < 0)
		dataValue = 0;

	text << dataValue;
	delete FitCurve;
	delete FitData;
	return 0;
}


int ChannelData :: WriteBaselineData (RGTextOutput& text, const RGString& delim, const RGString& indent) {

	if (mBaseLine == NULL)
		return 0;

	Endl endLine;
	text << indent << "<baselineStart>" << mBaselineStart << "</baselineStart>" << endLine;
	int NSamples = mData->GetNumberOfSamples ();
	int saveSamples = NSamples - 1;
	int j;
	text << indent << "<baselinePoints>";
	double dynamicBaseline = mBaseLine->EvaluateSequenceStart (0.0, 1.0);
	
	for (j=mBaselineStart; j<saveSamples; j++) {

		text << (int)floor (dynamicBaseline) << delim;
		dynamicBaseline = mBaseLine->EvaluateSequenceNext ();
	}

	text << (int)floor (dynamicBaseline);
	text << "</baselinePoints>" << endLine;
	return 1;
}


int ChannelData :: WriteLocusInfoToXML (RGTextOutput& text, const RGString& indent) {

	Locus* nextLocus;
	RGDListIterator it (mLocusList);

	while (nextLocus = (Locus*) it ())
		nextLocus->WriteXMLInfo (text, indent);

	return 0;
}


void ChannelData :: ReportGridLocusRow (RGTextOutput& text) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	int trigger = Notice::GetMessageTrigger ();
	int highest;

	while (nextLocus = (Locus*) it ()) {
	
		text << "\t";
		highest = nextLocus->GetHighestMessageLevel ();

		if ((highest > 0) && (highest <= trigger))
			text << "XX";
	}
}


void ChannelData :: ReportGridLocusRowWithLinks (RGTextOutput& text) {

	RGDListIterator it (mLocusList);
	Locus* nextLocus;
	int trigger = Notice::GetMessageTrigger ();
	int highest;
	int link;

	while (nextLocus = (Locus*) it ()) {
	
		text << "\t";
		highest = nextLocus->GetHighestMessageLevel ();

		if ((highest > 0) && (highest <= trigger)) {

			link = Notice::GetNextLinkNumber ();
			nextLocus->SetTableLink (link);
			text << nextLocus->GetTableLink () << "XX";
		}
	}
}


void ChannelData::InitializeModsData (SampleModList* sml) {

	if (mData == NULL)
		return;

	if (sml == NULL)
		return;

	int N = mData->GetNumberOfSamples ();
	mModsData = new bool [N];
	int i;

	for (i=0; i<N; i++)
		mModsData [i] = false;

	if (sml != NULL) {

		ChannelModPairs* cmp = sml->GetModPairsForChannel (mChannel);

		if (cmp != NULL) {

		//	cout << "Setting mod regions for channel " << mChannel << " for number of samples = " << N << "\n";
			cmp->SetModRegions (mModsData, N);
		}
	}
}


bool ChannelData::TestPeakAgainstModsData (const DataSignal* ds) const {

	if (mModsData == NULL) {

		//cout << "Mods data is null\n";
		return false;
	}

	double mean = ds->GetMean ();
	int low = (int)floor (mean);
	int high = (int)ceil (mean);

	if ((mean < 1143.0) && (mean > 1142.0)) {

		cout << "Peak with mean = " << mean << " has low = " << low << " and high = " << high;

		if (mModsData [low])
			cout << " with low index true";

		else
			cout << " with low index false";

		if (mModsData [high])
			cout << " and high index true\n";

		else
			cout << " and high index false\n";
	}

	if (!mModsData [low] && !mModsData [high])
		return false;

	if (mModsData [low] && mModsData [high])
		return true;

	bool equalsLow = ((mean - (double)low) == 0.0);
	bool equalsHigh = (((double)high - mean) == 0.0);

	if (equalsLow && mModsData [low])
		return true;

	if (equalsHigh && mModsData [high])
		return true;

	return false;
}



CSplineTransform* TimeTransform (const ChannelData& cd1, const ChannelData& cd2) {

	CSplineTransform* spline = new CSplineTransform (cd1.Means, cd2.Means, cd1.NumberOfAcceptedCurves);
	return spline;
}


CSplineTransform* TimeTransform (const ChannelData& cd1, const ChannelData& cd2, double* firstDerivs, int size) {

	CSplineTransform* spline = new CSplineTransform (cd1.Means, cd2.Means, firstDerivs, cd1.NumberOfAcceptedCurves, true);
	return spline;
}


CSplineTransform* TimeTransform (const ChannelData& cd1, const ChannelData& cd2, bool isHermite) {

	CSplineTransform* spline = new CSplineTransform (cd1.Means, cd2.Means, cd1.NumberOfAcceptedCurves, isHermite);
	return spline;
}


