// ORViewerProcessor.cc

#include "ORViewerProcessor.h"
#include "LoadOrcaFileThread.hh"
#include "ORRunContext.hh"
#include "ORLogger.hh"
#include <netinet/in.h>
#include <unistd.h>
#include <bitset>
#include "iostream"

#include <Viewer/DataStore.hh>
using namespace std;

#define GT_MASK     0xffffff            // GTID's are 24 bits

#define FEC_GT_16(a) (UInt_t)((a)&0x0000FFFF)
#define FEC_GTID(a)    (ULong_t) (*(a) & 0x0000FFFF) + (ULong_t) ( ( *(a+2) & 0x0000F000 ) >> 12 << 16 ) + (ULong_t) ( ( *(a+2) & 0xF0000000 ) >> 28 << 20 )


#define FEC_CH(a)   (UInt_t)(((a)&0x001F0000)>>16) 
#define FEC_CR(a)   (UInt_t)(((a)&0x03e00000)>>21)    
#define FEC_SL(a)   (UInt_t)(((a)&0x3C000000)>>26)
#define FEC_LCN(a)  (  512 * FEC_CR(a) + 32 * FEC_SL(a) + FEC_CH(a)    )
#define FEC_SYN(a)  (UInt_t)((((a)&0x80000000)>>31) + ((a)&0x40000000)>>30 ) 

#define FEC_QLX(a)   (UInt_t) (( *(a+1) & 0x000007FF))
#define FEC_QHS(a)   (UInt_t) (( *(a+1) & 0x07FF0000)>>16)

#define FEC_QHL(a)   (UInt_t) (( *(a+2) & 0x000007FF))
#define FEC_TAC(a)   (UInt_t) (( *(a+2) & 0x07FF0000)>>16)

#define XL3_FIFO(a)

#define MTC_10MHZ(a)  (ULong_t)( (ULong_t)(*(a+1) & 0xFFFFFFFF ) + (ULong_t) (*(a+2) & 0x001FFFFF )<<32 )
#define MTC_50MHZ(a)  (ULong_t)( (ULong_t)(*(a+2) & 0xFFE00000 )>>21 + (ULong_t) (*(a+3) & 0xFFFFFFFF )<<32 )

#define MTC_GTID(a)  (UInt_t)((*(a+4) & 0x00FFFFFF) )
#define MTC_GT16(a)  (UInt_t)((*(a+4) & 0x0000FFFF) )
#define MTC_N100(a)  (bool)(*(a+4) &  0x07000000)
#define MTC_N20(a)  (bool)(*(a+4) &  0x18000000) 
#define MTC_ESUM(a)  (bool)(*(a+4) &  0x60000000)
#define MTC_OWL(a)    (bool)((*(a+4) &  0x80000000) + (*(a+5) &  0x00000003)   )
#define MTC_PED(a)    (bool)(*(a+5) &  0x0000001C) 
#define MTC_CLOCK(a)   (bool)(*(a+5) &  0x00000060) 
#define MTC_EXT(a)     (bool)(*(a+5) & 0x0001ff80) 
#define MTC_SOFT(a)     (bool)(*(a+5) & 0x00020000) 
#define MTC_MISS(a)     (bool)(*(a+5) & 0x00040000) 
#define MTC_SYN16(a)     (bool)(*(a+6) & 0x00C00000) 
#define MTC_SYN24(a)     (bool)(*(a+6) & 0x03000000) 


ORViewerProcessor::ORViewerProcessor(std::string /*label*/)
    : fRunStart(0)
{
    SetComponentBreakReturnsFailure();

    fMTCProcessor = new ORDataProcessor(&fMTCDecoder);
    AddProcessor(fMTCProcessor);

    fPMTProcessor = new ORDataProcessor(&fPMTDecoder);
    AddProcessor(fPMTProcessor);

    fCaenProcessor = new ORDataProcessor(&fCaenDecoder);
    AddProcessor(fCaenProcessor);

    fRunProcessor = new ORDataProcessor(&fRunDecoder);
    AddProcessor(fRunProcessor);
}


ORViewerProcessor::~ORViewerProcessor()
{
    delete fMTCProcessor;
    delete fPMTProcessor;
    delete fCaenProcessor;
    delete fRunProcessor;
}


