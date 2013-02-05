// ORViewerProcessor.cc

#include "ORViewerProcessor.h"
#include "LoadOrcaFileThread.hh"
//#include "PZdabFile.h"
#include "ORRunContext.hh"
#include "ORLogger.hh"
//#include "CUtils.h"
//#include "builder.h"
#include <netinet/in.h>
#include <unistd.h>
#include <bitset>
#include "iostream"

#include <Viewer/DataStore.hh>
using namespace std;

#define GT_MASK     0xffffff            // GTID's are 24 bits
#define GT(a)       (*(a+3) & GT_MASK)

#define FEC_GT_16(a) (UInt_t)((a)&0x0000FFFF)
#define FEC_CR(a)   (UInt_t)(((a)&0x03e00000)>>21)    
#define FEC_SL(a)   (UInt_t)(((a)&0x3C000000)>>26)
#define FEC_CH(a)   (UInt_t)(((a)&0x001F0000)>>16) 

#define FEC_LCN(a)  (  512 * FEC_CR(a) + 32 * FEC_SL(a) + FEC_CH(a)    )

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
//    SNO_printf(5, ORCA_FAC, "Got StartRun() signal! (run %ld)\n",(long)GetRunContext()->GetRunNumber());
    cout << "Got Runstart Signal"  << endl;

    fMTCDataId = fMTCProcessor->GetDataId();
    fPMTDataId = fPMTProcessor->GetDataId();
    fCaenDataId = fCaenProcessor->GetDataId();
    fRunId = fRunProcessor->GetDataId();

      fViewerData4.resize(10752);
      fViewerData3.resize(10752);
      fViewerData2.resize(10752);
      fViewerData1.resize(10752);
   
      fViewerClock = clock();

    if (0x0000ABCD == htonl(0x0000ABCD)) fMustSwap = kFALSE;
    else fMustSwap = kTRUE;

	return kSuccess;
    
}


ORDataProcessor::EReturnCode ORViewerProcessor::EndRun()
{
	return kSuccess;
}


ORDataProcessor::EReturnCode ORViewerProcessor::ProcessDataRecord(UInt_t* record)
{
  unsigned int thisDataId = fMTCDecoder.DataIdOf(record); // any long decoder would do 



  if (thisDataId == fMTCDataId) 
    {

  if(clock() > fViewerClock+0.2*CLOCKS_PER_SEC) {
      Viewer::DataStore::GetInstance().Add( record , fViewerData1, fViewerData2, fViewerData3, fViewerData4 );

      for(int chn = 0; chn<32; chn++) {
        fViewerData3[512*19+0*32+chn] = fViewerData3[512*20+0*32+(fViewerEventNum +chn)%32]; 
        fViewerData3[512*19+1*32+chn] = fViewerData3[512*20+1*32+(fViewerEventNum +chn)%32]; 
        fViewerData3[512*19+2*32+chn] = fViewerData3[512*20+2*32+(fViewerEventNum +chn)%32]; 
        fViewerData3[512*19+3*32+chn] = fViewerData3[512*20+3*32+(fViewerEventNum +chn)%32]; 
        fViewerData3[512*19+4*32+chn] = fViewerData3[512*20+4*32+(fViewerEventNum +chn)%32]; 
        fViewerData3[512*19+5*32+chn] = fViewerData3[512*20+5*32+(fViewerEventNum +chn)%32]; 

/*
        fViewerData3[512*20+0*32+(fViewerEventNum +chn)%32]=0;
        fViewerData3[512*20+1*32+(fViewerEventNum +chn)%32]=0;
        fViewerData3[512*20+2*32+(fViewerEventNum +chn)%32]=0;
        fViewerData3[512*20+3*32+(fViewerEventNum +chn)%32]=0;
        fViewerData3[512*20+4*32+(fViewerEventNum +chn)%32]=0;
        fViewerData1.clear(); 
        fViewerData2.clear(); 
        fViewerData3.clear(); 
        fViewerData4.clear(); 
*/
      //  fViewerData3.clear(); 
      //fViewerData3.resize(10752);

      }



      fViewerEventNum++;
      fViewerClock = clock();

  }


     // cout << "MTC GTID: " << (UInt_t)(record[4]&0x00FFFFFF)  <<  endl;
     // cout << "NHIT 100: " << (UInt_t)(record[4] &  0x07000000) << endl;
     // cout << "NHIT 20: " << (UInt_t)(record[4] &  0x18000000) << endl;
     // cout << "ESUM: "    << (UInt_t)(record[4] & 0x60000000) << endl;

       fViewerData2[512*19+(fViewerEventNum )%32] = (double) (((UInt_t)(record[4]&0x00FFFFFF) % 800));


       fViewerData3[512*20+0*32+(fViewerEventNum )%32]++ ;
       fViewerData3[512*20+1*32+(fViewerEventNum )%32] = (double)((UInt_t)(record[4]&0x00FFFFFF));
//       cout << (double)((UInt_t)(record[4]&0x00FFFFFF)) << endl;
       fViewerData3[512*20+2*32+(fViewerEventNum )%32] += (bool)(record[4] &  0x07000000); 
       fViewerData3[512*20+3*32+(fViewerEventNum )%32] +=  (bool)(record[4] &  0x18000000); 
       fViewerData3[512*20+4*32+(fViewerEventNum )%32] += (bool)(record[4] &  0x60000000); 
       fViewerData3[512*20+5*32+(fViewerEventNum )%32] += 1000*(bool)(record[5] &  0x02000000); 

       fViewerData4[512*19+(((UInt_t)(record[4]&0x00FFFFFF) )%512)] = (double) (((UInt_t)(record[4]&0x00FFFFFF) % 800));

    }
  else if (thisDataId == fPMTDataId) 
    {

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
              //PMT W1:
              recnum++;
              ORUtils::Swap(record[recnum]);
              // cout << "PMT W1: " <<  bitset<32>(record[recnum]) << endl;
              // cout << "PMT Slot: " << FEC_SL(record[recnum]) << endl;
              //PMT W2:
              recnum++;
              ORUtils::Swap(record[recnum]);
              // cout << "PMT W2: " <<  bitset<32>(record[recnum]) << endl;
              //PMT W3:
              recnum++;
              ORUtils::Swap(record[recnum]);
              // cout << "PMT W3: " <<  bitset<32>(record[recnum]) << endl;
              //}
              fViewerData3[(int)FEC_LCN(record[recnum-2])] = FEC_GT_16(record[recnum-2]); 
              fViewerData2[(int)FEC_LCN(record[recnum-2])]++;
            }
         
       }
     } 
  }
