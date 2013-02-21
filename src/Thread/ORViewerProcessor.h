// ORViewerProcessor.h

#ifndef __Viewer__ORViewerProcessor_hh_
#define __Viewer__ORViewerProcessor_hh_

#include "ORCompoundDataProcessor.hh"
#include "ORMTCDecoder.hh"
#include "ORPMTDecoder.hh"
#include "ORRunDecoder.hh"
#include "ORCaen1720Decoder.hh"

#include <time.h>
#include <map>
#include <string>

class OrcaReader;

class ORViewerProcessor : public ORCompoundDataProcessor
{
  public:
    ORViewerProcessor(std::string label = "SNOPackedFile.root");
    virtual ~ORViewerProcessor();
    virtual EReturnCode StartRun();
    virtual EReturnCode ProcessDataRecord(UInt_t* record);
    virtual EReturnCode EndRun();

    void SetReader(OrcaReader *reader) { fReader = reader; }

    UInt_t fViewerEventNum;
    UInt_t fGTCount;
    clock_t fViewerClock; 
    UInt_t fViewerLastMtcGtid;
    UInt_t fViewerCurrentMtcGtid;

    std::vector<double> fViewerTruthData1;
    std::vector<double> fViewerTruthData2;
    std::vector<double> fViewerTruthData3;
    std::vector<double> fViewerTruthData4;

    std::vector<double> fViewerCalData1;
    std::vector<double> fViewerCalData2;
    std::vector<double> fViewerCalData3;
    std::vector<double> fViewerCalData4;

    std::vector<double> fViewerUncalData1;
    std::vector<double> fViewerUncalData2;
    std::vector<double> fViewerUncalData3;
    std::vector<double> fViewerUncalData4;




  protected:
    ORMTCDecoder        fMTCDecoder;
    ORPMTDecoder        fPMTDecoder;
    ORCaen1720Decoder   fCaenDecoder;
    ORRunDecoder        fRunDecoder;

    UInt_t  fMTCDataId; 
    UInt_t  fPMTDataId; 
    UInt_t  fCaenDataId;
    UInt_t  fRunId;
    UInt_t  fRunStart;  // flags for run start
    bool    fMustSwap;

    ORDataProcessor* fMTCProcessor;
    ORDataProcessor* fPMTProcessor;
    ORDataProcessor* fCaenProcessor;
    ORDataProcessor* fRunProcessor;
    OrcaReader*      fReader;
};

#endif

