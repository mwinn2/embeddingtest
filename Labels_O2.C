#include <fstream>      // std::ifstream 
#include <iostream>
using namespace std;


#include "TFile.h"
#include "TTree.h"
#include "TChain.h"

#include "MCHSimulation/Geometry.h"
#include "MCHSimulation/GeometryTest.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"
#include "MCHSimulation/Digit.h"
#include "MCHMappingInterface/Segmentation.h"
#include "TGeoManager.h"

using namespace o2::mch;



void retrieveDigits(TTree* chain,
                  const char* brname,
                  //                  int sourceID,                                                                                                                                                        
                  int entryID,
		    std::vector<Digit>* digits)
{
  auto br = chain->GetBranch(brname);
  if (!br) {
    return;
  }
  br->SetAddress(&digits);
  br->GetEntry(entryID);
}

/*void retrieveMCTruthContainer(TTree* chain,
                  const char* brname,
                  int entryID,
			      o2::dataformats::MCTruthContainer* labels)
{
  auto br = chain->GetBranch(brname);
  if (!br) {
    cout << "No branch found" << endl;
    return;
  }
  br->SetAddress(&labels);
  br->GetEntry(entryID);
  }*/


Long64_t retrieveEntries(TTree* chain,
                  const char* brname){

 auto br = chain->GetBranch(brname);
  if (!br) {
    cout << "No branch found" << endl;
    return 0;
  }
  return br->GetEntries();


}



struct GEOMETRY {
  GEOMETRY()
  {
    if (!gGeoManager) {
      o2::mch::test::createStandaloneGeometry();
    }
  }
};



std::map<int, int> createDEMap()
{
  std::map<int, int> m;
  int i{ 0 };
  o2::mch::mapping::forEachDetectionElement([&m, &i](int deid) {
    m[deid] = i++;
  });
  return m;
}

int deId2deIndex(int detElemId)
{
  static std::map<int, int> m = createDEMap();
  return m[detElemId];
}

std::vector<o2::mch::mapping::Segmentation> createSegmentations()
{
  std::vector<o2::mch::mapping::Segmentation> segs;
  o2::mch::mapping::forEachDetectionElement([&segs](int deid) {
    segs.emplace_back(deid);
  });
  return segs;
}

const o2::mch::mapping::Segmentation& segmentation(int detElemId)
{
  static auto segs = createSegmentations();
  return segs[deId2deIndex(detElemId)];
}