ORDataProcessor::EReturnCode ORViewerProcessor::StartRun()
{
    cout << "Got StartRun Signal"  << endl;

    fMTCDataId = fMTCProcessor->GetDataId();
    fPMTDataId = fPMTProcessor->GetDataId();
    fCaenDataId = fCaenProcessor->GetDataId();
    fRunId = fRunProcessor->GetDataId();

    fViewerTruthData1.resize(10752);
    fViewerTruthData2.resize(10752);
    fViewerTruthData3.resize(10752);
    fViewerTruthData4.resize(10752);
  
    fViewerCalData1.resize(10752);
    fViewerCalData2.resize(10752);
    fViewerCalData3.resize(10752);
    fViewerCalData4.resize(10752);

    fViewerUncalData1.resize(10752);
    fViewerUncalData2.resize(10752);
    fViewerUncalData3.resize(10752);
    fViewerUncalData4.resize(10752);

    fViewerClock = clock();
    fViewerLastMtcGtid = 0;

    if (0x0000ABCD == htonl(0x0000ABCD)) fMustSwap = kFALSE;
    else fMustSwap = kTRUE;

	return kSuccess;
    
}


ORDataProcessor::EReturnCode ORViewerProcessor::EndRun()
{
    cout << "Got EndRun Signal"  << endl;

    return kSuccess;
}


