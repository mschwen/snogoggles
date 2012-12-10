// ORViewerProcessor.h

#ifndef __Viewer__ORViewerProcessor_hh_
#define __Viewer__ORViewerProcessor_hh_

#include "ORCompoundDataProcessor.hh"
#include "ORMTCDecoder.hh"
#include "ORPMTDecoder.hh"
#include "ORRunDecoder.hh"
#include "ORCaen1720Decoder.hh"

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
    std::vector<double> fViewerData1;
    std::vector<double> fViewerData2;
    std::vector<double> fViewerData3;
    std::vector<double> fViewerData4;

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