return kSuccess;
}



/*
	unsigned int thisDataId = fMTCDecoder.DataIdOf(record); // any long decoder would do the job
          cout <<"got a data record" << endl;


	if (thisDataId == fMTCDataId) {
          cout <<"mtc data" << endl;
//         Viewer::DataStore& events = DataStore::GetInstance();
            if(fViewerData1.size() > 52)
              {
              Viewer::DataStore::GetInstance().Add( record , fViewerData1, fViewerData2, fViewerData3, fViewerData4 ); 

            }

            if(fViewerData1.size() > 512*18)
              {
              Viewer::DataStore::GetInstance().Add( record , fViewerData1, fViewerData2, fViewerData3, fViewerData4 ); 
              fViewerData1.clear(); 
              fViewerData2.clear(); 
              fViewerData3.clear(); 
              fViewerData4.clear(); 

            }
       }
<comment>
		//ORVReader swaps the dataID only, swap the rest now
        ORDataProcessor::EReturnCode code = ORCompoundDataProcessor::ProcessDataRecord(record);
        if (code != kSuccess) return code;
		UInt_t n = fMTCDecoder.LengthOf(record) - 1;

        if (fRunStart && n >= 6) {
            long runNum = GetRunContext()->GetRunNumber();
            u_int32 firstGTID = GT( record + 1 );

            SNO_printf( 0, ORCA_FAC, "Run %ld START (%s, %s) GT %.6x\n", runNum,
                fRunStart & 0x2 ? "soft" : "hard",
                fRunStart & 0x4 ? "remote" : "local",
                (int)firstGTID);
//
// *** THIS IS A TEMPORARY HACK UNTIL WE GET THE PROPER HEADER RECORDS FROM ORCA! ***
//
            // generate a new run record
            struct {
                GenericRecordHeader grh;
                RunRecord           rec;
            } run;
            run.grh.RecordID = RUN_RECORD;
            run.grh.RecordLength = sizeof(RunRecord);
            run.grh.RecordVersion = 0; //TEST
            run.rec.Date = 0; //TEST
            run.rec.Time = 0; //TEST
            run.rec.DAQCodeVersion = 100; //TEST
            run.rec.RunNumber = GetRunContext()->GetRunNumber();
            //GetRunContext()->GetSubRunNumber();
            run.rec.CalibrationTrialNumber = 0; //TEST
            run.rec.SourceMask = 0; //TEST
            run.rec.GTCrateMsk = 0; //TEST
            run.rec.FirstGTID = firstGTID; //TEST
            run.rec.ValidGTID = firstGTID; //TEST
            memset(&run.rec.Spares, 0, sizeof(run.rec.Spares));

            // send the header to the builder thread
            while (fReader->AddHeader((tCBWord *)(&run), sizeof(run)/sizeof(tCBWord)) == BUFFER_OVERFLOW) {
                fReader->Waiting(kWaitHeader);
		        usleep(10);
	        }
            fReader->Waiting(0);

            fRunStart = 0;  // reset run start flag
        }

//printf("MTC %.6x (%d bytes)\n",(int)UNPK_MTC_GT_ID(record+1),(int)n);
		//the MTC word0 is record[1] ... MTC word5 is record[6]

        UInt_t t = record[0];
        // initialize header word for raw data circular buffer
        record[0] = MTC_DATA;           // identify MTC data
        while (fReader->AddData((tCBWord *)record, n + 1) == BUFFER_OVERFLOW) {
            fReader->Waiting(kWaitData);
            usleep(10);
        }
        fReader->Waiting(0);
        record[0] = t;                  // restore ORCA header word

	}
	else if (thisDataId == fPMTDataId) {
		//XL3 MegaBundle, number of PMT bundles it contains is:
		//unsigned int bundle_length = (fPMTDecoder.LengthOf(record) - 2) / 3;

		//the first PMT bundle is then word0 record[2], word1 record[3], word2 record[4]
		//all the PMT bundles are big endian as shipped by XL3		
		
		UInt_t n = fPMTDecoder.LengthOf(record) - 2;

		// swap bytes
		if (fMustSwap) SWAP_INT32(record+2, n);
		
//printf("PMT %d 0x%x\n",n,UNPK_FEC_GT_ID(record+2));
        UInt_t t = record[1];
        // initialize header word for raw data circular buffer
        record[1] = FEC_DATA;           // identify FEC data
        while (fReader->AddData((tCBWord *)(record + 1), n + 1) == BUFFER_OVERFLOW) {
            fReader->Waiting(kWaitData);
            usleep(10);
        }
        fReader->Waiting(0);
        record[1] = t;                  // restore ORCA header word
	}
	else if (thisDataId == fCaenDataId) {
        ORDataProcessor::EReturnCode code = ORCompoundDataProcessor::ProcessDataRecord(record);
        if (code != kSuccess) return code;

        // parse the data block and add the CAEN records separately to the queue
        UInt_t *pt = record + 2;    // point to start of first CAEN record
        UInt_t *last = record + fCaenDecoder.LengthOf(record);
        for (;;) {
            UInt_t n_words = UNPK_CAEN_WORD_COUNT(pt);
            if (!n_words) {
                SNO_printf( 0, ORCA_FAC, "Invalid CAEN data length (0 words)\n");
                break;
            }
            if (pt + n_words > last) {
                SNO_printf( 0, ORCA_FAC, "Invalid CAEN data length (%d words, but only %d remain)\n",
                           (int)n_words, (int)(last - pt));
                break;
            }
            // add CAEN record to the queue
            while (fReader->AddCaen((tCBWord *)pt, n_words) == BUFFER_OVERFLOW) {
                fReader->Waiting(kWaitCaen);
                usleep(10);
            }
            fReader->Waiting(0);
            // step to next CAEN record (all done if we reached the end of the data)
            if ((pt+=n_words) == last) break;
        }
	}
    else if (thisDataId == fRunId) {
        ORDataProcessor::EReturnCode code = ORCompoundDataProcessor::ProcessDataRecord(record);
        if (code != kSuccess) return code;
        long runNum = GetRunContext()->GetRunNumber();
        if (record[1] & 0x8) {
            SNO_printf( 7, ORCA_FAC, "Run %ld heartbeat\n", runNum);
        }
        else if (record[1] & 0x1) { // Run start
            fRunStart = record[1];
        }
        else {
            SNO_printf( 0, ORCA_FAC, "Run %ld END (%s, %s)\n", runNum,
                record[1] & 0x2 ? "soft" : "hard",
                record[1] & 0x4 ? "remote" : "local");
//
// *** THIS IS A TEMPORARY HACK UNTIL WE GET THE PROPER HEADER RECORDS FROM ORCA! ***
//
            if (!(record[1] & 0x2)) { // hard end?
              // insert end-run marker into data stream
              tCBWord rec = END_RUN_MARKER;
              while (fReader->AddData(&rec, 1) == BUFFER_OVERFLOW) { // (in data stream!)
                  fReader->Waiting(kWaitData);
                  usleep(10);
              }
              fReader->Waiting(0);
            }
        }
    }
</comment>
 else if (thisDataId == fPMTDataId) {
		//XL3 MegaBundle, number of PMT bundles it contains is:
		//unsigned int bundle_length = (fPMTDecoder.LengthOf(record) - 2) / 3;

		//the first PMT bundle is then word0 record[2], word1 record[3], word2 record[4]
		//all the PMT bundles are big endian as shipped by XL3		
		
		UInt_t n = fPMTDecoder.LengthOf(record) - 2;

		// swap bytes
		if (fMustSwap) SWAP_INT32(record+2, n);
                //SWAP_INT32(record+2, n);
//                cout << "Got an XL3 MegaBundle with " << (fPMTDecoder.LengthOf(record) - 2) / 3 << " PMT bundles" << endl << "Header Word 1: " << bitset<32>(record[0]) << endl << "Header Word 2: " << bitset<32>(record[1]) << endl;		
                //cout <<"GTID:::::" << (unsigned int)((record[2])&0x0000FFFF) << endl;
//                cout <<"GTID:::::" << (double)(unsigned int)( record[2]&0x0000FFFF  | ( record[2+2] <<4 ) & 0x000F0000   | ( record[2+2] >> 8 ) & 0x00F00000   ) << endl;
                cout <<"-----------------------------------------" << endl;

//                cout << bitset<32>((u_int32)record[0])<< endl ;
//                cout << bitset<32>((u_int32)record[1])<< endl ;
//                UInt_t iCrate = fPMTDecoder.CrateOf(record);
//                UInt_t iCard = fPMTDecoder.CardOf(record);
//                UInt_t iCrate = (record[1] & 0x01e00000) >> 21;
                UInt_t iCard = (record[1] & 0x001f0000) >> 16;
                  UInt_t iCrate = (record[2] & 0x000001F0) >> 4;

                cout << "Got PMT data of length " << (fPMTDecoder.LengthOf(record)) / 3 << " from crate " << iCrate << " slot " << iCard << " around GTID " <<  (int)((record[1]&0xFFFF0000) >> 16) << endl;

                  cout << bitset<32>((u_int32)record[0])<<  ":"<<(UInt_t) ((record[0]&0xFFFF0000) >> 16)<<endl;
                  cout << bitset<32>((u_int32)record[1])<< ":"<<(UInt_t) ((record[1]&0xFFFF0000) >> 16)<<endl;

                for(int iPMT=3;iPMT<=n;iPMT+=3) {
                  unsigned int gtid8 = (record[iPMT+2] >> 24)&0x000000F0 |(record[iPMT+2] >> 12)&0x0000000F  ;
                  unsigned int gtid16 = (record[1]&0xFFFF0000) >> 16;
                  unsigned int gtid24 = gtid16|gtid8;
 //                  cout <<"::" << gtid16 << "::" << gtid8 << "::" << gtid24<< endl;
                //  cout << "a:b " << gtida<<":"<<gtidb<<endl;
//                  cout <<"-----------------------------------------" << endl;
//                  cout <<"GTID: "<< record[2] & 0x0000FFFF<<endl;
//                  cout <<"GTID: "<< gtid<<endl;
                  cout << bitset<32>((u_int32)record[iPMT])<< ":"<<(UInt_t) ((record[iPMT]&0xFFFF0000) >> 16) << endl ;
                  //cout << bitset<32>((uint32_t)record[iPMT+1])<< endl ;
//                  cout << bitset<32>((uint32_t)record[iPMT+2]) << endl;

                  fViewerData1.push_back((double)(unsigned int)((record[iPMT])&0x0000FFFF)) ;
                  fViewerData2.push_back(0) ;

                  fViewerData2.push_back(0);//(double)(unsigned int)((record[iPMT])&0x0000FFFF)) ;
                  fViewerData3.push_back((double)(unsigned int)((record[iPMT])&0xFFFF0000)) ;
                  fViewerData4.resize(9728);
//                  cout <<"crate/card/channel: " << (unsigned int)(record[iPMT]&0x03E00000) << "/" << (unsigned int)(record[iPMT]&0x3C000000) << "/" << (unsigned int)(record[iPMT]&0x001F0000) << endl;
                  fViewerData4[512*(int)iCrate+32*(int)iCard+10]=((double)523) ;
//                  fViewerData4[(int)(23)]=((double)record[iPMT+1]) ;


                  }
//printf("PMT %d 0x%x\n",n,UNPK_FEC_GT_ID(record+2));
//        UInt_t t = record[1];
        // initialize header word for raw data circular buffer
//        record[1] = FEC_DATA;           // identify FEC data
//        while (fReader->AddData((tCBWord *)(record + 1), n + 1) == BUFFER_OVERFLOW) {
//            fReader->Waiting(kWaitData);
//            usleep(10);
//        }
//        fReader->Waiting(0);
//        record[1] = t;                  // restore ORCA header word
	}
else {
       // SNO_printf( 7, ORCA_FAC, "Unhandled record ID=0x%x size=%d\n",
        //    (int)thisDataId, (int)fMTCDecoder.LengthOf(record));
        cout << "unhandled record received"<<endl;
    }
	return kSuccess;
}
*/
