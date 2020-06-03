//
//  TouchClassifier.h
//
//
//  Created by Pierre Klintefors.
//

#ifndef TouchClassifier_
#define TouchClassifier_

#include "IKAROS.h"


class TouchClassifier: public Module
{
public:
    static Module * Create(Parameter * p) { return new TouchClassifier(p); }

    TouchClassifier(Parameter * p) : Module(p) {}
    virtual ~TouchClassifier();

    void Init();
    void Tick();

    float*  MatrixMean(int, int);
    float*  CheckingForStrokes();

    float *     input_array;
    int         input_array_size;


    float **    internal_matrix;
    int         internal_matrix_size_y;
    
    int         internal_matrix_row;

    float **    output_matrix;
    int         output_matrix_size_x;
    int         output_matrix_size_y;

    float **    classifier_matrix;
    int         classifier_matrix_size_x ;
    int         classifier_matrix_size_y;


};

#endif
