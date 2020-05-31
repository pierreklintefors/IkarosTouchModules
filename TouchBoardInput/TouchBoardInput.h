//
//  TouchBoardInput.h
//
//
//  Created by Pierre Klintefors on 2020-03-28.
//

#ifndef TouchBoardInput_
#define TouchBoardInput_

#include "IKAROS.h"


class TouchBoardInput: public Module
{
public:
    static Module *Create(Parameter * p) { return new TouchBoardInput(p); }

    TouchBoardInput(Parameter * p) : Module(p) {}
    virtual ~TouchBoardInput();

    void Init();
    void Tick();


    Serial *s;

    char * rcvmsg;


    float *	output;

};

#endif
