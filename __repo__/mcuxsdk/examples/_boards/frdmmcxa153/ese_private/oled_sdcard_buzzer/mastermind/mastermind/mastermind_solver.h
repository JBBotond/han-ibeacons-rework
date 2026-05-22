/******************************************************************************
 * Project        : Mastermind
 * File           : Header Mastermind Solver functions
 * Copyright      : Hugo Arends 2014
 ******************************************************************************
  Change History:

    Version 1.0 - July 2017
    > Initial revision

******************************************************************************/
#ifndef _MASTERMIND_SOLVER_H_
#define _MASTERMIND_SOLVER_H_

#include <stdint.h>

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
void solver_reset(void);
uint8_t solver_next_code(unsigned char *code, int p, int n);

#endif /* _MASTERMIND_SOLVER_H_ */