ORDataProcessor::EReturnCode ORViewerProcessor::ProcessDataRecord(UInt_t* record)
{
  unsigned int thisDataId = fMTCDecoder.DataIdOf(record); // any long decoder would do 

    // Create Viewer event with accumulated data if > 0.2 s passed
    if(clock() > fViewerClock+0.2*CLOCKS_PER_SEC) {

        fViewerLastMtcGtid = fViewerCurrentMtcGtid;

       for(int p=0;p<9727;p++) {
         if(fViewerTruthData1[p]) {
          fViewerTruthData1[p]=fViewerTruthData1[p]/fViewerUncalData1[p];
          fViewerTruthData2[p]=fViewerTruthData2[p]/fViewerUncalData1[p];
          fViewerTruthData3[p]=fViewerTruthData3[p]/fViewerUncalData1[p];
          fViewerTruthData4[p]=fViewerTruthData4[p]/fViewerUncalData1[p];
         }
       }

        Viewer::DataStore::GetInstance().Add( fViewerTruthData1, 
                                              fViewerTruthData2, 
                                              fViewerTruthData3, 
                                              fViewerTruthData4,
                                              fViewerCalData1, 
                                              fViewerCalData2, 
                                              fViewerCalData3, 
                                              fViewerCalData4,
                                              fViewerUncalData1, 
                                              fViewerUncalData2, 
                                              fViewerUncalData3, 
                                              fViewerUncalData4);

      for(int chn = 0; chn<32; chn++) {
        fViewerTruthData3[512*19+0*32+chn] = fViewerTruthData3[512*20+0*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+1*32+chn] = fViewerTruthData3[512*20+1*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+2*32+chn] = fViewerTruthData3[512*20+2*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+3*32+chn] = fViewerTruthData3[512*20+3*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+4*32+chn] = fViewerTruthData3[512*20+4*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+5*32+chn] = fViewerTruthData3[512*20+5*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+6*32+chn] = fViewerTruthData3[512*20+6*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+7*32+chn] = fViewerTruthData3[512*20+7*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+8*32+chn] = fViewerTruthData3[512*20+8*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+9*32+chn] = fViewerTruthData3[512*20+9*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+10*32+chn] = fViewerTruthData3[512*20+10*32+(fViewerEventNum +chn)%32]; 
        fViewerTruthData3[512*19+11*32+chn] = fViewerTruthData3[512*20+11*32+(fViewerEventNum +chn)%32]; 

        for(int sl = 0; sl<16; sl++) {
          fViewerTruthData1[512*19+sl*32+chn] = fViewerTruthData3[512*19+sl*32+chn];
          fViewerTruthData2[512*19+sl*32+chn] = fViewerTruthData3[512*19+sl*32+chn];
          fViewerTruthData4[512*19+sl*32+chn] = fViewerTruthData3[512*19+sl*32+chn];
        }
 
      }

       for(int p=0;p<9727;p++) {
          fViewerTruthData4[p]=0;
          fViewerTruthData3[p]=0;
          fViewerTruthData2[p]=0;
          fViewerTruthData1[p]=0;

          fViewerCalData4[p]=0;
          fViewerCalData3[p]=0;
          fViewerCalData2[p]=0;
          fViewerCalData1[p]=0;

          fViewerUncalData4[p]=0;
          fViewerUncalData3[p]=0;
          fViewerUncalData2[p]=0;
          fViewerUncalData1[p]=0;


        }


      fViewerEventNum++;
      fViewerClock = clock();
      fGTCount = 0;
  } 

  //Deal with MTC trigger data
  /*
  //   word 0 
  u_int32 Bc10_1          :32;
  //   word 1 
  u_int32 Bc10_2          :21;
  u_int32 Bc50_1          :11;
  //   word 2 
  u_int32 Bc50_2          :32;
  //   word 3 
  u_int32 BcGT            :24; //   LSB 
  unsigned Nhit_100_Lo          :1;
  unsigned Nhit_100_Med         :1;
  unsigned Nhit_100_Hi          :1;
  unsigned Nhit_20              :1;
  unsigned Nhit_20_LB           :1;
  unsigned ESum_Lo              :1;
  unsigned ESum_Hi              :1;
  unsigned Owln                 :1; //   MSB 

  //   word 4 
  unsigned Owle_Lo              :1;
  unsigned Owle_Hi              :1;
  unsigned Pulse_GT             :1;
  unsigned Prescale             :1;
  unsigned Pedestal             :1;
  unsigned Pong                 :1;
  unsigned Sync                 :1;
  unsigned Ext_Async            :1;
  unsigned Hydrophone           :1;
  unsigned Ext_3                :1;
  unsigned Ext_4                :1;
  unsigned Ext_5                :1;
  unsigned Ext_6                :1;
  unsigned NCD_Shaper           :1;
  unsigned Ext_8                :1;
  unsigned Special_Raw          :1;
  unsigned NCD_Mux              :1;
  unsigned Soft_GT              :1;
  unsigned Miss_Trig            :1;
  unsigned Peak                 :10;
  unsigned Diff_1               :3;

  //  word 5 
  unsigned Diff_2               :7;
  unsigned Int                  :10;
  unsigned TestGT               :1;
  unsigned Test50               :1;
  unsigned Test10               :1;
  unsigned TestMem1             :1;
  unsigned TestMem2             :1;
  unsigned SynClr16             :1;
  unsigned SynClr16_wo_TC16     :1;
  unsigned SynClr24             :1;
  unsigned SynClr24_wo_TC24     :1;
  unsigned FIFOsNotAllEmpty     :1;
  unsigned FIFOsNotAllFull      :1;
  unsigned FIFOsAllFull         :1;
  unsigned Unused1              :1;
  unsigned Unused2              :1;
  unsigned Unused3              :1;
  */
  if (thisDataId == fMTCDataId) 
    {
     // Integrate trigger type (N100,N20,ESUM,OWL,etc)
     // Keep history of previous intervals
     // Use PMT space from crate 19 to display MTC data
    fViewerCurrentMtcGtid = (UInt_t)MTC_GTID(record);
     

     fGTCount++; 
//      cout << "MTC GTID#: " << bitset<32>((UInt_t)(record[4]&0x00FFFFFF))  <<  endl;
      cout << "MTC GTID: " << (UInt_t)MTC_GTID(record) << endl;
     // cout << "NHIT 100: " << (UInt_t)(record[4] &  0x07000000) << endl;
     // cout << "NHIT 20: " << (UInt_t)(record[4] &  0x18000000) << endl;
     // cout << "NHIT 20#: " << MTC_N20(record) << endl;
     // cout << "10 MHZ: " << MTC_10MHZ(record) << endl;
     // cout << "ESUM: "    << (UInt_t)(record[4] & 0x60000000) << endl;
       fViewerTruthData2[512*19+(fViewerEventNum )%32] = (double) (((UInt_t)(record[4]&0x00FFFFFF) % 800));


       fViewerTruthData3[512*20+0*32+(fViewerEventNum )%32]++ ;
       fViewerTruthData3[512*20+1*32+(fViewerEventNum )%32] += MTC_N100(record);
       fViewerTruthData3[512*20+2*32+(fViewerEventNum )%32] += MTC_N20(record); 
       fViewerTruthData3[512*20+3*32+(fViewerEventNum )%32] += MTC_ESUM(record); 
       fViewerTruthData3[512*20+4*32+(fViewerEventNum )%32] += MTC_OWL(record); 
       fViewerTruthData3[512*20+5*32+(fViewerEventNum )%32] += MTC_PED(record); 
       fViewerTruthData3[512*20+6*32+(fViewerEventNum )%32] += MTC_CLOCK(record); 
       fViewerTruthData3[512*20+7*32+(fViewerEventNum )%32] += MTC_EXT(record); 
       fViewerTruthData3[512*20+8*32+(fViewerEventNum )%32] += MTC_SOFT(record); 
       fViewerTruthData3[512*20+9*32+(fViewerEventNum )%32] += MTC_MISS(record); 
       fViewerTruthData3[512*20+10*32+(fViewerEventNum )%32] += MTC_SYN16(record); 
       fViewerTruthData3[512*20+11*32+(fViewerEventNum )%32] += MTC_SYN24(record); 
        


    }
  else if (thisDataId == fPMTDataId) 
    {
    // Integrate TAC and Qs in PMTTruth data
    // Store first GTID, last GTID,  hit count, and SYNC/CLR,  in PmtUnCal data
    // Store hit count in PMTCal data
    /*
  //   word 1 (starts from LSB): 
  unsigned GTID1                :16; //   lower 16 bits 
  unsigned ChannelID            :5;
  unsigned CrateID              :5;
  unsigned BoardID              :4;
  unsigned CGT_ES16             :1;
  unsigned CGT_ES24             :1;

  //   word 2: 
  unsigned Qlx                  :11;
  unsigned SignQlx              :1;
  unsigned CellID               :4;
  unsigned Qhs                  :11;
  unsigned SignQhs              :1;
  unsigned MissedCount          :1;
  unsigned NC_CC                :1;
  unsigned LGI_Select           :1;
  unsigned Cmos_ES16            :1;

  //   word 3            : 
  unsigned Qhl                  :11;
  unsigned SignQhl              :1;
  unsigned GTID2                :4;        //   bits 17-20 
  unsigned TAC                  :11;
  unsigned SignTAC              :1;
  unsigned GTID3                :4;        //   bits 21-24 
  */
      // cout << "Got PMT data record of length " << fPMTDecoder.LengthOf(record) << endl;

      int recnum=1;//Words 0&1 are from ORCA

        //Megabundle header W1:
        recnum++;
        ORUtils::Swap(record[recnum]); 
        // cout << "Megabundle payload from crate " << (UInt_t)((record[recnum]&0x1F000000)>>24) << " of length " << (UInt_t)(record[recnum]&0x00FFFFFF) << endl;
        // cout << "Mega Header W1: " <<  bitset<32>(record[recnum]) << endl;
        //Megabundle header W2:
        recnum++;
        ORUtils::Swap(record[recnum]); 
        // cout << "Passmin counter:" << bitset<32>(record[recnum]) << endl;
        //Megabundle header W3:
        recnum++;
        ORUtils::Swap(record[recnum]); 
        // cout << "XL3 Clock: " <<  bitset<32>(record[recnum]) << endl;
      
        while(recnum < fPMTDecoder.LengthOf(record)-1) 
        {
          //Minibundle Header:
          recnum++;
          ORUtils::Swap(record[recnum]);
          // cout << "Minibundle payload from card " << (UInt_t)((record[recnum]&0xF000000)>>24) << " of length " << (UInt_t)(record[recnum]&0x00FFFFFF) << endl;
          // cout << "Mini Header: " <<  bitset<32>(record[recnum]) << endl;
          //Passmin Minibundle:
          if((UInt_t)(record[recnum]&0x80000000)) 
          { 
            recnum++;
            ORUtils::Swap(record[recnum]);
            // cout << "Passmin Minibundle:" <<  bitset<32>(record[recnum]) << endl;
          }
          else 
          {
            //PMT bundles
            for(int ministop = recnum + (UInt_t)(record[recnum]&0x00FFFFFF); recnum<ministop; )
            {
//              cout << "PMT GTID: " << FEC_GTID(record[recnum+1]) << endl;
              //PMT W1:
              recnum++;
              ORUtils::Swap(record[recnum]);
              // cout << "PMT W1: " <<  bitset<32>(record[recnum]) << endl;
              //cout << "PMT Slot: " << FEC_SL(record[recnum]) << endl;
              //PMT W2:
              recnum++;
              ORUtils::Swap(record[recnum]);
              // cout << "PMT W2: " <<  bitset<32>(record[recnum]) << endl;
              //PMT W3:
              recnum++;
              ORUtils::Swap(record[recnum]);
              // cout << "PMT W3: " <<  bitset<32>(record[recnum]) << endl;
              //}
             // cout << "TAC: " << (UInt_t) FEC_TAC(record[recnum]) << endl;
              //cout << "PMT GTID: " << bitset<32>(FEC_GTID(&record[recnum-2])) << endl;
//              fViewerTruthData3[(int)FEC_LCN(record[recnum-2])] = FEC_GTID(record[recnum-2]); 
//              fViewerTruthData1[(int)FEC_LCN(record[recnum-2])] += FEC_TAC(record[recnum]);
//              fViewerTruthData4[(int)FEC_LCN(record[recnum-2])] += FEC_QHS(record[recnum]);
//              fViewerTruthData2[(int)FEC_LCN(record[recnum-2])]++;


//                if(!fViewerCalData1[(int)FEC_LCN(record[recnum-2])]) {
 //                   fViewerCalData1[(int)FEC_LCN(record[recnum-2])] = 512;
  //                }

    //            fViewerCalData1[(int)FEC_LCN(record[recnum-2])] = min((double)(FEC_GTID(record[recnum-2])-fViewerLastMtcGtid),fViewerCalData1[(int)FEC_LCN(record[recnum-2])]);
               // cout << "FEC GTID: " << FEC_GTID(record[recnum-2]) << "MTC GTID: " << fViewerLastMtcGtid<<endl;
               // cout << "PMT GTID: " << bitset<32>(FEC_GTID(record[recnum-2])) << endl;

      //          if(fViewerCalData1[(int)FEC_LCN(record[recnum-2])] > 512) {
        //          fViewerCalData1[(int)FEC_LCN(record[recnum-2])] = 512;
          //      }

            //   fViewerCalData1[(int)FEC_LCN(record[recnum-2])] = (UInt_t)(FEC_GTID(record[recnum-2]))%512;
                 //cout << (UInt_t)((FEC_GTID(record[recnum-2])))%(UInt_t)512 << endl;
               // if(!fViewerCalData2[(int)FEC_LCN(record[recnum-2])]) {
                 //   fViewerCalData2[(int)FEC_LCN(record[recnum-2])] = (double)(FEC_GTID(record[recnum-2])-fViewerLastMtcGtid)/1000;
                 // }
                //else {
                //    fViewerCalData2[(int)FEC_LCN(record[recnum-2])] = (double)min((double)(FEC_GTID(record[recnum-2])-fViewerLastMtcGtid),fViewerCalData2[(int)FEC_LCN(record[recnum-2])])/1000;
              //    }
                 
              //  fViewerCalData4[(int)FEC_LCN(record[recnum-2])] ++;
                 

                fViewerUncalData1[(int)FEC_LCN(record[recnum-2])]++;
                fViewerUncalData2[(int)FEC_LCN(record[recnum-2])] = (double) ((FEC_GTID(&record[recnum-2]))%4096 );
                //cout << "PMTGTID: " << FEC_GTID(&record[recnum-2])  << "/" <<  fViewerUncalData2[(int)FEC_LCN(record[recnum-2])] << "/" << (FEC_GTID(&record[recnum-2]))%4096<< endl;
                fViewerUncalData3[(int)FEC_LCN(record[recnum-2])] = (double)((FEC_GTID(&record[recnum-2])-fViewerLastMtcGtid)+500);

                fViewerUncalData4[(int)FEC_LCN(record[recnum-2])] += FEC_SYN(record[recnum-2]);



                fViewerTruthData1[(int)FEC_LCN(record[recnum-2])] += FEC_TAC(&(record[recnum-2]))/8;
                fViewerTruthData2[(int)FEC_LCN(record[recnum-2])] += FEC_QHL(&record[recnum-2]);
                fViewerTruthData3[(int)FEC_LCN(record[recnum-2])] += FEC_QHS(&record[recnum-2]);
                fViewerTruthData4[(int)FEC_LCN(record[recnum-2])] += FEC_QLX(&record[recnum-2]);


            }
         
       }
     } 
  }
else if (thisDataId == 1572864) {

 // cout << "might this be an XL3 ping packet?" << endl;
 // cout << "Ping from Crate " << record[1] << endl;

  for(int ss = 2; ss < 18; ss++) {
   // cout << "fecMemLevel[" << ss-2 << "]: " << record[ss] << endl;
    for(int cc = 0; cc < 32; cc++) {
      fViewerCalData3[512*record[1]+32*(ss-2)+cc] =  max(((double)record[ss]+1)*40,fViewerCalData3[512*record[1]+32*(ss-2)+cc] );
    }

    }
  //cout << "mbqLevel: " << record[18] << endl;

}
 
else 
{
  cout << "Received data with ID = " << thisDataId << " of length " << fPMTDecoder.LengthOf(record) << endl;
}

return kSuccess;
}




