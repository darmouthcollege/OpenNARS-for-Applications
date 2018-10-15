#include "Table.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

void Table_Add(Table *table, Implication *imp)
{
    double impTruthExp = Truth_Expectation(imp->truth);
    for(int i=0; i<TABLE_SIZE; i++)
    {
        if(i==table->itemsAmount || impTruthExp > Truth_Expectation(table->array[i].truth))
        {
            //ok here it has to go, move down the rest, evicting the last element if we hit TABLE_SIZE-1.
            for(int j=MIN(table->itemsAmount, TABLE_SIZE-1); j>i; j--)
            {
                table->array[j] = table->array[j-1];
            }
            table->array[i] = *imp;
            table->itemsAmount = MIN(table->itemsAmount+1, TABLE_SIZE);
            return;
        }
    }
}

void Table_AddAndRevise(Table *table, Implication *imp)
{
    //1. get closest item in the table
    int best_i = -1;
    double best_expectation = 0;
    Truth best_truth;
    for(int i=0; i<table->itemsAmount; i++)
    {
        Truth tsim = SDR_Similarity(&imp->sdr, &table->array[i].sdr);
        double cur_expectation = Truth_Expectation(tsim);
        if(cur_expectation > best_expectation)
        {
            best_i = i;
            best_expectation = cur_expectation;
            best_truth = tsim;
        }
    }
    //2. if there was one, revise with closest, and add the revised element
    if(best_i != -1)
    {
        Implication* closest = &table->array[best_i];
        if(!Stamp_checkOverlap(&closest->stamp, &imp->stamp))
        {
            Implication penalized_closest = *closest;
            Implication penalized_imp = *imp;
            penalized_closest.truth = Truth_Intersection(penalized_closest.truth, best_truth);
            penalized_imp.truth = Truth_Intersection(penalized_imp.truth, best_truth);
            Implication revised = Inference_ImplicationRevision(&penalized_closest, &penalized_imp);
            if(revised.truth.confidence > penalized_closest.truth.confidence &&
               revised.truth.confidence > penalized_imp.truth.confidence)
            {
                Table_Add(table, &revised);
            }
        }
    }
    //3. add imp too:
    Table_Add(table, imp);
}

Implication Table_PopHighestTruthExpectationElement(Table *table)
{
    Implication result = table->array[0];
    for(int i=1; i<table->itemsAmount; i++)
    {
        table->array[i-1] = table->array[i];
    }
    table->itemsAmount--;
    return result;
}