void Labels_O2(){

  GEOMETRY();

  
  TFile* filo2 = TFile::Open("mchdigits.root");
  TTree * alio2 = (TTree*)filo2->Get("o2sim");  
  
  std::vector<Digit> digits;
  // cout <<"digits "<< digits << endl;
  //o2::dataformats::MCTruthContainer container; //dataformats
  o2::dataformats::MCTruthContainer<o2::MCCompLabel>* container = nullptr;//new o2::dataformats::MCTruthContainer<o2::MCCompLabel>();
  
  //  cout <<"container " << container << endl;
  alio2->SetBranchAddress("MCHMCLabels", &container);
  alio2->GetEntry(0);
  

  const auto& labels = container->getLabels(1);
  //  o2::MCCompLabel labtest = labels[0];

  //  cout <<"test 4never " << endl;
  
  TFile* o2file = TFile::Open("o2file_2ndlabel.root","recreate");
  
  int   padid;
  double  adc;
  double time;
  int detID_out;
  int detID_out2;
  int station;
  double padx;
  double pady;
  bool bending;
  
  
  int   padid_int;
  double  adc_int;
  double time_int;
  int detID_int;
  int station_int;
  double padx_int;
  double pady_int;
  int eventid;
  int eventid2 = -1;
  int srcid;
  int srcid2;
  int trackid;
  int trackid2 = -1;
  
  TTree o2out("o2out","blable");

  o2out.Branch("O2padid", &padid);
  o2out.Branch("O2adc", &adc);
  o2out.Branch("O2time", &time);
  o2out.Branch("O2detID",&detID_out);
  o2out.Branch("O2station",&station);
  o2out.Branch("O2padx", &padx_int);
  o2out.Branch("O2pady", &pady_int);
  o2out.Branch("bending", &bending);
  o2out.Branch("eventid", &eventid);
  o2out.Branch("eventid2", &eventid2);
  o2out.Branch("trackid", &trackid);
  o2out.Branch("trackid2", &trackid2);
  o2out.Branch("srcid", &srcid);
  o2out.Branch("srcid2",&srcid2);
  
  Long64_t nentries = retrieveEntries(alio2, (const char*) "MCHDigit");
  
  // alio2->GetEntry(0);
  // cout <<"alio2->GetEntry(0) " << alio2->GetEntry(0) << endl;
  // cout <<"digits " << digits->size() << endl;
  //  int size = digits->size();
  for(int j =0; j< nentries; ++j ){

    
    retrieveDigits(alio2, (const char*) "MCHDigit", j, &digits);

    //    retrieveMCTruthContainer(alio2, (const char*) "MCHMCLabels", j, &container);
    
    
    int size = digits.size();
    //   int sizelab = container.getSize();
    //    cout <<"size Truth COntainer " << sizelab << endl;
    for(int i = 0; i< size; ++i) {
      //TODO add event-id, not there, time stamp, check what I wrote inside...as time
      //get cathode type per digit! via Segmentation: isBendingPad bool Ausgabetyp!!
      //  auto time = event_time + hit.GetTime(); hit input distribution time seems to be a constant
      //i.e. in order to combine one "cluster" need to look for all digits having the same time stamp, belonging to the same station and to the same cathode type

      const auto&  labels_for_i = container->getLabels(i);
      o2::MCCompLabel lab = labels_for_i[0];

      //cout << "&lab " << &lab << endl;
      //cout <<"labels_for_i " << labels_for_i  << endl;
      int evtid = lab.getEventID();
      int track_id = lab.getTrackID();
      int src_id = lab.getSourceID();
      int src_id2 = -1000;
      int size = labels_for_i.size();
      if(size>1){
	cout <<"TEST more than one lable " << endl;
	o2::MCCompLabel lab2 = labels_for_i[1];
	eventid2= lab2.getEventID();
	trackid2 = lab2.getTrackID();
	src_id2 = lab2.getSourceID();
      }
      //TODO also write trackID!
      padid_int = digits.at(i).getPadID();
      adc_int   = digits.at(i).getADC();
      time_int  = digits.at(i).getTimeStamp();
      detID_int = digits.at(i).getDetID();//to be implemented in Digitizer
      auto& seg = segmentation(detID_int);//detID to be exchanged with detID_int

      bool isbending = seg.isBendingPad(padid_int);
      
      auto t = o2::mch::getTransformation(detID_int, *gGeoManager);
      
      Point3D<float> gpos;
      Point3D<float> lpos(seg.padPositionX(padid_int), seg.padPositionY(padid_int), 0.0);
      t.LocalToMaster(lpos, gpos);
      padx_int = gpos.X();
      pady_int = gpos.Y();
      
    
      if(detID_int<300){
	station_int = 1;
      }else if(detID_int<500){
	station_int = 2;
      }else if(detID_int<700){
	station_int = 3;
      }else if(detID_int<900){
	station_int = 4;
      }else{
	station_int =5;
      }
      
      
      
      eventid = evtid;
      trackid = track_id;
      padid = padid_int;
      adc   = adc_int;
      time  = time_int;
      detID_out = detID_int;
      padx = padx_int;
      pady = pady_int;
      station = station_int;
      bending = isbending;
      srcid = src_id;
      srcid2 = src_id2;
      
      o2out.Fill();
    }

  }
     o2out.SetDirectory(o2file);
     o2out.Write();
     o2file->Close();
}

