//
//  TouchClassifier.cpp
//
//
//
//

#include "TouchClassifier.h"


//#include "../../Kernel/IKAROS_Serial.h"

#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <array>


using namespace ikaros;
using namespace std;



TouchClassifier::~TouchClassifier()
{
    // Destroy data structures that you allocated in Init.
    destroy_matrix(internal_matrix);

}


void
TouchClassifier::Init()
{


    // Get pointer to a matrix and treat it as a matrix. If an array is
    // connected to this input, size_y will be 1.

    input_array = GetInputArray("INPUT1");
    input_array_size = GetInputSize("INPUT1");



    classifier_matrix =GetOutputMatrix("OUTPUT1");
    classifier_matrix_size_y = GetOutputSizeY("OUTPUT1");
    classifier_matrix_size_x = GetOutputSizeX("OUTPUT1");

    output_matrix = GetOutputMatrix("OUTPUT2");
    output_matrix_size_y = GetOutputSizeY("OUTPUT2");
    output_matrix_size_x = GetOutputSizeX("OUTPUT2");

    internal_matrix_size_y = output_matrix_size_y;

    //Create an internal matrix to perform calculations with
    internal_matrix = create_matrix(input_array_size, internal_matrix_size_y);

    int internal_matrix_row =  0; // The row count, determining whcih rows thats going to be filled in internal_matrix
}


