// Not all libraries below are required
#include <iostream>
#include <fstream>
#include <cstdlib>

#include <TROOT.h>
#include <TCanvas.h>
#include <TApplication.h>
#include <TString.h>

#include "Garfield/GeometrySimple.hh"
#include "Garfield/SolidTube.hh"
#include "Garfield/SolidBox.hh"

#include "Garfield/MediumMagboltz.hh"

#include "Garfield/ComponentAnalyticField.hh"
#include "Garfield/Sensor.hh"
#include "Garfield/AvalancheMicroscopic.hh"

using namespace Garfield;
using namespace std;

/*******************************************/
/******************* Main ******************/
/*******************************************/

int main(int argc, char * argv[]) {

  // Required for visualisations to work
  TApplication app("app", &argc, argv);
  
  /*******************************************/
  /******************* Setup *****************/
  /*******************************************/
  
  // The gas mixture can be that used for the x-ray tests or that used in the final detector
  bool xrayTest = 0;  // If 0 then the real gas mix is used, else the ArCO2 mix for the xray vessel
  
  // The high voltage used for the anodes and the entrance window
  double vAnodeWire = 2100;
  double vWindow = -1000;  


  /*******************************************/
  /******************** Gas ******************/
  /*******************************************/


  // Define a new gas mixture
  MediumMagboltz* gas = new MediumMagboltz();  
      
  // Define the makeup of the gas
  if (xrayTest){
    gas->SetComposition("Ar", 80. , "CO2", 20.);
    
    // Define the pressure [Torr] and temperature [K] of the gas
    gas->SetPressure(1*760.0);
    gas->SetTemperature(293.15);
  }
  
  else{
    gas->SetComposition("he3", 81.25 , "cf4", 18.75);
    
    // Define the pressure [Torr] and temperature [K] of the gas
    gas->SetPressure(7.5*760.0);
    gas->SetTemperature(293.15);
  }

  // Increase the maximum electron energy (default is 40 eV)
  gas->SetMaxElectronEnergy(150);


  /*******************************************/
  /***************** Geometry ****************/
  /*******************************************/ 
  
  // Two infinite planes in y with two layers of wires between them
  // Cathode stripes are at y = 0
  // Entrance window at y = yWindow 

  double yCathodePad = 0.0; // Position of cathode pads

  double yAnodeWire = 0.16; // Anode wire position relative to cathode pad plane in cm
  double yCathodeWire = 0.32; // Cathode wire position relative to cathode pad plane in cm
  double yWindow = 1.637; // Window position relative to cathode pad plane in cm

  double dAnodeWire = 15e-4; // Anode wire diameter in cm (radius = 15/2 um)
  double dCathodeWire = 50e-4; // Cathode wire diameter in cm (radius = 50/2 um)

  double wirePitch = 0.16; // Pitch between adjacent anode/cathode wires in cm

  int nWires = 10; // The number of anode/cathode wires - this should be replaced with periodicity unless the boundary effects are required


  // World box - centred on x = 0, stretches to accommodate all wire pairs
  
  double worldSizeX = wirePitch * nWires; // This leaves 1/2 wire pitch free on both edges
  double worldXmin = -(worldSizeX/2.);
  double worldXmax = (worldSizeX/2.);
  double worldYmin = 0.;
  double worldYmax = yWindow;
  double worldSizeY = yWindow;
  double worldSizeZ = 1.0; // This shouldn't affect the results
  
  SolidBox* box = new SolidBox(0,             // World centre in x
                               yWindow/2.,    // y
                               0.,            // cz
                               worldSizeX/2., // lx half length in x
                               worldSizeY/2., // ly
                               worldSizeZ/2.  // lz
  );  
  
  // Build the geometry
  GeometrySimple *geo = new GeometrySimple();
  
  geo->AddSolid(box, // The world solid
               gas); // The medium filling the solid


  /*******************************************/
  /**************** Components ***************/
  /*******************************************/

  // Make a component and associate it with the geometry above
  ComponentAnalyticField *cmp = new ComponentAnalyticField();
  cmp->SetGeometry(geo);


  // Add the cathode pads as a single plane
  cmp->AddPlaneY(0,             // Y position of the plane
                 0,             // Voltage of the plane
                 "cathodePads"  // Name of the component
  );

  // Add the entrance window as a plane
  cmp->AddPlaneY(yWindow,       // Y position of the plane
                 vWindow,       // Voltage of the plane
                 "window"       // Name of the component
  );

  // Add wires (add one anode and one cathode per loop)
  for (int wireNum = 0; wireNum < nWires; wireNum++){

    // Position of current wire pair in x (careful with integer division here)
    double xWire = (wireNum - ((nWires - 1)/2.) ) * wirePitch;

    // Anode wires, labelled as a0, a1 etc.
    string anodeName = string(Form("a%d", wireNum));
    cmp->AddWire(xWire,       // x coord
               yAnodeWire,    // y coord
               dAnodeWire,    // wire diameter
               vAnodeWire,    // wire voltage
               anodeName     // label
    );

    // Cathode wires, labelled as c0, c1 etc.
    string cathodeName = string(Form("c%d", wireNum));
    cmp->AddWire(xWire,       // x coord
               yCathodeWire,  // y coord
               dCathodeWire,  // wire diameter
               0,             // wire voltage
               cathodeName   // label
    );

  }


  /*******************************************/
  /************* Drift electrons *************/
  /*******************************************/

  AvalancheMicroscopic *avalMicro = new AvalancheMicroscopic();
  
  // Create a sensor and attach the component and AvalancheMicroscopic
  Sensor *sensor = new Sensor();
  sensor->AddComponent(cmp);
  avalMicro->SetSensor(sensor);


  // The number of electrons, holes and ions resulting from each avalanche
  int ne, nh, ni;
  // Sum over all events
  int neSum = 0;

  int nLines = 1e4;
  for (int i = 0; i<nLines; i++){
    // Create electrons above the central electrode
    avalMicro->AvalancheElectron(0.08,    // x0
                                 0.4,  // y0
                                 0,    // z0
                                 0,    // t0
                                 0     // e0
                                 );

    avalMicro->GetAvalancheSize(ne, nh, ni);
    //cout << "ne: " << ne << " nh: " << nh << " ni: " << ni << endl;
    neSum += ne;
  }

  cout << "Average avalanche size: " << double(neSum/double(nLines)) << endl;

  // This will keep the program running - need to kill it manually from terminal
  app.Run(kTRUE);

}
