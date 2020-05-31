//
//  TouchBoardInput.cpp
//
//
//  Created by Pierre Klintefors.
//

#include "TouchBoardInput.h"


//#include "../../Kernel/IKAROS_Serial.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <array>


using namespace ikaros;
using namespace std;


TouchBoardInput::~TouchBoardInput()
{
    s->Close();
}

void
TouchBoardInput::Init()
{
    s = new Serial(GetValue("port"), 57600);
    s->Flush();
    rcvmsg = new char [13];
    output = GetOutputArray("OUTPUT1");
}

void
TouchBoardInput::Tick() {
  if(!s)
    return; // Not connected to board

  int rate = 13;
  int timeMs = 10;
  //Receives sended char from serial
  s->ReceiveBytes(rcvmsg, rate, timeMs);

//  std::cout << rcvmsg << '\n';

  std::string numbers;
  std::string active ("1");
  std::stringstream stream;

  //converts char to string
  stream <<rcvmsg;
  stream >> numbers;

  //Empties GetOutputArray
  std::fill_n(output, numbers.length(), 0);

  //rsvmsg contains an array of 1 and 0. The following code will search for every occurance of 1 in "numbers"
  //that is a converted string of that array.

  //create a vector to store positions of active electrodes
  std::vector<size_t> vec;

  // Get the first occurrence
  size_t pos = numbers.find(active);

  //Mark electrode as active in the output
  output[pos]= 1;

  // Repeat till end is reached
  while( pos != std::string::npos)
  {
    // Add position to the vector
    vec.push_back(pos);

    // Get the next occurrence from the current position
    pos =numbers.find(active, pos + active.size());

    //Mark the rest of active electrodes in the output
    output[pos]= 1;

 }//While()






}//Tick()


static InitClass init("TouchBoardInput", &TouchBoardInput::Create, "Source/UserModules/TouchBoardInput/");