void
TouchClassifier::Tick()
{

  //Filling the row of internal_matrix with same values as the input array
  for (int i=0; i<input_array_size; i++)
      internal_matrix[internal_matrix_row][i] = input_array[i];



  //If the internal_matrix is not filled
  if(internal_matrix_row < internal_matrix_size_y)
    internal_matrix_row++; //After this tick next row of internal_matrix will be filled.


  //When the internal matrix calculations are performed on the matrix
  if(internal_matrix_row == internal_matrix_size_y)
  {
    //Printing the internal_matrix
    std::cout << "This is internal_matrix" << '\n';
    for(int i= 0; i < internal_matrix_size_y; i ++)
    {
      std::cout << i;
      std::cout << ":  ";
      for(int j = 0; j < input_array_size; j++)
      std::cout << internal_matrix[i][j];
      std::cout << '\n';
    }

    std::cout << "End of internal matrix" << '\n';
    std::cout << '\n';

    //Copying the values of the internal_matrix into the output_matrix
    for (int j=0; j<output_matrix_size_y; j++)
    {
      for (int i=0; i<output_matrix_size_x; i++)
          output_matrix[j][i] = internal_matrix[j][i];
    }

    //create a pointer array to get meanarray from the function MatrixMean().
    float* meanMatrixCol;

    //Create a array to store the values
    float fullMatrixMeanArray[input_array_size];
    meanMatrixCol = MatrixMean(0, internal_matrix_size_y );// (MatrixMean(int start, int end))
    //Copy values of the pointer
    copy_array(fullMatrixMeanArray, meanMatrixCol, input_array_size);

    // //Create an array to store each column of first half of matrix
    float meanFirstHalfOfMatrixCol[input_array_size];
    meanMatrixCol = MatrixMean(0, (internal_matrix_size_y / 2) );//  (MatrixMean(int start, int end))
    //Creates a copy array to save value of the pointer
    copy_array(meanFirstHalfOfMatrixCol, meanMatrixCol, input_array_size);

    //Create an array to store each column of second half of matrix
    float meanSecondHalfOfMatrixCol[input_array_size];
    meanMatrixCol = MatrixMean( (internal_matrix_size_y / 2), internal_matrix_size_y);//  (MatrixMean(int start, int end))
    //Creates a copy array to save value of the pointer
    copy_array(meanSecondHalfOfMatrixCol, meanMatrixCol, input_array_size);


    //Difference between first and second half
    float meanMatrixDiff[input_array_size];
    for(int i = 0; i < input_array_size; i++)
      meanMatrixDiff[i] = meanFirstHalfOfMatrixCol[i] - meanSecondHalfOfMatrixCol[i];






    // Classification of touch- description of rows
    // Row 0 - Binary array of active electrodes
    // Row 1 - Mean of number of ticks where electrodes where active- higher number indicates longer touch
    // Row 2 - Difference in mean between first half of the row and second half of the rows.
    // Row 3 - Number of new touches per field
    // Row 4 - Strokes. If a stroke is detected. The sequential order of activation will be displayed on this row.
    //        Maximum 3 fields can be involved in the sam stroke and will get values 1, 2 and 3.

    int numberOfTouch;
    int numberOfTouchPerCol[input_array_size];

    for(int col = 0; col < input_array_size; col++)
    {
      numberOfTouch = 0; //resets sum with every initiated column

      for(int row = 0; row < internal_matrix_size_y; row++)
      {
        if(row < internal_matrix_size_y-1 && internal_matrix[row][col] == 1 && internal_matrix[row+1][col] == 0)
        {
          numberOfTouch ++;
        }

        numberOfTouchPerCol[col]=numberOfTouch;

      }// for row

    }// for col

    //Identifies strokes
    float * strokeArray;
    strokeArray = CheckingForStrokes();




    //Filling Row 1, Row 2 and Row 3 of classifier_matrix
    for(int i = 0; i < classifier_matrix_size_x; i++)
    {
      if(fullMatrixMeanArray[i] > 0)
      {
        classifier_matrix[0][i] = 1; // The electrodes that has been active in the internal matrix
      }
      classifier_matrix[1][i] = fullMatrixMeanArray[i]; //Mean of number of active ticks
      classifier_matrix[2][i] = meanMatrixDiff[i]; //Differences in mean between first and second half.
      classifier_matrix[3][i] = numberOfTouchPerCol[i]; //Number of touches for every electrode
      classifier_matrix[4][i] = strokeArray[i]; //Sequence
    }




    //Print classifier_matrix
    std::cout << "Classification of touch - description of rows"<< '\n';
    std::cout << "Row 0 - Binary array of active electrodes" << '\n';
    std::cout << "Row 1 - Mean of number of ticks where electrodes where active- higher number indicates longer touch" << '\n';
    std::cout << "Row 2 - Difference in mean between first half of the row and second half of the rows." << '\n';
    std::cout << "Row 3 - Number of new touches per field" << '\n';
    std::cout << "Row 4 - Strokes. If a stroke is detected. The sequential order of activation will be displayed on this row." << '\n';
    std::cout << "        Maximum 3 fields can be involved in the sam stroke and will get values 1, 2 and 3." << '\n';


    std::cout  << '\n';
    std::cout << "This is classifier_matrix" << '\n';
    for(int i= 0; i < classifier_matrix_size_y; i ++)
    {
      for(int j = 0; j < classifier_matrix_size_x; j++)
        std::cout << classifier_matrix[i][j] << ' ';

      std::cout << '\n';
    }
    std::cout << "End of classifier_matrix " << '\n';
    std::cout << '\n';



    //Resets row count and creates new internal matrix.
    internal_matrix_row = 0; //resets the row count so the internal matrix can be overwritten
    std::cout << '\n';
    std::cout << '\n';
    std::cout << "Row count is now 0 again" << '\n';
    std::cout << '\n';
    destroy_matrix(internal_matrix);
    internal_matrix = create_matrix(input_array_size, internal_matrix_size_y); //Creates a new internal matrix
    for(int i = 0; i < classifier_matrix_size_x; i++)
    {
      classifier_matrix[0][i] = 0; //Resets first row of 'active' electrodes in classifier_matrix
    }


  }// end of if (internal_matrix_row == internal_matrix_size_y)


}//Tick()


////Function that calculates mean of matrix.
float*
TouchClassifier::MatrixMean(int startPoint, int endPoint)
{
  //create an array to store  mean of each column of first half of the rows internal matrix.
  static float meanMatrixArray[12];

  float sum;
  for(int col = 0; col < input_array_size; col++)
  {
    sum = 0; //resets sum after every finsihed colum

    for(int row = startPoint; row < endPoint; row++)
    {
      sum += internal_matrix[row][col];
    }

    //Clalculate mean and round it to two digits after decimal point
    float value = (int)( (sum / (endPoint-startPoint) )  * 100 + .5);
    meanMatrixArray[col]= (float)value / 100;
  }

  return meanMatrixArray;
}

