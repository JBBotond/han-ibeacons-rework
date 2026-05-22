/******************************************************************************
 * Project        : Mastermind
 * File           : Implementation Mastermind Solver functions
 * Copyright      : Hugo Arends 2014
 ******************************************************************************
  Change History:

    Version 1.0 - July 2017
    > Initial revision

******************************************************************************/
#include <stdlib.h>

#include "mastermind_solver.h"
#include "mastermind.h"

// ----------------------------------------------------------------------------
// Local variables
// ----------------------------------------------------------------------------

// 6x6x6x6 = 1296 bytes
unsigned char codes[6][6][6][6];
uint8_t turn=0;
uint16_t codes_left=0;

// ----------------------------------------------------------------------------
// Function implementation
// ----------------------------------------------------------------------------
void solver_reset(void)
{
    uint8_t i,j,k,l;

    turn=0;
    codes_left=1296; // 6*6*6*6;

    // Mark all codes valid
    for(i=0; i<6; i++){
        for(j=0; j<6; j++){
            for(k=0; k<6; k++){
                for(l=0; l<6; l++){
                    codes[i][j][k][l] = 1;
                }
            }
        }
    }
}

uint8_t solver_next_code(unsigned char *code, int p, int n)
{
    uint8_t a,b,c,d;
    unsigned char tmp[4];
    mm_result_t mm_result;

    // Invalid inputs
    if((p+n > 4) || (p==3 && n==1))
    {
        return(turn);
    }

    // Game won?
    if(p == 4 && n == 0)
    {
        return(turn);
    }

    turn++;

    if(turn == 1)
    {
        code[0]=1;
        code[1]=1;
        code[2]=2;
        code[3]=2;
        return(turn);
    }

    // Remove impossible answers
    for(a=1; a<=6; a++)
    {
        for(b=1; b<=6; b++)
        {
            for(c=1; c<=6; c++)
            {
                for(d=1; d<=6; d++)
                {
                    // Process valid codes only
                    if(codes[a-1][b-1][c-1][d-1] == 1)
                    {
                        tmp[0]=a;
                        tmp[1]=b;
                        tmp[2]=c;
                        tmp[3]=d;

                        // Supply current code
                        set_secret_code(tmp);

                        // Check the result
                        mm_result = check_secret_code(code);

                        // Does this code have a different result then the submitted code?
                        if(mm_result.correct_num_and_pos != p || mm_result.correct_num != n)
                        {
                            // Remove this code from the list
                            codes[a-1][b-1][c-1][d-1] = 0;
                        }
                    }
                }
            }
        }
    }

    // For debugging
    codes_left=0;
    for(a=0; a<6; a++)
    {
        for(b=0; b<6; b++)
        {
            for(c=0; c<6; c++)
            {
                for(d=0; d<6; d++)
                {
                    if(codes[a][b][c][d] == 1)
                    {
                        codes_left++;
                    }
                }
            }
        }
    }

    // Get first valid code
    for(a=0; a<6; a++)
    {
        for(b=0; b<6; b++)
        {
            for(c=0; c<6; c++)
            {
                for(d=0; d<6; d++)
                {
                    if(codes[a][b][c][d] == 1)
                    {
                        code[0] = a + 1;
                        code[1] = b + 1;
                        code[2] = c + 1;
                        code[3] = d + 1;
                        return(turn);
                    }
                }
            }
        }
    }

    return(turn);
}