//This function checks if the electrodes has been activated in a sequential manner that resembles a stroke.
//Every field/electrode has 3-5 possible directions where activations
float*
TouchClassifier::CheckingForStrokes()
{

  //create an array to store positions of electrodes involves in stroke as well as sequence of the stroke/activation of electrodes .
  static float strokeArray[12];

  for(int i = 0; i < classifier_matrix_size_x; i++)
  {
    strokeArray[i] = 0; //Resets strokeArray
  }


//Integers for evey field where the value corresponds with the connected electrode
  int FrontTopLeftCorner=0;
  int FrontBottomLeftCorner=1;
  int BackLeftSide = 2;
  int Mouth = 3;
  int Forehead = 4;
  int FrontBottom = 5;
  int FrontTop = 6;
  int BackTop = 7;
  int Back = 8;
  int BackRightSide = 9;
  int FrontBottomRightCorner=10;
  int FrontTopRightCorner=11;



  float sequence = 0; //int to represent sequence of activastion of electrodes involved in stroke
  int delay = 20; //number of ticks berfore checking for second field involved in stroke

//Nested for-loop searching the matrix for an active electrode that has become inactive.
  for(int col = 0; col < classifier_matrix_size_x; col++)
  {
    for(int row = 0; row < internal_matrix_size_y; row++)
    {
      if(row < internal_matrix_size_y-1 && internal_matrix[row][col] == 1 && internal_matrix[row+1][col] == 0) //when active electrode becomes inactive
      {
        std::cout << "Touch ended ";
        std::cout << "Colum: ";
        std::cout << col;
        std::cout << ", Row: ";
        std::cout << row << '\n';
        std::cout << '\n';

        //Switch statement testing condotions in terms of sequential activations of adjacent fields to the active electrode that has become inactive (col)
        switch (col)
        {
          case 0: //FrontTopLeftCorner (Direction 1: right upwards; Direction 2: left downwards; Direction 3: to back; Direction 4: to front)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTop]==1) // Direction 1, to right over top
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTop]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTopRightCorner]==1 && internal_matrix[currentrow][FrontTop]==0)
                  strokeArray[FrontTopRightCorner]= sequence + 2;
              }

               break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomLeftCorner]==1) //stroke in second direction, down to left
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottomLeftCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottom]==1 && internal_matrix[currentrow][FrontBottomLeftCorner]==0)
                  strokeArray[FrontBottom]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackLeftSide]==1) //stroke in the third direction (to back)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackLeftSide]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Back]==1 && internal_matrix[currentrow][BackLeftSide]==0)
                  strokeArray[Back]= sequence + 2;
              }


              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Forehead]==1) //stroke in the fourth direction (to Forehead)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Forehead]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTopRightCorner]==1 && internal_matrix[currentrow][Forehead]==0)
                  strokeArray[FrontTopRightCorner]= sequence + 2;
              }


              break;
            }

          break;

          case 1: //Front bottom left corner (Direction 1: upwards; Direction 2: downwards; Direction 3: to back; Direction 4: to front)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopLeftCorner]==1) // Direction 1, upwards to top
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTopLeftCorner]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTop]==1 && internal_matrix[currentrow][FrontTopLeftCorner]==0)
                  strokeArray[FrontTop]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottom]==1) //stroke in second direction, down to bottom and right side
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottom]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottomRightCorner]==1 && internal_matrix[currentrow][FrontBottom]==0)
                  strokeArray[FrontTopRightCorner]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackLeftSide]==1) //stroke in the third direction (to back)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackLeftSide]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Back]==1 && internal_matrix[currentrow][BackLeftSide]==0)
                  strokeArray[Back]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Mouth]==1) //stroke in the fourth direction (to Forehead)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Mouth]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottomRightCorner]==1 && internal_matrix[currentrow][Mouth]==0)
                  strokeArray[FrontBottomRightCorner]= sequence + 2;
              }

              break;
            }
          break;

          case 2://Back left side (Direction 1: upwards to right side; Direction 2: to back right side; Direction 3: to front upper side; Direction 4: to front lower side)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackTop]==1) // Direction 1, upwards to top and right
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackTop]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][BackRightSide]==1 && internal_matrix[currentrow][BackTop]==0)
                  strokeArray[BackRightSide]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Back]==1) //stroke in second direction, to back right side
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Back]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][BackRightSide]==1 && internal_matrix[currentrow][Back]==0)
                  strokeArray[BackRightSide]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopLeftCorner]==1) //stroke in the third direction (front to Forehead)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTopLeftCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Forehead]==1 && internal_matrix[currentrow][FrontTopLeftCorner]==0)
                  strokeArray[Forehead]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomLeftCorner]==1) //stroke in the fourth direction (front to mouth)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottomLeftCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Mouth]==1 && internal_matrix[currentrow][FrontBottomLeftCorner]==0)
                  strokeArray[Mouth]= sequence + 2;
              }

              break;
            }
          break;

          case 3: // Mouth (Direction 1: upwards; Direction 2: downwards; Direction 3: to back lef side; Direction 4: to back right side)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Forehead]==1) // Direction 1, upwards to top
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Forehead]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTop]==1 && internal_matrix[currentrow][Forehead]==0)
                  strokeArray[FrontTop]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottom]==1) //stroke in second direction, down to bottom and right side
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottom]=  sequence + 1;

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomLeftCorner]==1) //stroke in the third direction (to back left side)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottomLeftCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][BackLeftSide]==1 && internal_matrix[currentrow][FrontBottomLeftCorner]==0)
                  strokeArray[BackLeftSide]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomRightCorner]==1) //stroke in the fourth direction (to back right side)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottomRightCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][BackRightSide]==1 && internal_matrix[currentrow][FrontBottomRightCorner]==0)
                  strokeArray[BackRightSide]= sequence + 2;
              }

              break;
            }
          break;

          case 4:// Forehead (Direction 1: upwards; Direction 2: downwards; Direction 3: to back lef side; Direction 4: to back right side)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTop]==1) // Direction 1, upwards to back top
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTop]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][BackTop]==1 && internal_matrix[currentrow][FrontTop]==0)
                  strokeArray[BackTop]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Mouth]==1) //stroke in second direction, down to bottom
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Mouth]=  sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottom]==1 && internal_matrix[currentrow][Mouth]==0)
                  strokeArray[FrontBottom]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopLeftCorner]==1) //stroke in the third direction (to back left side)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTopLeftCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][BackLeftSide]==1 && internal_matrix[currentrow][FrontTopLeftCorner]==0)
                  strokeArray[BackLeftSide]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopRightCorner]==1) //stroke in the fourth direction (to back right side)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTopRightCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][BackRightSide]==1 && internal_matrix[currentrow][FrontTopRightCorner]==0)
                  strokeArray[BackRightSide]= sequence + 2;
              }

              break;
            }
          break;

          case 5://Front bottom (Direction 1: upwards front;  Direction 2:  upwards left side; Direction 3: upwards right side)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Mouth]==1) // Direction 1, upwards to Forehead
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Mouth]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Forehead]==1 && internal_matrix[currentrow][Mouth]==0)
                  strokeArray[Forehead]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomLeftCorner]==1) //stroke in the second direction (up the left side)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottomLeftCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTopLeftCorner]==1 && internal_matrix[currentrow][FrontBottomLeftCorner]==0)
                  strokeArray[FrontTopLeftCorner]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomRightCorner]==1) //stroke in the third direction (up the right side)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottomRightCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTopRightCorner]==1 && internal_matrix[currentrow][FrontBottomRightCorner]==0)
                  strokeArray[FrontTopRightCorner]= sequence + 2;
              }

              break;
            }
          break;

          case 6: //Front top (Direction 1: down left;  Direction 2:  down right side; Direction 3: down the back; Direction 4: down the front)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopLeftCorner]==1) //Direction 1, down the left
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTopLeftCorner]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottomLeftCorner]==1 && internal_matrix[currentrow][FrontTopLeftCorner]==0)
                  strokeArray[FrontBottomLeftCorner]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopRightCorner]==1) //stroke in second direction, down to right
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTopRightCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottomRightCorner]==1 && internal_matrix[currentrow][FrontTopRightCorner]==0)
                  strokeArray[FrontBottomRightCorner]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackTop]==1) //stroke in the third direction down to back
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackTop]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Back]==1 && internal_matrix[currentrow][BackTop]==0)
                  strokeArray[Back]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Forehead]==1) //stroke in the fourth direction down to front
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Forehead]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Mouth]==1 && internal_matrix[currentrow][Forehead]==0)
                  strokeArray[Mouth]= sequence + 2;
              }

              break;
            }
          break;

          case 7: //Back top (Direction 1: down left;  Direction 2:  down right side; Direction 3: down the back; Direction 4: down the front)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackLeftSide]==1) //Direction 1, down the left
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackLeftSide]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottom]==1 && internal_matrix[currentrow][BackLeftSide]==0)
                  strokeArray[FrontBottom]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackRightSide]==1) //stroke in second direction, down to right
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackRightSide]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottom]==1 && internal_matrix[currentrow][BackRightSide]==0)
                  strokeArray[FrontBottom]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Back]==1) //stroke in the third direction down to back
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Back]=  sequence + 1;


              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTop]==1) //stroke in the fourth direction to front
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTop]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Forehead]==1 && internal_matrix[currentrow][FrontTop]==0)
                  strokeArray[Forehead]= sequence + 2;
              }

              break;
            }
          break;

          case 8: //Back  (Direction 1: left upper side;  Direction 2:  right upper side; Direction 3: left lower side;  Direction 4:  right lower side; Direction 5: upwards to the Forehead)
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackLeftSide]==1) //Direction 1,  left
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[BackLeftSide]= sequence + 1;

                for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][FrontTopLeftCorner]==1 && internal_matrix[currentrow][BackLeftSide]==0)
                    strokeArray[FrontTopLeftCorner]= sequence + 2;
                }
                break;
              }
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackRightSide]==1) //stroke in second direction, down to right
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[BackRightSide]=  sequence + 1;

                for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][FrontTopRightCorner]==1 && internal_matrix[currentrow][BackRightSide]==0)
                    strokeArray[FrontTopRightCorner]= sequence + 2;
                }
                break;
              }
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackLeftSide]==1) //stroke in third direction, to lower left side
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[BackLeftSide]=  sequence + 1;

                for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][FrontBottomLeftCorner]==1 && internal_matrix[currentrow][BackLeftSide]==0)
                    strokeArray[FrontBottomLeftCorner]= sequence + 2;
                }
                break;
              }
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackRightSide]==1) //stroke in the fourth direction to lower right side
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[BackRightSide]=  sequence + 1;

                for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][FrontBottomRightCorner]==1 && internal_matrix[currentrow][BackRightSide]==0)
                    strokeArray[FrontBottomRightCorner]= sequence + 2;
                }

                break;
              }
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackTop]==1) //stroke in the fitfth direction up to front top
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[BackTop]=  sequence + 1;

                for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][FrontTop]==1 && internal_matrix[currentrow][BackTop]==0)
                    strokeArray[FrontTop]= sequence + 2;
                }

                break;
              }
            break;

            case 9://Back right side (Direction 1: upwards to left side; Direction 2: to back and left side; Direction 3: to front upper side; Direction 4: to front lower side)
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackTop]==1) // Direction 1, upwards to top and left side
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[BackTop]= sequence + 1;

                for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][BackLeftSide]==1 && internal_matrix[currentrow][BackTop]==0)
                    strokeArray[BackLeftSide]= sequence + 2;
                }
                break;
              }
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Back]==1) //stroke in second direction, to back left side
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[Back]=  sequence + 1;

                for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][BackLeftSide]==1 && internal_matrix[currentrow][Back]==0)
                    strokeArray[BackLeftSide]= sequence + 2;
                }
                break;
              }
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopRightCorner]==1) //stroke in the third direction (front to Forehead)
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[FrontTopRightCorner]=  sequence + 1;

                for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][Forehead]==1 && internal_matrix[currentrow][FrontTopRightCorner]==0)
                    strokeArray[Forehead]= sequence + 2;
                }

                break;
              }
              if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomRightCorner]==1) //stroke in the fourth direction (front to mouth)
              {
                sequence ++;
                strokeArray[col]= sequence;
                strokeArray[FrontBottomRightCorner]=  sequence + 1;

                for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
                {
                  if(internal_matrix[currentrow+delay][Mouth]==1 && internal_matrix[currentrow][FrontBottomRightCorner]==0)
                    strokeArray[Mouth]= sequence + 2;
                }

                break;
              }
            break;


          case 10://Front bottom right corner (Direction 1: upwards; Direction 2: downwards; Direction 3: to back; Direction 4: to front)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTopRightCorner]==1) // Direction 1, upwards to top
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTopRightCorner]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTop]==1 && internal_matrix[currentrow][FrontTopRightCorner]==0)
                  strokeArray[FrontTop]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottom]==1) //stroke in second direction, down to bottom and left side
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottom]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottomLeftCorner]==1 && internal_matrix[currentrow][FrontBottom]==0)
                  strokeArray[FrontBottomLeftCorner]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackRightSide]==1) //stroke in the third direction (to back)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackRightSide]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Back]==1 && internal_matrix[currentrow][BackRightSide]==0)
                  strokeArray[Back]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Mouth]==1) //stroke in the fourth direction (to Forehead)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Mouth]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottomLeftCorner]==1 && internal_matrix[currentrow][Mouth]==0)
                  strokeArray[FrontBottomLeftCorner]= sequence + 2;
              }

              break;
            }
          break;

          case 11: // Front top right side (Direction 1: upwards to left; Direction 2: downwards; Direction 3: to back; Direction 4: to front)
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontTop]==1) // Direction 1, to right over top
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontTop]= sequence + 1;

              for (int currentrow = row; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTopLeftCorner]==1 && internal_matrix[currentrow][FrontTop]==0)
                  strokeArray[FrontTopLeftCorner]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][FrontBottomRightCorner]==1) //stroke in second direction, down
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[FrontBottomRightCorner]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontBottom]==1 && internal_matrix[currentrow][FrontBottomRightCorner]==0)
                  strokeArray[FrontBottom]= sequence + 2;
              }
              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][BackRightSide]==1) //stroke in the third direction (to back)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[BackRightSide]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][Back]==1 && internal_matrix[currentrow][BackRightSide]==0)
                  strokeArray[Back]= sequence + 2;
              }

              break;
            }
            if(row < internal_matrix_size_y - delay && internal_matrix[row+delay][Forehead]==1) //stroke in the fourth direction (to Forehead)
            {
              sequence ++;
              strokeArray[col]= sequence;
              strokeArray[Forehead]=  sequence + 1;

              for (int currentrow = row+delay; currentrow < internal_matrix_size_y-delay; currentrow++) //new loop for checking for third field involved in stroke
              {
                if(internal_matrix[currentrow+delay][FrontTopLeftCorner]==1 && internal_matrix[currentrow][Forehead]==0)
                  strokeArray[FrontTopLeftCorner]= sequence + 2;
              }

              break;
            }
         break;

          sequence=0;
        }//end switch (col)
      }//end if(internal_matrix[col][row] == 0 && internal_matrix[col][row-1] == 1)
    }// end for(int row = 0; row < classifier_matrix_size_x; row++)

  }

  return strokeArray;
}

static InitClass init("TouchClassifier", &TouchClassifier::Create, "Source/UserModules/TouchClassifier/");
